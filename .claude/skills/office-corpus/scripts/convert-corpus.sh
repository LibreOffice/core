#!/usr/bin/env bash
# Fan the sample corpus out into the ODF and RTF spellings of each document's own family,
# using headless LibreOffice, so the OpenDocument and RTF readers get corpus coverage.
#
# Why this exists: MANIFEST.tsv's `ext` column holds only docx/pptx/xlsx/doc/xls/ppt/xlsm.
# There is no ODF, no RTF and no template in the corpus at all, so three readers named in
# `dotnet/CLAUDE.md`'s Scope have never been measured against a real document.
#
# What the output is, and is not. Each file is *LibreOffice's own export* of a corpus
# document, so it is not the document a third-party writer would have produced. That is
# fine for the question it answers -- both renderers read the same file, so a divergence
# is ours -- and it is NOT evidence about how well we read ODF written by anything else.
#
# The output mirrors the corpus layout (family/batch/ext/stem.ext) so `batch-check.sh`
# and `ref-baseline.sh` glob it unchanged.
set -uo pipefail

SOFFICE="${SOFFICE:-/opt/libreoffice26.2/program/soffice}"
corpus="${1:?usage: convert-corpus.sh CORPUS OUTDIR [FAMILY_REGEX] [WORKERS]}"
outdir="${2:?usage: convert-corpus.sh CORPUS OUTDIR [FAMILY_REGEX] [WORKERS]}"
# A regex matched against MANIFEST.tsv's family column, not a shell glob: "." is all
# three tracks, "words" one of them. A "*" here is not a valid regex and awk rejects it.
family_re="${3:-.}"
workers="${4:-6}"

command -v timeout >/dev/null || { echo "timeout(1) required" >&2; exit 3; }
[ -x "$SOFFICE" ] || { echo "no soffice at $SOFFICE" >&2; exit 3; }

mkdir -p "$outdir"; outdir="$(cd "$outdir" && pwd)"
corpus="$(cd "$corpus" && pwd)"

# The target spellings, by family. Flat ODF is deliberately absent: a flat export of a
# 700-page document is tens of megabytes and this container's writable allowance is finite.
targets_for() {
    case "$1" in
        words)  echo "odt rtf" ;;
        sheets) echo "ods" ;;
        slides) echo "odp" ;;
        *)      echo "" ;;
    esac
}

# One document, one spelling. soffice names its output after the input stem alone, so each
# conversion gets its own directory and there is nothing for a same-stem file to overwrite.
convert_one() {
    local src="$1" family="$2" batch="$3" want="$4"
    local stem; stem="$(basename "${src%.*}")"
    local dest="$outdir/$family/$batch/$want"
    local out="$dest/$stem.$want"

    [ -s "$out" ] && { printf 'SKIP\t%s\n' "$out"; return 0; }
    mkdir -p "$dest"

    local profile; profile="$(mktemp -d "${TMPDIR:-/tmp}/convprof.XXXXXX")"
    timeout 180 "$SOFFICE" --headless --norestore --nolockcheck --nodefault \
        -env:UserInstallation="file://$profile" \
        --convert-to "$want" --outdir "$dest" "$src" >/dev/null 2>&1
    local rc=$?
    rm -rf "$profile"

    # Assert the instrument produced output. A conversion that fails silently and is then
    # swept looks exactly like a document we cannot read.
    if [ -s "$out" ]; then printf 'OK\t%s\t%s\n' "$out" "$(stat -c%s "$out")"
    else printf 'FAIL\trc=%s\t%s\t%s\n' "$rc" "$src" "$want"; fi
}
export -f convert_one
export outdir SOFFICE

# Drive from MANIFEST.tsv rather than a filesystem walk: the corpus mount is
# case-insensitive and a walk sees alias entries that git never created.
manifest="$corpus/MANIFEST.tsv"
[ -f "$manifest" ] || { echo "no MANIFEST.tsv under $corpus" >&2; exit 3; }

awk -F'\t' -v g="$family_re" 'NR>1 && $1 ~ g { print $1"\t"$2"\t"$3 }' "$manifest" |
while IFS=$'\t' read -r family batch path; do
    for want in $(targets_for "$family"); do
        printf '%s\t%s\t%s\t%s\n' "$corpus/$path" "$family" "$batch" "$want"
    done
done | xargs -P "$workers" -d'\n' -I{} bash -c 'IFS=$'"'"'\t'"'"' read -r s f b w <<< "{}"; convert_one "$s" "$f" "$b" "$w"'
