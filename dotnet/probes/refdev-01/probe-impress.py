#!/usr/bin/env python3
"""Impress's ascent and line height for 195 (face, size) pairs, and which device fits.

One slide per (face, size) holding a six-line text box whose frame top is authored exactly, so
the first baseline's distance below the frame top *is* the ascent and the five equal gaps below
it *are* the line height. Both are read out of the PDF's own text matrices at full precision —
LibreOffice writes three decimals, which is a fortieth of the hundredth of a millimetre this
device works in.

The box asks for `style:font-independent-line-spacing="false"`, which is the ODF default and the
branch that consults the face's metrics at all; a PPTX text body sets the flag and gets
`ImplCalculateFontIndependentLineSpacing` instead, where no metric is read.

Candidate devices are scored side by side rather than one being assumed. Usage:

    probe-impress.py <outdir>              measure the reference and score every candidate
    PROBE_SET=extra probe-impress.py ...   the eight faces no prior round measured
"""
import math
import os
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pdftext import rows                                              # noqa: E402

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)

L = "/usr/share/fonts/truetype/liberation/"
X = "/usr/share/fonts/truetype/crosextra/"
D = "/usr/share/fonts/truetype/dejavu/"

SETS = {
    "core": [
        ("Liberation Serif", "Liberation Serif", "", L + "LiberationSerif-Regular.ttf"),
        ("Liberation Sans", "Liberation Sans", "", L + "LiberationSans-Regular.ttf"),
        ("Carlito", "Carlito", "", X + "Carlito-Regular.ttf"),
        ("Caladea", "Caladea", "", X + "Caladea-Regular.ttf"),
        ("DejaVu Sans", "DejaVu Sans", "", D + "DejaVuSans.ttf"),
    ],
    "extra": [
        ("Liberation Mono", "Liberation Mono", "", L + "LiberationMono-Regular.ttf"),
        ("Lib Serif Bold", "Liberation Serif", "bold", L + "LiberationSerif-Bold.ttf"),
        ("Lib Sans Italic", "Liberation Sans", "italic", L + "LiberationSans-Italic.ttf"),
        ("Caladea Bold", "Caladea", "bold", X + "Caladea-Bold.ttf"),
        ("DejaVu Serif", "DejaVu Serif", "", D + "DejaVuSerif.ttf"),
        ("DejaVu Sans Mono", "DejaVu Sans Mono", "", D + "DejaVuSansMono.ttf"),
        ("OpenSymbol", "OpenSymbol", "", "/usr/share/fonts/truetype/libreoffice/opens___.ttf"),
        ("IPAGothic", "IPAGothic", "", "/usr/share/fonts/truetype/fonts-japanese-gothic.ttf"),
    ],
}
SET = os.environ.get("PROBE_SET", "core")
FACES = SETS[SET]
SIZES = [h / 2.0 for h in range(10, 49)]          # 5.0 .. 24.0 pt in half points
LINES = 6
FRAME_TOP_MM100 = 1000                            # svg:y="1cm"


# ------------------------------------------------------------------ font tables

def metrics(path):
    """The three numbers `ImplCalcLineSpacing` believes, and the em they are in."""
    d = open(path, "rb").read()
    off = 0
    if d[:4] == b"ttcf":
        off, = struct.unpack(">I", d[12:16])
    num, = struct.unpack(">H", d[off + 4:off + 6])
    t = {}
    for i in range(num):
        rec = d[off + 12 + 16 * i: off + 12 + 16 * i + 16]
        o, ln = struct.unpack(">II", rec[8:16])
        t[rec[:4].decode("latin1")] = (o, ln)
    ho, _ = t["head"]
    upem, = struct.unpack(">H", d[ho + 18:ho + 20])
    hh, _ = t["hhea"]
    asc, desc, gap = struct.unpack(">hhh", d[hh + 4:hh + 10])
    a, dsc, g = asc, -desc, gap
    if "OS/2" in t:
        oo, _ = t["OS/2"]
        fs, = struct.unpack(">H", d[oo + 62:oo + 64])
        tA, tD, tG = struct.unpack(">hhh", d[oo + 68:oo + 74])
        if (fs >> 7) & 1 and tA >= 0 and tD <= 0:
            a, dsc, g = tA, -tD, tG
    return upem, a, dsc, g


MET = {f[0]: metrics(f[3]) for f in FACES}


# ------------------------------------------------------------------ the models

def rnd(x):
    """C++ std::round / llround: half away from zero."""
    return math.floor(x + 0.5) if x >= 0 else -math.floor(-x + 0.5)


def size_mm100(pt):
    """The em as the item set holds it. ODF's `fo:font-size` is converted once, on import."""
    return rnd(pt * 2540.0 / 72.0)


def device(label, face, pt, dpi, per_inch, group):
    """(ascent, line height) in the device's logical unit.

    `group` is how the ascent and descent are converted back: 'split' converts each on its own,
    which is what `OutputDevice::GetFontMetric` does (`vcl/source/outdev/font.cxx`:351-352), and
    'sum' converts the pair once, which is what `GetTextHeight` does and what Writer uses.
    """
    upem, a, d, _g = MET[face]
    em_logical = size_mm100(pt) if per_inch == 2540 else rnd(pt * 20)
    px = rnd(em_logical * dpi / per_inch)
    ap = rnd(a * px / upem)
    dp = rnd(d * px / upem)
    asc = rnd(ap * per_inch / dpi)
    if group == "split":
        return asc, asc + rnd(dp * per_inch / dpi)
    if group == "sum":
        return asc, rnd((ap + dp) * per_inch / dpi)
    # 'max': EditEngine keeps the taller of the text portion and the formatter metric —
    # `nLineHeight > pLine->GetHeight()` (editeng/source/editeng/impedit3.cxx:1516-1518).
    return asc, max(asc + rnd(dp * per_inch / dpi), rnd((ap + dp) * per_inch / dpi))


