#!/usr/bin/env python3
"""Which corpus worksheets have a band AND print at a scale other than 100%?

Those, and only those, are the worksheets whose body origin moves when the header band stops
being subtracted from the body at full size.  The movement is `HeaderHeight * (1 - zoom/100)`
downwards, so a sheet at 100% moves nothing and a sheet with no band moves nothing.

WHAT THIS CENSUS CANNOT SEE, written down before the sweep as HANDOVER.md s7 requires:

  * **`.xls` and `.xlsb` are invisible to it.** It reads OOXML parts only.  64 of the 307 sheets
    documents are `.xls`, and round 56's blind spot 1 fired exactly there.
  * **A `fitToPage` sheet's real zoom is not in the file.**  It is the bisection's answer, so this
    counts such a sheet as "scaled" whenever `fitToWidth`/`fitToHeight` are set, which over-counts
    every sheet that happens to fit at 100%.
  * **It counts worksheets, not pages.**  A scaled banded worksheet that contributes no page to
    the printout (hidden, or outside the print range) is counted here and moves nothing.
  * It cannot see whether a moved body changes a *verdict*: the gate reads page count, word count
    and fonts, and a uniform vertical translation changes none of the three unless it pushes a
    token off the paper.
"""
import csv, re, sys, zipfile

ROOT = "/c/sandbox/workdir/sample-files/"
paths = []
with open(ROOT + "MANIFEST.tsv") as f:
    r = csv.reader(f, delimiter="\t")
    next(r)
    for row in r:
        if row[0] == "sheets":
            paths.append(row[2])

ooxml = scaled_banded = banded = scaled = 0
docs = []
for p in paths:
    if not p.lower().endswith((".xlsx", ".xlsm", ".xltx", ".xltm")):
        continue
    try:
        z = zipfile.ZipFile(ROOT + p)
    except Exception:
        continue
    ooxml += 1
    hits = []
    for nm in z.namelist():
        if not re.match(r"xl/worksheets/sheet\d+\.xml$", nm):
            continue
        d = z.read(nm).decode("utf8", "replace")
        hf = re.search(r"<headerFooter.*?</headerFooter>", d, re.S)
        has_band = bool(hf) and bool(
            re.search(r"<(odd|even|first)(Header|Footer)>[^<]", hf.group(0)))
        s = re.search(r"<pageSetup[^>]*>", d)
        st = s.group(0) if s else ""
        scale = re.search(r'scale="(\d+)"', st)
        fitw = re.search(r'fitToWidth="(\d+)"', st)
        fith = re.search(r'fitToHeight="(\d+)"', st)
        fitpage = bool(re.search(r'fitToPage="1"', d))
        # A `fitToPage` sheetPr makes fitToWidth/fitToHeight live; without it the `scale` rules.
        is_scaled = ((scale and scale.group(1) != "100")
                     or (fitpage and (fitw or fith)))
        if has_band:
            hits.append((nm, st.strip(), bool(is_scaled)))
    if not hits:
        continue
    banded += 1
    sc = [h for h in hits if h[2]]
    if sc:
        scaled_banded += 1
        docs.append((p, len(hits), len(sc)))

print("xlsx-family sheets documents:              %d" % ooxml)
print("  ... with at least one banded worksheet:  %d" % banded)
print("  ... banded AND scaled on some worksheet: %d" % scaled_banded)
print()
for p, n, s in sorted(docs):
    print("  %2d/%2d banded-scaled  %s" % (s, n, p))
