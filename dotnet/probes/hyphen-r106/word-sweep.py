"""Authored fixtures: twenty-two second words, one dictionary, two renderers.

The trigger was established twice already (probes/chart-axisrot-r91 swapped the words,
probes/chart-hyph-r105 held every glyph fixed and switched the reference's dictionary off).
What this measures is the IMPLEMENTATION: given a word, does this tree turn the axis exactly
where LibreOffice 26.2.4.2 turns it?

Every variant is `038_Competitive_Advantage_Card`'s five category labels rewritten to
`Cost <word>`, so the first word, the font, the size, the frame, the tick spacing and the
number of categories are all held fixed and only the second word varies. The words are
chosen so that WIDTH CANNOT ORDER THEM: `Quality` (7 characters) has a hyphenation point and
`Thoughts` (8) has none; `Marketing` (9) has two and `Straights` (9) has none;
`Breakthrough` (12) has one and `Screeched` (9) has none.

    python3 word-sweep.py <dir>                    write the variants
    /opt/libreoffice26.2/program/soffice --headless --convert-to pdf --outdir <dir>/ref <dir>/*.pptx
    <cli> render --outdir <dir>/ours <dir>/*.pptx
    python3 word-sweep.py --classify <dir>         the agreement table
"""
import glob
import os
import sys
import zipfile

SRC = ("/home/user/sample-files/slides/chartset-008/pptx/"
       "038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx")

CATS = ["Product Quality", "Innovation", "Brand Reputation", "Cost Efficiency",
        "Customer Service"]

# Second words, with the point list hyph_en_US.dic gives each. The order here is the order
# of the table in results.md and is by length so the width claim can be read off it.
WORDS = [
    "Quality", "Service", "Thoughts", "Strength", "Splashed", "Stretched", "Strengths",
    "Scratched", "Straights", "Screeched", "Squelched", "Marketing", "Efficiency",
    "Reputation", "Governance", "Throughput", "Excellence", "Resilience", "Stringency",
    "Consumption", "Streamlined", "Breakthrough",
]


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
                        text = text.replace(
                            "<c:v>%s</c:v>" % category, "<c:v>Cost %s</c:v>" % word)
                    data = text.encode("utf-8")

                out.writestr(item, data)

    print("wrote", len(WORDS), "variants to", into)


def arrangement(path):
    """`turned` or `upright`, for the axis of the whole drawn page.

    <strong>Three traps, each of which produced a wrong table on the way here.</strong>

    The card BEHIND the chart carries body text reading `Cost Efficiency` whatever the axis
    says, so matching on the token finds it and calls a turned axis upright -- six of these
    twenty-two, on the first cut. A band around the axis does not fix that either: the legend
    sits in the same band.

    PyMuPDF reports a rotated span's AXIS-ALIGNED box, so a turned label reads as a square far
    too wide for its slot; the line's own `dir` vector is the only discriminator.

    And a turned label the reference OUTLINES is absent from the text layer entirely, because
    VclProcessor2D accepts a text primitive only while abs(fontScaling.getY() * fShearX) < 1
    and a chart whose labels overflow is squeezed anisotropically to fit. Counting the
    glyph-sized filled paths is what tells that apart from an axis drawn upright -- an upright
    page of this deck has 2 of them and a single-line one 12, against 77 to 92 for an outlined
    axis, so the threshold below is nowhere near anything.

    A label that fits its slot on ONE line is upright and is NOT the two-line wrap: an earlier
    cut looked for the first word alone on a line and read every single-line variant as
    outlined, which inverted five rows of the leading sweep.
    """
    import pymupdf

    with pymupdf.open(path) as doc:
        page = doc[0]
        turned_lines = 0

        for block in page.get_text("dict")["blocks"]:
            if block["type"] != 0:
                continue
            for line in block["lines"]:
                dx, dy = line["dir"]
                if abs(dx - 0.70711) < 0.004 and abs(dy + 0.70711) < 0.004:
                    turned_lines += 1

        fills = sum(
            1 for d in page.get_drawings()
            if d["fill"] is not None
            and 0.3 < d["rect"].width < 15 and 0.3 < d["rect"].height < 15)

    if turned_lines:
        return "turned(text %d)" % turned_lines
    if fills >= 30:
        return "turned(outlined %d)" % fills
    return "upright"


def classify(root):
    print("word\tlen\treference\tours")
    agree = 0
    total = 0

    for word in WORDS:
        ref = os.path.join(root, "ref", word + ".pdf")
        ours = glob.glob(os.path.join(root, "ours", word + ".pdf"))

        if not os.path.exists(ref) or not ours:
            print(f"{word}\t{len(word)}\t-\t-")
            continue

        a, b = arrangement(ref), arrangement(ours[0])
        total += 1

        # `outlined` and `turned` are the same ARRANGEMENT drawn two ways: 26.2.4.2 outlines a
        # turned label inside an anisotropically squeezed chart and this tree draws it as text.
        # The question here is rotate-or-wrap, so the two count as agreeing and the raw words
        # are printed so the distinction stays visible.
        same = a.startswith("upright") == b.startswith("upright")
        agree += same
        print(f"{word}\t{len(word)}\t{a}\t{b}\t{'' if same else 'DIFFERS'}")

    print(f"\nagree {agree} of {total}")


if __name__ == "__main__":
    if sys.argv[1] == "--classify":
        classify(sys.argv[2])
    else:
        write(sys.argv[1])
