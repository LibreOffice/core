#!/usr/bin/env bash
# Produce reference text extractions from documents using headless LibreOffice.
#
# The filter depends on the document family, and Impress has no plain-text filter at
# all, so presentations are exported as HTML. See SKILL.md.
set -uo pipefail

usage() {
    cat <<'USAGE'
Usage: lo-extract.sh --outdir DIR FILE...

Picks LibreOffice's text-export filter from each input's extension:
  Writer formats  -> .txt   (filter: Text)
  Calc formats    -> .csv   (filter: Text - txt - csv (StarCalc)) - FIRST SHEET ONLY
  Impress formats -> .html  (filter: impress_html_Export)

Output goes to DIR/<stem>/ , one directory per input, so same-stem inputs
cannot overwrite each other.

Exit status is non-zero if any input failed.
USAGE
}

outdir=""
inputs=()
while [ $# -gt 0 ]; do
    case "$1" in
        --outdir) outdir="${2:?--outdir needs a value}"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        --*) printf 'unknown option: %s\n' "$1" >&2; usage >&2; exit 2 ;;
        *) inputs+=("$1"); shift ;;
    esac
done
if [ -z "$outdir" ] || [ "${#inputs[@]}" -eq 0 ]; then usage >&2; exit 2; fi
command -v soffice >/dev/null 2>&1 || { echo "soffice not on PATH" >&2; exit 3; }

mkdir -p "$outdir"; outdir="$(cd "$outdir" && pwd)"
profile="$(mktemp -d)"; trap 'rm -rf "$profile"' EXIT

# Map extension -> conversion target. Kept explicit rather than inferred so that an
# unrecognised extension is an error rather than a silently wrong filter.
target_for() {
    case "$(printf '%s' "$1" | tr '[:upper:]' '[:lower:]')" in
        doc|docx|docm|dot|dotx|dotm|rtf|odt|ott|fodt|sxw|stw) echo "txt:Text" ;;
        xls|xlsx|xlsm|xlt|xltx|xltm|xlsb|ods|ots|fods|sxc|stc|csv) echo "csv" ;;
        ppt|pptx|pptm|pot|potx|potm|pps|ppsx|ppsm|odp|otp|fodp|sxi|sti) echo "html" ;;
        *) echo "" ;;
    esac
}

failures=0
for input in "${inputs[@]}"; do
    if [ ! -f "$input" ]; then
        printf 'MISSING  %s\n' "$input" >&2; failures=$((failures + 1)); continue
    fi
    base="$(basename "$input")"
    ext="${base##*.}"
    stem="${base%.*}"
    target="$(target_for "$ext")"
    if [ -z "$target" ]; then
        printf 'SKIPPED  %s (no text filter known for .%s)\n' "$input" "$ext" >&2
        failures=$((failures + 1)); continue
    fi

    abs="$(cd "$(dirname "$input")" && pwd)/$base"
    # Keyed on the full basename, not the stem: 'doc.docx' and 'doc.xlsx' share a stem
    # and would otherwise overwrite each other.
    key="$(printf '%s' "$base" | tr -c 'A-Za-z0-9._-' '_')"
    dest="$outdir/$key"; mkdir -p "$dest"

    soffice --headless --norestore --nolockcheck --nodefault \
            -env:UserInstallation="file://$profile" \
            --convert-to "$target" --outdir "$dest" "$abs" >/dev/null 2>&1

    # -type f matters: the destination directory is itself named after the full
    # basename (e.g. "t.docx"), so a bare -name "$stem.*" glob matches the directory.
    produced="$(find "$dest" -maxdepth 1 -type f -name "$stem.*" ! -name '*.pdf' | head -1)"
    if [ -z "$produced" ]; then
        # soffice exits 0 even when it converts nothing, so check for the file.
        printf 'FAILED   %s (no output produced)\n' "$input" >&2
        failures=$((failures + 1)); continue
    fi
    bytes="$(wc -c < "$produced" | tr -d ' ')"
    printf 'OK       %s -> %s (%s bytes)\n' "$input" "${produced#"$outdir"/}" "$bytes"
    case "$target" in
        csv) printf '         note: CSV export covers the FIRST SHEET only\n' ;;
    esac
done

[ "$failures" -gt 0 ] && { printf '\n%s input(s) failed.\n' "$failures" >&2; exit 1; }
exit 0
