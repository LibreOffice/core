#!/usr/bin/env python3
"""O43 -- one instrument for BOTH of the two ladders, at more than one tick pitch.

`038_Competitive_Advantage_Card`'s bar chart is rewritten so that four things vary
independently and nothing else does:

  * the category label, as a run of one repeated letter (no hyphenation point exists in
    one, which removes r106's confound) optionally prefixed by a second such run (the two-word shape),
  * the number of categories, which is the ONLY way to move the tick pitch without touching
    the frame, the font or the plot,
  * the category-axis font size,
  * the chart frame's own width, which moves the tick pitch continuously where the category
    count moves it in steps -- and moves nothing about the label,
  * and the run language, retagged `de-DE` throughout so 26.2.4.2's hyphenator is off
    (r105's `delang.py` recipe) whatever the label says.

Widths are the drawn advance on chart2's own 96 dpi device -- the installed Carlito through
`round(size x 96/72) / (size x 96/72)`, which `probes/chart-geom-r108` measured against the
reference's drawn ink to 0.05 pt.

    ladder.py write   <dir> <spec>...   spec = shape:cats:size:text  (shape = one|two)
    ladder.py classify <dir>            read <dir>/ref/*.pdf, print one row per variant
"""
import os
import re
import sys
import zipfile

import pymupdf

SRC = ("/home/user/sample-files/slides/chartset-008/pptx/"
       "038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx")
CHART = "ppt/charts/chart1.xml"
FRAME_CX = 3585210      # the `p:graphicFrame` extent `038` states, in EMU
FRAME_X = 5218265       # and its offset; a wider frame is re-centred on the 9144000 slide
FACE = "/usr/share/fonts/truetype/crosextra/Carlito-Regular.ttf"

# The label shapes. Every fixed word is a run of one letter, like the ladder's own, so that
# no word can hyphenate and none collides with a word the surrounding slide already draws --
# r106's `Cost ` does, in the body text beside the chart, which makes it unreadable here.
SHAPES = {
    "one": "%s",            # the ladder's run alone
    "two": "ooo %s",        # r106's shape: a short first word, the run second
    "two1": "o %s",         # the same with a narrower first word
    "two6": "oooooo %s",    # and a wider one
    "rev": "%s ooo",        # the run FIRST, so it is line one rather than line two
    "three": "ooo %s ooo",
}

_FONT = pymupdf.Font(fontfile=FACE)


def advance(text, size):
    """The drawn advance of one string on chart2's 96 dpi device, in points."""
    exact = size * 96.0 / 72.0
    return _FONT.text_length(text, fontsize=size) * (round(exact) / exact)


def label_of(shape, text):
    return SHAPES[shape] % text


def _cache(tag, inner_fmt, n, value):
    pts = "".join(inner_fmt % (i, value) for i in range(n))
    return '<c:ptCount val="%d"/>%s' % (n, pts)


def rewrite_chart(xml, label, cats, size, lang):
    """`label` is one string, or a list of `cats` strings when they are to differ."""
    labels = label if isinstance(label, list) else [label] * cats

    # categories: every strCache that carries the five category strings
    def cat_sub(m):
        pts = "".join('<c:pt idx="%d"><c:v>%s</c:v></c:pt>' % (i, labels[i])
                      for i in range(cats))
        return '<c:strCache><c:ptCount val="%d"/>%s</c:strCache>' % (cats, pts)
    xml = re.sub(r'<c:strCache><c:ptCount val="5"/>.*?</c:strCache>', cat_sub, xml)

    # values: keep them distinct enough to see, but the count has to follow the categories
    def val_sub(m):
        vals = [70, 90, 30, 45, 85, 55, 65, 25, 95, 40, 75, 35]
        pts = "".join('<c:pt idx="%d"><c:v>%d</c:v></c:pt>' % (i, vals[i % len(vals)])
                      for i in range(cats))
        return ('<c:numCache><c:formatCode>General</c:formatCode>'
                '<c:ptCount val="%d"/>%s</c:numCache>' % (cats, pts))
    xml = re.sub(r'<c:numCache><c:formatCode>General</c:formatCode>'
                 r'<c:ptCount val="5"/>.*?</c:numCache>', val_sub, xml)

    # the category axis' own font size, which is the only 1100 in the part
    if size is not None:
        head = xml.index("<c:catAx>")
        tail = xml.index("</c:catAx>")
        block = xml[head:tail].replace('sz="1100"', 'sz="%d"' % int(round(size * 100)))
        xml = xml[:head] + block + xml[tail:]

    # every run property retagged, which is r105's hyphenator switch
    def lang_sub(m):
        tag, attrs, slash = m.group(1), m.group(2), m.group(3)
        attrs = re.sub(r'\slang="[^"]*"', '', attrs)
        return '<a:%s lang="%s"%s%s>' % (tag, lang, attrs, slash)
    xml = re.sub(r'<a:(defRPr|rPr|endParaRPr)\b([^>/]*?)(/?)>', lang_sub, xml)
    return xml


def name_of(shape, cats, size, text, cx=FRAME_CX):
    return "%s-c%d-s%s-x%d-%s" % (shape, cats, ("%g" % size).replace(".", "_"), cx, text)


