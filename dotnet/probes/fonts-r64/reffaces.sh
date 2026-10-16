#!/usr/bin/env bash
# The reference's face set for a named list of documents, one row per file.
#
#   reffaces.sh <docs.txt> <out.tsv> [outdir]
#
# Only the documents a change moved are worth a `soffice` run, so this takes the list rather than
# walking the corpus. 26.2.4.2 with the three font confounds moved aside is the target; see
# `dotnet/CLAUDE.md`, "Installing a specific LibreOffice".
set -uo pipefail
LIST="${1:?usage: reffaces.sh <docs.txt> <out.tsv> [outdir]}"
OUT="${2:?usage: reffaces.sh <docs.txt> <out.tsv> [outdir]}"
DIR="${3:-$(mktemp -d)}"
ROOT=/home/user/sample-files
LO26="${LO26:-/opt/libreoffice26.2/program/soffice}"
PROFILE="${LO26_PROFILE:-$DIR/profile}"
mkdir -p "$DIR/pdf"
: > "$OUT"

# The three confounds by name, and only those: the tarball also ships DejaVu *Condensed* and the
# script-specific Noto, which duplicate nothing installed but carry coverage the system genuinely
# lacks, and every stored figure in this tree was taken with them in place.
D=/opt/libreoffice26.2/share/fonts/truetype
for f in NotoSans-Regular.ttf NotoSerif-Regular.ttf DejaVuSans.ttf DejaVuSerif.ttf \
         Carlito-Regular.ttf Caladea-Regular.ttf LiberationSerif-Regular.ttf opens___.ttf; do
  [ -e "$D/$f" ] || continue
  echo "!! $D still holds $f; the 26.2 column would be the tarball, not the version." >&2
  echo "!! Move the confounds aside first -- see dotnet/CLAUDE.md." >&2
  exit 1
done

while IFS= read -r rel; do
  [ -n "$rel" ] || continue
  stem="$(printf '%s' "$rel" | md5sum | cut -c1-12)"
  work="$DIR/pdf/$stem"; mkdir -p "$work"
  timeout 300 "$LO26" --headless -env:UserInstallation="file://$PROFILE" \
    --convert-to pdf --outdir "$work" "$ROOT/$rel" >/dev/null 2>&1
  pdf="$(find "$work" -name '*.pdf' | head -1)"
  if [ -n "$pdf" ]; then
    faces="$(pdffonts "$pdf" 2>/dev/null | tail -n +3 | awk 'NF{print $1}' \
             | sed -E 's/^[A-Z]{6}\+//' | sort -u | paste -sd, -)"
  else
    faces="(no render)"
  fi
  printf '%s\t%s\n' "$rel" "$faces" >> "$OUT"
  rm -rf "$work"
done < "$LIST"
wc -l < "$OUT"
