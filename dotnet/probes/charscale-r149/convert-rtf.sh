#!/bin/sh
# Build the converted `.rtf` column the corpus does not have: 26.2.4.2's own `--convert-to rtf`
# of every words-track document, into a directory OUTSIDE the repository.
#
# Three things this has to get right, each of which has cost this project a round:
#   * a UNIQUE STEM per output -- `soffice --convert-to` names its output after the input stem
#     alone, so two inputs sharing a stem silently become one output.  The stem is the first 12
#     hex of the md5 of the input's ABSOLUTE path, which is exactly the rule
#     `/home/user/corpus-odf/odt` was built with, so the two columns line up document for
#     document;
#   * a BOUNDED `timeout -k 30` -- `soffice` execs `oosplash`, which ignores SIGTERM, so a plain
#     `timeout` waits forever (dotnet/CLAUDE.md records an 87-minute hang);
#   * ONE PROFILE PER WORKER -- two live `soffice` in one user installation destroy each other.
set -eu
CORPUS=/home/user/sample-files
OUT=${1:-/home/user/corpus-odf/rtf}
JOBS=${2:-6}
SOFFICE=/opt/libreoffice26.2/program/soffice
mkdir -p "$OUT"

awk -F'\t' 'NR>1 && $1=="words"{print $3}' "$CORPUS/MANIFEST.tsv" | sort -u \
| xargs -P "$JOBS" -I{} sh -c '
    rel="{}"
    src="'"$CORPUS"'/$rel"
    stem=$(printf %s "$src" | md5sum | cut -c1-12)
    work=$(mktemp -d /tmp/charscale-r149-XXXXXX)
    ext=$(printf %s "$rel" | sed "s/.*\.//")
    base=$(basename "$rel" ".$ext")
    cp "$src" "$work/$stem-$base.$ext"
    if timeout -k 30 300 '"$SOFFICE"' --headless --norestore \
         -env:UserInstallation="file://$work/profile" \
         --convert-to rtf --outdir "'"$OUT"'" "$work/$stem-$base.$ext" >/dev/null 2>&1 \
       && [ -s "'"$OUT"'/$stem-$base.rtf" ]; then
        echo "ok	$stem	$rel"
    else
        echo "FAIL	$stem	$rel"
    fi
    rm -rf "$work"
'
