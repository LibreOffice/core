#!/usr/bin/env bash
# probe-run.sh <dir-of-docx> — render each with soffice and with the CLI, report the y of
# the first word on page 2.
S=/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad
CLI="${PAPERLESS_CLI:-/home/user/libreoffice-core/.claude/worktrees/agent-aafbce72884dcea0e/dotnet/tools/Paperless.Cli/bin/Debug/net10.0/linux-x64/Paperless.Cli}"
D="$1"
mkdir -p "$D/ref" "$D/ours"
for f in "$D"/*.docx; do
  stem="$(basename "$f" .docx)"
  soffice --headless -env:UserInstallation=file://"$D"/prof --convert-to pdf --outdir "$D/ref" "$f" >/dev/null 2>&1
  "$CLI" render "$f" --format pdf --outdir "$D/ours" >/dev/null 2>&1
  ry=$(pdftotext -bbox -f 2 -l 2 "$D/ref/$stem.pdf" - 2>/dev/null | grep -m1 '<word' | sed 's/.*yMin="\([0-9.]*\)".*/\1/')
  oy=$(pdftotext -bbox -f 2 -l 2 "$D/ours/$stem.pdf" - 2>/dev/null | grep -m1 '<word' | sed 's/.*yMin="\([0-9.]*\)".*/\1/')
  rp=$(pdfinfo "$D/ref/$stem.pdf" 2>/dev/null | awk '/^Pages/{print $2}')
  op=$(pdfinfo "$D/ours/$stem.pdf" 2>/dev/null | awk '/^Pages/{print $2}')
  printf "%-32s ref pg2 y=%-10s (%s pages)   ours pg2 y=%-10s (%s pages)\n" "$stem" "$ry" "$rp" "$oy" "$op"
done
