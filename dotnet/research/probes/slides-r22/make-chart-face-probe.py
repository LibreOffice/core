#!/usr/bin/env python3
"""Three decks that separate the three readings of "what face is a chart's text in".

    make-chart-face-probe.py <outdir>

`SlideChart` set every chart label in Liberation Sans for four rounds, on the evidence that
`pdffonts` reported Liberation Sans in LibreOffice's own PDF of `chart-bar-deck.pptx`. That
deck's chart states `<a:latin typeface="Arial"/>` eleven times and fontconfig substitutes Arial
with Liberation Sans, so the measurement is equally consistent with a *fixed* face and with the
*stated* one. The corpus cannot separate them either without also changing the theme, the sizes
and the data at the same time.

So: one deck, three variants, one thing different between them.

    A   the chart states no `a:latin` at all
    B   the chart states `+mn-lt`
    C   the chart states a literal face that is neither the theme's nor the default

with the theme's minor Latin face set to a *monospaced* family in all three, because a
monospace is the widest possible separation from either proportional candidate and shows up in
the plot area's left edge as well as in the embedded font list.

Predicted, if the face is the theme's minor Latin with a stated `a:latin` overriding it:

    A  →  the theme's face      B  →  the theme's face      C  →  the stated face

Predicted, if the face is fixed: all three come back the same, and it is the fixed one.
Predicted, if only a stated face is read: A and B fall back and C does not.
"""
import os
import re
import shutil
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.join(HERE, '..', '..', '..', 'tests', 'corpus', 'features',
                      'chart-bar-deck.pptx')

THEME_FACE = 'Liberation Mono'
LITERAL_FACE = 'Liberation Serif'

VARIANTS = {
    'chart-face-unstated': None,
    'chart-face-theme': '+mn-lt',
    'chart-face-literal': LITERAL_FACE,
}


def retheme(data):
    """Set both theme font faces to the monospace, so the minor one is unambiguous."""
    text = data.decode('utf-8')
    text = re.sub(r'(<a:minorFont><a:latin typeface=")[^"]*(")',
                  rf'\g<1>{THEME_FACE}\g<2>', text)
    text = re.sub(r'(<a:majorFont><a:latin typeface=")[^"]*(")',
                  rf'\g<1>{THEME_FACE}\g<2>', text)
    return text.encode('utf-8')


def rechart(data, face):
    """Rewrite every a:latin in the chart part, or drop them all when face is None."""
    text = data.decode('utf-8')
    if face is None:
        return re.sub(r'<a:latin typeface="[^"]*"\s*/>', '', text).encode('utf-8')
    return re.sub(r'(<a:latin typeface=")[^"]*(")', rf'\g<1>{face}\g<2>',
                  text).encode('utf-8')


def main():
    if len(sys.argv) != 2:
        print(__doc__, file=sys.stderr)
        return 2

    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)

    source = os.path.abspath(SOURCE)
    if not os.path.exists(source):
        print(f'no source deck at {source}', file=sys.stderr)
        return 2

    for name, face in VARIANTS.items():
        target = os.path.join(out, f'{name}.pptx')
        shutil.copyfile(source, target + '.tmp')

        with zipfile.ZipFile(target + '.tmp') as src, \
                zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as dst:
            for item in src.infolist():
                data = src.read(item.filename)
                if re.match(r'ppt/theme/theme\d+\.xml$', item.filename):
                    data = retheme(data)
                elif re.match(r'ppt/charts/chart\d+\.xml$', item.filename):
                    data = rechart(data, face)
                dst.writestr(item, data)

        os.remove(target + '.tmp')
        print(f'{target}  theme minor={THEME_FACE}  chart a:latin={face!r}')

    return 0


if __name__ == '__main__':
    sys.exit(main())
