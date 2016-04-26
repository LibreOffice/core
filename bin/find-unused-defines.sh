#
# This is a pretty brute-force approach. It takes several hours to run on a top-spec MacbookAir.
# It also produces some false positives, so it requires careful examination and testing of the results.
#
# Algorithm Summary:
#   First we find all #defines,
#   then we search for each of them in turn,
#   and if we find only one instance of a #define, we print it out.
#
# Algorithm Detail:
# (1) find #defines, excluding the externals folder
# (2) extract just the constant name from the search results
# (3) sort and uniq the results, mostly so I have an idea how far along the process is
# (4) for each result:
#   (5) grep for the constant
#   (6) use awk to check if only one match for a given constant was found
#   (7) if so, generate a sed command to remove the #define
#
( git grep -hP '^\w*#define\s+\w+.*\\' -- "[!e][!x][!t]*" \
  && \
  git grep -hP '^\w*#define\s+\w+\s*$' -- "[!e][!x][!t]*" ) \
  | sed -r 's/[ ]*#define[ ]+([a-zA-Z0-9_]+).*/\1/' \
  | grep -v '_idl' \
  | grep -vE '^INCLUDED_' \
  | sort \
  | uniq \
  | xargs -Ixxx -n 1 -P 8 sh -c \
    "git grep -w 'xxx' | awk -f bin/find-unused-defines.awk -v p1=xxx && echo \"xxx\" 1>&2"


