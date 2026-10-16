#!/usr/bin/env bash
# Render every probe through the reference alone, in parallel.
# One `soffice` profile per document, keyed on an md5 of its own path, because
# `-env:UserInstallation=file://…` truncates at the first space.
#
#   refrender.sh <probe-dir> <outdir> [jobs]
set -uo pipefail
IN="${1:?probe dir}"; OUT="${2:?outdir}"; JOBS="${3:-4}"
REF="${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}"
mkdir -p "$OUT/ref"
one() {
  local f="$1" out="$2" ref="$3"
  local d; d=$(printf '%s' "$f" | md5sum | cut -c1-16)
  timeout -k 30 300 "$ref" --headless -env:UserInstallation="file://${TMPDIR:-/tmp}/lo-$d" \
      --convert-to pdf --outdir "$out/ref" "$f" >/dev/null 2>&1
}
export -f one
ls "$IN"/*.rtf | xargs -P "$JOBS" -I{} bash -c 'one "$@"' _ {} "$OUT" "$REF"
n=$(ls "$IN"/*.rtf | wc -l); r=$(ls "$OUT/ref"/*.pdf 2>/dev/null | wc -l)
echo "probes=$n ref=$r"
[ "$r" -eq "$n" ] || { echo "MISSING OUTPUT — do not compare" >&2; exit 1; }
