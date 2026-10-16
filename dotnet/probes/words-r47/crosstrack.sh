#!/usr/bin/env bash
# Render slides + sheets with our own CLI into $1. No soffice involved.
set -uo pipefail
OUT="$1"
CLI=/home/user/libreoffice-core/.claude/worktrees/words-r47/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
export SOURCE_DATE_EPOCH=1700000000
mkdir -p "$OUT"
n=0
while IFS= read -r -d '' f; do
  stem="$(basename "${f%.*}")__$(echo "${f##*.}" | tr 'A-Z' 'a-z')"
  d="$OUT/$stem"
  mkdir -p "$d"
  timeout 300 "$CLI" render --outdir "$d" "$f" >/dev/null 2>&1
  n=$((n+1))
done < <(find /workspace/sample-files/slides /workspace/sample-files/sheets -type f -print0)
echo "rendered $n documents into $OUT"
