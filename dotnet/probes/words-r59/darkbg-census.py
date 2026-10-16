#!/usr/bin/env python3
"""How many corpus documents put text on a background LibreOffice calls dark.

    darkbg-census.py <manifest>

Resolves each `w:shd` the way `CellColorHandler::getProperties` does — `w:val` to a per-mille
weight, `w:color` (auto = black) blended over `w:fill` (auto = white) at it — and then asks
`Color::IsDark()`: `GetWCAGLuminance() <= 87`, except for `COL_DEFAULT_SHAPE_FILLING` `0x729FCF`,
which asks `GetLuminance() <= 62` instead and comes out *bright*.  Both branches are measured
rather than assumed: `probes/words-r59/autocolour.py`'s `A/729FCF-the-discriminator` draws black
text on 26.2.4.2 and `A/6F9BCB-just-below` draws white, and no other input in the domain separates
the two functions.

Printed as three arms, because they are three code paths and a document can be in more than one:

  A. a dark background we **already** paint, whose text we draw black and should draw white.
  B. a dark background we paint **nothing** for today, because its `w:val` is a pattern and we
     read only its `w:fill` — so this arm needs the pattern fix before the colour fix can see it.
  C. a *bright* background, which is the control: nothing about it may move.

What it cannot see:
  * the 66 `.doc` documents, whose shading descriptors are binary.
  * whether the shaded cell holds any text, and whether that text states a colour of its own — a
    run with `w:color` is not automatic and never moves. So every arm is an upper bound.
  * table-style conditional shading, which can put a fill on a cell that states none: an
    **under**-count, and the only one here that points that way.
"""
import csv, os, re, sys, zipfile, collections

WEIGHT = {'clear': 0, 'nil': 0, 'solid': 1000}
for pct in (5, 10, 12, 15, 20, 25, 30, 35, 37, 40, 45, 50, 55, 60, 62, 65, 70, 75, 80, 85, 87,
            90, 95):
    WEIGHT['pct%d' % pct] = {12: 125, 15: 150, 37: 375, 62: 625, 87: 875}.get(pct, pct * 10)


def weight(val):
    if val in WEIGHT:
        return WEIGHT[val]
    return 333            # every striped and crossed value


def normalise(v):
    v /= 255.0
    return v / 12.92 if v < 0.04045 else ((v + 0.055) / 1.055) ** 2.4


def wcag(rgb):
    r, g, b = (rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255
    return int((normalise(r) * 0.2126 + normalise(g) * 0.7152 + normalise(b) * 0.0722) * 255)


def perceived(rgb):
    r, g, b = (rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255
    return (b * 29 + g * 151 + r * 76) >> 8


def is_dark(rgb):
    return perceived(rgb) <= 62 if rgb == 0x729FCF else wcag(rgb) <= 87


def resolved(val, colour, fill):
    """The colour the cell ends up, or None when it ends up unfilled."""
    w = weight(val)
    back = 0xFFFFFF if fill in (None, 'auto') else fill
    fore = 0x000000 if colour in (None, 'auto') else colour
    if w == 0:
        return None if fill in (None, 'auto') else back
    out = 0
    for shift in (16, 8, 0):
        out |= ((((fore >> shift) & 255) * w + ((back >> shift) & 255) * (1000 - w)) // 1000) << shift
    return out


SHD = re.compile(r'<w:shd\b[^>]*?/?>')
ATTR = re.compile(r'w:(val|color|fill)="([^"]*)"')


def rgb(text):
    return int(text, 16) if text and re.fullmatch(r'[0-9A-Fa-f]{6}', text) else None


if __name__ == '__main__':
    man = sys.argv[1]
    root = os.path.dirname(os.path.abspath(man))
    arms = collections.defaultdict(collections.Counter)
    notexamined = 0
    with open(man, newline='', encoding='utf-8') as f:
        for r in csv.DictReader(f, delimiter='\t'):
            if r['family'] != 'words':
                continue
            if r['ext'] != 'docx':
                notexamined += 1
                continue
            try:
                z = zipfile.ZipFile(os.path.join(root, r['path']))
            except Exception:
                continue
            for part in ('word/document.xml', 'word/styles.xml'):
                try:
                    body = z.read(part).decode('utf-8', 'replace')
                except KeyError:
                    continue
                for element in SHD.findall(body):
                    a = dict(ATTR.findall(element))
                    val = a.get('val', 'clear')
                    now = rgb(a.get('fill')) if val != 'nil' else None
                    then = resolved(val, rgb(a.get('color')), rgb(a.get('fill')))
                    if then is None:
                        continue
                    if not is_dark(then):
                        arms['C bright'][r['path']] += 1
                    elif now is not None:
                        arms['A dark, already painted'][r['path']] += 1
                    else:
                        arms['B dark, painted only after the pattern fix'][r['path']] += 1

    print('docx examined; .doc not examined: %d\n' % notexamined)
    for arm in sorted(arms):
        by = arms[arm]
        print('%-44s %6d elements in %3d documents' % (arm, sum(by.values()), len(by)))
    print()
    for arm in sorted(arms):
        if arm.startswith('C'):
            continue
        print('=== %s' % arm)
        for path, n in arms[arm].most_common(20):
            print('   %5d  %s' % (n, path))
