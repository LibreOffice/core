#!/bin/bash

for i in */data/*
do
    file=$(basename $i)
    if ! git grep -q $file; then
        echo "WARNING: $i is not used, write a testcase for it!"
    fi
done

# vi:set shiftwidth=4 expandtab:
