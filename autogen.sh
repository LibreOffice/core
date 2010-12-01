#!/bin/sh
# Run this to generate all the initial makefiles, etc.

if test "z$1" = "z--clean"; then
    echo "Cleaning"

    rm -Rf autom4te.cache
    rm -f missing install-sh mkinstalldirs libtool ltmain.sh
    exit 1;
fi

requote()
{
        local q=\'
        set -- "${@//\'/$q\'$q}"        # quote inner instances of '
        set -- "${@/#/$q}"              # add ' to start of each param
        set -- "${@/%/$q}"              # add ' to end of each param
        echo "$*"                       # ' in a comment to stop confusing vim
}

distro()
{
    name=''
    while test "$#" -gt 0 ; do
    case "$1" in
        --with-distro=*) name=${1#--with-distro=} ;;
    esac
    shift
    done
    echo $name
}

old_args=""
if test $# -eq 0 && test -f autogen.lastrun; then
    old_args=$(cat autogen.lastrun)
    echo "re-using arguments from last configure: $old_args";
fi

touch ChangeLog

if test "z$ACLOCAL_FLAGS" = "z" -a "z`uname -s`" = "zDarwin" ; then
    ACLOCAL_FLAGS="-I ./m4/mac"
fi
if test "z`uname -s`" != "zDarwin" ; then
    AUTOMAKE_EXTRA_FLAGS=--warnings=no-portability
fi

conf_args=$(requote "$@")
distro_name=$(distro "$@")
if test "z${distro_name}" != "z" ; then
    cumul=""
    if test -f "./distro-configs/${distro_name}.conf" ; then
        IFS="$(printf '\n')"
        for opt in $(cat distro-configs/${distro_name}.conf) ; do cumul="$cumul $opt" ; done ;
        unset IFS
        conf_args=$(requote "$@" | sed -e "s/'--with-distro=[^']*'/$cumul/")
    else
        echo "Warning: there is no pre-set configuration for ${distro_name}, ignoring --with-distro=${distro_name}"
    fi
fi

aclocal $ACLOCAL_FLAGS || exit 1;
#automake --gnu --add-missing --copy || exit 1;
#intltoolize --copy --force --automake
autoconf || exit 1;
if test "x$NOCONFIGURE" = "x"; then
    if test -n "$old_args" ; then
        eval `echo ./configure $old_args`
    else
        echo "$(requote "$@")" > autogen.lastrun
        echo "./configure ${conf_args}"
        eval `echo ./configure ${conf_args}`
    fi
else
    echo "Skipping configure process."
fi
