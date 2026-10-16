#!/usr/bin/env bash
# Our own face set for every corpus document, one row per file.
#
#   ourfaces.sh <out.tsv> [workers]
#
# The reference half is not rendered here on purpose: a font change is measured by what *moves*
# between two of our own sweeps, and only the documents that moved are worth a `soffice` run. Each
# PDF is read for its face list and deleted immediately, so the sweep costs no disk.
set -uo pipefail
OUT="${1:?usage: ourfaces.sh <out.tsv> [workers]}"
WORKERS="${2:-4}"
ROOT=/home/user/sample-files
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI to the binary you mean to measure}"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-1700000000}"

one() {
  local f="$1" id d pdf faces
  id="$(printf '%s' "$f" | md5sum | cut -c1-12)"
  d="$WORK/$id"; mkdir -p "$d"
  timeout 300 "$CLI" render "$f" --format pdf --outdir "$d" >/dev/null 2>&1
  pdf="$(find "$d" -name '*.pdf' | head -1)"
  if [ -n "$pdf" ]; then
    faces="$(pdffonts "$pdf" 2>/dev/null | tail -n +3 | awk 'NF{print $1}' \
             | sed -E 's/^[A-Z]{6}\+//' | sort -u | paste -sd, -)"
  else
    faces="(no render)"
  fi
  printf '%s\t%s\n' "${f#"$ROOT"/}" "$faces"
  rm -rf "$d"
}
export -f one
export WORK CLI ROOT

cut -f3 "$ROOT/MANIFEST.tsv" | tail -n +2 | sed "s|^|$ROOT/|" | sort -u \
  | xargs -P "$WORKERS" -I{} bash -c 'one "$@"' _ {} > "$OUT"
wc -l < "$OUT"
