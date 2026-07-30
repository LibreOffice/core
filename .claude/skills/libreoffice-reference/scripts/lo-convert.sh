#!/usr/bin/env bash
# Produce reference PDFs and/or per-page PNGs from documents using headless LibreOffice.
#
# Handles the traps described in SKILL.md:
#   * gives every input its own output directory, so same-stem inputs cannot
#     overwrite each other;
#   * uses a private UserInstallation profile, so concurrent runs do not interfere;
#   * verifies the output exists, because soffice exits 0 even when it converts nothing;
#   * goes via PDF for page images, because --convert-to png only emits page 1.
set -uo pipefail

usage() {
    cat <<'USAGE'
Usage: lo-convert.sh [--pdf] [--png] [--dpi N] --outdir DIR FILE...

  --pdf          Produce a reference PDF per input (default when neither is given).
  --png          Produce per-page PNGs per input (implies --pdf; PNGs come from it).
  --dpi N        Rasterisation resolution for --png. Default 150.
  --outdir DIR   Where to write results. One subdirectory per input.
  --quiet        Only report failures.

Layout of the output:
  DIR/<stem>/<stem>.pdf
  DIR/<stem>/page-1.png, page-2.png, ...

Exit status is non-zero if any input failed to convert.
USAGE
}

want_pdf=0 want_png=0 dpi=150 outdir="" quiet=0
inputs=()
while [ $# -gt 0 ]; do
    case "$1" in
        --pdf)    want_pdf=1; shift ;;
        --png)    want_png=1; want_pdf=1; shift ;;
        --dpi)    dpi="${2:?--dpi needs a value}"; shift 2 ;;
        --outdir) outdir="${2:?--outdir needs a value}"; shift 2 ;;
        --quiet)  quiet=1; shift ;;
        -h|--help) usage; exit 0 ;;
        --*)      printf 'unknown option: %s\n' "$1" >&2; usage >&2; exit 2 ;;
        *)        inputs+=("$1"); shift ;;
    esac
done

[ "$want_pdf" = 0 ] && [ "$want_png" = 0 ] && want_pdf=1
if [ -z "$outdir" ] || [ "${#inputs[@]}" -eq 0 ]; then usage >&2; exit 2; fi
command -v soffice >/dev/null 2>&1 || { echo "soffice not on PATH" >&2; exit 3; }
if [ "$want_png" = 1 ] && ! command -v pdftoppm >/dev/null 2>&1; then
    echo "pdftoppm not on PATH (needed for --png); install poppler-utils" >&2; exit 3
fi

mkdir -p "$outdir"
outdir="$(cd "$outdir" && pwd)"

# A private profile keeps this run from colliding with any other soffice process.
profile="$(mktemp -d)"
trap 'rm -rf "$profile"' EXIT

note() { [ "$quiet" = 1 ] || printf '%s\n' "$*"; }

failures=0
declare -A used_keys=()
for input in "${inputs[@]}"; do
    if [ ! -f "$input" ]; then
        printf 'MISSING  %s\n' "$input" >&2
        failures=$((failures + 1))
        continue
    fi
    base="$(basename "$input")"
    abs="$(cd "$(dirname "$input")" && pwd)/$base"
    stem="${base%.*}"

    # One directory per input, keyed on the FULL basename including the extension --
    # NOT on the stem. Keying on the stem reintroduces exactly the trap this script
    # exists to avoid: 'doc.docx' and 'doc.xlsx' share the stem 'doc', so they would
    # land in the same directory and the second would overwrite the first.
    key="$(printf '%s' "$base" | tr -c 'A-Za-z0-9._-' '_')"
    # Two inputs from different directories can still share a basename; disambiguate.
    if [ -n "${used_keys[$key]+set}" ]; then
        used_keys[$key]=$(( used_keys[$key] + 1 ))
        key="$key-${used_keys[$key]}"
    else
        used_keys[$key]=1
    fi

    dest="$outdir/$key"
    mkdir -p "$dest"

    soffice --headless --norestore --nolockcheck --nodefault \
            -env:UserInstallation="file://$profile" \
            --convert-to pdf --outdir "$dest" "$abs" >/dev/null 2>&1

    pdf="$dest/$stem.pdf"
    if [ ! -f "$pdf" ]; then
        # soffice exits 0 on failure, so the file's existence is the only real signal.
        printf 'FAILED   %s (no PDF produced)\n' "$input" >&2
        failures=$((failures + 1))
        continue
    fi

    pages="?"
    if command -v pdfinfo >/dev/null 2>&1; then
        pages="$(pdfinfo "$pdf" 2>/dev/null | awk '/^Pages:/{print $2}')"
    fi

    if [ "$want_png" = 1 ]; then
        pdftoppm -r "$dpi" -png "$pdf" "$dest/page" 2>/dev/null
        rendered="$(find "$dest" -maxdepth 1 -name 'page-*.png' | wc -l | tr -d ' ')"
        if [ "$rendered" -eq 0 ]; then
            printf 'FAILED   %s (PDF made, but rasterising produced nothing)\n' "$input" >&2
            failures=$((failures + 1))
            continue
        fi
        note "OK       $input -> $pages page(s), $rendered PNG(s) at ${dpi}dpi"
    else
        note "OK       $input -> $pages page(s)"
    fi

    [ "$want_pdf" = 1 ] || rm -f "$pdf"
done

if [ "$failures" -gt 0 ]; then
    printf '\n%s input(s) failed.\n' "$failures" >&2
    exit 1
fi
note ""
note "All ${#inputs[@]} input(s) converted into $outdir"
