#!/usr/bin/env python3
"""Where the automatic-colour flip sits as a shape fill's transparency rises, on three colours.

`alphaauto.py` shows the fill's transparency decides. This pins the *formula*:
`SdrAllFillAttributesHelper::getAverageColor` interpolates the fill toward
`aGlobalRetoucheColor` — white — by the transparency, and `Color::IsDark`'s WCAG rule is then
asked of the blend. That predicts a **different** flip transparency for every fill colour, with
no free parameter.

**And the first cut of this file got its own arithmetic wrong, which the arms caught.** It
bisected on the *continuous* luminance ≤ 87 and predicted flips at 8.796 / 36.882 / 61.900 %.
`Color::GetWCAGLuminance` returns a `sal_uInt8`, so the comparison is against the **truncated**
value: the flip is where the blend first reaches 88.0, not 87.0. That moves the three predictions
to

    #8496B0  →  9.571 %      #0070C0  →  37.454 %      #000000  →  62.222 %

and the arm that missed — `#8496B0` at 9.4 %, predicted black, measured white — sits between the
two readings and separates them on its own. The truncating model gets 6 of 6; the continuous one
gets 5. Both sets of arms are kept below, the loose pair as the record and a tight pair straddling
each corrected flip by 0.4–0.8 points.

A constant threshold, a threshold that ignores the fill colour, and no blend at all are all
refuted by the same renderings, because the three colours flip at three different transparencies.

    threshold.py <outdir>
"""
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from alphaauto import DOCS, author, rect_fill, render  # noqa: E402

# (fill, predicted flip transparency %, [(transparency %, predicted colour), …])
CASES = [
    ('#8496b0', 9.571, [(8.4, 'WHITE'), (9.4, 'WHITE'), (9.2, 'WHITE'), (10.0, 'black')]),
    ('#0070c0', 37.454, [(36.2, 'WHITE'), (37.8, 'black'), (37.0, 'WHITE'), (37.9, 'black')]),
    ('black', 62.222, [(61.0, 'WHITE'), (63.0, 'black'), (62.0, 'WHITE')]),
]


def shows(pdf, page):
    here = os.path.dirname(os.path.abspath(__file__))
    out = subprocess.run([sys.executable, os.path.join(here, 'textcolour.py'), pdf, str(page)],
                         capture_output=True, text=True).stdout
    head = out.split('by y')[0]
    return {c.upper(): int(n) for c, n in re.findall(r'#([0-9A-Fa-f]{6})\s+(\d+)', head)}


def main(outdir):
    os.makedirs(outdir, exist_ok=True)
    profile = os.path.join(outdir, 'prof')
    ok = 0
    total = 0
    for fill, flip, arms in CASES:
        for transparency, predicted in arms:
            opacity = round((100.0 - transparency) / 100.0 * 65536)
            name = 'thr-%s-%s' % (fill.strip('#'), str(transparency).replace('.', 'p'))
            docx = os.path.join(outdir, name + '.docx')
            author(DOCS['069'], docx,
                   lambda x, f=fill, o=opacity: rect_fill(x, (f, '<v:fill opacity="%df"/>' % o)))
            render(docx, outdir, profile)
            got = shows(os.path.join(outdir, name + '.pdf'), 1)
            white = got.get('FFFFFF', 0)
            answer = 'WHITE' if white else 'black'
            total += 1
            ok += answer == predicted
            print('%-24s fill %-8s transparency %5.1f%% (flip %.3f%%)  predicted %-5s  got %-5s  %s'
                  % (name, fill, transparency, flip, predicted, answer,
                     'ok' if answer == predicted else 'MISS'))
    print('\n%d of %d arms land where the blend predicts' % (ok, total))


if __name__ == '__main__':
    main(sys.argv[1])
