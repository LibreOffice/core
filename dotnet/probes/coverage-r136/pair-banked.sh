#!/usr/bin/env bash
# Compose one page of ours against the BANKED reference, with no `soffice` anywhere.
#
#   pair-banked.sh <our.pdf> <ref.pdf> <page> <outdir> <label>
#
# `page-vision`'s own `pair.sh` chains `look.py`, which re-renders the reference through
# `soffice`.  A round scoring against a banked gate must not: the bank is the leg its
# figures were taken against, and re-rendering it invites C11's run-to-run variation into
# the picture as well as costing the reference binary.
set -euo pipefail
OURS="$1"; REF="$2"; PG="$3"; OUT="$4"; LABEL="$5"; DPI="${6:-150}"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
mkdir -p "$OUT"
T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT
pdftoppm -r "$DPI" -f "$PG" -l "$PG" -png "$OURS" "$T/ours"
pdftoppm -r "$DPI" -f "$PG" -l "$PG" -png "$REF"  "$T/ref"
python3 "$HERE/../../../.claude/skills/page-vision/scripts/compose.py" \
    "$(find "$T" -name 'ours-*.png' | head -1)" \
    "$(find "$T" -name 'ref-*.png' | head -1)" \
    -o "$OUT/$LABEL-p$PG-pair.png" >&2
echo "$OUT/$LABEL-p$PG-pair.png"
