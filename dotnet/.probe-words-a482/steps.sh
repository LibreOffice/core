#!/usr/bin/env bash
cd /tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/words-a482 || exit 1
for v in "$@"; do
  echo "== $v =="
  ./step.sh "probe/$v.pdf"
done
