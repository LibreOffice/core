#!/usr/bin/env bash

if [ ! -e bin/get_config_variables ]; then
    cd ../../..
fi

. bin/get_config_variables SRCDIR

if [ "$PWD" != "$SRCDIR" ]; then
    echo "error: not in the expected SRCDIR"
    exit 1
fi

cd writerfilter
echo 'include Makefile' > watch.mk
echo 'watch:' >> watch.mk
echo $'\techo $(writerfilter_ALL)' >> watch.mk
mydir=workdir/CustomTarget/writerfilter
writerfilter_ALL=$(make -sr -f watch.mk watch|sed "s|$SRCDIR/$mydir/||g")
rm watch.mk
cd - >/dev/null

case $1 in
reference)
    rm -rf $mydir-reference
    mkdir -p $mydir-reference/source/ooxml
    for i in $writerfilter_ALL
    do
        cp $mydir/"$i" $mydir-reference/"$i"
    done
    ;;
compare)
    for i in $writerfilter_ALL
    do
        if [ "$(basename "$i")" == "model_preprocessed.xml" ]; then
            continue
        fi
        diff -u $mydir-reference/"$i" $mydir/"$i"
    done
    ;;
*)
    echo "usage: $0 [ reference | compare ]"
    echo
    echo "$0 first saves a reference output of all generated files by writerfilter, then"
    echo "allows comparing against it. This helps seeing the effect of changes made on"
    echo "the code generator scripts."
    ;;
esac

# vi:set shiftwidth=4 expandtab:
