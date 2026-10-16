#!/usr/bin/env bash
# Decide whether a corpus document's verdict moved because of *this* round.
#
# `sweep.py` scores against the gate at `2f4709c08`, and this round's base is `faadf7dda` — many
# commits later — so a moved verdict may belong to anything landed between them. This builds a
# binary with only this round's five files returned to their committed base contents, renders the
# named documents with it, and prints the gate columns both ways.
#
# The five files are copied aside and restored with `cp` followed by `touch`, never `mv`: a
# restored file with an older mtime than the assembly makes MSBuild skip the project and the
# "reverted" binary still carries the change. See dotnet/CLAUDE.md.
#
#     attribute.sh <out-dir> <corpus-relative-path>...
set -uo pipefail

ROOT=/home/user/wt-chartlayout/dotnet
CORPUS=/home/user/sample-files
CLI="$ROOT/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"

FILES=(
    src/XRay.Office.Core/Charts/ChartAxisLabels.cs
    src/XRay.Office.Core/Charts/ChartLayout.cs
    src/XRay.Office.Core/Charts/ChartPlot.cs
    src/XRay.Office.Ooxml/DrawingML/DrawingChartPlot.cs
    src/XRay.Office.Presentations/Layout/SlideChart.cs
)

# The round's own tests name types the base sources do not have, so they go aside with them or
# the base build fails before it can render anything.
TESTS=(
    tests/XRay.Office.Core.Tests/ChartVerticalAxisLabelTests.cs
    tests/XRay.Office.Presentations.Tests/DrawingChartManualLayoutTests.cs
)

out="${1:?usage: attribute.sh <out-dir> <path>...}"; shift
mkdir -p "$out/base"
cd "$ROOT" || exit 1

for f in "${FILES[@]}"; do cp "$f" "$f.round"; git checkout -- "$f"; touch "$f"; done
for t in "${TESTS[@]}"; do mv "$t" "$t.aside"; done
trap 'for f in "${FILES[@]}"; do cp "$f.round" "$f" && touch "$f" && rm -f "$f.round"; done
      for t in "${TESTS[@]}"; do [ -f "$t.aside" ] && cp "$t.aside" "$t" && touch "$t" && rm -f "$t.aside"; done' EXIT

dotnet build XRay.Office.slnx -v q -nologo >/dev/null 2>&1 || { echo "base build failed"; exit 1; }

for rel in "$@"; do
    src="$CORPUS/$rel"
    "$CLI" render "$src" --format pdf --outdir "$out/base" >/dev/null 2>&1
    stem="$(basename "${src%.*}")"
    pdf="$out/base/$stem.pdf"
    if [ -f "$pdf" ]; then
        pages=$(pdfinfo "$pdf" | awk '/^Pages:/{print $2}')
        glyphs=$(pdftotext "$pdf" - | tr -cd '[:alnum:]' | wc -c)
        echo -e "$rel\tbase\tpages=$pages\tglyphs=$glyphs"
    else
        echo -e "$rel\tbase\tRENDER-FAILED"
    fi
done
