#!/usr/bin/env bash
# Render every document under a directory with both LibreOffice and Paperless, and
# report whether the two PDFs agree.
#
#   corpus-parity.sh <dir> [outdir]
#
# Writes a TSV to stdout: path, ext, pages(ours/ref), words(ours/ref), fonts(ours/ref),
# unembedded(ours), verdict.
#
# Three checks, and the order matters — each is cheap and rules out a whole class:
#
#   pages   pagination. A wrong page count means every later comparison is comparing
#           different pages, so nothing after it is meaningful.
#   words   text presence and extractability. Catches a PDF whose glyphs land perfectly
#           but whose text cannot be selected — invisible to any geometric comparison,
#           and a defect this project has shipped twice.
#   fonts   every referenced face carries a font program. A PDF that embeds nothing
#           renders as tofu in a viewer while extracting flawlessly, so neither of the
#           checks above sees it.
#
# What this does NOT check is where the ink is. That is the fidelity suite's job, which
# compares our PDF's operators against LibreOffice's directly. Parity here is a gate,
# not a proof: it says a document is worth measuring precisely, not that it is correct.
set -uo pipefail

DIR="${1:?usage: corpus-parity.sh <dir> [outdir]}"
OUT="${2:-$(mktemp -d)}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)"
CLI="$ROOT/dotnet/tools/Paperless.Cli/bin/Debug/net10.0/linux-x64/Paperless.Cli"

[ -x "$CLI" ] || { echo "no CLI at $CLI — build it first" >&2; exit 1; }
mkdir -p "$OUT/ours" "$OUT/ref"

printf "path\text\tpages\twords\tfonts\tunemb\tverdict\n"

while IFS= read -r -d '' f; do
  base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
  case "$ext" in
    odt|ott|fodt|docx|docm|dotx|dotm|doc|dot|rtf) ;;
    ods|ots|fods|xlsx|xlsm|xltx|xltm|xlsb|xls|xlt|csv) ;;
    odp|otp|fodp|pptx|pptm|potx|potm|ppsx|ppsm|ppt|pot|pps) ;;
    *) continue ;;
  esac

  # Per-format names throughout: two documents differing only by extension both
  # convert to <stem>.pdf and one silently overwrites the other. This has cost
  # several people an afternoon each.
  id="${stem}__${ext}"
  o="$OUT/ours/$id.pdf"; r="$OUT/ref/$id.pdf"

  timeout 240 "$CLI" render "$f" --format pdf --outdir "$OUT/ours" >/dev/null 2>&1
  [ -f "$OUT/ours/$stem.pdf" ] && mv -f "$OUT/ours/$stem.pdf" "$o"
  timeout 300 soffice --headless --convert-to pdf --outdir "$OUT/ref" "$f" >/dev/null 2>&1
  [ -f "$OUT/ref/$stem.pdf" ] && mv -f "$OUT/ref/$stem.pdf" "$r"

  if [ ! -f "$o" ] && [ ! -f "$r" ]; then v="both-failed"
  elif [ ! -f "$o" ]; then v="ours-failed"
  elif [ ! -f "$r" ]; then v="ref-failed"
  else v=""
  fi

  op="-"; rp="-"; ow="-"; rw="-"; of="-"; rf="-"; un="-"
  if [ -f "$o" ]; then
    op=$(pdfinfo "$o" 2>/dev/null | awk '/^Pages/{print $2}')
    ow=$(pdftotext "$o" - 2>/dev/null | wc -w)
    of=$(pdffonts "$o" 2>/dev/null | tail -n +3 | grep -c .)
    un=$(pdffonts "$o" 2>/dev/null | tail -n +3 | awk '$(NF-3)=="no"' | wc -l)
  fi
  if [ -f "$r" ]; then
    rp=$(pdfinfo "$r" 2>/dev/null | awk '/^Pages/{print $2}')
    rw=$(pdftotext "$r" - 2>/dev/null | wc -w)
    rf=$(pdffonts "$r" 2>/dev/null | tail -n +3 | grep -c .)
  fi

  if [ -z "$v" ]; then
    v="match"
    [ "$op" = "$rp" ] || v="pages"
    [ "$ow" = "$rw" ] || v="${v/match/}${v:+,}words"
    [ "${un:-0}" = "0" ] || v="${v/match/}${v:+,}unembedded"
    [ -n "$v" ] || v="match"
  fi

  printf "%s\t%s\t%s/%s\t%s/%s\t%s/%s\t%s\t%s\n" \
    "${f#"$DIR"/}" "$ext" "$op" "$rp" "$ow" "$rw" "$of" "$rf" "$un" "$v"
done < <(find "$DIR" -type f -print0 | sort -z)
