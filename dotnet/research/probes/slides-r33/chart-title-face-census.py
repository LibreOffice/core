#!/usr/bin/env python3
"""How many chart parts name a face on their title that the chart space does not.

`ChartPlot.TextFamily` is one family for a whole chart and is right for its axis labels,
its legend and its data labels.  A *title* is authored separately, and OOXML lets it say
so — which is what `ChartPlot.TitleFamily` now carries.  This counts the parts where the
two disagree, so the change's ceiling is a number rather than an impression.

It reads the OOXML half of whatever tracks it is pointed at.  A `.ppt`, `.xls` or `.doc`
states a chart's faces somewhere no zip-level census can see, so the count is a ceiling
**over the formats it can read**; the row it prints says how many documents that was.

    chart-title-face-census.py /workspace/sample-files/slides
    chart-title-face-census.py /workspace/sample-files
"""
import re
import sys
import zipfile
from pathlib import Path

DRAWINGML = "http://schemas.openxmlformats.org/drawingml/2006/main"
CHART = "http://schemas.openxmlformats.org/drawingml/2006/chart"

OOXML = {".pptx", ".pptm", ".potx", ".potm", ".ppsx", ".ppsm",
         ".xlsx", ".xlsm", ".xltx", ".xltm", ".docx", ".docm", ".dotx", ".dotm"}


def literal_face(fragment):
    """The first literal a:latin/@typeface in a fragment of markup, or None."""
    for match in re.finditer(r'<a:latin[^>]*typeface="([^"]*)"', fragment):
        face = match.group(1)
        if face and not face.startswith('+'):
            return face
    return None


def space_face(body):
    """The chart space's own statement: its direct c:txPr, else anything literal in it.

    `c:txPr` is a *child* of `c:chartSpace` and follows `c:chart` in CT_ChartSpace's order,
    so it has to be looked for after `</c:chart>` — searching before it finds the title's
    own `c:txPr` instead, which is exactly the confusion this census exists to measure.
    """
    tail = body.split('</c:chart>')[-1]
    txpr = re.search(r'<c:txPr>.*?</c:txPr>', tail, re.S)
    if txpr and (face := literal_face(txpr.group(0))):
        return face
    return literal_face(body)


def title_face(body):
    """The main title's own face — c:chart's direct c:title, not an axis'."""
    chart = re.search(r'<c:chart[ >].*?</c:chart>', body, re.S)
    if not chart:
        return None
    title = re.search(r'<c:title>.*?</c:title>', chart.group(0), re.S)
    return literal_face(title.group(0)) if title else None


def main(argv):
    roots = [Path(p) for p in argv] or [Path("/workspace/sample-files")]
    parts = documents = disagree = disagree_docs = 0
    readable = 0

    for root in roots:
        for path in sorted(root.rglob("*")):
            if not path.is_file() or path.suffix.lower() not in OOXML:
                continue
            readable += 1
            try:
                archive = zipfile.ZipFile(path)
            except Exception:
                continue
            names = [n for n in archive.namelist()
                     if re.search(r'charts?/chart\d*\.xml$', n)]
            if not names:
                continue
            documents += 1
            hit = False
            for name in names:
                try:
                    body = archive.read(name).decode("utf-8", "replace")
                except Exception:
                    continue
                parts += 1
                title, space = title_face(body), space_face(body)
                if title and title != space:
                    disagree += 1
                    hit = True
                    print("%s\t%s\ttitle %s vs chart space %s"
                          % (path.name, name.split('/')[-1], title, space or "-"))
            if hit:
                disagree_docs += 1

    print()
    print("OOXML documents read           %d" % readable)
    print("documents holding chart parts  %d" % documents)
    print("chart parts                    %d" % parts)
    print("parts whose title disagrees    %d  over %d documents"
          % (disagree, disagree_docs))


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
