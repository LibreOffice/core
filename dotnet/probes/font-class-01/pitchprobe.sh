#!/usr/bin/env bash
# Does `pitchFamily` on <a:latin> decide which face LibreOffice draws?
#
#   pitchprobe.sh <deck.pptx> <part> <typeface> <pitchFamily-value> [outdir]
#
# One attribute, changed and not changed, on an otherwise byte-identical re-zip of a real deck. The
# control matters: re-packing a corpus file is itself a change, and without rendering the unmodified
# re-zip there is no way to know whether the attribute or the packing moved the answer.
#
# Run for this round as:
#   pitchprobe.sh .../airbus-powerpoint-presentation-2019-20-without-video_diy_2019-20.pptx \
#       ppt/slides/slide12.xml "Lucida Console" 49
#
#   control (pitchFamily="49" kept):  … DejaVuSans DejaVuSansMono LiberationSans …
#   removed:                          … DejaVuSans               LiberationSans …
#
# So the declared fixed pitch is the whole of it: with it, Lucida Console is DejaVu Sans Mono; with
# it gone, it is DejaVu Sans, which is fontconfig's answer for a family it files under no generic.
#
# `unzip` rather than python's zipfile because real corpus decks fail its CRC check — this one's
# [Content_Types].xml does — and the probe is about the deck as found.
set -euo pipefail

DECK="${1:?usage: pitchprobe.sh <deck.pptx> <part> <typeface> <value> [outdir]}"
PART="${2:?}"
TYPEFACE="${3:?}"
VALUE="${4:?}"
OUT="${5:-$(mktemp -d)}"

mkdir -p "$OUT/ctrl" "$OUT/test"
(cd "$OUT/ctrl" && unzip -oq "$DECK" 2>/dev/null || true)
(cd "$OUT/test" && unzip -oq "$DECK" 2>/dev/null || true)

sed -i "s/\(typeface=\"$TYPEFACE\"[^\/>]*\) pitchFamily=\"$VALUE\"/\1/g" "$OUT/test/$PART"

python3 - "$OUT" <<'PY'
import os, sys, zipfile
out = sys.argv[1]
for name in ('ctrl', 'test'):
    root = os.path.join(out, name)
    with zipfile.ZipFile(os.path.join(out, name + '.pptx'), 'w', zipfile.ZIP_DEFLATED) as z:
        for base, _, files in os.walk(root):
            for f in files:
                p = os.path.join(base, f)
                z.write(p, os.path.relpath(p, root))
PY

export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-1700000000}"
soffice --headless --convert-to pdf --outdir "$OUT" "$OUT/ctrl.pptx" >/dev/null 2>&1
soffice --headless --convert-to pdf --outdir "$OUT" "$OUT/test.pptx" >/dev/null 2>&1

faces() { pdffonts "$1" 2>/dev/null | tail -n +3 | awk 'NF{print $1}' | sed -E 's/^[A-Z]{6}\+//' | sort -u; }

echo "== control: pitchFamily=\"$VALUE\" kept, re-zipped =="; faces "$OUT/ctrl.pdf"
echo "== test: the attribute removed =="; faces "$OUT/test.pdf"
echo "(working directory $OUT)"
