#!/usr/bin/env bash
# Render the reference half for the ten .rtf documents the odf-gate-01 sweep died before reaching,
# so the column is 338 rather than 328. 26.2.4.2 from the TDF tarball, all four font confounds aside.
set -uo pipefail
SOF=/opt/libreoffice26.2/program/soffice
OUT="${1:?outdir}"; mkdir -p "$OUT/ref" "$OUT/prof"
while IFS= read -r rel; do
  f="/home/user/corpus-odf/$rel"
  base="$(basename "$f")"; stem="${base%.*}"
  d="$OUT/prof/$(echo -n "$rel" | md5sum | cut -c1-16)"
  mkdir -p "$d"
  timeout 300 "$SOF" --headless --norestore "-env:UserInstallation=file://$d" \
      --convert-to pdf --outdir "$d" "$f" >/dev/null 2>&1
  if [ -f "$d/$stem.pdf" ]; then mv -f "$d/$stem.pdf" "$OUT/ref/${stem}__rtf.pdf"; fi
  rm -rf "$d"
  printf '%s\t%s\n' "$rel" "$([ -f "$OUT/ref/${stem}__rtf.pdf" ] && echo ok || echo FAILED)"
done
