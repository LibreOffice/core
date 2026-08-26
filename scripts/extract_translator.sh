#!/usr/bin/env bash

#
# author {
#  name "kuesji koesnu"
#  email "kuesji@koesnu.com"
#  website "kuesji.koesnu.com"
# }
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#



SINCE=$(date +%s --date="-1 month")
TEXT='Translated using Weblate'
INPUT_FILE=""
CSV_FILE=""
OUTPUT_FILE=""

set -ef

print_help(){
    usage='
 options
    --full    generate translator list from beginning ( only last month generated without this )
    -i file    give input file for previous contributors
    -o file    give output file for latest contributors list including previous
    --csv file    give input from csv file
    '

    echo "$usage"
}

find_hashs(){
    # if since is greater than 0 take last month's authors else full
    if [ "$SINCE" -gt 0 ]
    then
        git log --since="$SINCE" --pretty=format:"%h %s" --grep="$TEXT" | cut -d ' ' -f1
    else
        git log --pretty=format:"%h %s" --grep="$TEXT" | cut -d ' ' -f1
    fi
}

collect_authors(){
    for hash in $(find_hashs)
    do
        info=$(git show "$hash" --pretty=format:'%an' | head -n 1)
        echo $info
    done
}


# show help if no argument passed
if [ $# -lt 1 ]
then
    print_help
    exit 1
fi


# parsing arguments
while [ $# -gt 1 ];
do
    case "$1" in
        '--full')
            shift; SINCE="0";
            ;;
        '-i')
            shift; INPUT_FILE="$1"; shift;
            ;;
        '-o')
            shift; OUTPUT_FILE="$1"; shift;
            ;;
        '--csv')
            shift; CSV_FILE="$1"; shift;
            ;;
        *)
            shift;
            ;;
    esac
done

# we need an output list
if [ -z "$OUTPUT_FILE" ]
then
    echo 'error: please give output file with "-o filename" format' >&2
    exit 1
fi

# read input list if given
if [ ! -z "$INPUT_FILE" ] && [ -f "$INPUT_FILE" ]
then
    cat "$INPUT_FILE" > 'tmp.txt'
fi

# load csv if csv file given
if [ ! -z "$CSV_FILE" ] && [ -f "$CSV_FILE" ]
then
    # backup and switch delimiter to comma
    OLDIFS="$IFS"
    IFS=','
    for author in $( cat "$CSV_FILE" )
    do
        # first remove leading spaces, second sed trim trailing space
        echo "$author" |  sed 's,^ *,,g' | sed 's, *$,,g' >> 'tmp.txt'
    done
    # restore old delimitier
    IFS="$OLDIFS"
fi



# generate latest list and combine
collect_authors >> 'tmp.txt'
# read all, sort all, make unique and write to output file
cat 'tmp.txt' | sort | uniq > "$OUTPUT_FILE"

if [ ! -z "$CSV_FILE" ]
then
    # put a comma on end of every line
    sed -e 's/$/, /g' "$OUTPUT_FILE" > 'tmp.txt'
    # delete newline characters
    cat tmp.txt | tr -d '\n' > "$OUTPUT_FILE"
fi

# thanks for service tmp.txt, but we don't need you anymore
rm 'tmp.txt'

# vim:set shiftwidth=4 softtabstop=4 expandtab:
