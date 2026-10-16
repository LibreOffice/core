#!/usr/bin/env python3
"""The same decks, with the boundary crossed by the *frame width* instead of the count.

A category count is an integer, so the count sweep can only bracket the limit to one category
— 0.03 of the tick spacing at fifteen categories, which is wide enough to leave two corpus
documents on opposite sides of it.  The frame width is continuous: shrinking the chart shrinks
the tick spacing by any amount at all, so the boundary can be located as finely as the decks
are spaced.

Two sizes, chosen because the pixel-em correction has **opposite sign** at them — 0.975 at
10 pt and 1.023 at 11 pt.  A limit that is a pure fraction of the tick spacing puts both series
on the same number; one that carries an em-proportional term does not.
"""
import os, re, sys, zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
SOURCE = os.path.join(HERE, "..", "..", "tests", "corpus", "features",
                      "chart-face-theme-minor.pptx")
EMU = 12700.0

sys.path.insert(0, HERE)
spec = os.path.join(HERE, "make-rot-probe.py")
ns = {"__file__": spec, "__name__": "rotprobe"}
exec(compile(open(spec).read(), spec, "exec"), ns)   # reuse `variant` verbatim
variant = ns["variant"]

FRAME = re.compile(r'(<p:xfrm><a:off x="\d+" y="\d+"/><a:ext cx=")(\d+)(")')

# (size, count, first frame cx in EMU, last, step)
SERIES = [
    (1000, 15, 7480000, 7960000, 15000),
    (1100, 13, 7560000, 8040000, 15000),
]

# The confirmation series, at two sizes the first pair cannot speak for: 7 pt, where the em
# rounds *down* to 9 px (0.9643), and 14 pt, where it rounds *up* to 19 px (1.0179).  A limit
# that is one constant times the tick spacing puts these boundaries at a spacing this file's
# own `prediction.md` names before they are rendered.
CONFIRM = [
    (700, 22, 7745000, 7850000, 3000),
    (1400, 11, 8150000, 8400000, 4000),
]


def main(out, confirm=False):
    os.makedirs(out, exist_ok=True)
    n = 0
    for size, count, lo, hi, step in (CONFIRM if confirm else SERIES):
        for cx in range(lo, hi + 1, step):
            name = f"fine-z{size}-n{count}-w{cx}"
            target = os.path.join(out, f"{name}.pptx")
            with zipfile.ZipFile(SOURCE) as src, \
                    zipfile.ZipFile(target, "w", zipfile.ZIP_DEFLATED) as dst:
                for item in src.infolist():
                    data = src.read(item.filename)
                    if re.match(r"ppt/charts/chart\d+\.xml$", item.filename):
                        data = variant(data.decode("utf-8"),
                                       chars=6, count=count, size=size).encode("utf-8")
                    elif item.filename == "ppt/slides/slide1.xml":
                        text = data.decode("utf-8")
                        text, k = FRAME.subn(lambda m: m.group(1) + str(cx) + m.group(3), text, 1)
                        if k != 1:
                            raise SystemExit("frame extent not found")
                        data = text.encode("utf-8")
                    dst.writestr(item, data)
            n += 1
    print(n, "decks")




# ---------------------------------------------------------------------------
# The face series.  Everything above is Liberation Mono, because a monospaced face makes a
# character count a width.  `023_Waterfall_Chart_Template_for_Excel` is Liberation Sans (its
# theme names Arial) and its widest category word is `Column`, and the limit derived above
# says its axis should stay upright where the reference plainly turns it.  So: the same frame
# sweep, in that face, on that word, which measures the reference's own advance for it.
FACE = [(1000, 13, "Column", 6420000, 6800000, 6000),
        # The same word and face at *twelve* categories, so that the one-word/two-word
        # difference the `cats` decks show cannot be a difference of category count.
        (1000, 12, "Column", 5900000, 6480000, 8000)]


def face_decks(out):
    os.makedirs(out, exist_ok=True)
    n = 0
    for size, count, word, lo, hi, step in FACE:
        for cx in range(lo, hi + 1, step):
            name = f"face-z{size}-n{count}-w{cx}"
            with zipfile.ZipFile(SOURCE) as src, \
                    zipfile.ZipFile(os.path.join(out, name + ".pptx"), "w",
                                    zipfile.ZIP_DEFLATED) as dst:
                for item in src.infolist():
                    data = src.read(item.filename)
                    if re.match(r"ppt/charts/chart\d+\.xml$", item.filename):
                        text = variant(data.decode("utf-8"), chars=6, count=count, size=size)
                        text = text.replace("<c:v>WWWWWW</c:v>", f"<c:v>{word}</c:v>")
                        data = text.encode("utf-8")
                    elif item.filename == "ppt/theme/theme1.xml":
                        data = data.decode("utf-8").replace(
                            'typeface="Liberation Mono"', 'typeface="Arial"').encode("utf-8")
                    elif item.filename == "ppt/slides/slide1.xml":
                        text, k = FRAME.subn(
                            lambda m: m.group(1) + str(cx) + m.group(3),
                            data.decode("utf-8"), 1)
                        if k != 1:
                            raise SystemExit("frame extent not found")
                        data = text.encode("utf-8")
                    dst.writestr(item, data)
            n += 1
    print(n, "face decks")


if __name__ == "__main__":
    if len(sys.argv) > 2 and sys.argv[2] == "--face":
        face_decks(sys.argv[1])
    else:
        main(sys.argv[1], confirm=(len(sys.argv) > 2 and sys.argv[2] == "--confirm"))
