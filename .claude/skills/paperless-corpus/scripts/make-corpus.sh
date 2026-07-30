#!/usr/bin/env bash
# Generate a minimal test corpus covering every format Paperless targets, using
# headless LibreOffice to fan each source document out across its format family.
#
# Every output gets a unique stem, because soffice names output after the input stem
# alone and same-stem files silently overwrite each other.
set -uo pipefail

usage() {
    cat <<'USAGE'
Usage: make-corpus.sh --outdir DIR [--family writer|calc|impress|all]

Produces one small document per format:
  writer  : odt ott fodt doc docx rtf
  calc    : ods ots fods xls xlsx csv
  impress : odp otp fodp ppt pptx

Files are named "<content>-<format>.<ext>" so no two share a stem.
USAGE
}

outdir="" family="all"
while [ $# -gt 0 ]; do
    case "$1" in
        --outdir) outdir="${2:?--outdir needs a value}"; shift 2 ;;
        --family) family="${2:?--family needs a value}"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) printf 'unknown option: %s\n' "$1" >&2; usage >&2; exit 2 ;;
    esac
done
[ -z "$outdir" ] && { usage >&2; exit 2; }
command -v soffice >/dev/null 2>&1 || { echo "soffice not on PATH" >&2; exit 3; }

mkdir -p "$outdir"; outdir="$(cd "$outdir" && pwd)"
work="$(mktemp -d)"; profile="$work/profile"
trap 'rm -rf "$work"' EXIT

lo() { soffice --headless --norestore --nolockcheck --nodefault \
               -env:UserInstallation="file://$profile" "$@" >/dev/null 2>&1; }

made=0 failed=0

# convert <source> <target-ext> <output-stem>
# soffice names output after the SOURCE stem, so the source is copied to the desired
# stem first. That is what keeps every generated file uniquely named.
convert() {
    src="$1" ext="$2" stem="$3"
    staged="$work/$stem.${src##*.}"
    cp "$src" "$staged"
    lo --convert-to "$ext" --outdir "$outdir" "$staged"
    if [ -f "$outdir/$stem.$ext" ]; then
        printf '  OK    %s.%s\n' "$stem" "$ext"; made=$((made + 1))
    else
        printf '  FAIL  %s.%s\n' "$stem" "$ext" >&2; failed=$((failed + 1))
    fi
}

# ------------------------------------------------------------------ Writer sources
if [ "$family" = all ] || [ "$family" = writer ]; then
    echo "== writer =="
    # Deliberately exercises headings, a long paragraph that must wrap, and a list --
    # the wrapping is what makes font substitution problems visible.
    cat > "$work/src-text.txt" <<'DOC'
Paperless test document

This paragraph is deliberately long enough that it has to wrap across more than one
line at any sensible page width, because line breaking is where font substitution
problems first become visible.

Item one
Item two
Item three
DOC
    for ext in odt ott fodt doc docx rtf; do
        convert "$work/src-text.txt" "$ext" "prose-$ext"
    done
fi

# -------------------------------------------------------------------- Calc sources
if [ "$family" = all ] || [ "$family" = calc ]; then
    echo "== calc =="
    # Mixed value types plus a formula: numbers, text, a date and a computed cell all
    # render through different paths.
    cat > "$work/src-sheet.csv" <<'DOC'
Region,Units,Price,Total
North,12,4.50,=B2*C2
South,7,4.50,=B3*C3
East,23,3.25,=B4*C4
West,4,3.25,=B5*C5
DOC
    for ext in ods ots fods xls xlsx; do
        convert "$work/src-sheet.csv" "$ext" "sheet-$ext"
    done
    cp "$work/src-sheet.csv" "$outdir/sheet-csv.csv" && \
        { printf '  OK    sheet-csv.csv\n'; made=$((made + 1)); }
fi

# ----------------------------------------------------------------- Impress sources
if [ "$family" = all ] || [ "$family" = impress ]; then
    echo "== impress =="
    # Two slides, so page/slide counts above one are exercised.
    cat > "$work/src-slides.fodp" <<'DOC'
<?xml version="1.0" encoding="UTF-8"?>
<office:document
  xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
  xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
  xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
  xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
  xmlns:presentation="urn:oasis:names:tc:opendocument:xmlns:presentation:1.0"
  xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
  xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
  office:version="1.3"
  office:mimetype="application/vnd.oasis.opendocument.presentation">
 <office:automatic-styles/>
 <office:body>
  <office:presentation>
   <draw:page draw:name="page1">
    <draw:frame draw:layer="layout" svg:width="20cm" svg:height="3cm" svg:x="2cm" svg:y="2cm">
     <draw:text-box><text:p>Paperless: slide one</text:p></draw:text-box>
    </draw:frame>
    <draw:frame draw:layer="layout" svg:width="20cm" svg:height="4cm" svg:x="2cm" svg:y="6cm">
     <draw:text-box><text:p>Body text on the first slide.</text:p></draw:text-box>
    </draw:frame>
   </draw:page>
   <draw:page draw:name="page2">
    <draw:frame draw:layer="layout" svg:width="20cm" svg:height="3cm" svg:x="2cm" svg:y="2cm">
     <draw:text-box><text:p>Paperless: slide two</text:p></draw:text-box>
    </draw:frame>
   </draw:page>
  </office:presentation>
 </office:body>
</office:document>
DOC
    for ext in odp otp ppt pptx; do
        convert "$work/src-slides.fodp" "$ext" "slides-$ext"
    done
    cp "$work/src-slides.fodp" "$outdir/slides-fodp.fodp" && \
        { printf '  OK    slides-fodp.fodp\n'; made=$((made + 1)); }
fi

echo
echo "$made file(s) written to $outdir"
if [ "$failed" -gt 0 ]; then
    echo "$failed conversion(s) failed" >&2
    exit 1
fi
# Confirm every stem is unique -- the whole point of the naming scheme.
dupes="$(find "$outdir" -maxdepth 1 -type f -printf '%f\n' 2>/dev/null \
         | sed 's/\.[^.]*$//' | sort | uniq -d)"
if [ -n "$dupes" ]; then
    echo "WARNING: duplicate stems found; these will collide on conversion:" >&2
    printf '%s\n' "$dupes" >&2
    exit 1
fi
echo "all stems unique - safe to batch-convert"
