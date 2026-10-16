#!/usr/bin/env python3
"""Decks whose category labels are two words, one word, or short, at a swept word length.

The question is what a category axis does with a label that is wider than its slot but whose
*words* are not.  chart2 lays each label out inside `TextMaximumFrameWidth` -- 0.95 of the tick
spacing (`VCartesianAxis.cxx:753-759`) -- so it breaks at the blank and comes out at most one
word wide; `lcl_hasWordBreak` (`:369-404`) only turns line breaking off where a line starts in
the middle of a word, which a break at a blank never does.  Every label is therefore drawn.

Three arms separate that from the two things it can be confused with:

  A  two words of n characters each, so the label is over the limit and each word is under it;
  B  one word of 2n characters, the same total width with no break opportunity at all;
  C  one word of n characters, which fits and must not move.

The reference's answer is read out of its own PDF text layer: how many of the eight distinct
labels are drawn.  A draws eight upright on two lines, B turns 45 degrees, C draws eight on one
line.  A renderer that measures the unwrapped run instead finds a collision that is not there
and thins A out to every second label.
"""
import os, re, sys, zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.join(HERE, "..", "..", "tests", "corpus", "features",
                      "chart-face-theme-minor.pptx")

MAIN_TITLE = re.compile(r"<c:title>.*?</c:title>\s*<c:autoTitleDeleted val=\"0\"/>", re.S)
LEGEND = re.compile(r"<c:legend>.*?</c:legend>", re.S)
BASE = [120, 95, 143, 168, 88, 132, 101, 121]
WIDTHS = (4, 6, 8, 10, 12, 14, 16)


def labels(arm, n):
    """Eight labels, each carrying its own index so the drawn ones can be counted."""
    if arm == "A":
        return [f"Kat{i}{'x' * n} Tal{i}{'y' * n}" for i in range(8)]
    if arm == "B":
        return [f"Kat{i}{'x' * n}Tal{i}{'y' * n}" for i in range(8)]
    return [f"Kat{i}{'x' * n}" for i in range(8)]


def chart(text, cats):
    text = MAIN_TITLE.sub('<c:autoTitleDeleted val="1"/>', text, count=1)
    text = LEGEND.sub("", text, count=1)
    count = len(cats)
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
    return text


def main(out):
    os.makedirs(out, exist_ok=True)
    n = 0
    for arm in ("A", "B", "C"):
        for width in WIDTHS:
            name = f"wrap{arm}-n{width:02d}"
            with zipfile.ZipFile(SOURCE) as src, \
                    zipfile.ZipFile(os.path.join(out, name + ".pptx"), "w",
                                    zipfile.ZIP_DEFLATED) as dst:
                for item in src.infolist():
                    data = src.read(item.filename)
                    if re.match(r"ppt/charts/chart\d+\.xml$", item.filename):
                        data = chart(data.decode("utf-8"),
                                     labels(arm, width)).encode("utf-8")
                    elif item.filename == "ppt/theme/theme1.xml":
                        data = data.decode("utf-8").replace(
                            'typeface="Liberation Mono"', 'typeface="Arial"').encode("utf-8")
                    dst.writestr(item, data)
            n += 1
    print(n, "decks")


if __name__ == "__main__":
    main(sys.argv[1])
