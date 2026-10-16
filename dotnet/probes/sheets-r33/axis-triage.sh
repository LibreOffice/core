#!/usr/bin/env bash
# For each document: the largest column-band index and the largest row-band index our
# pagination produces, over all its sheets. A document whose every sheet has one column band
# cannot have a column-fit defect at all.
P=/home/user/libreoffice-core/.claude/worktrees/sheets-r33/dotnet/probes/sheets-r33/GridProbe/bin/Debug/net10.0/linux-x64/GridProbe
while IFS= read -r f; do
  [ -f "$f" ] || { echo "MISSING $f"; continue; }
  out=$("$P" "$f" "" 0 0 2>/dev/null | sed -n 's/.* band \([0-9]*\)\/\([0-9]*\) .*/\1 \2/p')
  cb=$(echo "$out" | awk '{print $1}' | sort -n | tail -1)
  rb=$(echo "$out" | awk '{print $2}' | sort -n | tail -1)
  n=$(echo "$out" | grep -c .)
  printf 'colBands<=%-4s rowBands<=%-5s pages=%-6s %s\n' "$((cb+1))" "$((rb+1))" "$n" "$(basename "$f")"
done
