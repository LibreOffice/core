#!/usr/bin/env bash
# Render one page both ways and compose it into a single labelled image for review.
#
#   pair.sh <document> [--worst | --page N] [--dpi 150] [--outdir DIR]
#
# Prints the path of the composed image. That path is the whole artefact: hand it to a
# reviewer — a fresh subagent, or yourself — and nothing else is needed to read the page.
#
# This exists because the two halves must be rendered at the SAME dpi, and composing them
# by hand is where that goes wrong. look.py already renders both sides correctly; this
# chains it to compose.py without giving anyone a chance to mismatch the scales.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOOK="$HERE/../../render-comparison/scripts/look.py"

DOC=""; SEL=(--worst); DPI=150; OUTDIR="./pairs"
while [[ $# -gt 0 ]]; do
    case "$1" in
        --worst)  SEL=(--worst); shift ;;
        --page)   SEL=(--page "$2"); shift 2 ;;
        --dpi)    DPI="$2"; shift 2 ;;
        --outdir) OUTDIR="$2"; shift 2 ;;
        *)        DOC="$1"; shift ;;
    esac
done
[[ -n "$DOC" ]] || { echo "usage: pair.sh <document> [--worst|--page N] [--dpi N] [--outdir DIR]" >&2; exit 2; }
[[ -n "${XRAY_OFFICE_CLI:-}" ]] || { echo "XRAY_OFFICE_CLI is not set — it must point at the tree you mean to measure" >&2; exit 2; }

mkdir -p "$OUTDIR"
RAW="$(mktemp -d)"
trap 'rm -rf "$RAW"' EXIT

# look.py prints the ink figures on stdout; keep them, they say which side drew more.
python3 "$LOOK" "$DOC" "${SEL[@]}" --dpi "$DPI" --out "$RAW" >&2

OURS="$(find "$RAW" -name '*-ours-*.png' | head -1)"
REF="$(find "$RAW" -name '*-ref-*.png'  | head -1)"
[[ -n "$OURS" && -n "$REF" ]] || { echo "look.py did not produce both renderings" >&2; exit 1; }

STEM="$(basename "$OURS" | sed 's/-ours-.*//')"
OUT="$OUTDIR/$STEM-pair.png"
python3 "$HERE/compose.py" "$OURS" "$REF" -o "$OUT" >&2
echo "$OUT"
