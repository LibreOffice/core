#!/usr/bin/env bash

add_pdb()
{
    extension=$1
    type=$2
    list=$3
    for file in $(find "${INSTDIR}/" -name "*.${extension}"); do
        # store dll/exe itself (needed for minidumps)
        if [ -f "$file" ]; then
            cygpath -w "$file" >> "$list"
        fi
        # store pdb file
        filename=$(basename "$file" ".${extension}")
        pdb="${WORKDIR}/LinkTarget/${type}/${filename}.pdb"
        if [ -f "$pdb" ]; then
            cygpath -w "$pdb" >> "$list"
        fi
    done
}

# check preconditions
if [ -z "${INSTDIR}" ] || [ -z "${WORKDIR}" ]; then
    echo "INSTDIR or WORKDIR not set - script expects calling inside buildenv"
    exit 1
fi
if [ ! -d "${INSTDIR}" ] || [ ! -d "${WORKDIR}" ]; then
    echo "INSTDIR or WORKDIR not present - script expects calling after full build"
    exit 1
fi
which symstore.exe > /dev/null 2>&1 || {
    echo "symstore.exe is expected in the PATH"
    exit 1
}

# defaults
MAX_KEEP=5
SYM_PATH=${WORKDIR}/symstore

USAGE="Usage: $0 [-h|-k <keep_num_versions>|-p <symbol_store_path>]
       -h:         this cruft
       -k <int>:   keep this number of old symbol versions around
                   (default: ${MAX_KEEP}. Set to 0 for unlimited)
       -p <path>:  specify full path to symbol store tree
If no path is specified, defaults to ${SYM_PATH}.
"

# process args
while :
do
   case "$1" in
    -k|--keep) MAX_KEEP="$2"; shift 2;;
    -p|--path) SYM_PATH="$2"; shift 2;;
    -h|--help) echo "${USAGE}"; exit 0; shift;;
    -*) echo "${USAGE}" >&2; exit 1;;
    *) break;;
   esac
done

if [ $# -gt 0 ]; then
    echo "${USAGE}" >&2
    exit 1
fi

# populate symbol store from here
TMPFILE=$(mktemp) || exit 1
trap '{ rm -f ${TMPFILE}; }' EXIT

# add dlls and executables
add_pdb dll Library "${TMPFILE}"
add_pdb exe Executable "${TMPFILE}"

# stick all of it into symbol store
symstore.exe add /compress /f "@$(cygpath -w "${TMPFILE}")" /s "$(cygpath -w "${SYM_PATH}")" /t "${PRODUCTNAME}" /v "${LIBO_VERSION_MAJOR}.${LIBO_VERSION_MINOR}.${LIBO_VERSION_MICRO}.${LIBO_VERSION_PATCH}${LIBO_VERSION_SUFFIX}${LIBO_VERSION_SUFFIX_SUFFIX}"
rm -f "${TMPFILE}"

# Cleanup symstore, older revisions will be removed.  Unless the
# .dll/.exe changes, the .pdb should be shared, so with incremental
# tinderbox several revisions should not be that space-demanding.
if [ "${MAX_KEEP}" -gt 0 ] && [ -d "${SYM_PATH}/000Admin" ]; then
    to_remove=$(ls -1 "${SYM_PATH}/000Admin" | grep -v '\.txt' | grep -v '\.deleted' | sort | head -n "-${MAX_KEEP}")
    for revision in $to_remove; do
        symstore.exe del /i "${revision}" /s "$(cygpath -w "${SYM_PATH}")"
    done
fi
