#!/usr/bin/env bash
# What does each stack resolve an absent family to?
#
#   run.sh [outdir] [--with-shipped]
#
# One family per file, so `pdffonts` attributes the answer without ambiguity, and every column
# is a face read out of a PDF the binary itself produced rather than an `fc-match` guess.
#
# ---------------------------------------------------------------- the precondition, read this
#
# **The TDF tarball ships its own Latin Noto and it voids the 26.2 column.**
# `/opt/libreoffice26.2/share/fonts/truetype` carries `NotoSans-*` and `NotoSerif-*`, eight faces
# that duplicate *nothing* installed — so the documented `mv` for the metric-compatible
# duplicates does not catch them — and LibreOffice reads its own bundle, so they become
# fontconfig's answer for every family the system lacks. A 26.2 column measured with them in
# place says `NotoSerif-Regular` on every unfiled row and looks like a coherent result. It is
# not one: it is the tarball, not the version, and a distro 26.2 answers DejaVu. Two agents lost
# hours to it independently in one session, and a whole round was dispatched on the strength of
# it. This script refuses to print a clean column it did not get.
#
#   D=/opt/libreoffice26.2/share/fonts/truetype
#   mkdir -p $D/.duplicates-aside && mv $D/{Carlito,Caladea,Liberation,DejaVu}*.ttf $D/.duplicates-aside/
#   mkdir -p $D/.noto-aside       && mv $D/Noto{Sans,Serif}-*.ttf $D/.noto-aside/
#
# Leave the script-specific Noto — `NotoSansArabic`, `NotoSerifHebrew` and the rest — in place.
# It carries coverage the system genuinely lacks and removing it changes what an Arabic or
# Hebrew document can draw at all.
#
# ------------------------------------------------------------------------------- the two rules
#
#   * 24.2.7.2 lets the family *name* decide: DejaVu Sans for a name nothing files,
#     Liberation Sans for Helvetica through 30-metric-aliases.conf, DejaVu Serif for Georgia and
#     Garamond through 45-latin.conf.
#   * 26.2.4.2 lets a declared family *class* beat the name. `FontConfigManager::Substitute`
#     appends "serif" as a second FC_FAMILY for FAMILY_ROMAN and "sans" for FAMILY_SWISS
#     (vcl/unx/generic/font/fontconfig.cxx:1075-1088), and that switch does not exist in 24.2.
#     A DOCX with no font table inherits Writer's roman default, so every unfiled family in a
#     word-processing document lands on a serif.
#
# **26.2 is the target.** A font-family divergence measured against 24.2 is very probably the
# version rule and not a defect.
#
# `--with-shipped` adds the as-shipped 26.2 column, and *mutates the shared install* to get it:
# it moves the Latin Noto back, renders, and moves it aside again under a trap. Off by default,
# because other sessions render against that install too. LO24, LO26 and XRAY_OFFICE_CLI override
# the three binaries.
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT="${1:-/tmp/fontprobe}"; [ "${OUT#--}" = "$OUT" ] || { OUT=/tmp/fontprobe; set -- "" "$1"; }
WITH_SHIPPED=0; for a in "$@"; do [ "$a" = "--with-shipped" ] && WITH_SHIPPED=1; done
LO24="${LO24:-soffice}"
LO26="${LO26:-/opt/libreoffice26.2/program/soffice}"
CLI="${XRAY_OFFICE_CLI:-$HERE/../../tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli}"
FONTS="${LO26_FONTS:-/opt/libreoffice26.2/share/fonts/truetype}"
mkdir -p "$OUT/ours" "$OUT/ref24" "$OUT/ref26" "$OUT/ref26shipped"

face() {
  pdffonts "$1" 2>/dev/null | tail -n +3 | awk 'NF{print $1}' \
    | sed 's/^[A-Z]\{6\}+//' | sort -u | paste -sd, -
}
render() {  # render <binary> <outdir> <src>
  [ -f "$2/$(basename "${3%.*}").pdf" ] || "$1" --headless --convert-to pdf --outdir "$2" "$3" >/dev/null 2>&1
}

# Is the 26.2 column clean? Named explicitly rather than assumed, because the failure is silent.
NOTO_PRESENT=0
compgen -G "$FONTS/NotoSans-*.ttf" >/dev/null && NOTO_PRESENT=1
compgen -G "$FONTS/NotoSerif-*.ttf" >/dev/null && NOTO_PRESENT=1
if [ ! -x "$LO26" ]; then
  LABEL26="26.2 (absent)"
elif [ "$NOTO_PRESENT" = 1 ]; then
  LABEL26="26.2 *** NOTO! ***"
  echo "!! $FONTS still holds the Latin Noto. The 26.2 column below is the TARBALL, not the" >&2
  echo "!! version, and is not comparable to anything. Move it aside — see the header." >&2
else
  LABEL26="26.2 clean (target)"
fi

if [ "$WITH_SHIPPED" = 1 ]; then
  [ -d "$FONTS/.noto-aside" ] || { echo "--with-shipped: no $FONTS/.noto-aside to restore from" >&2; exit 1; }
  restore() { mv -f "$FONTS"/NotoSans-*.ttf "$FONTS"/NotoSerif-*.ttf "$FONTS/.noto-aside/" 2>/dev/null; }
  trap restore EXIT
  mv -f "$FONTS/.noto-aside"/*.ttf "$FONTS/" 2>/dev/null
  i=0
  while IFS= read -r _fam; do
    render "$LO26" "$OUT/ref26shipped" "$HERE/src/$(printf 'f%02d' "$i").docx"; i=$((i + 1))
  done < "$HERE/families.txt"
  restore; trap - EXIT
fi

i=0
if [ "$WITH_SHIPPED" = 1 ]; then
  printf "%-26s %-20s %-20s %-20s %-20s\n" "requested" "24.2.7.2" "26.2 as shipped" "$LABEL26" "ours"
else
  printf "%-26s %-20s %-20s %-20s\n" "requested" "24.2.7.2" "$LABEL26" "ours"
fi
while IFS= read -r fam; do
  n=$(printf "f%02d" "$i"); i=$((i + 1))
  src="$HERE/src/$n.docx"
  render "$LO24" "$OUT/ref24" "$src"
  [ -x "$LO26" ] && render "$LO26" "$OUT/ref26" "$src"
  "$CLI" render "$src" --format pdf --outdir "$OUT/ours" >/dev/null 2>&1
  if [ "$WITH_SHIPPED" = 1 ]; then
    printf "%-26s %-20s %-20s %-20s %-20s\n" "$fam" "$(face "$OUT/ref24/$n.pdf")" \
      "$(face "$OUT/ref26shipped/$n.pdf")" "$(face "$OUT/ref26/$n.pdf")" "$(face "$OUT/ours/$n.pdf")"
  else
    printf "%-26s %-20s %-20s %-20s\n" "$fam" "$(face "$OUT/ref24/$n.pdf")" \
      "$(face "$OUT/ref26/$n.pdf")" "$(face "$OUT/ours/$n.pdf")"
  fi
done < "$HERE/families.txt"
