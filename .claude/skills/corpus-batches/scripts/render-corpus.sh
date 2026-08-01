#!/usr/bin/env bash
# Render an entire document corpus with LibreOffice, in parallel and resumably.
#
#   render-corpus.sh <corpus-root> <outdir> <worker-index> <worker-count> [max-files]
#
# Run one process per worker index:
#
#   for i in 0 1 2; do render-corpus.sh /workspace/sample-files /tmp/triage $i 3 & done
#
# Three details this exists to get right, each of which has cost real time here:
#
#   * **Separate soffice profiles.** Two headless instances sharing ~/.config/libreoffice
#     block on the profile lock, and the loser converts nothing while still exiting 0.
#   * **Per-format output names.** `report.doc` and `report.docx` both convert to
#     `report.pdf`, and one silently overwrites the other. The id keeps the source
#     extension and the source path, so nothing collides.
#   * **Resumable.** Anything already rendered or already recorded as failed is skipped, so
#     a worker can be killed and restarted — which matters, because a whole-corpus render
#     runs long enough to outlive whatever started it.
#
# `soffice` exits 0 even when it converts nothing, so success is decided by the output
# file existing, never by the exit code.
set -uo pipefail

SRC="${1:?usage: render-corpus.sh <corpus-root> <outdir> <worker-index> <worker-count> [max-files]}"
OUT="${2:?outdir}"
IDX="${3:?worker index, 0-based}"
N="${4:?worker count}"
LIMIT="${5:-100000}"

mkdir -p "$OUT/pdf" "$OUT/tmp$IDX" "$OUT/prof$IDX"
done_n=0; fail=0; i=-1

while IFS= read -r -d '' f; do
  i=$((i + 1)); [ $((i % N)) -eq "$IDX" ] || continue
  base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
  id="$(printf '%s' "${f#"$SRC"/}" | tr '/ ' '__' | sed 's/\.[^.]*$//')__${ext}"

  [ -f "$OUT/pdf/$id.pdf" ] && continue
  grep -qxF "$f" "$OUT/failed-$IDX.txt" 2>/dev/null && continue
  [ "$done_n" -ge "$LIMIT" ] && break

  rm -rf "${OUT:?}/tmp$IDX"; mkdir -p "$OUT/tmp$IDX"
  if timeout 150 soffice -env:UserInstallation="file://$OUT/prof$IDX" \
       --headless --convert-to pdf --outdir "$OUT/tmp$IDX" "$f" >/dev/null 2>&1 \
     && [ -f "$OUT/tmp$IDX/$stem.pdf" ]; then
    mv "$OUT/tmp$IDX/$stem.pdf" "$OUT/pdf/$id.pdf"
  else
    echo "$f" >> "$OUT/failed-$IDX.txt"; fail=$((fail + 1))
  fi
  done_n=$((done_n + 1))
done < <(find "$SRC" -type f \
           \( -iname '*.doc' -o -iname '*.docx' -o -iname '*.xls' -o -iname '*.xlsx' \
           -o -iname '*.ppt' -o -iname '*.pptx' -o -iname '*.odt' -o -iname '*.ods' \
           -o -iname '*.odp' -o -iname '*.rtf' \) \
           -not -path '*/.git/*' -print0 | sort -z)

echo "WORKER-$IDX-DONE rendered=$done_n failed=$fail"
