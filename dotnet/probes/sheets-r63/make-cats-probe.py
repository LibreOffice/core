#!/usr/bin/env python3
"""`023_Waterfall`'s own category list, in a deck, with the tick spacing swept through its own.

The single-word sweeps put the limit at 0.998 of the tick spacing and the reference's advance
for `Column` at 10 pt in Liberation Sans within 0.06% of `design x PixelEmScale`.  Both agree
that `023`'s category axis — spacing 34.00, widest word `Column` at 33.60 — should stay
upright, and the reference plainly turns it.  Something about that axis is not the width of its
widest word.

Three series separate the candidates, and they differ in one label:

  A  the twelve categories exactly as the workbook states them, `Middle Column` among them;
  B  the same with `Middle Column` written as one word, `MiddleColumn`;
  C  the same with that label shortened to `Middle`, whose width no candidate rule can reach.

If A turns where B and C do not, the trigger is the *two-word* label and not the widest word.
If all three turn together, the trigger is the widest word and the limit is not 0.998.
If C turns too, it is neither, and the axis is being turned by something this file has not
modelled at all.
"""
import os, re, sys, zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.join(HERE, "..", "..", "tests", "corpus", "features",
                      "chart-face-theme-minor.pptx")
FRAME = re.compile(r'(<p:xfrm><a:off x="\d+" y="\d+"/><a:ext cx=")(\d+)(")')
BASE = [120, 95, 143, 168, 88, 132, 101, 121]

MAIN_TITLE = re.compile(r"<c:title>.*?</c:title>\s*<c:autoTitleDeleted val=\"0\"/>", re.S)
CAT_TITLE = re.compile(r"(<c:catAx>.*?)<c:title>.*?</c:title>(.*?</c:catAx>)", re.S)
LEGEND = re.compile(r"<c:legend>.*?</c:legend>", re.S)

WATERFALL = ["START"] + [f"Delta {i}" for i in (1, 2)] + ["Middle Column"] + \
            [f"Delta {i}" for i in range(3, 10)] + ["END"]

SERIES = {
    "A": WATERFALL,
    "B": [c.replace("Middle Column", "MiddleColumn") for c in WATERFALL],
    "C": [c.replace("Middle Column", "Middle") for c in WATERFALL],
}

# Series A turns at a tick spacing of 35.41 where its widest word `Column` is 33.60 wide, so
# the trigger is not the widest word on its own.  Four arms, each of which comes out
# differently under the rival readings: two vary the *first* word of the two-word label and
# two vary the *second*.  If the boundary follows the second word alone, the rule is a room
# allowance next to it; if it follows the first, the rule is about the line the break leaves
# behind; if it follows the whole string, it is neither.
for key, label in (("D", "Mi Column"), ("E", "MiddleMiddleMi Column"),
                   ("F", "Middle Colum"), ("G", "Middle Columnn")):
    SERIES[key] = [c.replace("Middle Column", label) for c in WATERFALL]

# The same two-word boundary at 11 pt, where the pixel em rounds *up* (1.0227) instead of down
# (0.9750).  One constant times the tick spacing puts both sizes on the same number; the ruler
# the constant is applied to is what the two sizes separate.
SERIES["H"] = WATERFALL
SIZES = {"H": 1100}


def chart(text, cats, size):
    text = MAIN_TITLE.sub('<c:autoTitleDeleted val="1"/>', text, count=1)
    text = CAT_TITLE.sub(r"\1\2", text, count=1)
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
    head, sep, tail = text.partition("<c:catAx>")
    return head + sep + tail.replace('sz="1000"', f'sz="{size}"', 1)


def main(out):
    os.makedirs(out, exist_ok=True)
    n = 0
    for key, cats in SERIES.items():
        for cx in (range(6300000, 7520001, 25000) if key == "H"
                   else range(5860000, 6480001, 20000) if key in "ABC"
                   else range(5560000, 6800001, 40000)):
            name = f"cats{key}-w{cx}"
            with zipfile.ZipFile(SOURCE) as src, \
                    zipfile.ZipFile(os.path.join(out, name + ".pptx"), "w",
                                    zipfile.ZIP_DEFLATED) as dst:
                for item in src.infolist():
                    data = src.read(item.filename)
                    if re.match(r"ppt/charts/chart\d+\.xml$", item.filename):
                        data = chart(data.decode("utf-8"), cats,
                                     SIZES.get(key, 1000)).encode("utf-8")
                    elif item.filename == "ppt/theme/theme1.xml":
                        data = data.decode("utf-8").replace(
                            'typeface="Liberation Mono"', 'typeface="Arial"').encode("utf-8")
                    elif item.filename == "ppt/slides/slide1.xml":
                        text, k = FRAME.subn(lambda m: m.group(1) + str(cx) + m.group(3),
                                             data.decode("utf-8"), 1)
                        if k != 1:
                            raise SystemExit("frame extent not found")
                        data = text.encode("utf-8")
                    dst.writestr(item, data)
            n += 1
    print(n, "decks")


if __name__ == "__main__":
    main(sys.argv[1])
