#!/bin/bash

gcc -o factorial main.c factorial.c dbug.c

for i in example?.c main.c factorial.c ;
do
  sed -e 's!\\!\\\\!g' $i > ${i/\.c/\.r}
done

./factorial 1 2 3 4 5 | cat > output1.r
./factorial -\#t:o 2 3 | cat > output2.r
./factorial -\#d:t:o 3 | cat > output3.r
./factorial -\#d,result:o 4 | cat > output4.r
./factorial -\#d:f,factorial:F:L:o 3 | cat >output5.r

#nroff -mm user.r > user.t
#groff -mm user.r > user.ps
groff -mm -rcR=0 -Tlatin1 -P -bcu user.r > dbug.txt
