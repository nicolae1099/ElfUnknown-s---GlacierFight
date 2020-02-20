build: santa.c
	gcc -g -Wall -Werror -std=c99 santa.c -lm -o snowfight 
clean:
	rm -rf snowfight
