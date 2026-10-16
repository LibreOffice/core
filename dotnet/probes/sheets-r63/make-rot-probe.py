#!/usr/bin/env python3
"""Round 30's rotation decks, re-run at six sizes.

Round 30 bracketed the axis wrap limit at [0.990, 1.056] of the tick spacing by comparing
LibreOffice's own rotation decision against *our* word widths.  Round 62 showed those widths
were measured on an unquantised ruler and that chart2's is a whole number of 96 dpi device
pixels, so every one of those boundaries is a measurement of `true limit / scale(10 pt)`.

Rescaling the bracket is arithmetic and needs no deck.  What needs decks is the question the
rescaling raises: `scale` is a **sawtooth** in the size (0.975 at 10 pt, 1.023 at 11 pt), so a
limit that is a pure fraction of the tick spacing and a limit that carries an em-proportional
term move apart when the size changes.  Round 30 measured three boundaries at one size and
could not separate them.  This runs the same generator at six.

Every variant is `chars` copies of `W` in the theme's Liberation Mono (monospaced, so a
character count is a width), `count` categories, at one stated size.  The reference's decision
is read out of its own `chart:coordinate-region` in the `.odp` it exports; the label's drawn
width is read out of its own PDF.  Our renderer never runs.
"""
import os, re, sys, zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.join(HERE, "..", "..", "tests", "corpus", "features",
                      "chart-face-theme-minor.pptx")

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


# The naive boundary count at a size is 15.5 x 10/size, because the label width is
# proportional to the size and the spacing to 1/count.  Sweep a window around it wide enough
# that a 5% sawtooth cannot walk out of it in either direction.
WINDOWS = {
    700:  range(17, 30),
    800:  range(15, 27),
    1000: range(11, 22),
    1100: range(10, 21),
    1300: range(8, 19),
    1400: range(7, 18),
}

VARIANTS = {}
for size, window in WINDOWS.items():
    for n in window:
        VARIANTS[f"rot-z{size}-n{n:02d}"] = dict(chars=6, count=n, size=size)
# Round 30's own two character-count boundary series, unchanged, as the re-run's control.
for k in (3, 4, 5, 6):
    VARIANTS[f"rot-n20c{k:02d}"] = dict(chars=k, count=20, size=1000)
for k in (7, 8, 9, 10, 11):
    VARIANTS[f"rot-n10c{k:02d}"] = dict(chars=k, count=10, size=1000)


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    for name, kw in sorted(VARIANTS.items()):
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
