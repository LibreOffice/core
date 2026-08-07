#!/usr/bin/env bash
# renderours.sh <list> <outdir> — render our side only for each listed corpus document,
# and report pages and extractable words. The reference does not move for a
# Paperless-only change, so this is enough to compare two of our own binaries.
set -u
LIST="$1"; OUT="$2"
ROOT=/workspace/sample-files
CLI=/home/user/libreoffice-core/.claude/worktrees/agent-a42a22c09b7b3f134/dotnet/tools/Paperless.Cli/bin/Debug/net10.0/linux-x64/Paperless.Cli
: > "$OUT/rows.tsv"
while IFS= read -r rel; do
  [ -n "$rel" ] || continue
  src="$ROOT/$rel"
  base=$(basename "$rel")
  stem="${base%.*}"
  dir="$OUT/one"
  rm -rf "$dir"; mkdir -p "$dir"
  timeout 900 "$CLI" render --format pdf --outdir "$dir" --quiet "$src" >/dev/null 2>&1
  pdf=$(ls "$dir"/*.pdf 2>/dev/null | head -1)
  if [ -n "${pdf:-}" ] && [ -f "$pdf" ]; then
    pages=$(pdfinfo "$pdf" 2>/dev/null | awk '/^Pages:/{print $2}')
    words=$(pdftotext "$pdf" - 2>/dev/null | wc -w)
  else
    pages=0; words=0
  fi
  printf '%s\t%s\t%s\n' "$rel" "${pages:-0}" "${words:-0}" >> "$OUT/rows.tsv"
done < "$LIST"
rm -rf "$OUT/one"
wc -l "$OUT/rows.tsv"
