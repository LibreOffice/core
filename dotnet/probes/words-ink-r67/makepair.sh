#!/usr/bin/env bash
# Compose one page of an already-rendered pair into a single labelled image.
#   makepair.sh <id> <page> [dpi] [outdir]
set -euo pipefail
ID="$1"; PAGE="$2"; DPI="${3:-150}"; OUT="${4:-/home/user/tmp-words67/pairs}"
O=/home/user/tmp-words67/ours-after/"$ID".pdf
R=/home/user/tmp-words67/ref26/"$ID".pdf
[ -f "$O" ] && [ -f "$R" ] || { echo "missing pdf for $ID" >&2; exit 1; }
mkdir -p "$OUT"
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
pdftoppm -r "$DPI" -f "$PAGE" -l "$PAGE" -png "$O" "$T/ours"
pdftoppm -r "$DPI" -f "$PAGE" -l "$PAGE" -png "$R" "$T/ref"
a=$(ls "$T"/ours-*.png | head -1); b=$(ls "$T"/ref-*.png | head -1)
[ -s "$a" ] && [ -s "$b" ] || { echo "pdftoppm produced nothing for $ID p$PAGE" >&2; exit 1; }
python3 /home/user/wt-words67/.claude/skills/page-vision/scripts/compose.py \
    "$a" "$b" -o "$OUT/$ID-p$PAGE.png" >&2
echo "$OUT/$ID-p$PAGE.png"
