#!/bin/sh
# Re-create the .ods half of /home/user/corpus-odf with 26.2.4.2, one output
# directory per DOCUMENT (never per worker slot -- two live conversions in one
# directory delete each other's output, silently).
SOFF=/opt/libreoffice26.2/program/soffice
OUT=/home/user/corpus-odf/ods
LOG=/home/user/corpus-odf/convert-ods.log
: > "$LOG"
i=0
find /home/user/sample-files/sheets -type f \( -iname '*.xlsx' -o -iname '*.xls' -o -iname '*.xlsm' \) \
  | sort | while IFS= read -r f; do
    i=$((i+1))
    d=$(printf '%s' "$f" | md5sum | cut -c1-12)
    w="$OUT/.w/$d"
    mkdir -p "$w"
    timeout -k 30 300 "$SOFF" --headless -env:UserInstallation="file:///tmp/xray-office-lo-conv$((i % 3))" \
        --convert-to ods --outdir "$w" "$f" >/dev/null 2>&1
    o=$(ls "$w"/*.ods 2>/dev/null | head -1)
    if [ -n "$o" ]; then
        mv "$o" "$OUT/$d-$(basename "$o")"
        echo "ok	$d	$f" >> "$LOG"
    else
        echo "FAILED	$d	$f" >> "$LOG"
    fi
    rmdir "$w" 2>/dev/null
done
echo "DONE $(grep -c '^ok' "$LOG") ok, $(grep -c '^FAILED' "$LOG") failed" >> "$LOG"
