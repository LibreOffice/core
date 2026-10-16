#!/usr/bin/env python3
"""Census OOXML chart-type elements across the sample-files corpus.
One row per (document, chart-type-element) pair. Reads only zip members whose
name matches charts/chart*.xml or charts/chartEx*.xml."""
import csv, re, sys, zipfile, os

ROOT = "/home/user/sample-files"
MAN = os.path.join(ROOT, "MANIFEST.tsv")

C_TYPES = ["area3DChart","areaChart","bar3DChart","barChart","bubbleChart",
           "line3DChart","lineChart","stockChart","ofPieChart","doughnutChart",
           "pie3DChart","pieChart","radarChart","scatterChart","surface3DChart",
           "surfaceChart"]
CX_TYPES = ["boxWhisker","clusteredColumn","funnel","paretoLine","regionMap",
            "sunburst","treemap","waterfall"]

c_re  = {t: re.compile((r"<(?:[A-Za-z0-9_]+:)?%s[ />\r\n\t]" % t).encode()) for t in C_TYPES}
cx_re = {t: re.compile((r'layoutId="%s"' % t).encode()) for t in CX_TYPES}
cx_re2= {t: re.compile((r"<(?:[A-Za-z0-9_]+:)?%s[ />\r\n\t]" % t).encode()) for t in CX_TYPES}
# radar/bar sub-discriminators
sub_re = {
    "radarStyle_filled": re.compile(rb'<(?:[A-Za-z0-9_]+:)?radarStyle[^>]*val="filled"'),
    "barDir_bar":        re.compile(rb'<(?:[A-Za-z0-9_]+:)?barDir[^>]*val="bar"'),
    "grouping_stacked":  re.compile(rb'<(?:[A-Za-z0-9_]+:)?grouping[^>]*val="stacked"'),
    "grouping_pctstk":   re.compile(rb'<(?:[A-Za-z0-9_]+:)?grouping[^>]*val="percentStacked"'),
    "ofPieType_bar":     re.compile(rb'<(?:[A-Za-z0-9_]+:)?ofPieType[^>]*val="bar"'),
    "scatterStyle_line": re.compile(rb'<(?:[A-Za-z0-9_]+:)?scatterStyle[^>]*val="(line|lineMarker|smooth|smoothMarker)"'),
    "holeSize":          re.compile(rb'<(?:[A-Za-z0-9_]+:)?holeSize'),
    "smooth_1":          re.compile(rb'<(?:[A-Za-z0-9_]+:)?smooth[^>]*val="1"'),
    "wireframe_1":       re.compile(rb'<(?:[A-Za-z0-9_]+:)?wireframe[^>]*val="1"'),
    "upDownBars":        re.compile(rb'<(?:[A-Za-z0-9_]+:)?upDownBars'),
    "binning":           re.compile(rb'<(?:[A-Za-z0-9_]+:)?binning'),
}

part_re = re.compile(r"(^|/)chart(Ex)?[0-9]*\.xml$", re.I)

out = csv.writer(sys.stdout, delimiter="\t", lineterminator="\n")
out.writerow(["path","ext","family","batch","kind","part","feature"])

with open(MAN, newline="") as f:
    rows = list(csv.DictReader(f, delimiter="\t"))

for r in rows:
    ext = r["ext"].lower()
    if ext not in ("docx","xlsx","pptx","xlsm"): continue
    p = os.path.join(ROOT, r["path"])
    if not os.path.exists(p): 
        sys.stderr.write("MISSING %s\n" % p); continue
    try:
        z = zipfile.ZipFile(p)
        names = z.namelist()
    except Exception as e:
        sys.stderr.write("BADZIP %s %s\n" % (p, e)); continue
    for n in names:
        if not part_re.search(n): continue
        try: data = z.read(n)
        except Exception as e:
            sys.stderr.write("BADPART %s %s %s\n" % (p, n, e)); continue
        feats = []
        for t, rx in c_re.items():
            if rx.search(data): feats.append("c:"+t)
        for t in CX_TYPES:
            if cx_re[t].search(data) or cx_re2[t].search(data): feats.append("cx:"+t)
        for k, rx in sub_re.items():
            if rx.search(data): feats.append("@"+k)
        if not feats: feats = ["(none)"]
        for ft in feats:
            out.writerow([r["path"], ext, r["family"], r["batch"], r["kind"], n, ft])
    z.close()
