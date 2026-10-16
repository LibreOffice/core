#!/usr/bin/env python3
"""Tag each reading with the defect classes it describes.

The rules match phrases I used deliberately when writing the readings, so the
taxonomy is derived from the descriptions rather than guessed from the metrics.
"""
import json, pathlib, re
from collections import Counter

RULES = [
    ("missing-graphics", r"background (gradient|wash|fill)|not painted|watermark|logo (is|are)? ?(missing|not drawn)|line-art|illustration|logos? (and|are)? ?missing|graphic on the cover|artwork|photograph is|drop shadows"),
    ("shape-fallback",  r"drawn as (a )?(plain )?(vertical bar|rectangle|square)|as concentric <em>squares|chevron|arrowhead|ellipse preset|shapes collapse|plain rectangles|as <em>squares</em>"),
    ("list-markers",    r"bullet glyph|bullet glyphs|roman numerals|lettered markers|numbered circle|circle bullets|list markers|no marker at all|adds a bullet|custom bullet|bullets? (is|are) gone|numbering (counter|runs away)|numbering in the ID column|automatic numbering|stray outline numbers|section numbering"),
    ("field-values",    r"page[- ]number(ing)? (field|offset)|page field|<code>Page|date field|recalculat|stored value|field embedded in the heading|running head resolves|copyright line|TOC|table-of-contents|contents entr|hyperlink where the reference|blue underlined"),
    ("table-rules",     r"borders|border style|dashed separators|cell rules|rule under|double rule|change-bar|separator|frame around|page border|loses its frame"),
    ("chart",           r"chart|series|gridlines|legend|axis|plot area|bars are filled|data label"),
    ("text-style",      r"printed blue|come out blue|comes out in the template's blue|bold weight|set in a heavier weight|colour differs|printed grey|in grey rather|underlined where the reference|highlight|italic"),
    ("overlap-clip",    r"overlap|clipped|overprint|collide|cut in half|runs off|overrun|overflow|obscur|hidden|cut by"),
    ("content-missing", r"missing|absent|not drawn|is gone|are gone|invisible|renders? (this page )?(completely )?blank|dropped|drops"),
    ("reflow",          r"reflow|measure|line break|lines break|breaks? (a word |at different|later|earlier)|row heights?|leading|line spacing|paragraph spacing|indent|column widths?|fits more|fits (noticeably )?less|carries more|tighter|wider than the reference"),
    ("pagination",      r"page count|pages against|paginat|one page longer|one page shorter|offset by|drift"),
    ("lo-broken",       r"reference is the (one that|broken one)|the reference is the|LibreOffice (loses|leaves|runs|clips|renders this slide)|Counted as a divergence only because|reference is the one that"),
]

def main():
    cases = json.loads(pathlib.Path("/data/bench/pl-cases.json").read_text())
    notes = {}
    for p in pathlib.Path("/data/bench/analysis").glob("*.json"):
        j = json.loads(p.read_text())
        notes[j["rank"]] = j["text"]
    counts = Counter()
    # Negated mentions ("nothing is missing", "no content differs") must not
    # count as the defect they name.
    NEGATED = re.compile(r"(nothing|no content|not|nothing else)[^.]{0,40}?(missing|differs|absent)", re.I)
    for c in cases:
        text = notes.get(c["rank"], "")
        scan = NEGATED.sub("", text)
        tags = [name for name, pat in RULES if re.search(pat, scan, re.I)]
        if not tags:
            tags = ["reflow"]
        c["analysis"] = text
        c["tags"] = tags
        counts.update(tags)
    pathlib.Path("/data/bench/pl-cases.json").write_text(json.dumps(cases))
    for k, v in counts.most_common():
        print(f"{v:4d}  {k}")
    print("untagged:", sum(1 for c in cases if not c["analysis"]))

main()
