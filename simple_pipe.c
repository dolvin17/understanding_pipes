/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simple_pipe.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dolvin17 <grks_17@hotmail.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/23 18:00:32 by dolvin17          #+#    #+#             */
/*   Updated: 2023/09/23 20:25:12 by dolvin17         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*
La ejecución de:  $ ./simple_pipe 'It was a bright cold day in April, '/'and the clocks were striking thirteen.'
debe devolver: It was a bright cold day in April, and the clocks were striking thirteen.

Paso 1: Llama a pipe() para crear un pipe.
Paso 2: Llama a fork() para crear un proceso hijo.
Paso 3: Cerrar el FD de escritura. (pipe_fd[1]).
Paso 4: Crea un bucle para que el proceso hijo lea todo lo que encuentre y escribimos a la vez en la salida estandar.
Paso 5: Si no hay nada más que leer cerramos FD de lectura (pipe_fd[0]).
*/
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define BUF_SIZE 10

void check_error(bool if_error, int value, const char *str)
{
	if (if_error)
	{
		errno = value;
		perror(str);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	int pipe_fd[2];
	char buf[BUF_SIZE];
	ssize_t bytes_readed;
	pid_t child_pid;

	/* Pipe file descriptors */
	check_error((argc != 2 || strcmp(argv[1], "--help") == 0), EINVAL, "Error invalid argument");
	/* Create the pipe */
	check_error(pipe(pipe_fd) == -1, errno, "Error creating pipe failure");
	/* Create the child process */
	child_pid = fork();
	/*Check if the child process was created succesfully*/
	check_error(child_pid == -1, errno, "Error, fork failed");
	if (child_pid == 0) /*proceso hijo creado*/
	{
		/* Child - close write end because it is unused */
		check_error(close(pipe_fd[1]) < 0, errno, "Error, failure closing writer fd");
		/* Read data from pipe, echo on stdout while there is something to read */
		while ((bytes_readed = read(pipe_fd[0], buf, BUF_SIZE)) > 0)
		{
			check_error((write(STDOUT_FILENO, buf, bytes_readed) != bytes_readed), errno, "child - partial/failed write");
		}
		/* End-of-file */
		write(STDOUT_FILENO, "\n", 1);
		check_error(close(pipe_fd[0]) == -1, errno, "Error, failure closing reader fd");
		_exit(EXIT_SUCCESS);
	}
	else /*se ejecutará todo por parte del proceso padre*/
	{
		/* Parent - close read end because it is unused */
		check_error(close(pipe_fd[0]) == -1, errno, "close - parent fd reader failed");
		/* Parent - writes to pipe */
		check_error(write(pipe_fd[1], argv[1], strlen(argv[1])) != strlen(argv[1]), errno, "parent - partial/failed write");
		/* End-of-file */
		check_error(close(pipe_fd[1]) < 0, errno, "Error, failure closing fd writer parent");
		/* Wait for child to finish */
		wait(NULL);
		exit(EXIT_SUCCESS);
	}
}