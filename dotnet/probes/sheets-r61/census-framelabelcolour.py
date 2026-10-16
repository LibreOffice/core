#!/usr/bin/env python3
"""The two divergences round 61's blind readers found, sized over all 946 manifest documents.

  A. **the chart area's default border.** 26.2.4.2 draws a light-grey `#D9D9D9` rectangle round
     a chart whose `c:chartSpace` states no `c:spPr` at all; we draw one only when the file
     states a line. Measured on `005_Contextures_chart_sample_6e279b08` (3 charts, reference 3
     `#D9D9D9` strokes, ours 0) and `microsoft_learn_multi_chart_examples` (3 and 0), against
     the four `advanced_excel_pie` documents, whose charts *do* state a `c:spPr` and where both
     sides draw exactly one. So the test is: a chart part with no `c:chartSpace`-level `c:spPr`.

  B. **a data label's stated text colour.** `c:dLbls/c:txPr//a:defRPr/a:solidFill` — `005`'s pie
     states `<a:schemeClr val="bg1"/>`, the reference draws the labels white and we draw them
     black. The test is: a `c:dLbls` (or per-point `c:dLbl`) carrying a `c:txPr` with a
     `a:solidFill` anywhere inside it.

Counts *documents*, case-folded where it accumulates, and refuses to summarise unless every
manifest row produced an answer. BIFF is not decoded and is reported separately, as an unread
remainder rather than a zero.
"""
import collections
import os
import re
import sys
import zipfile

CORPUS = "/c/sandbox/workdir/sample-files"
CHART_PART = re.compile(r"charts?/chart\d*\.xml$", re.I)
# The chartSpace-level spPr is the one that is a direct child of c:chartSpace: it follows the
# close of </c:chart> (or of </c:externalData>, which never precedes it) rather than sitting
# inside a plot area or a series.
SPPR_AFTER_CHART = re.compile(r"</(?:[A-Za-z0-9_.-]+:)?chart>\s*<(?:[A-Za-z0-9_.-]+:)?spPr[ >]")
DLBLS = re.compile(r"<(?:[A-Za-z0-9_.-]+:)?dLbls[ >].*?</(?:[A-Za-z0-9_.-]+:)?dLbls>", re.S)
TXPR = re.compile(r"<(?:[A-Za-z0-9_.-]+:)?txPr[ >].*?</(?:[A-Za-z0-9_.-]+:)?txPr>", re.S)
SOLIDFILL = re.compile(r"<a:solidFill[ >]")

OOXML = ("xlsx", "xlsm", "xltx", "xltm", "docx", "docm", "dotx", "dotm",
         "pptx", "pptm", "potx", "potm", "ppsx", "ppsm")
BIFF = ("xls", "xlt", "xlsb", "ppt", "pot", "pps", "doc", "dot")


def main():
    rows = []
    with open(os.path.join(CORPUS, "MANIFEST.tsv"), encoding="utf-8") as fh:
        fh.readline()
        for line in fh:
            f = line.rstrip("\n").split("\t")
            rows.append((f[0], f[2], f[3], f[7]))

    noframe = collections.Counter()
    noframe_parts = collections.Counter()
    coloured = collections.Counter()
    coloured_parts = collections.Counter()
    biff = collections.Counter()
    hits = collections.defaultdict(list)
    unread, seen = [], set()

    for family, path, ext, status in rows:
        key = path.lower()
        if key in seen:
            continue
        seen.add(key)
        full = os.path.join(CORPUS, path)
        if not os.path.exists(full):
            unread.append(path)
            continue
        e = ext.lower()
        if e in BIFF:
            biff[family] += 1
            continue
        if e not in OOXML:
            continue
        try:
            nf = nc = 0
            with zipfile.ZipFile(full) as z:
                for n in z.namelist():
                    if not CHART_PART.search(n):
                        continue
                    x = z.read(n).decode("utf-8", "replace")
                    if not SPPR_AFTER_CHART.search(x):
                        nf += 1
                    if any(SOLIDFILL.search(t) for block in DLBLS.findall(x)
                           for t in TXPR.findall(block)):
                        nc += 1
        except Exception as exc:                                  # noqa: BLE001
            unread.append("%s (%s)" % (path, exc))
            continue
        if nf:
            noframe[family] += 1
            noframe_parts[family] += nf
            hits[("frame", family)].append((status, path, nf))
        if nc:
            coloured[family] += 1
            coloured_parts[family] += nc
            hits[("colour", family)].append((status, path, nc))

    if unread:
        print("REFUSING TO SUMMARISE — %d manifest rows could not be read:" % len(unread),
              file=sys.stderr)
        for u in unread[:20]:
            print("   ", u, file=sys.stderr)
        sys.exit(2)

    print("distinct manifest paths read: %d\n" % len(seen))
    print("%-8s %-34s %-34s %s" % ("family", "A: chart with no chartSpace spPr",
                                   "B: dLbls stating a text colour", "BIFF (undecoded)"))
    for family in ("sheets", "slides", "words"):
        print("%-8s %3d documents / %4d parts%14s %3d documents / %4d parts%9s %d"
              % (family, noframe[family], noframe_parts[family], "",
                 coloured[family], coloured_parts[family], "", biff[family]))
    for kind in ("frame", "colour"):
        print("\n%s — sheets, by manifest status:" % kind)
        c = collections.Counter(s for s, _, _ in hits[(kind, "sheets")])
        for k, v in c.most_common():
            print("   %-8s %d" % (k, v))


if __name__ == "__main__":
    main()
