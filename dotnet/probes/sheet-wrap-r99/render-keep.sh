#!/bin/bash
# render-keep.sh <list> <outdir>  -- one directory per document, PDF kept, %%EOF checked
set -u
LIST="$1"; OUT="$2"
mkdir -p "$OUT"
while IFS= read -r src; do
  [ -z "$src" ] && continue
  base=$(basename "$src"); stem="${base%.*}"; ext="${base##*.}"
  ident="${stem}__${ext}"
  d="$OUT/$ident"; rm -rf "$d"; mkdir -p "$d"
  SOURCE_DATE_EPOCH=1757462400 "$XRAY_OFFICE_CLI" render "$src" --outdir "$d" >/dev/null 2>&1
  f=$(ls "$d"/*.pdf 2>/dev/null | head -1)
  if [ -z "$f" ]; then echo "$ident FAILED"; continue; fi
  if ! tail -c 2048 "$f" | grep -q '%%EOF'; then echo "$ident TRUNCATED"; continue; fi
  mv "$f" "$OUT/$ident.pdf"; rmdir "$d"
  echo "$ident ok $(stat -c %s "$OUT/$ident.pdf")"
done < "$LIST"
