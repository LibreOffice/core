#!/usr/bin/env bash
set -euo pipefail
W=/home/user/libreoffice-core/.claude/worktrees/slides-r30
DST="$1"
rm -rf "$DST"; mkdir -p "$DST"
cp -a "$W/dotnet/tools/Paperless.Cli/bin/Debug/net10.0/linux-x64/." "$DST/"
md5sum "$DST"/Paperless*.dll | sort > "$DST.md5"
echo "snapshot $DST"
