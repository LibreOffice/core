#!/bin/sh

aclocal $ACLOCAL_FLAGS || exit 1
automake --gnu --add-missing --copy || exit 1
autoconf || exit 1

if test "x$NOCONFIGURE" = "x"; then
    ./configure "$@"
else
    echo "Skipping configure process."
fi
