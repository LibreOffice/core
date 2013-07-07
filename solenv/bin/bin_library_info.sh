#!/usr/bin/env bash
#
#    Copyright (C) 2013 Norbert Thiebaud
#    License: GPLv3
#

do_help()
{
cat <<EOF
bin_library_info.sh is a tool that create a unique filename for a binary tar file that
contain the build of the given source tarfile. the unicity is based on the source tarfile which contains
a md5 already and the calculated sha1 of config_host_.mk and of the tree object associated with the top_level_module
in git.

syntax: bin_library_info.sh -m|--module <top_level_module> -l|--location <TARFILE_LOCATION> -s|--srcdir <SRCDIR> -b <BUILDDIR> -r|--tarfile <LIBRARY_TARFILE> [ -m|--mode verify|name ]

the default mode is 'name' which just print the assocaited binary tarfile name.
in 'verify' mode the programe print the name if the assocaited binary tarfile exist
and print nothing and return an error code if the file does not exist

Note: --location --builddir and --srcdir are optional if they are already in the env in the form of TARFILE_LOCATION and BUILDDIR SRCDIR respectively
EOF

exit 0;
}

die()
{
    [ $V ] && echo "Error:" "$@"
    exit -1;
}


get_config_sha()
{
    pushd ${SRCDIR?} > /dev/null
    cat ${BUILDDIR?}/config_host.mk | git hash-object --stdin
    popd ${SRCDIR?} > /dev/null
}

get_library_gbuild_sha()
{
    local module="$1"

    pushd ${SRCDIR?} > /dev/null
    git ls-tree HEAD | grep "\t${module?}$" | cut -f 1 | cut -d " " -f 3
    popd ${SRCDIR?} > /dev/null
}


determine_binary_package_name()
{
    local module="$1"
    local tarball="$2"
    local csha=""
    local gsha=""
    local binfile=""

    csha=$(get_config_sha)
    gsha=$(get_library_gbuild_sha "${module?}")
    if [ -n "${csha?}" -a -n "${gsha}" ] ; then
        binfile="${csha?}_${gsha?}_${tarball?}.${INPATH?}.tar.gz"
    fi
    echo "${binfile}"

}

MODULE=""
SOURCE_TARFILE=""
MODE="name"
V=1

while [ "${1}" != "" ]; do
    parm=${1%%=*}
    arg=${1#*=}
    has_arg=
    if [ "${1}" != "${parm?}" ] ; then
        has_arg=1
    else
        arg=""
    fi

    case "${parm}" in
        -h|--help)  # display help
            do_help
            exit
            ;;
        -b|--builddir)
            if [ -z "${has_arg}" ] ; then
                shift;
                arg="$1"
            fi
            BUILDDIR="${arg}"
            ;;
        -o|--module)
            if [ -z "${has_arg}" ] ; then
                shift;
                arg="$1"
            fi
            MODULE="${arg}"
            ;;

        -l|--location)
            if [ -z "${has_arg}" ] ; then
                shift;
                arg="$1"
            fi
            TARFILE_LOCATION="${arg}"
            ;;
        -m|--mode)
            # test if the binary package exist
            if [ -z "${has_arg}" ] ; then
                shift;
                arg="$1"
            fi
            MODE="$arg"
            ;;
        -p|--platform)
            # test if the binary package exist
            if [ -z "${has_arg}" ] ; then
                shift;
                arg="$1"
            fi
            INPATH="$arg"
            ;;
        -q)
            V=0
            ;;
        -s|--srcdir)  # do not override the local autogen.lastrun if present
            if [ -z "${has_arg}" ] ; then
                shift;
                arg="$1"
            fi
            SRCDIR="${arg}"
            ;;

        -t|--tarfile)
            if [ -z "${has_arg}" ] ; then
                shift;
                arg="$1"
            fi
            SOURCE_TARFILE="${arg}"
            ;;
        -*)
            die "Invalid option $1"
            ;;
        *)
            die "Invalid argument $1"
            ;;
    esac
    shift
done

if [ -z "${MODULE?}" ] ; then
    die "Missing --module"
fi
if [ -z "${TARFILE_LOCATION}" ] ; then
    die "Missing --location"
fi
if [ -z "${SOURCE_TARFILE}" ] ; then
    die "Missing --tarfile"
fi
if [ -z "${SRCDIR}" ] ; then
    die "Missing --srcdir"
fi


BINARY_TARFILE="$(determine_binary_package_name ${MODULE?} ${SOURCE_TARFILE?})"

if [ -z "${BINARY_TARFILE}" ] ; then
    exit 2
fi

if [ "${MODE?}" = "verify" ] ; then
    if [ -f "${TARFILE_LOCATION?}/${BINARY_TARFILE?}" ] ; then
        echo "${BINARY_TARFILE?}"
    else
        exit 1
    fi
else
    echo "${BINARY_TARFILE?}"
fi

exit 0
