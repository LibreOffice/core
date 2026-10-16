#!/usr/bin/env python3
"""Partition the 192 non-matching documents into eight lanes.

The axis is SOURCE-FILE OWNERSHIP, not document count: two agents editing one
.cs file is what produces a merge conflict, so each lane owns a disjoint set of
directories and every document is assigned to exactly one lane.
"""
import json, pathlib, re, shutil
from collections import Counter

CASES = json.loads(pathlib.Path("/data/bench/pl-cases.json").read_text())
OUT = pathlib.Path("/data/bench/fix-lanes")

LANES = {
 "L1-text-metrics": dict(
   owns=["dotnet/src/XRay.Office.Text/**"],
   title="Text metrics, shaping and line breaking",
   focus="Cases whose only fault is that a line breaks a word early or late: the "
         "glyph advances differ. CLAUDE.md already seats this in the unhinted-vs-"
         "grid-fitted advance and forbids re-deriving the kerning and grid probes."),
 "L2-docx-layout": dict(
   owns=["dotnet/src/XRay.Office.WordProcessing/Layout/**"],
   title="Word-processing layout: indents, table geometry, page breaking",
   focus="Structural geometry: a block indented to the wrong x, a table column "
         "sized differently, row heights, where the page breaks."),
 "L3-docx-reader": dict(
   owns=["dotnet/src/XRay.Office.WordProcessing/Ooxml/**",
         "dotnet/src/XRay.Office.WordProcessing/Model/**"],
   title="DOCX reader: fields, numbering, run properties, borders",
   focus="Values the reader resolves wrongly: TOC field styling, PAGE/NUMPAGES, "
         "STYLEREF running heads, date fields, list counters that advance on "
         "unnumbered rows, run colour and underline, table border properties."),
 "L4-doc-legacy": dict(
   owns=["dotnet/src/XRay.Office.WordProcessing/Ww8/**",
         "dotnet/src/XRay.Office.WordProcessing/Rtf/**"],
   title="Legacy .doc (WW8) reader",
   focus="Everything wrong on a .doc and nothing else: heading fields resolving "
         "to text, revision change-bars, blank paragraphs dropped from table "
         "cells, symbol bullets, CJK list punctuation."),
 "L5-slides": dict(
   owns=["dotnet/src/XRay.Office.Presentations/**"],
   title="Presentations: bullets, autofit, placeholder measure",
   focus="Bullet glyphs that resolve to nothing or to tofu, numbered and lettered "
         "markers dropped, autofit shrink applied or not, placeholder width."),
 "L6-sheets": dict(
   owns=["dotnet/src/XRay.Office.Spreadsheets/**"],
   title="Spreadsheets: print pagination and sheet layout",
   focus="Where the printed page breaks, row heights when cell text wraps, "
         "header/footer fields, print titles colliding with the header row, "
         "banding and borders."),
 "L7-charts": dict(
   owns=["dotnet/src/XRay.Office.Core/Charts/**",
         "dotnet/src/XRay.Office.Ooxml/DrawingML/DrawingChart*.cs"],
   title="Charts",
   focus="Series fills drawn as stripes, legends missing, data labels printing "
         "[CELLRANGE] instead of resolving, axis labels dropped or overlapping, "
         "plot-area fill."),
 "L8-drawing": dict(
   owns=["dotnet/src/XRay.Office.Ooxml/DrawingML/** (except DrawingChart*.cs)",
         "dotnet/src/XRay.Office.Core/Graphics/**",
         "dotnet/src/XRay.Office.Rendering/**"],
   title="Drawing: shape presets, fills, backgrounds, pictures",
   focus="Preset shapes falling back to rectangles, page and slide background "
         "gradients not painted, watermarks and logos absent, picture crop and "
         "rotation not applied."),
}

METRIC_WORDS = re.compile(
  r"measure|glyph|advance|breaks? (a word|later|earlier)|lines break|"
  r"set (slightly |fractionally |a shade )?(wider|narrower)|wider than the reference", re.I)
STRUCT_WORDS = re.compile(
  r"indent|column width|row height|margin|placeholder|table is (laid out |set )?wider|"
  r"page break|split (a |the )?row|leading|paragraph spacing|line spacing", re.I)


def lane_of(c):
    tags, fam, ext, txt = set(c["tags"]), c["family"], c["ext"], c["analysis"]
    if "chart" in tags:
        return "L7-charts"
    if fam == "slides":
        if tags & {"shape-fallback"} or ("missing-graphics" in tags
                                         and not (tags & {"list-markers"})):
            return "L8-drawing"
        return "L5-slides"
    if fam == "sheets":
        return "L6-sheets"
    # docs
    if ext == "doc":
        return "L4-doc-legacy"
    if tags & {"shape-fallback"} or "missing-graphics" in tags:
        return "L8-drawing"
    if tags & {"field-values", "list-markers", "text-style", "table-rules"}:
        return "L3-docx-reader"
    if STRUCT_WORDS.search(txt) and not METRIC_WORDS.search(txt):
        return "L2-docx-layout"
    if METRIC_WORDS.search(txt) and not STRUCT_WORDS.search(txt):
        return "L1-text-metrics"
    return "L2-docx-layout"


def main():
    if OUT.exists():
        shutil.rmtree(OUT)
    buckets = {k: [] for k in LANES}
    for c in CASES:
        buckets[lane_of(c)].append(c)

    for lane, rows in buckets.items():
        d = OUT / lane
        d.mkdir(parents=True)
        meta = LANES[lane]
        lines = [f"# {lane} — {meta['title']}", "",
                 f"**{len(rows)} documents.**", "", meta["focus"], "",
                 "## Source files this lane owns exclusively", ""]
        lines += [f"- `{p}`" for p in meta["owns"]]
        lines += ["", "No other lane will touch these. Do not propose an edit outside "
                      "them; if a fix needs one, record it as a cross-lane dependency.",
                  "", "## Cases", ""]
        for c in sorted(rows, key=lambda x: x["rank"]):
            w = c["worst"]
            lines += [
              f"### #{c['rank']:03d} · {c['name']}",
              f"- corpus path: `{c['dir']}/{c['name']}`",
              f"- rendered pair (LibreOffice left, XRay.Office right): "
              f"`/data/bench/pairs-view/{c['rank']:03d}.jpg`",
              f"- reference PDF `/data/bench/lo/{c['id']}/out.pdf` · "
              f"ours `/data/bench/pl/{c['id']}/out.pdf`" if "id" in c else "",
              f"- divergent page {c['page']} of {c['ref_pages']}; "
              f"we produce {c['engine_pages']} pages",
              (f"- SSIM {w['ssim']:.3f} · MAE {w['mae']:.3f} · ink x{w['ink_ratio']:.2f} "
               f"· {c['defect_pages']} of {c['compared']} compared pages defective"
               if w else "- pixels match on the compared pages; only the page count differs"),
              f"- tags: {', '.join(c['tags'])}",
              "",
              re.sub(r"<[^>]+>", "", c["analysis"]).replace("&mdash;", "—")
                 .replace("&ldquo;", "“").replace("&rdquo;", "”")
                 .replace("&times;", "x").replace("&amp;", "&"),
              ""]
        (d / "cases.md").write_text("\n".join(l for l in lines if l is not None))

    print(f"{'lane':18s} {'docs':>5s}  families")
    for lane, rows in buckets.items():
        fams = Counter(r["family"] for r in rows)
        print(f"{lane:18s} {len(rows):5d}  {dict(fams)}")
    print("total", sum(len(v) for v in buckets.values()))


main()
