#!/usr/bin/env python3
"""O58 -- how much room a 45-degree value label takes at the FAR end of its axis.

`027_Simple_personal_cash_flow_statement`'s savings chart (`xl/charts/chart44.xml`) is a
bar chart: category axis down the left, value axis along the bottom, and 26.2.4.2 turns the
value labels 45 degrees because they collide.  This rewrites ONE thing at a time in that
chart and reads the drawn geometry back out of 26.2.4.2's own PDF.

Two knobs:

  * `fmt`  -- the value axis' number format, which changes the LABEL WIDTH and nothing else
              once the scale is pinned;
  * `rot`  -- a stated `a:bodyPr/@rot` on the value axis' `c:txPr`, which changes the ANGLE.

The scale is pinned (`c:min` 0, `c:max` 14000, `c:majorUnit` 2000) in every variant so the
tick set never moves and the automatic-increment cap cannot enter.

The instrument is the chart's own longest bar.  Its value is 12000 of a pinned 14000, so the
plot's width is `bar * 14/12` whatever else moved, and it is read off the filled path rather
than off any label.  Everything is then divided by the DRAWN font size, because an embedded
chart is fitted to its own drawn extent and that fit is a free scale factor
(`ViewContactOfSdrOle2Obj::createPrimitive2DSequenceWithParameters`); size-relative figures
are the only ones comparable between variants.

    turn027.py write <dir> [spec ...]     spec = fmt|rot|<value>
    turn027.py read  <dir>                reads <dir>/ref/*.pdf
"""
import re
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

SRC = ("/home/user/sample-files/sheets/chartset-014/xlsx/"
       "027_Simple_personal_cash_flow_statement_675c6584.xlsx")
CHART = "xl/charts/chart44.xml"
PAGE = 5                       # the savings chart's page, 0-based
PLOT_LEFT_VALUE = 12000.0      # the longest bar's value
PINNED_MAX = 14000.0

FORMATS = {
    "base":  '&quot;$&quot;#,##0',
    "w0":    '&quot;$&quot;#,##0&quot;&quot;',
    "w2":    '&quot;$&quot;#,##0&quot;WW&quot;',
    "w4":    '&quot;$&quot;#,##0&quot;WWWW&quot;',
    "w6":    '&quot;$&quot;#,##0&quot;WWWWWW&quot;',
    "n2":    '&quot;$&quot;#,##0&quot;nn&quot;',
    "n4":    '&quot;$&quot;#,##0&quot;nnnn&quot;',
    "none":  None,          # tick labels turned off entirely -- the availableRight probe
}


def rewrite(xml, fmt, rot, sz=None, mu=2000):
    m = re.search(r"<c:valAx>.*?</c:valAx>", xml, re.S)
    ax = m.group(0)

    ax2 = ax.replace(
        '<c:scaling><c:orientation val="minMax"/></c:scaling>',
        '<c:scaling><c:orientation val="minMax"/>'
        '<c:max val="%g"/><c:min val="0"/></c:scaling>' % PINNED_MAX)
    if fmt is None:
        ax2 = ax2.replace('<c:tickLblPos val="nextTo"/>', '<c:tickLblPos val="none"/>')
    else:
        ax2 = re.sub(r'<c:numFmt formatCode="[^"]*" sourceLinked="1"/>',
                     '<c:numFmt formatCode="%s" sourceLinked="0"/>' % fmt, ax2)
    ax2 = re.sub(r'(<a:bodyPr )rot="-?\d+"', r'\1rot="%d"' % rot, ax2)
    # a pinned major unit, appended where the schema wants it
    if sz:
        ax2 = re.sub(r'<a:defRPr sz="\d+"', '<a:defRPr sz="%d"' % sz, ax2)
    ax2 = ax2.replace('<c:crossBetween val="between"/>',
                      '<c:crossBetween val="between"/><c:majorUnit val="%d"/>' % mu)
    assert ax2 != ax
    return xml.replace(ax, ax2)


def write(out, specs):
    out = Path(out)
    (out / "src").mkdir(parents=True, exist_ok=True)
    for spec in specs:
        parts = spec.split("|")
        fmt, rot = parts[0], parts[1]
        sz = int(parts[2]) if len(parts) > 2 and parts[2] else None
        mu = int(parts[3]) if len(parts) > 3 else 2000
        name = "%s_%s_%s_%d" % (fmt, rot, sz or "d", mu)
        dst = out / "src" / ("v_%s.xlsx" % name)
        with zipfile.ZipFile(SRC) as z:
            xml = z.read(CHART).decode("utf-8")
            new = rewrite(xml, FORMATS[fmt], int(rot), sz, mu)
            with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as o:
                for info in z.infolist():
                    data = z.read(info.filename)
                    if info.filename == CHART:
                        data = new.encode("utf-8")
                    o.writestr(info, data)
        print("wrote", dst)


def read(out):
    import pymupdf
    rows = []
    for pdf in sorted(Path(out, "ref").glob("*.pdf")):
        d = pymupdf.open(pdf)
        p = d[PAGE]
        left = bars = None
        for dr in p.get_drawings():
            r = dr["rect"]
            if dr["type"] != "f" or r.y0 < 160 or r.y1 > 340 or r.x1 < 650:
                continue
            if r.width > (bars or 0):
                bars, left = r.width, r.x0
        labels = []
        for b in p.get_text("rawdict")["blocks"]:
            for l in b.get("lines", []):
                for s in l["spans"]:
                    t = "".join(c["c"] for c in s["chars"]).strip()
                    if not t.startswith("$") or s["bbox"][0] < 600:
                        continue
                    labels.append((t, s["size"], s["bbox"], l["dir"]))
        if bars is None or not labels:
            print(pdf.name, "NO GEOMETRY")
            continue
        size = labels[0][1]
        width = bars * PINNED_MAX / PLOT_LEFT_VALUE
        last = max(labels, key=lambda x: x[2][2])
        rows.append(dict(
            name=pdf.stem, size=size, plotleft=left, plotwidth=width,
            plotright=left + width, labels=len(labels),
            lastlabel=last[0], lastright=last[2][2],
            lastink=last[2][2] - last[2][0], dir=last[3],
            overhang=last[2][2] - (left + width)))
    hdr = ("variant size plotleft plotwidth plotright labels last lastink "
           "lastright overhang over/size width/size")
    print("\t".join(hdr.split()))
    for r in rows:
        print("%s\t%.3f\t%.2f\t%.3f\t%.2f\t%d\t%s\t%.3f\t%.2f\t%.3f\t%.4f\t%.4f" % (
            r["name"], r["size"], r["plotleft"], r["plotwidth"], r["plotright"],
            r["labels"], r["lastlabel"], r["lastink"], r["lastright"], r["overhang"],
            r["overhang"] / r["size"], r["plotwidth"] / r["size"]))


if __name__ == "__main__":
    if sys.argv[1] == "write":
        write(sys.argv[2], sys.argv[3:])
    else:
        read(sys.argv[2])
