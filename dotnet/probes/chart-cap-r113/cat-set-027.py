#!/usr/bin/env python3
"""O51 -- which rectangle the interval cap's NUMERATOR is taken against.

`estimateMaximumAutoMainIncrementCount` divides `nTotalAvailable` -- the axis' main line at
the moment the *second* `doAutoScaling` runs -- by `m_nMaximumTextWidthSoFar`. Round 112
pinned the denominator (`MaxLabelTickIter`, the value axis' first three labels) and bracketed
the numerator at [125.4, 138) pt where the drawn axis is 115.7. This instrument tests the
numerator.

Reading `ChartView::impl_createDiagramAndContent`:559-604 by hand: the numerator is the inner
rectangle `adjustInnerSize(aConsumedOuterRect)` produces, and `aConsumedOuterRect` is the
bounding box of the diagram **plus the maximum labels** -- which `createMaximumLabels` builds
through the same `MaxLabelTickIter`, on the CROSSING axis too, overlap-allowed, line-break
forbidden and never auto-rotated (`canAutoAdjustLabelPlacement` returns false the moment
`m_bOverlapAllowed` is set, `VCartesianAxis.cxx`:539-556).

On `027`'s savings chart the crossing axis is the vertical category axis, five categories:

    idx 0 Other 1   1 Other 2   2 Cash Reserves   3 Savings/Investment   4 401(k)/Etc

`MaxLabelTickIter` seeds itself with the longest label's index, RESETS it to 0 when that is
the last or the last-but-one, then holds three consecutive indices. The longest here is
index 3 = nMaxIndex-1, so it resets: the set is {0, 1, 2} and the widest of it is
**Cash Reserves**, not Savings/Investment.

So widening index 3 or index 4 must change NOTHING about the value axis' interval, however
much the plot area shrinks, and widening index 0 or 2 must coarsen it. That is round 112's
experiment run on the axis at right angles to its own.

The categories are taken from the chart part's own `c:strCache` -- this chart is a pivot
chart and Calc does not relink it to the cells, measured: rewriting `xl/worksheets/sheet48.xml`
cell AA13 into an inline string moves nothing at all, and rewriting the cache moves everything.

    cat-set-027.py <outdir> [pad-sweep-max]
    soffice --headless --convert-to pdf --outdir <outdir>/ref <outdir>/027_*.xlsx
    cat-set-027.py --read <outdir>/ref
"""
import glob
import os
import sys
import zipfile
from pathlib import Path

SRC = ("/home/user/sample-files/sheets/chartset-014/xlsx/"
       "027_Simple_personal_cash_flow_statement_675c6584.xlsx")
CHART = "xl/charts/chart44.xml"

BASE = ["Other 1", "Other 2", "Cash Reserves", "Savings/Investment", "401(k)/Etc"]


def variant(out, name, idx, text):
    with zipfile.ZipFile(SRC) as zin:
        chart = zin.read(CHART).decode("utf-8")
        old = "<c:v>%s</c:v>" % BASE[idx]
        assert chart.count(old) == 1, "category %d is not unique in the part" % idx
        chart = chart.replace(old, "<c:v>%s</c:v>" % text)
        path = out / ("027_%s.xlsx" % name)
        with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zout:
            for item in zin.infolist():
                data = zin.read(item.filename)
                if item.filename == CHART:
                    data = chart.encode("utf-8")
                zout.writestr(item.filename, data)
    return path


def build(out, steps):
    out.mkdir(parents=True, exist_ok=True)
    print(variant(out, "c0", 2, BASE[2]))                       # the control
    for idx in (0, 1, 2, 3, 4):                                 # one index at a time
        print(variant(out, "w8i%d" % idx, idx, BASE[idx] + "W" * 8))
    for k in range(1, steps + 1):                               # the fine sweep on index 2
        print(variant(out, "l%02d" % k, 2, BASE[2] + "l" * k))


def read(refdir):
    """One row per variant: the value axis' labels and the widest maximum-set category."""
    import pymupdf
    print("variant\tlabels\tstep\twidest012\tcat3\taxis_pt\tvalue_w")
    for path in sorted(glob.glob(os.path.join(refdir, "027_*.pdf"))):
        name = os.path.basename(path)[4:-4]
        with pymupdf.open(path) as doc:
            page = doc[5]
            money, cats = [], []
            for block in page.get_text("dict")["blocks"]:
                if block["type"] != 0:
                    continue
                for line in block["lines"]:
                    for span in line["spans"]:
                        x0, _, x1, _ = span["bbox"]
                        if x0 < 580 or span["size"] > 6:
                            continue
                        text = span["text"].strip()
                        (money if text.startswith("$") else cats).append((x0, x1, text))
            money.sort()
            widest = max((x1 - x0 for x0, x1, t in cats
                          if t.startswith(("Other 1", "Other 2", "Cash Reserves"))), default=0.0)
            cat3 = max((x1 - x0 for x0, x1, t in cats
                        if t.startswith("Savings/Investment")), default=0.0)
            # the widest of the first three VALUE labels, which is the denominator
            vw = max((x1 - x0 for x0, x1, t in money[:3]), default=0.0)
            bars = [d["rect"] for d in page.get_drawings()
                    if d["type"] == "f" and d["rect"].x0 > 580
                    and 150 < d["rect"].y0 < 360 and 14 < d["rect"].height < 20
                    and d["rect"].width > 0.5]
            axis = max((r.width for r in bars), default=0.0) / 12000.0
            step = "-"
            if len(money) >= 2:
                step = money[1][2]
            print("%s\t%d\t%s\t%.2f\t%.2f\t%.2f\t%.2f"
                  % (name, len(money), step, widest, cat3,
                     axis * _axis_max(money), vw))


def _axis_max(money):
    """The axis maximum, read off its own last label."""
    if not money:
        return 0.0
    return float(money[-1][2].replace("$", "").replace(",", ""))


if __name__ == "__main__":
    if sys.argv[1] == "--read":
        read(sys.argv[2])
    else:
        build(Path(sys.argv[1]), int(sys.argv[2]) if len(sys.argv) > 2 else 12)
