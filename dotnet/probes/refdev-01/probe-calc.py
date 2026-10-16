#!/usr/bin/env python3
"""Calc's ascent and line height for 195 (face, size) pairs, and which device fits.

One printed page per (face, size): a sheet whose rows each carry a manual page break, so a page
holds exactly one row, and that row's cell holds two paragraphs. The row is top-aligned, its
padding is zero and its height is fixed, so the first baseline's distance below the print area's
top *is* the ascent and the gap between the two baselines *is* the line height. Read out of the
PDF's own text matrices at full precision.

A cell holding two paragraphs is an `EditCell` and formats through EditEngine against Calc's own
reference device, which is the question. A single-line cell takes `ScOutputData::LayoutStrings`
instead — `PROBE_MODE=plain` measures that one, one line per page, ascent only.

    probe-calc.py <outdir>                     measure the reference and score every candidate
    PROBE_SET=extra PROBE_MODE=plain ...
"""
import math
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pdftext import rows                                              # noqa: E402
from probe_faces import FACES, MET, SIZES, rnd, size_mm100            # noqa: E402

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)
MODE = os.environ.get("PROBE_MODE", "rich")
SET = os.environ.get("PROBE_SET", "core")
LINES = 1 if MODE == "plain" else 2

MARGIN_MM100 = 1000          # fo:margin-top="1cm"
MM100 = 2540.0 / 72.0


# ------------------------------------------------------------------ the models

def device(face, pt, dpi, per_inch, group):
    upem, a, d, _g = MET[face]
    em_logical = size_mm100(pt) if per_inch == 2540 else rnd(pt * 20)
    px = rnd(em_logical * dpi / per_inch)
    ap = rnd(a * px / upem)
    dp = rnd(d * px / upem)
    asc = rnd(ap * per_inch / dpi)
    dsc = rnd(dp * per_inch / dpi)
    if group == "split":
        return asc, asc + dsc
    if group == "sum":
        return asc, rnd((ap + dp) * per_inch / dpi)
    return asc, max(asc + dsc, rnd((ap + dp) * per_inch / dpi))


def exact(face, pt):
    """What the tree does today: scale exactly, no device."""
    upem, a, d, _g = MET[face]
    em = pt * 2540.0 / 72.0
    return rnd(a * em / upem), rnd((a + d) * em / upem)


MODELS = {
    "600/mm100 max": lambda f, p: device(f, p, 600, 2540, "max"),
    "720/mm100 max": lambda f, p: device(f, p, 720, 2540, "max"),
    "720/mm100 split": lambda f, p: device(f, p, 720, 2540, "split"),
    "8640/mm100 max": lambda f, p: device(f, p, 8640, 2540, "max"),
    "8640/mm100 split": lambda f, p: device(f, p, 8640, 2540, "split"),
    "8640/mm100 sum": lambda f, p: device(f, p, 8640, 2540, "sum"),
    "96/mm100 max": lambda f, p: device(f, p, 96, 2540, "max"),
    "exact (tree)": exact,
}


# ------------------------------------------------------------------ the document

HEAD = '''<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
<office:font-face-decls>
%s
</office:font-face-decls>
<office:automatic-styles>
 <style:page-layout style:name="PM1">
  <style:page-layout-properties fo:page-width="21cm" fo:page-height="29.7cm"
    fo:margin-top="1cm" fo:margin-bottom="1cm" fo:margin-left="1cm" fo:margin-right="1cm"
    style:print-orientation="portrait" style:writing-mode="lr-tb" style:scale-to="100%%"/>
  <style:header-style/>
  <style:footer-style/>
 </style:page-layout>
 <style:style style:name="co1" style:family="table-column">
  <style:table-column-properties style:column-width="18cm"/></style:style>
 <style:style style:name="ta1" style:family="table" style:master-page-name="Default">
  <style:table-properties table:display="true"/></style:style>
%s
</office:automatic-styles>
<office:master-styles>
 <style:master-page style:name="Default" style:page-layout-name="PM1">
  <style:header style:display="false"/><style:footer style:display="false"/>
  <style:header-left style:display="false"/><style:footer-left style:display="false"/>
 </style:master-page>
</office:master-styles>
<office:body><office:spreadsheet>
 <table:table table:name="S" table:style-name="ta1">
  <table:table-column table:style-name="co1"/>
%s
 </table:table>
</office:spreadsheet></office:body>
</office:document>
'''

