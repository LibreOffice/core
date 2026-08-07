#!/usr/bin/env bash
# inkcompare.sh <before-sweep-dir> <after-sweep-dir> [names-file]
# Runs pdf-image-diff for both sweeps' output against the same reference PDFs and prints
# the major-page count per document.
S=/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad
DIFF=/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py
BEFORE="$1"; AFTER="$2"; LIST="${3:-}"
printf "%-58s %8s %8s\n" document before after
while read -r stem; do
  [ -n "$stem" ] || continue
  ref="$BEFORE/ref/$stem.pdf"
  [ -f "$ref" ] || ref="$AFTER/ref/$stem.pdf"
  [ -f "$ref" ] || { printf "%-58s  no ref\n" "$stem"; continue; }
  b=$(python3 "$DIFF" "$BEFORE/ours/$stem.pdf" "$ref" --quiet --outdir "$S/w11-ink/b" 2>/dev/null \
      | grep -c MAJOR)
  a=$(python3 "$DIFF" "$AFTER/ours/$stem.pdf" "$ref" --quiet --outdir "$S/w11-ink/a" 2>/dev/null \
      | grep -c MAJOR)
  printf "%-58s %8s %8s\n" "${stem:0:58}" "$b" "$a"
done < "$LIST"
