#!/bin/sh
# Render the movers through 26.2.4.2, one at a time, each into its own directory and each with
# its own user installation -- and check the output exists before anything is read from it.
OUT="$1"
REF=/opt/libreoffice26.2/program/soffice
mkdir -p "$OUT"
while IFS= read -r rel; do
    [ -n "$rel" ] || continue
    b=`echo "$rel" | tr / _`
    mkdir -p "$OUT/$b"
    timeout -k 30 900 "$REF" --headless --norestore \
        -env:UserInstallation="file:///tmp/cfmix-r144-ref-$$" \
        --convert-to pdf --outdir "$OUT/$b" "/home/user/sample-files/$rel" >/dev/null 2>&1
    if ls "$OUT/$b"/*.pdf >/dev/null 2>&1; then
        echo "ok   $rel"
    else
        echo "FAIL $rel"
    fi
done < "$2"
