#!/usr/bin/env bash
# sl14-sweep.sh <outdir-name>
# Whole slides track, word gate + unaccounted ink, against the checksummed sl14 snapshot,
# reusing sl12-after2's reference PDFs (nothing this round touches soffice).
set -uo pipefail
S=/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad
W=/home/user/libreoffice-core/.claude/worktrees/agent-afd045030560da190
NAME="${1:?usage: sl14-sweep.sh <outdir-name>}"
"$W/.claude/skills/corpus-batches/scripts/track-ink-sweep.sh" \
  /workspace/sample-files 'slides/batch-0*' "$S/$NAME" 2 \
  "$S/sl14-snap/linux-x64/Paperless.Cli" \
  "$S/sl12-after2/ref"
