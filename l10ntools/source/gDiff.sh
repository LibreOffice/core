#!/bin/bash

cd workdir/pot
find . > /tmp/jix1
cd ../jan
find . > /tmp/jix2

echo ">>>>> FILELIST DIFF >>>"
diff /tmp/jix1 /tmp/jix2

for i in `find . -name \*.pot`
do
 CMD=$(diff ../pot/$i $i | sed '/^6c6$/,/^16c16$/d')
 if [ "$CMD" != "" ]
 then
   echo ">>>>>>>> " $i
   echo "$CMD"
 fi
done

