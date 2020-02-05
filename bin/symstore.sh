#!/usr/bin/env bash

# Files listed here would not be store in the symbolestore-server.
# The format is a string with files e.g.
#  BLACKLIST="python.exe
#  file.dll
#  next_file.exe"
#
# It removes "python.exe", "file.dll", and "next_file.exe" from what's
# added to the symstore. Separator is the newline
BLACK_LIST="python.exe"

# List files here where it's ok for this script to find more than one
# occurrence in the build tree. Files _not_ included here will generate
# an error, if duplicates are found.
#
# Same format as for BLACK_LIST above
MOREPDBS_OKLIST="libcurl.dll"


add_pdb()
{
    extension=$1
    pdbext=$2
    list=$3
    stats_notfound=0
    stats_found=0
    stats_morefound=0
    declare -a pdball
    echo "Collect $extension"
    ret=$(find "${INSTDIR}/" -type f -name "*.${extension}" | grep -vF "$BLACK_LIST")
    while IFS= read -r file
    do
        # store dll/exe itself (needed for minidumps)
        if [ $WITHEXEC == 1 ] ; then
            cygpath -w "$file" >> "$list"
        fi
        # store pdb file
        filename=$(basename "$file" ".${extension}")
        pdball+=($(grep -i "/${filename}${pdbext}" <<< ${ALL_PDBS}))
        if [ -n "${pdball[0]}" ]; then
            cygpath -w "${pdball[0]}" >> "$list"
        fi
        case ${#pdball[@]} in
            0) ((++stats_notfound)) ;;
            1) ((++stats_found)) ;;
            *) ((++stats_morefound))
                if [ -z "$(echo $file | grep -F "$MOREPDBS_OKLIST")" ]; then
                    echo "Error: found duplicate PDBs:"
                    for morepdbs in ${pdball[@]} ; do
                       echo " $morepdbs"
                    done
                    exit 1
                fi
            ;;
        esac
        unset pdball
    done <<EOF
${ret}
EOF

    echo "  Found PDBs    : $stats_found"
    echo "  Missing PDBs  : $stats_notfound"
    echo "  Multiple PDBs : $stats_morefound"
}

# check preconditions
if [ -z "${INSTDIR}" -o -z "${WORKDIR}" ]; then
    echo "INSTDIR or WORKDIR not set - script expects calling inside buildenv"
    exit 1
fi
if [ ! -d "${INSTDIR}" -o ! -d "${WORKDIR}" ]; then
    echo "INSTDIR or WORKDIR not present - script expects calling after full build"
    exit 1
fi

# defaults
MAX_KEEP=5
SYM_PATH=${WORKDIR}/symstore
COMMENT=""
COMCMD=""
WITHEXEC=1
DRYRUN=""

USAGE="Usage: $0 [-h|-k <keep_num_versions>|-p <symbol_store_path>]
       -h:          this cruft
       -c <comment> specifies a comment for the transaction
       -n           do not store exe/dll on the symbole server
       -k <int>:    keep this number of old symbol versions around
                    (default: ${MAX_KEEP}. Set to 0 for unlimited)
       -p <path>:   specify full path to symbol store tree
       -d <file>:   made dry run, for test only, copy store data
                    to file
If no path is specified, defaults to ${SYM_PATH}.
"

# process args
while :
do
   case "$1" in
    -k|--keep) MAX_KEEP="$2"; shift 2;;
    -p|--path) SYM_PATH="$2"; shift 2;;
    -d|--dryrun) DRYRUN="$2"; shift 2;;
    -c|--comment) COMCMD="/c"; COMMENT="$2"; shift 2;;
    -n|--noexec) WITHEXEC=0; shift ;;
    -h|--help) echo "${USAGE}"; exit 0;;
    -*) echo "${USAGE}" >&2; exit 1;;
    *) break;;
   esac
done

if [ $# -gt 0 ]; then
    echo "${USAGE}" >&2
    exit 1
fi

# check is symstore.exe set in PATH
if [ -z ${DRYRUN} ]; then
    which symstore.exe > /dev/null 2>&1 || {
        echo "symstore.exe is expected in the PATH"
        exit 1
    }
fi

# populate symbol store from here
TMPFILE=$(mktemp) || exit 1
trap '{ rm -f ${TMPFILE}; }' EXIT

# collect all PDBs
ALL_PDBS=$(find "${WORKDIR}/" -type f -name "*.pdb")

# add dlls and executables
add_pdb dll .pdb "${TMPFILE}"
add_pdb exe .pdb "${TMPFILE}"
add_pdb bin .bin.pdb "${TMPFILE}"

if [ -z ${DRYRUN} ]; then
    # stick all of it into symbol store
    symstore.exe add /compress /f "@$(cygpath -w "${TMPFILE}")" /s "$(cygpath -w "${SYM_PATH}")" /t "${PRODUCTNAME}" /v "${LIBO_VERSION_MAJOR}.${LIBO_VERSION_MINOR}.${LIBO_VERSION_MICRO}.${LIBO_VERSION_PATCH}${LIBO_VERSION_SUFFIX}${LIBO_VERSION_SUFFIX_SUFFIX}" "${COMCMD}" "${COMMENT}"
else
    cp "${TMPFILE}" ${DRYRUN}
fi

rm -f "${TMPFILE}"

# Cleanup symstore, older revisions will be removed.  Unless the
# .dll/.exe changes, the .pdb should be shared, so with incremental
# tinderbox several revisions should not be that space-demanding.
if [ "${MAX_KEEP}" -gt 0 -a -d "${SYM_PATH}/000Admin" ]; then
    to_remove=$(ls -1 "${SYM_PATH}/000Admin" | grep -v '\.txt' | grep -v '\.deleted' | sort | head -n "-${MAX_KEEP}")
    for revision in $to_remove; do
        echo "Remove $revision from symstore"
        symstore.exe del /i "${revision}" /s "$(cygpath -w "${SYM_PATH}")"
    done
fi
