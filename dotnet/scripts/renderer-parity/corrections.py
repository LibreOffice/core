#!/usr/bin/env python3
"""Fold the eight lanes' findings back into the published catalogue.

Three kinds of change, kept apart because they mean different things:
  * a REFUTED reading -- the case note described something the measurement
    disproves, and the corrected text says what is actually there;
  * a RE-TAG -- the document moves to `reference at fault`, because the lane
    established that our output is the correct one;
  * a VERSION DIVERGENCE -- neither engine is wrong; the tree is calibrated to
    LibreOffice 26.2.4.2 and this sweep's reference is 24.2.7.2.
"""
import json, pathlib

CASES = pathlib.Path("/data/bench/pl-cases.json")
cases = json.loads(CASES.read_text())
by_rank = {c["rank"]: c for c in cases}

# --- documents the lanes established are version divergences, not defects ----
AUTOFIT = [30, 47, 56, 71, 79, 83, 86, 99, 103, 104, 110, 113, 118, 119,
           122, 124, 125, 128, 132, 138, 149, 156, 168, 171]          # L5 cause A
VERSION = {r: ("L5", "slide autofit: the ladder is a correct port of 26.2.4.2; "
                     "24.2.7.2 answers a 2.5% font grid this table cannot produce")
           for r in AUTOFIT}
VERSION.update({
 163: ("L6", "column digit width: <code>DigitWidthCarry</code> is 0.57 for 26.2.4.2 and "
             "0.67 for 24.2.7.2; our 449 pages is the 26.2 answer exactly"),
 183: ("L6", "our 88 pages is the 26.2.4.2 reference's own count; 24.2 gives 109"),
 184: ("L6", "our 201 pages is the 26.2.4.2 reference's own count; 24.2 gives 220"),
 100: ("L8", "the corner-to-linear gradient branch was removed in round 59 on 26.2 "
             "evidence; six arms on the installed 24.2.7.2 reproduce its condition exactly"),
 101: ("L1", "<code>SystemFontResolver</code>'s family ordering was measured on 26.2.4.2; "
             "24.2 draws Liberation Sans for a swiss-classed Helvetica where we draw DejaVu"),
})

# --- readings the lanes refuted, with the corrected text ---------------------
REFUTED = {
 29: "<b>Corrected.</b> Not a page-numbering offset &mdash; neither file contains "
     "<code>w:pgNumType</code>. The <code>PAGE</code> field sits in a text box anchored in a "
     "footer table, where Writer's draw-layer outliner has no page field at all, so the "
     "reference prints one frozen cached value on every page. <b>Our output is the correct "
     "one.</b> The control is a sibling document in the same house style whose footer is not "
     "in a table: there the reference is live and matches us.",
 133: "<b>Corrected.</b> Same mechanism as #029 &mdash; a <code>PAGE</code> field inside a "
      "footer text box, printing a frozen value on every page of the reference. <b>Ours is "
      "correct.</b> The stray <code>1.1.1</code>/<code>1.6.1</code> outline numbers are real "
      "and are the covered-cell list counter defect.",
 55: "<b>Corrected.</b> The data-point markers are <em>not</em> missing: at 200&nbsp;dpi both "
     "engines draw squares, diamonds and down-triangles on the three series, and both draw the "
     "same grey grid with the same orange secondary-axis lines. Only the <em>legend key</em> "
     "lacks its symbol &mdash; the reference draws the marker centred on each key's rule. The "
     "project's own notes already record two reviewers making this identical misreading of "
     "this identical page from a composed pair.",
 2: "<b>Corrected.</b> The black plot area is right: the chart part states "
    "<code>solidFill</code> &rarr; theme <code>dk1</code>, and the reference draws the same "
    "black chart &mdash; on <em>its</em> page 8. Our 5-versus-8 pagination put a different page "
    "under the comparison. The <code>[CELLRANGE]</code> labels are the real defect: "
    "<code>c15:datalabelsRange</code>, which holds the resolved text, is never read.",
 33: "<b>Corrected.</b> The ideographic comma is drawn &mdash; both PDFs read "
     "<code>1、</code> and <code>A、</code>. The real divergence on this document is the "
     "reflow, and a degenerate-rectangle shape filter that discards the revision change-bars.",
 80: "<b>Corrected.</b> The document contains no empty paragraphs at all; there is a single "
     "<code>U+000D</code>. The gap is a 280-twip HTML auto margin that is never handed back "
     "when a list ends at a cell wall, so a run reaching the cell boundary never meets the "
     "unnumbered paragraph that would restore it. Localised in one step by rendering "
     "LibreOffice's own flat-ODF re-export.",
 66: "<b>Corrected.</b> Not missing borders &mdash; at 200&nbsp;dpi both engines draw the "
     "rules and the grey bands. The divergence is print zoom: ours renders at 0.83&times;.",
 180: "<b>Corrected.</b> The title rows collide in <em>both</em> renderings; the reference "
      "simply clips 2&nbsp;pt higher. The columns are also fractionally narrower in ours.",
 49: "<b>Corrected.</b> The hatch is not a defect: the fourteen <code>a:pattFill</code> cells "
     "match. The block looks larger because the rows are taller.",
 176: "<b>Corrected.</b> The logo is neither rescaled nor clipped &mdash; it is the same image "
      "at 612.5&times;792.65&nbsp;pt on both sides, drawn on our page 2 and the reference's "
      "page 1. The whole &ldquo;cover graphics missing&rdquo; reading is one over-tall inline "
      "picture landing on the wrong page.",
 113: "<b>Corrected.</b> There is no tab stop to honour &mdash; the file uses thirteen spaces. "
      "The divergence is the autofit version difference below.",
}

# --- documents that move to `reference at fault` -----------------------------
RETAG = {
 29:  "our output is correct; the reference prints a frozen cached page field",
 133: "our output is correct; the reference prints a frozen cached page field",
 175: "a bar-of-pie chart: we draw the secondary bar and all sixteen legend entries; "
      "LibreOffice 24.2 predates of-pie support",
}

changed = {"refuted": 0, "retag": 0, "version": 0}
for c in cases:
    r = c["rank"]
    if r in REFUTED:
        c["analysis"] = REFUTED[r] + " <span class='wasnote'>Original reading: " \
                        + c["analysis"] + "</span>"
        c["corrected"] = True
        changed["refuted"] += 1
    if r in RETAG:
        if "lo-broken" not in c["tags"]:
            c["tags"] = ["lo-broken"] + [t for t in c["tags"] if t != "lo-broken"]
        c["retag"] = RETAG[r]
        changed["retag"] += 1
    if r in VERSION:
        lane, why = VERSION[r]
        c["version_divergence"] = why
        if "version-divergence" not in c["tags"]:
            c["tags"] = ["version-divergence"] + c["tags"]
        changed["version"] += 1

CASES.write_text(json.dumps(cases))
print(changed)
print("documents now tagged version-divergence:",
      sum(1 for c in cases if "version-divergence" in c["tags"]))
print("documents now tagged lo-broken:",
      sum(1 for c in cases if "lo-broken" in c["tags"]))
