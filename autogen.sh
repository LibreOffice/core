#!/bin/sh
# Run this to generate all the initial makefiles, etc.

if test "z$1" = "z--clean"; then
    echo "Cleaning"

    rm -Rf autom4te.cache
    rm -f missing install-sh mkinstalldirs libtool ltmain.sh
    exit 1;
fi

requote_args ()
{
    sed -r -e 's/.*configure //' -e 's/(["'"'"'])/\\\1/g' -e 's/=(([^"'"'"'-]|-[^-]| )*)( |$)/="\1" /g'
}

old_args=""
if test $# -eq 0 && test -f config.log; then
    old_args=`grep '\$ ./configure' config.log | requote_args`
    echo "re-using arguments from last configure: $old_args";
fi

touch ChangeLog

if test "z$ACLOCAL_FLAGS" = "z" -a "z`uname -s`" = "zDarwin" ; then
    ACLOCAL_FLAGS="-I ./m4/mac"
fi
if test "z`uname -s`" != "zDarwin" ; then
    AUTOMAKE_EXTRA_FLAGS=--warnings=no-portability
fi

aclocal $ACLOCAL_FLAGS || exit 1;
#automake --gnu --add-missing --copy || exit 1;
#intltoolize --copy --force --automake
autoconf || exit 1;
if test "x$NOCONFIGURE" = "x"; then
    if test -n "$old_args" ; then
        eval `echo ./configure $old_args`
    else
        ./configure "$@"
    fi
else
    echo "Skipping configure process."
fi
