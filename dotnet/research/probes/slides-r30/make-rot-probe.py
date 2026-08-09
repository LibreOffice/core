#!/usr/bin/env python3
"""Decks that separate the terms of a *rotated* category label's reserved depth.

The upright band is settled — round twenty-nine's probes agree with LibreOffice's own
`chart:coordinate-region` to a tenth of a point on every upright variant. What is left is
the 45 degree case, where our reserve is 7.94 pt too deep on a 26-category axis.

Our depth is `W_shape*sin + H_shape*cos` with the shape's insets in both terms
(ShapeFactory's 0.18 em each side across, 0.30 em each side down). Whether LibreOffice's
is the same quantity cannot be told from one probe, because one probe fixes W and H
together. So: hold the label height and vary its *width* by the character count, then hold
the width and vary the *size*. A term that is proportional to the count separates from one
proportional to the em, and both separate from a constant.

Carlito is not monospaced, so the carrier's Liberation Mono is what makes a character count
a width. 26 categories on a 545 pt axis give a 21 pt slot, which any label of four
characters or more overruns — so every variant here rotates.
"""
import os, re, sys, zipfile

SOURCE = ("/home/user/libreoffice-core/.claude/worktrees/slides-r30/"
          "dotnet/tests/corpus/features/chart-face-theme-minor.pptx")

MAIN_TITLE = re.compile(r"<c:title>.*?</c:title>\s*<c:autoTitleDeleted val=\"0\"/>", re.S)
CAT_TITLE = re.compile(r"(<c:catAx>.*?)<c:title>.*?</c:title>(.*?</c:catAx>)", re.S)
LEGEND = re.compile(r"<c:legend>.*?</c:legend>", re.S)
BASE = [120, 95, 143, 168, 88, 132, 101, 121]


def variant(text, *, chars, count, size):
    text = MAIN_TITLE.sub('<c:autoTitleDeleted val="1"/>', text, count=1)
    text = CAT_TITLE.sub(r"\1\2", text, count=1)
    text = LEGEND.sub("", text, count=1)

    cats = [("W" * chars) for _ in range(count)]
    block = re.search(r"<c:cat>.*?</c:cat>", text, re.S).group(0)
    pts = "".join(f'<c:pt idx="{i}"><c:v>{c}</c:v></c:pt>' for i, c in enumerate(cats))
    text = text.replace(block, '<c:cat><c:strRef><c:f>categories</c:f><c:strCache>'
                               f'<c:ptCount val="{count}"/>{pts}</c:strCache></c:strRef></c:cat>')
    for f in ("0", "1"):
        vals = re.search(r"<c:val><c:numRef><c:f>" + f + r"</c:f>.*?</c:val>", text, re.S)
        pts = "".join(f'<c:pt idx="{i}"><c:v>{BASE[i % 8]}</c:v></c:pt>' for i in range(count))
        text = text.replace(vals.group(0),
                            f'<c:val><c:numRef><c:f>{f}</c:f><c:numCache>'
                            f'<c:formatCode>General</c:formatCode>'
                            f'<c:ptCount val="{count}"/>{pts}</c:numCache></c:numRef></c:val>')
    if size is not None:
        head, sep, tail = text.partition("<c:catAx>")
        text = head + sep + tail.replace('sz="1000"', f'sz="{size}"', 1)
    return text


VARIANTS = {}
for k in (4, 6, 8, 10, 12):
    VARIANTS[f"rot-w{k:02d}"] = dict(chars=k, count=26, size=None)
for s in (700, 900, 1200, 1400):
    VARIANTS[f"rot-s{s}"] = dict(chars=6, count=26, size=s)
for n in (14, 20, 32):
    VARIANTS[f"rot-n{n:02d}"] = dict(chars=6, count=n, size=None)


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    for name, kw in VARIANTS.items():
        target = os.path.join(out, f"{name}.pptx")
        with zipfile.ZipFile(SOURCE) as src, \
                zipfile.ZipFile(target, "w", zipfile.ZIP_DEFLATED) as dst:
            for item in src.infolist():
                data = src.read(item.filename)
                if re.match(r"ppt/charts/chart\d+\.xml$", item.filename):
                    data = variant(data.decode("utf-8"), **kw).encode("utf-8")
                dst.writestr(item, data)
        print(target)


if __name__ == "__main__":
    sys.exit(main())
