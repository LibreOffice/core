#!/usr/bin/env python3
"""How many corpus charts state a data label that draws a CATEGORY NAME at all.

A label draws one two ways: a `c:dLbl`/`c:dLbls` with `<c:showCatName val="1"/>`, and a rich
custom label holding an `<a:fld type="CATEGORYNAME">`. Only a *numeric* category can take a
number format, so each is reported again split by whether the chart's category axis is a
`c:dateAx` and by whether the `c:cat` cache is a `numCache`/`numRef`.

The census is over the zip formats it can read and says so: a `.xls` chart lives in a BIFF
substream and no zip walk sees it. `probes/chart-fit/census.tsv` is the chart-bearing set and
its own `biff` column is the part this cannot reach.

    catname-census.py /home/user/sample-files
"""
import re
import sys
import zipfile
from pathlib import Path

CHART = re.compile(r"(xl|ppt|word)/charts/chart\d*\.xml$")


def main(root):
    docs = 0
    charts = 0
    hit_docs, hit_charts = set(), 0
    fld_docs, fld_charts = set(), 0
    num_docs, num_charts = set(), 0
    date_docs, date_charts = set(), 0
    rows = []

    for path in sorted(Path(root).rglob("*")):
        if not path.is_file() or path.suffix.lower() not in (
                ".xlsx", ".xlsm", ".pptx", ".docx", ".pptm", ".docm"):
            continue
        try:
            with zipfile.ZipFile(path) as z:
                parts = [n for n in z.namelist() if CHART.search(n)]
                if not parts:
                    continue
                docs += 1
                here = {"show": 0, "fld": 0, "num": 0, "date": 0}
                for name in parts:
                    charts += 1
                    x = z.read(name).decode("utf-8", "replace")
                    show = x.count('<c:showCatName val="1"/>')
                    fld = x.count('type="CATEGORYNAME"')
                    if not (show or fld):
                        continue
                    hit_charts += 1
                    here["show"] += bool(show)
                    here["fld"] += fld
                    date = "<c:dateAx>" in x
                    # a numeric CATEGORY cache is what a number format can reach; the values
                    # cache is a numCache too, so the element has to be looked for inside the
                    # <c:cat> it belongs to rather than anywhere in the part
                    numeric = any(
                        "<c:numRef>" in c or "<c:numLit>" in c
                        for c in re.findall(r"<c:cat>.*?</c:cat>", x, re.S))
                    if date:
                        date_charts += 1
                        here["date"] += 1
                    if numeric:
                        num_charts += 1
                        here["num"] += 1
                if here["show"] or here["fld"]:
                    hit_docs.add(str(path))
                    if here["fld"]:
                        fld_docs.add(str(path))
                        fld_charts += 0
                    if here["num"]:
                        num_docs.add(str(path))
                    if here["date"]:
                        date_docs.add(str(path))
                    rows.append((str(path.relative_to(root)), here["show"], here["fld"],
                                 here["num"], here["date"]))
        except (zipfile.BadZipFile, OSError):
            continue

    print("zip documents holding a chart part\t%d" % docs)
    print("chart parts\t%d" % charts)
    print("parts stating a category-name label\t%d" % hit_charts)
    print("  of those, over a c:dateAx\t%d" % date_charts)
    print("  of those, over a numeric c:cat cache\t%d" % num_charts)
    print("documents\t%d" % len(hit_docs))
    print("  with a CATEGORYNAME field\t%d" % len(fld_docs))
    print("  with a date axis\t%d" % len(date_docs))
    print("  with a numeric cache\t%d" % len(num_docs))
    print()
    print("path\tshowCatName\tCATEGORYNAME\tnumeric\tdateAx")
    for r in sorted(rows):
        print("%s\t%d\t%d\t%d\t%d" % r)


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "/home/user/sample-files")
