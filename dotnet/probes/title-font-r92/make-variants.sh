#!/usr/bin/env bash
# Build one-attribute variants of the deck by rewriting slideMaster2.xml's
# title <a:latin>.  Everything else is byte-identical to the original.
set -euo pipefail
SRC=${1:?source pptx}
DST=${2:?variant directory}
mkdir -p "$DST"
work=$(mktemp -d)
unzip -q "$SRC" -d "$work"
orig='typeface="Helvetica" panose="020B0500000000000000" pitchFamily="34" charset="0"'
mk() {                       # mk <name> <replacement attribute text>
    local name=$1 repl=$2
    rm -rf "$work.v"; cp -a "$work" "$work.v"
    python3 - "$work.v" "$orig" "$repl" <<'PY'
import sys, glob
root, a, b = sys.argv[1], sys.argv[2], sys.argv[3]
n = 0
for p in glob.glob(root + "/ppt/**/*.xml", recursive=True):
    s = open(p, encoding="utf-8").read()
    if a in s:
        n += s.count(a)
        open(p, "w", encoding="utf-8").write(s.replace(a, b))
assert n > 0, "no occurrence rewritten"
print("   rewrote", n, "occurrence(s)", file=sys.stderr)
PY
    (cd "$work.v" && zip -q -r -X "$DST/$name.pptx" .)
    echo "$DST/$name.pptx"
}
cp "$SRC" "$DST/v0-control.pptx"; echo "$DST/v0-control.pptx"
mk v1-arial      'typeface="Arial" panose="020B0604020202020204" pitchFamily="34" charset="0"'
mk v2-nopanose   'typeface="Helvetica"'
mk v3-libsans    'typeface="Liberation Sans" pitchFamily="34" charset="0"'
mk v4-absent     'typeface="Zzyzx Nonexistent" panose="020B0500000000000000" pitchFamily="34" charset="0"'
mk v5-absent-np  'typeface="Zzyzx Nonexistent"'
mk v6-nopitch    'typeface="Helvetica" panose="020B0500000000000000"'
mk v7-helvneue   'typeface="Helvetica Neue" panose="020B0500000000000000" pitchFamily="34" charset="0"'
mk v8-roman      'typeface="Helvetica" panose="020B0500000000000000" pitchFamily="18" charset="0"'
mk v9-nocharset  'typeface="Helvetica" panose="020B0500000000000000" pitchFamily="34"'
mk v10-charsetonly 'typeface="Helvetica" panose="020B0500000000000000" charset="0"'
mk v11-fixed     'typeface="Helvetica" panose="020B0500000000000000" pitchFamily="49" charset="0"'
mk v12-famnone   'typeface="Helvetica" panose="020B0500000000000000" pitchFamily="2" charset="0"'
mk v13-nopanose-pf 'typeface="Helvetica" pitchFamily="34" charset="0"'
rm -rf "$work" "$work.v"
