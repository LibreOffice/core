#!/usr/bin/env python3
"""Why LibreOffice draws one chart's text anisotropically, settled by mutation.

Four rounds read the residue on `southern-classic-kennesaw-state-university-final.pptx`
page 11 as a *size* — the reference draws a 14 pt axis label as `13.589 Tf` and a 16 pt
data label as `15.486 Tf` — and round thirty-one showed it is two numbers rather than one:
the same shows carry a text matrix whose horizontal scale is `1.030454`, so the glyphs are
14.00 and 15.96 pt *wide* and 13.589 and 15.486 pt *tall*.  The width is the stated size;
only the height is short.

This probe mutates one attribute of that real file at a time, renders with `soffice`, and
reads back `(a, Tf)` for the chart's own shows.  It answers three questions:

  * **Is it the frame's position?**  Round thirty-one recorded, explicitly as an untested
    correlation, that the one thing distinguishing that chart is a frame starting 1.36 pt
    off the left edge of the slide.  It is not: moving the chart wholly onto the slide,
    moving it eight times further off, and pushing either of the page's other two charts
    off the same edge all leave every `a` on the page bit-identical.

  * **Is it the frame's shape?**  A sweep of the frame *height* moves `a` from 1.9878 at
    40 pt to 1.0000 at 235 pt and above; a sweep of the frame *width* over a factor of
    three moves it in the fifth decimal.  So the chart is compressed along one axis only.

  * **Is it the chart's content?**  Yes, and this is the mechanism.  Dropping the data
    labels from 16 pt to 10 pt removes the compression outright (`a` 1.030454 → 1.000100);
    raising them to 24 pt deepens it (1.085123).  Removing the legend does not help and
    shrinking the legend's font barely moves it.  Raising the *legend* to 40 pt while the
    data labels fit gives `a = 0.863091` with `Tf` at the stated 40 — the mirror case, a
    **horizontal** compression.

So: LibreOffice composes a chart at its frame's size, and when the chart's own content
overflows that page — a data label above the top bar, a legend too wide — the picture it
hands the OLE frame is bigger than the frame along that axis and is scaled back into it.
The whole chart is scaled, not only its type: the legend key square measures 8.39 × 8.39 pt
where nothing overflows and 8.39 × 7.11 pt at a frame height of 85 pt, which is the vertical
font scale to three decimals.

Run with no arguments; it writes its variants and their PDFs into the working directory.
"""
import os
import re
import subprocess
import sys
import zipfile
import zlib
from collections import Counter

SRC = ("/workspace/sample-files/slides/batch-010/pptx/"
       "southern-classic-kennesaw-state-university-final.pptx")
SLIDE = "ppt/slides/slide11.xml"
CHART = "ppt/charts/chart15.xml"      # the frame named "Chart 25" on that slide
FRAME = "Chart 25"
BASE_CX = 6064192                     # EMU, the frame's own width
BASE_CY = 2265602                     # EMU, the frame's own height
EMU_PT = 12700.0

# `a 0 0 d e f Tm` immediately followed by `/Fn size Tf` — the shape LibreOffice's PDF
# export writes for every show.  A run whose matrix is anything else is skipped.
SHOW = re.compile(rb'([\d.]+) 0 0 ([\d.]+) (-?[\d.]+) (-?[\d.]+) Tm\s*/F(\d+) ([\d.]+) Tf')


