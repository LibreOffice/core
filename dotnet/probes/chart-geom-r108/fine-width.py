#!/usr/bin/env python3
"""Does 26.2.4.2 turn an axis for every single-word label wider than its wrap limit?

`width-ladder.py` locates the flip to one character; this samples the 2.7 pt window between
the wrap limit (0.95 x the tick spacing, 51.49 pt on `038_Competitive_Advantage_Card`'s
54.20 pt pitch) and the pitch itself, where the two candidate collision rules disagree: a box
that is the label's own text does not overlap its neighbour anywhere in that window, and one
that is anything wider does.

Fifteen words of `n` with a narrow tail are enough to sample it at about 0.2 pt.  Each word's
drawn advance is computed from the installed Carlito through `chart2`'s own 96 dpi pixel-em
rounding -- `round(size x 96/72) / (size x 96/72)`, `MetricGrid.Chart.PixelEmScale` -- which
agrees with this tree's own chart measurer to 0.01 pt on every letter run in
`width-ladder-oneword.tsv` and with the reference's drawn ink to 0.05 pt.

    fine-width.py <dir>          write the variants
    fine-width.py --classify <dir>
"""
import os
import sys
import zipfile

import pymupdf

SRC = ("/home/user/sample-files/slides/chartset-008/pptx/"
       "038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx")
CATS = ["Product Quality", "Innovation", "Brand Reputation", "Cost Efficiency",
        "Customer Service"]
FACE = "/usr/share/fonts/truetype/crosextra/Carlito-Regular.ttf"
SIZE = 11.0

# The axis of `038` at its own size: five categories over a 54.20 pt pitch, so the wrap limit
# is 51.49 and the pitch is what a collision against the bare text would need.
PITCH = 54.202
LIMIT = PITCH * 0.95

WORDS = ["nnnnnnn" + t for t in ("iiii", "iiij", "iiil", "iiit", "iiif", "iiir", "iiis")] + \
        ["nnnnnnnn" + t for t in ("ii", "ij", "il", "it", "if", "ir", "is")] + ["nnnnnnnnn"]


def advance(text):
    """The drawn advance of one word on chart2's 96 dpi device."""
    font = pymupdf.Font(fontfile=FACE)
    exact = SIZE * 96.0 / 72.0
    return font.text_length(text, fontsize=SIZE) * (round(exact) / exact)


def write(into):
    os.makedirs(into, exist_ok=True)
    for word in WORDS:
        with zipfile.ZipFile(SRC) as zin, \
             zipfile.ZipFile(os.path.join(into, word + ".pptx"), "w",
                             zipfile.ZIP_DEFLATED) as out:
            for item in zin.infolist():
                data = zin.read(item.filename)
                if item.filename == "ppt/charts/chart1.xml":
                    text = data.decode("utf-8")
                    for category in CATS:
                        text = text.replace("<c:v>%s</c:v>" % category,
                                            "<c:v>%s</c:v>" % word)
                    data = text.encode("utf-8")
                out.writestr(item, data)
    print("wrote", len(WORDS), "variants to", into)


def arrangement(path):
    with pymupdf.open(path) as doc:
        page = doc[0]
        turned = sum(
            1 for b in page.get_text("dict")["blocks"] if b["type"] == 0
            for l in b["lines"] if abs(l["dir"][0] - 0.70711) < 0.004)
        fills = sum(
            1 for d in page.get_drawings()
            if d["fill"] is not None
            and 0.3 < d["rect"].width < 15 and 0.3 < d["rect"].height < 15)
    return "turned" if (turned or fills >= 30) else "upright"


def classify(root):
    print("word\tadvance\tover_limit\tover_pitch\treference\tours")
    for word in sorted(WORDS, key=advance):
        w = advance(word)
        sides = []
        for side in ("ref", "ours"):
            pdf = os.path.join(root, side, word + ".pdf")
            sides.append(arrangement(pdf) if os.path.exists(pdf) else "-")
        print(f"{word}\t{w:.3f}\t{'yes' if w > LIMIT else 'no'}"
              f"\t{'yes' if w > PITCH else 'no'}\t{sides[0]}\t{sides[1]}")


if __name__ == "__main__":
    if sys.argv[1] == "--classify":
        classify(sys.argv[2])
    else:
        write(sys.argv[1])
