#!/usr/bin/env bash
# Reach on the two tracks this round does not own, measured by rendering rather than censusing:
# every words and sheets document at both commits, byte-compared. Both renders set
# SOURCE_DATE_EPOCH, so a byte difference is the change and nothing else.
set -uo pipefail
SP=/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/slides-r29
ROOT=/workspace/sample-files
mkdir -p "$SP/cross/base" "$SP/cross/all" "$SP/cross/t0" "$SP/cross/t1"
: > "$SP/cross/changed.txt"
mapfile -t FILES < <(find "$ROOT"/words "$ROOT"/sheets -type f | sort)
echo "documents: ${#FILES[@]}"
one() {
  local idx="$1" i=-1 f base stem ext id
  for f in "${FILES[@]}"; do
    i=$((i+1)); [ $((i % 2)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"; id="${stem}__${ext,,}"
    for w in base all; do
      rm -rf "$SP/cross/t$idx"; mkdir -p "$SP/cross/t$idx"
      SOURCE_DATE_EPOCH=1700000000 timeout 300 "$SP/cli-$w/Paperless.Cli" render "$f" \
        --format pdf --outdir "$SP/cross/t$idx" >/dev/null 2>&1
      [ -f "$SP/cross/t$idx/$stem.pdf" ] && mv -f "$SP/cross/t$idx/$stem.pdf" "$SP/cross/$w/$id.pdf"
    done
    if [ -f "$SP/cross/base/$id.pdf" ] && [ -f "$SP/cross/all/$id.pdf" ]; then
      cmp -s "$SP/cross/base/$id.pdf" "$SP/cross/all/$id.pdf" || echo "${f#"$ROOT"/}" >> "$SP/cross/changed.txt"
    else
      echo "MISSING ${f#"$ROOT"/}" >> "$SP/cross/changed.txt"
    fi
    rm -f "$SP/cross/base/$id.pdf"
  done
}
for w in 0 1; do one "$w" & done
wait
echo "changed: $(grep -c . "$SP/cross/changed.txt")"
sort "$SP/cross/changed.txt"