def build(name, *, off=None, cx=None, cy=None, chart=None):
    """Write <name>.pptx: the corpus deck with at most one attribute changed."""
    zin = zipfile.ZipFile(SRC)
    zout = zipfile.ZipFile(name + ".pptx", "w", zipfile.ZIP_DEFLATED)
    for item in zin.infolist():
        data = zin.read(item.filename)
        if item.filename == SLIDE and (off, cx, cy) != (None, None, None):
            text = data.decode("utf-8")

            def frame(match):
                block = match.group(0)
                if 'name="%s"' % FRAME not in block:
                    return block
                if off is not None:
                    block = re.sub(r'<a:off x="(-?\d+)"', '<a:off x="%d"' % off,
                                   block, count=1)
                if cx is not None:
                    block = re.sub(r'(<a:ext cx=")\d+(")', r'\g<1>%d\g<2>' % cx,
                                   block, count=1)
                if cy is not None:
                    block = re.sub(r'(<a:ext cx="\d+" cy=")\d+(")',
                                   r'\g<1>%d\g<2>' % cy, block, count=1)
                return block

            data = re.sub(r'<p:graphicFrame>.*?</p:graphicFrame>', frame,
                          text, flags=re.S).encode("utf-8")
        if chart is not None and item.filename == CHART:
            data = chart(data.decode("utf-8")).encode("utf-8")
        zout.writestr(item, data)
    zout.close()
    zin.close()


def render(name):
    if os.path.exists("ref/%s.pdf" % name):
        return
    subprocess.run(["timeout", "300", "soffice",
                    "-env:UserInstallation=file://" + os.getcwd() + "/prof",
                    "--headless", "--convert-to", "pdf", "--outdir", "ref",
                    name + ".pptx"],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def shows(pdf, ordinal=10):
    """(a, Tf) counts for the nth content stream that draws text — page 11's."""
    data = open(pdf, "rb").read()
    seen = 0
    for match in re.finditer(rb'stream\r?\n', data):
        start = match.end()
        end = data.find(b'endstream', start)
        if end < 0:
            continue
        try:
            body = zlib.decompress(data[start:end])
        except Exception:
            continue
        if not SHOW.search(body):
            continue
        seen += 1
        if seen != ordinal:
            continue
        counts = Counter()
        for show in SHOW.finditer(body):
            counts[(round(float(show.group(1)), 6), round(float(show.group(6)), 3))] += 1
        return counts
    return Counter()


def report(name):
    counts = shows("ref/%s.pdf" % name)
    worst = max((abs(a - 1.0), a) for a, _ in counts) if counts else (0.0, 1.0)
    detail = "  ".join("%dx a=%.6f Tf=%.3f" % (n, a, size)
                       for (a, size), n in sorted(counts.items(), key=lambda kv: -kv[1])[:4])
    print("%-16s a*=%.6f   %s" % (name, worst[1], detail))


def main():
    os.makedirs("ref", exist_ok=True)
    os.makedirs("prof", exist_ok=True)

    print("# the frame's position — refutes the overhang correlation")
    for name, off in (("p-asis", None), ("p-on", 17268), ("p-far", -200000)):
        build(name, off=off)
        render(name)
        report(name)

    print("\n# the frame's height, in points")
    for pt in (40, 85, 130, 175, 220, 235, 265):
        name = "h-%d" % pt
        build(name, cy=int(round(pt * EMU_PT)))
        render(name)
        report(name)

    print("\n# the frame's width, as a multiple of its own")
    for mult in (0.5, 1.0, 1.5):
        name = "w-%02d" % round(mult * 10)
        build(name, cx=int(round(BASE_CX * mult)))
        render(name)
        report(name)

    print("\n# the chart's content")
    cases = {
        "c-base":     lambda s: s,
        "c-noleg":    lambda s: re.sub(r'<c:legend>.*?</c:legend>', '', s, flags=re.S),
        "c-dlbl-10":  lambda s: s.replace('sz="1600"', 'sz="1000"'),
        "c-dlbl-24":  lambda s: s.replace('sz="1600"', 'sz="2400"'),
        "c-leg-8":    lambda s: s.replace('sz="1400"', 'sz="800"'),
        "c-leg-40":   lambda s: s.replace('sz="1600"', 'sz="1000"')
                                 .replace('sz="1400"', 'sz="4000"'),
    }
    for name, edit in cases.items():
        build(name, cy=BASE_CY, chart=edit)
        render(name)
        report(name)


if __name__ == '__main__':
    sys.exit(main())
