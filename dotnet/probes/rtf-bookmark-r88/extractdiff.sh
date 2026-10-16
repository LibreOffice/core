#!/usr/bin/env bash
# Which of the documents that state a REF field actually take the second read.
#
# The second read runs only where a REF's expansion differs from the result the file
# cached, and where it runs it changes the text the reader reports -- so extracting with
# the base binary and with the changed one names exactly those documents, without any
# rendering. `refcensus.py`'s list is the input.
#
#   BASE=<base cli> HEAD=<head cli> extractdiff.sh <corpus root>
set -u
BASE=${BASE:?set BASE}
HEAD=${HEAD:?set HEAD}
ROOT=${1:?corpus root}
python3 "$(dirname "$0")/refcensus.py" "$ROOT" | sed -n 's/.*bkmkstart  //p' | while IFS= read -r name; do
  f=$(find "$ROOT" -type f -name "$name" | head -1)
  [ -n "$f" ] || { printf '%-70s NOT FOUND\n' "$name"; continue; }
  a=$(timeout -k 30 240 "$BASE" extract "$f" 2>/dev/null | md5sum | cut -d' ' -f1)
  b=$(timeout -k 30 240 "$HEAD" extract "$f" 2>/dev/null | md5sum | cut -d' ' -f1)
  if [ "$a" = "$b" ]; then printf '%-70s same\n' "$name"; else printf '%-70s CHANGED\n' "$name"; fi
done