ROWSTYLE = ''' <style:style style:name="ro%d" style:family="table-row">
  <style:table-row-properties style:row-height="3cm" style:use-optimal-row-height="false"%s/>
 </style:style>
 <style:style style:name="ce%d" style:family="table-cell">
  <style:table-cell-properties style:vertical-align="top" fo:padding="0cm"
    fo:wrap-option="wrap" style:text-align-source="fix" fo:border="none"/>
  <style:text-properties style:font-name="%s" fo:font-size="%spt"%s
    style:font-name-asian="%s" style:font-size-asian="%spt"
    style:font-name-complex="%s" style:font-size-complex="%spt"/>
 </style:style>
'''

ROW = '''  <table:table-row table:style-name="ro%d">
   <table:table-cell table:style-name="ce%d" office:value-type="string">%s</table:table-cell>
  </table:table-row>
'''

FONTFACE = ' <style:font-face style:name="%s" svg:font-family="&apos;%s&apos;"/>\n'


def build(path):
    styles, body, key = [], [], []
    n = 0
    for label, family, cut, _file in FACES[SET]:
        for pt in SIZES:
            weight = ''
            if cut == 'bold':
                weight = ' fo:font-weight="bold" style:font-weight-asian="bold"'
            elif cut == 'italic':
                weight = ' fo:font-style="italic" style:font-style-asian="italic"'
            n += 1
            s = '%g' % pt
            brk = '' if n == 1 else ' fo:break-before="page"'
            styles.append(ROWSTYLE % (n, brk, n, family, s, weight, family, s, family, s))
            ps = ''.join('<text:p>Hxy%d</text:p>' % i for i in range(LINES))
            body.append(ROW % (n, n, ps))
            key.append((label, pt))
    decls = ''.join(FONTFACE % (f[1], f[1]) for f in {g[1]: g for g in FACES[SET]}.values())
    open(path, 'w').write(HEAD % (decls, ''.join(styles), ''.join(body)))
    return key


def render(src, who):
    d = os.path.join(OUT, who)
    os.makedirs(d, exist_ok=True)
    pdf = os.path.join(d, os.path.splitext(os.path.basename(src))[0] + '.pdf')
    if os.path.exists(pdf):
        return pdf
    if who == 'ours':
        subprocess.run([os.environ['XRAY_OFFICE_CLI'], "render", src, "--format", "pdf",
                        "--outdir", d], capture_output=True)
    else:
        subprocess.run(["soffice", "-env:UserInstallation=file://" + os.path.abspath(OUT) + "/prof",
                        "--headless", "--convert-to", "pdf", "--outdir", d, src],
                       capture_output=True)
    return pdf


def measure(pdf, key):
    by_page = {}
    for page, _x, y, _sz, _f, _n in rows(pdf):
        by_page.setdefault(page, []).append(y)
    got = {}
    for i, k in enumerate(key, start=1):
        ys = sorted(by_page.get(i, []))
        if len(ys) != LINES:
            continue
        got[k] = (round(ys[0] * MM100) - MARGIN_MM100,
                  round((ys[-1] - ys[0]) * MM100) if LINES > 1 else 0)
    return got


src = os.path.join(OUT, 'calc-%s-%s.fods' % (SET, MODE))
key = build(src)
ref = measure(render(src, 'ref'), key)
ours = measure(render(src, 'ours'), key) if os.environ.get('XRAY_OFFICE_CLI') else {}

print('%18s %5s | %8s %6s | %s' % ('face', 'pt', 'ref asc', 'ref h',
                                   '  '.join('%s' % m for m in MODELS)))
score = {m: [0, 0] for m in MODELS}
n = 0
for k in key:
    if k not in ref:
        continue
    ra, rh = ref[k]
    n += 1
    cells = []
    for m, fn in MODELS.items():
        ma, mh = fn(k[0], k[1])
        score[m][0] += (ma == ra)
        score[m][1] += (mh == rh)
        cells.append('%+d/%+d' % (ma - ra, mh - rh))
    print('%18s %5.1f | %8d %6d | %s' % (k[0], k[1], ra, rh, '  '.join(cells)))

if ours:
    oa = sum(1 for k in ref if k in ours and ours[k][0] == ref[k][0])
    oh = sum(1 for k in ref if k in ours and ours[k][1] == ref[k][1])
    print('  %-18s ascent %3d/%d   line height %3d/%d'
          % ('TREE', oa, len(ref), oh, len(ref)))

print('\nmeasured %d of %d pairs, mode=%s' % (n, len(key), MODE))
for m in MODELS:
    print('  %-18s ascent %3d/%d   line height %3d/%d' % (m, score[m][0], n, score[m][1], n))
