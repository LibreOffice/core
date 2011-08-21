#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`
cd $srcdir
PROJECT=dmake
TEST_TYPE=-f
FILE=make.c

DIE=0

# cvs may leave some files unwritable
(chmod -Rf u+rw $srcdir) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have owner permissions for all files and subdirectories"
    echo "in $srcdir"
    DIE=1
}

(autoconf --version ) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have autoconf installed to compile $PROJECT."
    DIE=1
}

(automake --version ) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have automake installed to compile $PROJECT."
    DIE=1
}

if test "$DIE" -eq 1; then
    exit 1
fi

test $TEST_TYPE $FILE || {
    echo "You must run this script in the top-level $PROJECT directory"
    exit 1
}

if test -z "$*"; then
    echo "I am going to run ./configure with no arguments - if you wish "
        echo "to pass any to it, please specify them on the $0 command line."
fi

# needed when autotools version changed
aclocal

# After the first run of autoconf a configure file exists.
# 'configure --enable-maintainer-mode' will start all other needed autotool helpers.
autoconf

#automake -a -c -f

cd $ORIGDIR

$srcdir/configure --enable-maintainer-mode  --prefix=/usr/local "$@"

echo
echo "Now execute GNU make to compile $PROJECT."
