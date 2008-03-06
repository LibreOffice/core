#! /bin/sh

# un-test.sh: remove TEST_LAYOUT code from OO.o implementation

sed -e 's!^#!@!' < $1\
    -e 's!$! /*$*/!' \
    -e 's!^@\(.*TEST_LAYOUT\)!#\1!'\
    | tee i \
    | cpp -traditional -C -CC -DNON_LAYOUT_COMPILE_FIXES=1 \
    | grep -F '/*$*/' \
    | sed -e 's! /\*\$\*/!!' \
    -e 's!^@!#!' \
    -e 's!#define 1 1!#define NON_LAYOUT_COMPILE_FIXES 1!'