def exact(face, pt):
    """What the tree does today: scale exactly, then round each to a whole 1/100 mm."""
    upem, a, d, _g = MET[face]
    em = pt * 2540.0 / 72.0
    asc = rnd(a * em / upem)
    return asc, asc + rnd(d * em / upem)


MODELS = {
    "600/mm100 max": lambda f, p: device("", f, p, 600, 2540, "max"),
    "600/mm100 split": lambda f, p: device("", f, p, 600, 2540, "split"),
    "600/mm100 sum": lambda f, p: device("", f, p, 600, 2540, "sum"),
    "720/mm100 max": lambda f, p: device("", f, p, 720, 2540, "max"),
    "8640/mm100 max": lambda f, p: device("", f, p, 8640, 2540, "max"),
    "8640/twip max": lambda f, p: device("", f, p, 8640, 1440, "max"),
    "exact (tree)": exact,
}


# ------------------------------------------------------------------ the document

HEAD = '''<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.presentation">
<office:font-face-decls>
%s
</office:font-face-decls>
<office:automatic-styles>
 <style:page-layout style:name="PL">
  <style:page-layout-properties fo:page-width="28cm" fo:page-height="21cm"
    fo:margin-top="0cm" fo:margin-bottom="0cm" fo:margin-left="0cm" fo:margin-right="0cm"/>
 </style:page-layout>
 <style:style style:name="dp1" style:family="drawing-page"/>
 <style:style style:name="gr1" style:family="graphic">
  <style:graphic-properties draw:fill="none" draw:stroke="none"
    draw:auto-grow-height="false" draw:auto-grow-width="false" draw:fit-to-size="false"
    draw:textarea-vertical-align="top" draw:textarea-horizontal-align="left"
    fo:padding-top="0cm" fo:padding-bottom="0cm" fo:padding-left="0cm" fo:padding-right="0cm"
    fo:wrap-option="no-wrap" style:font-independent-line-spacing="false"/>
 </style:style>
%s
</office:automatic-styles>
<office:master-styles>
 <style:master-page style:name="Default" style:page-layout-name="PL" draw:style-name="dp1"/>
</office:master-styles>
<office:body><office:presentation>
%s
</office:presentation></office:body>
</office:document>
'''

PSTYLE = ''' <style:style style:name="P%d" style:family="paragraph">
  <style:paragraph-properties fo:margin-top="0cm" fo:margin-bottom="0cm"
    style:line-height-at-least="0cm" style:font-independent-line-spacing="false"/>
  <style:text-properties style:font-name="%s" fo:font-size="%spt"%s
    style:font-name-asian="%s" style:font-size-asian="%spt"
    style:font-name-complex="%s" style:font-size-complex="%spt"/>
 </style:style>
'''

FONTFACE = ' <style:font-face style:name="%s" svg:font-family="&apos;%s&apos;"/>\n'

PAGE = ''' <draw:page draw:name="p%d" draw:style-name="dp1" draw:master-page-name="Default">
  <draw:frame draw:style-name="gr1" svg:width="24cm" svg:height="18cm" svg:x="1cm" svg:y="1cm">
   <draw:text-box>%s</draw:text-box>
  </draw:frame>
 </draw:page>
'''


def fmt(pt):
    return ('%g' % pt)


def build(path):
    styles, body, key = [], [], []
    n = 0
    for label, family, cut, _file in FACES:
        for pt in SIZES:
            weight = ''
            if cut == 'bold':
                weight = ' fo:font-weight="bold" style:font-weight-asian="bold"'
            elif cut == 'italic':
                weight = ' fo:font-style="italic" style:font-style-asian="italic"'
            n += 1
            s = fmt(pt)
            styles.append(PSTYLE % (n, family, s, weight, family, s, family, s))
            ps = ''.join('<text:p text:style-name="P%d">Hxy%d</text:p>' % (n, i)
                         for i in range(LINES))
            body.append(PAGE % (n, ps))
            key.append((label, pt))
    decls = ''.join(FONTFACE % (f[1], f[1]) for f in {g[1]: g for g in FACES}.values())
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


# ------------------------------------------------------------------ measurement

MM100 = 2540.0 / 72.0


FRAME_LEFT_PT = 1000 / MM100


def measure(pdf, key):
    """The six baselines on each slide, in 1/100 mm below the frame's authored top.

    Filtered by the frame's own left edge: every master page contributes a date and a
    slide-number placeholder, and at 14 pt one of them collides with a probe size.
    """
    by_page = {}
    for page, x, y, _sz, _f, _n in rows(pdf):
        if abs(x - FRAME_LEFT_PT) > 0.05:
            continue
        by_page.setdefault(page, []).append(y)
    got = {}
    for i, k in enumerate(key, start=1):
        ys = sorted(by_page.get(i, []))
        if len(ys) != LINES:
            continue
        gaps = [round((ys[j + 1] - ys[j]) * MM100) for j in range(LINES - 1)]
        if len(set(gaps)) != 1:
            continue
        got[k] = (round(ys[0] * MM100) - FRAME_TOP_MM100, gaps[0])
    return got


src = os.path.join(OUT, 'impress-%s.fodp' % SET)
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

print('\nmeasured %d of %d pairs' % (n, len(key)))
for m in MODELS:
    print('  %-18s ascent %3d/%d   line height %3d/%d' % (m, score[m][0], n, score[m][1], n))
