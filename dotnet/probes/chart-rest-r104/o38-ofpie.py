#!/usr/bin/env python3
"""What separates a pie-of-pie 26.2.4.2 draws from one it declines to draw.

Round 102 left the seat with a weak instrument: it clustered "filled marks" by x and read
the clusters by eye.  This asks the question the source makes binary instead.
`PieChart::createShapes` draws, for `PieChartSubType_BAR` and `PieChartSubType_PIE` and for
neither other case, **exactly two two-point connector lines** through
`ShapeFactory::createLine2D` (`chart2/source/view/charttypes/PieChart.cxx`:1073-1105 and
:1121-1150, this tree).  A plain pie draws none.  So:

    two 2-point stroked polylines in the chart  ->  an of-pie was drawn
    none                                        ->  a plain pie was drawn

and the *form* is then read off the second plot's geometry: a bar-of-pie's right-hand plot is
a stack of axis-aligned rectangles, a pie-of-pie's is a second ring of wedges (paths with
curve segments).

Each variant differs from its original in one element.  Run:  o38-ofpie.py <workdir>
"""
import pathlib, re, subprocess, sys, zipfile
import pymupdf

SOFFICE = '/opt/libreoffice26.2/program/soffice'
P029 = ('/home/user/sample-files/words/chartset-011/docx/'
        '029_Unit_Circle_Chart_Pie_Theme_8a922142.docx')
P028 = ('/home/user/sample-files/words/chartset-010/docx/'
        '028_Unit_Circle_Chart_Optimized_Graph_83d9c756.docx')

NO_DPT = lambda t: re.sub(r'<c:dPt>.*?</c:dPt>', '', t, flags=re.S)
NO_EXPLODE = lambda t: re.sub(r'<c:explosion val="\d+"/>', '', t)
BASE029 = lambda t: NO_EXPLODE(NO_DPT(t))


def points(t, n, old):
    """Rewrite the cached category and value sequences to n points."""
    cats = ''.join(f'<c:pt idx="{i}"><c:v>Q{i + 1}</c:v></c:pt>' for i in range(n))
    vals = ''.join(f'<c:pt idx="{i}"><c:v>{60 - 3 * i}</c:v></c:pt>' for i in range(n))
    t = re.sub(rf'<c:strCache><c:ptCount val="{old}"/>.*?</c:strCache>',
               f'<c:strCache><c:ptCount val="{n}"/>{cats}</c:strCache>', t, flags=re.S)
    return re.sub(rf'<c:numCache><c:formatCode>General</c:formatCode><c:ptCount val="{old}"/>'
                  r'.*?</c:numCache>',
                  f'<c:numCache><c:formatCode>General</c:formatCode>'
                  f'<c:ptCount val="{n}"/>{vals}</c:numCache>', t, flags=re.S)


def split(t, pos):
    """State an explicit c:splitType/c:splitPos, which neither original does."""
    return t.replace('<c:gapWidth val="100"/>',
                     f'<c:splitType val="pos"/><c:splitPos val="{pos}"/>'
                     f'<c:gapWidth val="100"/>', 1)


VARIANTS = []
# 029 with the explosion and the per-point formats removed: the sub-pie the model then carries.
for n in (3, 4, 5, 6, 7, 8, 10, 12, 16, 20):
    VARIANTS.append((f'029-pie-n{n}', P029,
                     lambda t, n=n: points(BASE029(t), n, 4)))
    VARIANTS.append((f'029-bar-n{n}', P029,
                     lambda t, n=n: points(BASE029(t), n, 4).replace('val="pie"', 'val="bar"')))
# 028, whose original is a sixteen-point bar-of-pie, forced to each form at each count.
for n in (4, 8, 16):
    VARIANTS.append((f'028-pie-n{n}', P028,
                     lambda t, n=n: points(t, n, 16).replace('val="bar"', 'val="pie"')))
    VARIANTS.append((f'028-bar-n{n}', P028, lambda t, n=n: points(t, n, 16)))
# The split position, which neither original states.
for pos in (1, 2, 3, 5):
    VARIANTS.append((f'029-pie-n8-split{pos}', P029,
                     lambda t, pos=pos: split(points(BASE029(t), 8, 4), pos)))
    VARIANTS.append((f'029-bar-n8-split{pos}', P029,
                     lambda t, pos=pos: split(points(BASE029(t), 8, 4), pos)
                     .replace('val="pie"', 'val="bar"')))
# The controls: both originals as they stand.
VARIANTS.append(('029-asis', P029, lambda t: t))
VARIANTS.append(('028-asis', P028, lambda t: t))


def build(out, src, fn):
    zin = zipfile.ZipFile(src)
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        for name in zin.namelist():
            data = zin.read(name)
            if name == 'word/charts/chart1.xml':
                data = fn(data.decode('utf8')).encode('utf8')
            z.writestr(name, data)


def convert(path, fmt, outdir, profile):
    subprocess.run([SOFFICE, '--headless', '--norestore',
                    f'-env:UserInstallation=file://{profile}',
                    '--convert-to', fmt, '--outdir', str(outdir), str(path)],
                   capture_output=True, timeout=300)


def measure(pdf):
    """(connector lines, wedge clusters, bar rectangles) on the page."""
    page = pymupdf.open(pdf)[0]
    lines, curves, rects = 0, [], 0
    for d in page.get_drawings():
        items = d['items']
        if d['type'] == 's' and len(items) == 1 and items[0][0] == 'l':
            lines += 1
        if d['type'] in ('f', 'fs'):
            if any(i[0] == 'c' for i in items):
                curves.append(d['rect'])
            elif len(items) == 1 and items[0][0] == 're':
                rects += 1
    # cluster the wedge paths by their centre, 6 pt apart
    centres = []
    for r in curves:
        c = ((r.x0 + r.x1) / 2, (r.y0 + r.y1) / 2)
        if not any(abs(c[0] - o[0]) < 6 and abs(c[1] - o[1]) < 6 for o in centres):
            centres.append(c)
    return lines, len(curves), len(centres), rects


def main():
    work = pathlib.Path(sys.argv[1]); work.mkdir(parents=True, exist_ok=True)
    profile = work / 'profile'
    rows = []
    for name, src, fn in VARIANTS:
        doc = work / f'{name}.docx'
        build(doc, src, fn)
        convert(doc, 'fodt', work, profile)
        convert(doc, 'pdf', work, profile)

        flat = (work / f'{name}.fodt').read_text(encoding='utf8', errors='replace')
        model = 'none'
        m = re.search(r'loext:sub-(bar|pie)="true"', flat)
        if m:
            model = m.group(1)
            p = re.search(r'loext:split-position="(\d+)"', flat)
            if p:
                model += f' split={p.group(1)}'

        pdf = work / f'{name}.pdf'
        if pdf.exists():
            lines, wedges, rings, bars = measure(pdf)
            drawn = ('of-pie' if lines == 2 else 'plain pie' if lines == 0 else f'?{lines}')
        else:
            lines = wedges = rings = bars = -1
            drawn = 'no pdf'
        rows.append((name, model, drawn, lines, wedges, rings, bars))
        print('\t'.join(str(v) for v in rows[-1]), flush=True)

    out = pathlib.Path(__file__).with_name('o38-ofpie.tsv')
    with out.open('w') as fh:
        fh.write('variant\tmodel\tdrawn\tconnector_lines\twedge_paths\twedge_centres\tplain_rects\n')
        for r in rows:
            fh.write('\t'.join(str(v) for v in r) + '\n')

main()
