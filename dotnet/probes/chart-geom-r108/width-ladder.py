#!/usr/bin/env python3
"""Where the plain word-width limit turns a chart axis, on the reference and on this tree.

O43 is the residue r91 left and r106 seated: 26.2.4.2 turns `038_Competitive_Advantage_Card`'s
axis for `Cost Screeched` and `Cost Squelched` and leaves it upright for `Cost Scratched`,
`Cost Stretched`, `Cost Strengths` and `Cost Straights` -- the same length, the same shape, no
hyphenation point in any of them -- and this tree leaves all six upright.  r106 concluded the
boundary is a width and did not locate it.

A word of one repeated letter locates it, because its width is exactly n glyph advances and
nothing about the word can matter but its width.  Three letters of very different advance are
swept so the boundary can be read in points rather than in characters: if the two renderers
flip at the same *width* the seat is a measurement, and if they flip at different widths it is
the limit.

    width-ladder.py write <dir>
    width-ladder.py classify <dir>
"""
import glob
import os
import subprocess
import sys
import zipfile

SRC = ("/home/user/sample-files/slides/chartset-008/pptx/"
       "038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx")

CATS = ["Product Quality", "Innovation", "Brand Reputation", "Cost Efficiency",
        "Customer Service"]

LETTERS = ("i", "n", "W")
LENGTHS = range(2, 19)

# `Cost ` in front of the run, or the run alone.  With the prefix the label is two words and
# the fill breaks at the blank; alone it can only break inside the word, which is
# `lcl_hasWordBreak`'s own condition with nothing else in the way.
PREFIX = os.environ.get("LADDER_PREFIX", "Cost ")


def words():
    return [letter * n for letter in LETTERS for n in LENGTHS]


def write(into):
    os.makedirs(into, exist_ok=True)
    for word in words():
        with zipfile.ZipFile(SRC) as zin, \
             zipfile.ZipFile(os.path.join(into, word + ".pptx"), "w",
                             zipfile.ZIP_DEFLATED) as out:
            for item in zin.infolist():
                data = zin.read(item.filename)
                if item.filename == "ppt/charts/chart1.xml":
                    text = data.decode("utf-8")
                    for category in CATS:
                        text = text.replace(
                            "<c:v>%s</c:v>" % category, "<c:v>%s%s</c:v>" % (PREFIX, word))
                    data = text.encode("utf-8")
                out.writestr(item, data)
    print("wrote", len(words()), "variants to", into)


def arrangement(path):
    """`turned` or `upright` -- word-sweep.py's classifier, with its three traps."""
    import pymupdf

    with pymupdf.open(path) as doc:
        page = doc[0]
        turned = 0
        for block in page.get_text("dict")["blocks"]:
            if block["type"] != 0:
                continue
            for line in block["lines"]:
                dx, dy = line["dir"]
                if abs(dx - 0.70711) < 0.004 and abs(dy + 0.70711) < 0.004:
                    turned += 1
        fills = sum(
            1 for d in page.get_drawings()
            if d["fill"] is not None
            and 0.3 < d["rect"].width < 15 and 0.3 < d["rect"].height < 15)

    if turned:
        return "turned"
    if fills >= 30:
        return "turned"
    return "upright"


def classify(root):
    print("word\tletter\tn\treference\tours")
    for word in words():
        ref = os.path.join(root, "ref", word + ".pdf")
        ours = glob.glob(os.path.join(root, "ours", word + ".pdf"))
        if not os.path.exists(ref) or not ours:
            print(f"{word}\t{word[0]}\t{len(word)}\t-\t-")
            continue
        print(f"{word}\t{word[0]}\t{len(word)}\t{arrangement(ref)}\t{arrangement(ours[0])}")


if __name__ == "__main__":
    if sys.argv[1] == "write":
        write(sys.argv[2])
    else:
        classify(sys.argv[2])
