#!/bin/sh
# Render each arm through 26.2.4.2, one at a time, each into its own profile, and refuse to
# report an arm whose output does not exist -- an instrument that produced nothing must never be
# compared.
DIR="${1:-fixtures}"
OUT="${2:-out}"
EXT="${3:-docx}"
mkdir -p "$OUT"
for f in "$DIR"/*."$EXT"; do
    b=`basename "$f" ."$EXT"`
    timeout -k 30 900 /opt/libreoffice26.2/program/soffice --headless --norestore \
        -env:UserInstallation="file:///tmp/brline-r149-$b" \
        --convert-to pdf --outdir "$OUT" "$f" >/dev/null 2>&1
    if [ -s "$OUT/$b.pdf" ]; then echo "ok   $b"; else echo "FAIL $b"; fi
done
