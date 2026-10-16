#!/usr/bin/env bash
# Every `chart:chart/@chart:class` the converted-ODF corpus states, with its document count.
#
#   ./census.sh [/home/user/corpus-odf]
#
# The chart lives in an embedded object's own `content.xml`, so this reads the package rather
# than grepping the file -- an `.ods` is a zip and the string is not in it uncompressed.
set -uo pipefail
ROOT=${1:-/home/user/corpus-odf}
for d in "$ROOT"/ods "$ROOT"/odt; do
  [ -d "$d" ] || continue
  for f in "$d"/*; do
    unzip -p "$f" 'Object*/content.xml' 2>/dev/null \
      | grep -o '<chart:chart[^>]*chart:class="[^"]*"' \
      | grep -o 'chart:class="[^"]*"' | sort -u | sed 's#^#'"$(basename "$d")"'\t#'
  done
done | sort | uniq -c | sort -rn
