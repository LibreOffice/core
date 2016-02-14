#!/bin/sh

for i in $(find . -maxdepth 1 -type d -printf '%P\n' | sed '/^$/d' | sed '/^\./d')
do
    echo "Directory $i:"
    echo "- processing global includes in .hxx and .cxx files"
    sed -i "s/#include \"\($i\/.*\)\"/#include <\1>/g" $(git grep -l "#include \"$i\/.*\"" *.[hc]xx) 2> /dev/null
    echo "- processing global includes in .h and .c files"
    sed -i "s/#include \"\($i\/.*\)\"/#include <\1>/g" $(git grep -l "#include \"$i\/.*\"" *.[hc]) 2> /dev/null
    echo "- processing global includes in .mm files"
    sed -i "s/#include \"\($i\/.*\)\"/#include <\1>/g" $(git grep -l "#include \"$i\/.*\"" *.mm) 2> /dev/null
done
