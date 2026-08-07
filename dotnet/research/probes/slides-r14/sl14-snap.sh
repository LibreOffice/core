#!/usr/bin/env bash
# Snapshot this round's CLI and PROVE it matches the built tree.
# A sweep against a stale snapshot passes every other check the skill prescribes.
set -euo pipefail
W=/home/user/libreoffice-core/.claude/worktrees/agent-afd045030560da190
S=/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/sl14-snap
SRC="$W/dotnet/tools/Paperless.Cli/bin/Debug/net10.0/linux-x64"
DST="$S/linux-x64"

rm -rf "$DST"
mkdir -p "$S"
cp -r "$SRC" "$S/"

bad=0
for f in Paperless.Presentations.dll Paperless.Rendering.dll Paperless.Core.dll \
         Paperless.Ooxml.dll Paperless.MsBinary.dll Paperless.Text.dll Paperless.Cli.dll; do
  [ -f "$SRC/$f" ] || { echo "MISSING $f"; bad=1; continue; }
  a=$(md5sum "$SRC/$f" | cut -d' ' -f1)
  b=$(md5sum "$DST/$f" | cut -d' ' -f1)
  if [ "$a" = "$b" ]; then echo "ok   $f  $a"; else echo "DIFF $f  $a != $b"; bad=1; fi
done
echo "CLI: $DST/Paperless.Cli"
exit $bad
