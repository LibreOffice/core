#!/usr/bin/env python3
"""Decks that separate the terms of a chart legend's reserved width and row pitch.

    make-legend-probe.py <outdir>

`ChartLayout.Legend` measures a legend entry's name as the *text shape's* size — the text
plus `TextShapeInsetX` twice across and `TextShapeInsetY` twice down — because
`ShapeFactory::createText` sets `TextLeftDistance`/`TextUpperDistance` to 0.18 and 0.30 of
the font height, and every other chart text goes through it.

Reading `VLegend.cxx:180` says the legend's entries do *not*: `lcl_createTextShapes` calls
the plain-`OUString` overload (`ShapeFactory.cxx:2042`), which sets no distances at all. The
overload that sets them (`:2168`) takes a size, a position and an `XFormattedString`.

That is a claim about the running binary, so measure it. The theme's minor Latin face is
Liberation Mono in every variant, so a name of n characters is exactly n advances wide and
the width is linear in n with no shaping to argue about:

    width(n) = 2·padding + key + gap + n·advance + C

with C = 0 if the shape carries no inset and C = 0.36·font if it does. Varying n gives the
advance the reference actually uses, and the intercept gives C. Varying the *font size* at
fixed n separates a font-proportional C from a constant one.

The same variants read the row pitch, which is `rowHeight + yOffset` in
`lcl_placeLegendEntries` — `rowHeight` being the same shape's height, so the 0.30 inset is
in question there too, and 0.60·font is four times the quantity the axis labels moved by.
"""
import os
import re
import shutil
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = '/home/user/libreoffice-core/.claude/worktrees/slides-r23/dotnet/tests/corpus/features/chart-face-theme-minor.pptx'

# (name length, legend font in hundredths of a point)
VARIANTS = [(2, 1000), (5, 1000), (10, 1000), (16, 1000),
            (5, 1400), (10, 1400), (5, 700)]


def rechart(data, chars, size):
    text = data.decode('utf-8')
    # The two series names are the legend's two entries.
    text = text.replace('<c:v>North</c:v>', f'<c:v>{"M" * chars}</c:v>')
    text = text.replace('<c:v>South</c:v>', f'<c:v>{"N" * chars}</c:v>')
    # Only the legend's own font size moves; the axes keep theirs so the plot rectangle's
    # other terms stay put between variants.
    legend = text[text.index('<c:legend>'):text.index('</c:legend>')]
    text = text.replace(legend, legend.replace('sz="1000"', f'sz="{size}"'))
    return text.encode('utf-8')


def main():
    if len(sys.argv) != 2:
        print(__doc__, file=sys.stderr)
        return 2

    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)

    for chars, size in VARIANTS:
        name = f'legend-n{chars:02d}-s{size}'
        target = os.path.join(out, f'{name}.pptx')
        shutil.copyfile(SOURCE, target + '.tmp')

        with zipfile.ZipFile(target + '.tmp') as src, \
                zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as dst:
            for item in src.infolist():
                data = src.read(item.filename)
                if re.match(r'ppt/charts/chart\d+\.xml$', item.filename):
                    data = rechart(data, chars, size)
                dst.writestr(item, data)

        os.remove(target + '.tmp')
        print(f'{target}  entries={chars} chars  legend font={size / 100:.0f}pt')

    return 0


if __name__ == '__main__':
    sys.exit(main())
