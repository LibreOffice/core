#!/usr/bin/env python3
"""A second correction pass, using only evidence that does not depend on the
reference binary's version.

The first pass folded in the eight lanes' findings, and 29 of its 43 changes
were version divergences -- true of this reference and not of the one the tree
is built against. Those cannot be checked here. What *can* be checked here is
everything that is a statement about our own output: what our PDF's content
stream contains, in what order it paints, and at what size. That evidence is
as good against 26.2 as against 24.2, because the reference is not party to it.

Three findings, and one class that dissolved on inspection.

PAINT ORDER (5 documents). The pixels report `content missing`; the content is
in fact drawn and then painted over. Each was established three ways: an
opaque fill whose rectangle contains the text block's anchor appears LATER in
the same content stream; rasterising that patch yields one uniform colour; and
the crop, put beside the reference's, shows the reference drawing the text and
ours drawing a bare shape. This is one cause behind five separate readings,
and it is a different fix in a different place from a missing-content bug.

DEGENERATE HEADER (#177). Not missing: emitted at 0.120 pt where the reference
sets 7.887 pt, in the same face and the same colour, centred correctly. The
page carries exactly two spans under 1 pt -- the header and the footer -- and
everything else on it is 10 pt, so the fault is localised to header/footer
text and accounts for both halves of a reading that named two separate losses.

DISPLACED, NOT ABSENT (#164, #191). The phrase the reading quotes as missing
is present in our output, on another page. A reflow moved it; nothing was
dropped.

WITHDRAWN. A candidate `paint order` finding on #158 and #162 is not reported:
the detector locates a text block by the coordinates in the content stream and
does not apply the `cm` transforms above it, so a block inside a transformed
chart resolves to the wrong patch of paper. #158's crop showed the detector
had sampled empty margin. #162's hidden text is a five-character fragment at
the page edge of a slide whose reading says the REFERENCE is the broken one.
Both are withdrawn rather than reported weakly -- and the same caveat is why
the five above were each confirmed by eye before being written down.
"""
import json, pathlib

CASES = pathlib.Path("/data/bench/pl-cases.json")
cases = json.loads(CASES.read_text())

PAINT_ORDER = {
 24: "the whole slide &mdash; the white rounded card, the <em>human body</em> title, the "
     "three organ panels and the silhouette &mdash; is drawn, and then the dark grey page "
     "background is painted over all of it. 16 text blocks on this page sit under a later "
     "opaque fill; the reference has none.",
 62: "the numerals <em>01</em>&ndash;<em>05</em> are not missing. They are drawn, and the "
     "coloured milestone shapes are then painted over them: the crop shows the reference's "
     "orange circle carrying <em>01</em> and ours a bare orange square. The unpainted page "
     "gradient is a separate defect and stands.",
 114: "the <em>Delivery</em> label is drawn and the green legend box is then painted over "
      "it, so the box reads as empty. Same for five more blocks on this page.",
 148: "the captions are not hidden by the triangle being drawn as a rectangle &mdash; they "
      "are hidden because the shape is painted after them. 17 text blocks on this page are "
      "overpainted; correcting the chevron geometry alone would still leave them covered.",
 173: "<em>2021</em> is drawn at stream offset 2473 and the black year box is filled at "
      "offset 4180, over it. The box is not empty; its contents are underneath it.",
}

DEGENERATE = {
 177: "<b>Corrected.</b> The title is not missing &mdash; it is emitted at "
      "<b>0.120&nbsp;pt</b> where the reference sets <b>7.887&nbsp;pt</b>, in the same face "
      "(<code>LiberationSans-Bold</code>), the same colour and correctly centred. The page "
      "carries exactly two spans under 1&nbsp;pt, the header and the footer, and every other "
      "span on it is 10&nbsp;pt &mdash; so one degenerate header/footer scale accounts for "
      "both of the losses this reading names, and neither is a drawing failure.",
}

DISPLACED = {
 164: "<b>Corrected.</b> The numbered line <em>3. Please list any special qualifications you "
      "might have</em> is <b>not missing</b> &mdash; it is on our page 3, moved by the "
      "reflow. Nothing was dropped; the taller rows pushed it back a page. The oversized "
      "clipped header logo and the doubled rule are unaffected and stand.",
 191: "<b>Corrected.</b> <em>Title:</em> and <em>Date:</em> are <b>not missing</b> &mdash; "
      "they are on our page 2. The grid is drawn wider and taller than the reference's, which "
      "pushes the caption line onto a second page the reference does not have, and that is "
      "also the whole of the page-count divergence. One oversized grid, not three losses.",
}

# readings an independent, version-independent check upheld
VERIFIED = {
 39: "page alignment checked from both text layers: our page 5 is the reference's page 6, "
     "exactly as the reading says",
 52: "page alignment checked: our page 4 is the reference's page 5, as described",
 111: "page alignment checked: our page 5 is the reference's page 6, as described",
 182: "page alignment checked: our page 4 is the reference's page 5, as described",
 188: "page alignment checked and the pages DO correspond &mdash; an alignment flag here was "
      "an artefact of scoring pages on words while this page is almost all numerals",
 60: "the phrase is on our page 6, which is the offset the reading already describes",
 150: "the <em>AVIOSTART</em> entry is on our page 4, which is the offset the reading describes",
}

n = {"paint_order": 0, "degenerate": 0, "displaced": 0, "verified": 0}
for c in cases:
    r = c["rank"]
    if c.get("corrected2"):
        continue
    if r in PAINT_ORDER:
        c["paint_order"] = PAINT_ORDER[r]
        if "paint-order" not in c["tags"]:
            c["tags"] = ["paint-order"] + c["tags"]
        c["corrected2"] = True; n["paint_order"] += 1
    if r in DEGENERATE:
        c["analysis"] = DEGENERATE[r] + " <span class='wasnote'>Original reading: " \
                        + c["analysis"] + "</span>"
        c["corrected"] = True; c["corrected2"] = True; n["degenerate"] += 1
    if r in DISPLACED:
        c["analysis"] = DISPLACED[r] + " <span class='wasnote'>Original reading: " \
                        + c["analysis"] + "</span>"
        c["corrected"] = True; c["corrected2"] = True; n["displaced"] += 1
    if r in VERIFIED:
        c["verified"] = VERIFIED[r]; c["corrected2"] = True; n["verified"] += 1

CASES.write_text(json.dumps(cases))
print(n)
print("documents tagged paint-order:", sum(1 for c in cases if "paint-order" in c["tags"]))
