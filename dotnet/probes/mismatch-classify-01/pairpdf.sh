#!/usr/bin/env bash
# Compose one page of a stored gate rendering against its stored reference.
#
#   pairpdf.sh <id> <page> [refdir] [dpi]
#
# `<id>` is the per-format identity batch-check.sh writes (`stem__ext`).  Both halves come
# out of already-banked PDFs, so nothing is re-rendered and the two halves cannot end up at
# different dpi.  Prints the composed image path.
set -euo pipefail
ID="$1"; P="$2"; REFDIR="${3:-/home/user/gate-2f47/ref}"; DPI="${4:-150}"
OURS="/home/user/gate-2f47/ours/$ID.pdf"
REF="$REFDIR/$ID.pdf"
OUT="/home/user/mismatch-work/pairs"
mkdir -p "$OUT"
T="$(mktemp -d)"
trap 'rm -rf "$T"' EXIT
pdftoppm -r "$DPI" -f "$P" -l "$P" -png -singlefile "$OURS" "$T/o"
pdftoppm -r "$DPI" -f "$P" -l "$P" -png -singlefile "$REF"  "$T/r"
python3 /home/user/wt-mismatch/.claude/skills/page-vision/scripts/compose.py \
    "$T/o.png" "$T/r.png" -o "$OUT/${ID}-p${P}.png" >&2
echo "$OUT/${ID}-p${P}.png"