def write_one(into, shape, cats, size, text, cx=FRAME_CX, lang="de-DE"):
    """`text` may be `long|short`, which alternates two labels along the axis.

    That is the experiment that separates *the collision box is wider than its text* from
    *the axis turns whenever the wrap restarted*: alternating a word past the wrap limit
    with a very short one restarts the wrap and leaves every adjacent pair far apart.
    """
    name = name_of(shape, cats, size, text, cx)
    dst = os.path.join(into, name + ".pptx")
    if "|" in text:
        a, b = text.split("|")
        label = [label_of(shape, a if i % 2 == 0 else b) for i in range(cats)]
    else:
        label = label_of(shape, text)
    with zipfile.ZipFile(SRC) as zin, \
            zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as out:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == CHART:
                data = rewrite_chart(data.decode("utf-8"), label, cats, size,
                                     lang).encode("utf-8")
            elif item.filename.startswith("ppt/slides/slide") and cx != FRAME_CX:
                text = data.decode("utf-8")
                text = text.replace('<a:ext cx="%d"' % FRAME_CX, '<a:ext cx="%d"' % cx)
                text = text.replace('<a:off x="%d"' % FRAME_X,
                                    '<a:off x="%d"' % max(0, (9144000 - cx) // 2))
                data = text.encode("utf-8")
            out.writestr(item, data)
    return name


# ---------------------------------------------------------------- reading a rendering

def read_pdf(path):
    """turned text lines, whitespace-split words, and glyph-sized filled paths of page 1."""
    with pymupdf.open(path) as doc:
        page = doc[0]
        turned = [line
                  for b in page.get_text("dict")["blocks"] if b["type"] == 0
                  for line in b["lines"] if abs(line["dir"][0] - 0.70711) < 0.004]
        words = page.get_text("words")
        fills = [d for d in page.get_drawings()
                 if d["fill"] is not None
                 and 0.3 < d["rect"].width < 40 and 0.3 < d["rect"].height < 40]
    return turned, words, fills


def classify_one(path, shape, cats, size, text):
    """One rendering, read as: is the axis turned, on how many lines, at what pitch.

    The category labels are found by their own text -- a run of one letter, which no other
    text on the slide draws -- so the merging PyMuPDF does when several labels share a
    baseline cannot confuse them, and the axis-aligned box it reports for a rotated span
    is never relied on.
    """
    turned, words, fills = read_pdf(path)
    arrangement = "turned" if (turned or len(fills) >= 6 * cats) else "upright"
    label = label_of(shape, text)
    runs = [w for w in words if w[4] == text]
    xs = sorted((w[0] + w[2]) / 2 for w in runs)
    pitch = ((xs[-1] - xs[0]) / (len(xs) - 1)) if len(xs) > 1 else float("nan")
    ink = max((w[2] - w[0] for w in runs), default=float("nan"))
    # How many lines the label was drawn on: the distinct baselines its own words sit on.
    # Every label of one axis shares its baselines, so this counts lines and not labels.
    # It is only meaningful upright -- PyMuPDF reports an axis-aligned box for a turned run,
    # so a single turned line looks like one baseline per word.
    parts = set(label.split())
    ys = {round(w[1], 1) for w in words if w[4] in parts}
    lines = len(ys) if arrangement == "upright" else 0
    return {
        "arrangement": arrangement,
        "lines": lines,
        "ink": ink,
        "pitch_drawn": pitch,
        "n_runs": len(runs),
        "n_turned": len(turned),
        "n_fills": len(fills),
    }


def parse(spec):
    shape, cats, size, text = spec.split(":")
    return shape, int(cats), float(size), text


def main():
    mode, root = sys.argv[1], sys.argv[2]
    specs = [parse(s) for s in sys.argv[3:]]
    if mode == "write":
        os.makedirs(root, exist_ok=True)
        for shape, cats, size, text in specs:
            write_one(root, shape, cats, size, text)
        print("wrote", len(specs), "variants to", root)
        return
    print("shape\tcats\tsize\ttext\tadvance\tlabel_adv\tarrangement\tlines"
          "\tink\tpitch_drawn\tratio")
    for shape, cats, size, text in specs:
        name = "%s-c%d-s%s-%s" % (shape, cats, ("%g" % size).replace(".", "_"), text)
        pdf = os.path.join(root, "ref", name + ".pdf")
        if not os.path.exists(pdf):
            print("%s\t%d\t%g\t%s\t-\t-\tMISSING" % (shape, cats, size, text))
            continue
        r = classify_one(pdf, shape, cats, size, text)
        w = advance(text, size)
        ratio = (r["ink"] / r["pitch_drawn"]) if r["pitch_drawn"] == r["pitch_drawn"] \
            else float("nan")
        print("%s\t%d\t%g\t%s\t%.3f\t%.3f\t%s\t%d\t%.3f\t%.3f\t%.4f"
              % (shape, cats, size, text, w, advance(label_of(shape, text), size),
                 r["arrangement"], r["lines"], r["ink"], r["pitch_drawn"], ratio))


if __name__ == "__main__":
    main()
