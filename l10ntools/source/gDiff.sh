#!/bin/bash

cd workdir/jan
for i in `find . -name \*.pot`
do
 echo ">>>>>>>> " $i
 diff ../pot/$i $i | sed '/^6c6$/,/^16c16$/d'
done

