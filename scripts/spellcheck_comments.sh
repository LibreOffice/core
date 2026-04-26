#!/bin/sh

# get the directory of the script
SCRIPT_DIR="$(dirname "$(realpath "$0")")"

# find comments in source files (.css, .js, .ts, .hpp, .cpp)
# check spelling with hunspell
# filter out words with numbers in them and words with non starting upper case letters
# sort and uniqify the list
# filter out whitelist entries (manually checked)

git ls-files '*.css' '*.js' '*.ts' '*.*pp' '*.*xx' | xargs grep -Eho "//.*|/\*.*?\*/" | \
hunspell -l | \
grep -E '^[A-Z]?[a-z]*$' | \
sort | uniq | \
grep -F -v -w -f $SCRIPT_DIR/spelling_whitelist.txt
