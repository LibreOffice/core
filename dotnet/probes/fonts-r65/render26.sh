#!/usr/bin/env bash
# Render every DOCX in a directory through 26.2.4.2 and report the face `pdffonts` names.
#
#   render26.sh <srcdir> <out.tsv>
set -uo pipefail
SRC="${1:?usage: render26.sh <srcdir> <out.tsv>}"
OUT="${2:?usage: render26.sh <srcdir> <out.tsv>}"
LO26="${LO26:-/opt/libreoffice26.2/program/soffice}"
WORK="$(mktemp -d "${TMPDIR:-/home/user/wt-script/dotnet/probes/fonts-r65/work}/r26.XXXXXX")"
PROFILE="$WORK/profile"

D=/opt/libreoffice26.2/share/fonts/truetype
for f in NotoSans-Regular.ttf NotoSerif-Regular.ttf DejaVuSans.ttf DejaVuSerif.ttf \
         Carlito-Regular.ttf Caladea-Regular.ttf LiberationSerif-Regular.ttf opens___.ttf; do
  [ -e "$D/$f" ] || continue
  echo "!! $D still holds $f -- move the confounds aside first." >&2
  exit 1
done

: > "$OUT"
for f in "$SRC"/*.docx; do
  stem="$(basename "$f" .docx)"
  d="$WORK/$stem"; mkdir -p "$d"
  timeout 300 "$LO26" --headless -env:UserInstallation="file://$PROFILE" \
    --convert-to pdf --outdir "$d" "$f" >/dev/null 2>&1
  pdf="$(find "$d" -name '*.pdf' | head -1)"
  if [ -n "$pdf" ]; then
    faces="$(pdffonts "$pdf" 2>/dev/null | tail -n +3 | awk 'NF{print $1}' \
             | sed -E 's/^[A-Z]{6}\+//' | sort -u | paste -sd, -)"
  else
    faces="(no render)"
  fi
  printf '%s\t%s\n' "$stem" "$faces" >> "$OUT"
done
rm -rf "$WORK"
cat "$OUT"
