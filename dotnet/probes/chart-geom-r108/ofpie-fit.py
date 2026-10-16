#!/usr/bin/env python3
"""What sets the unit radius 26.2.4.2 draws an of-pie at.

O42 records the of-pie's radius as 1.37x the reference's on `028`, 1.065x on an eight-point
`029` and 1.017x on a plain pie -- a ratio that is not a constant, so not a scale factor.

`PieChart` draws a bar-of-pie across `m_fLeftShift - m_fLeftScale = -1.4167` to
`m_fBarRight = 1.25` unit radii and down +/- `m_fLeftScale = 0.6667` (`PieChart.hxx`:258-269,
this tree), so the drawn composition is 2.6667R x 1.3333R and R can be read straight off the
bar: a bar-of-pie's bar is exactly 0.5R wide and 1.0R tall.  That is the instrument here --
one number per rendering, taken from an axis-aligned rectangle, with no fitting.

Each variant removes exactly one thing from the chart.  If the reference's R moves when the
data labels go and not when the legend does, the shrink is the label pass; if it moves for
neither, it is the of-pie's own extent against the diagram rectangle.

    ofpie-fit.py <workdir>
"""
import pathlib, re, shutil, subprocess, sys, zipfile
import pymupdf

SOFFICE = '/opt/libreoffice26.2/program/soffice'
P028 = ('/home/user/sample-files/words/chartset-010/docx/'
        '028_Unit_Circle_Chart_Optimized_Graph_83d9c756.docx')

NO_LABELS = lambda t: t.replace('<c:showCatName val="1"/>', '<c:showCatName val="0"/>') \
                       .replace('<c:showPercent val="1"/>', '<c:showPercent val="0"/>')
NO_LEGEND = lambda t: re.sub(r'<c:legend>.*?</c:legend>', '', t, flags=re.S)
OUTSIDE = lambda t: t.replace('<c:dLblPos val="inEnd"/>', '<c:dLblPos val="outEnd"/>')
CTR = lambda t: t.replace('<c:dLblPos val="inEnd"/>', '<c:dLblPos val="ctr"/>')
SHORT = lambda t: re.sub(r'<c:v>(?:Leaf|Stem|Branch) \d+</c:v>', '<c:v>x</c:v>', t)
LONG = lambda t: re.sub(r'<c:v>(Leaf|Stem|Branch) (\d+)</c:v>',
                        lambda m: f'<c:v>{m.group(1)}{m.group(2)} ' + 'Wwwwwwwwww ' * 4 + '</c:v>', t)
# a per-point <c:dLbl> that deletes the label leaves exactly one point labelled
ONE = lambda t: t.replace(
    '<c:dLbls>',
    '<c:dLbls>' + ''.join(
        f'<c:dLbl><c:idx val="{i}"/><c:delete val="1"/></c:dLbl>' for i in range(1, 16)), 1)


def extent(t, cx, cy):
    """Rewrite the drawing frame's extent, in EMU."""
    t = re.sub(r'<wp:extent cx="\d+" cy="\d+"/>', f'<wp:extent cx="{cx}" cy="{cy}"/>', t)
    return re.sub(r'<a:ext cx="\d+" cy="\d+"/>', f'<a:ext cx="{cx}" cy="{cy}"/>', t)


VARIANTS = [
    ('base',            'chart', lambda t: t),
    ('nolabels',        'chart', NO_LABELS),
    ('nolegend',        'chart', NO_LEGEND),
    ('nolabels-nolegend', 'chart', lambda t: NO_LEGEND(NO_LABELS(t))),
    ('outside',         'chart', OUTSIDE),
    ('ctr',             'chart', CTR),
    ('shortcats',       'chart', SHORT),
    ('longcats',        'chart', LONG),
    ('onelabel',        'chart', ONE),
    ('shortcats-nolegend', 'chart', lambda t: NO_LEGEND(SHORT(t))),
    ('frame-half',      'doc',   lambda t: extent(t, 3781425, 2867025)),
    ('frame-wide',      'doc',   lambda t: extent(t, 7000000, 2867025)),
    ('frame-tall',      'doc',   lambda t: extent(t, 3781425, 5000000)),
]


def write(src, dst, edit, where):
    zin = zipfile.ZipFile(src)
    with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            hit = (item.filename.endswith('charts/chart1.xml') if where == 'chart'
                   else item.filename == 'word/document.xml')
            if hit:
                data = edit(data.decode('utf-8')).encode('utf-8')
            zout.writestr(item, data)
    zin.close()


def render(path, outdir, profile):
    subprocess.run(
        [SOFFICE, '--headless', f'-env:UserInstallation=file://{profile}',
         '--convert-to', 'pdf', '--outdir', str(outdir), str(path)],
        capture_output=True, check=False, timeout=300)
    pdfs = sorted(outdir.glob('*.pdf'))
    return pdfs[0] if pdfs else None


OPS = '/home/user/wt-chartgeom/.claude/skills/render-comparison/scripts/pdf-ops.py'
RECORD = re.compile(
    r"fill\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)")


def bars(pdf):
    """Every filled path's bounding box on page 1, from pdf-ops.py's own dump."""
    dump = subprocess.run(['python3', OPS, 'dump', str(pdf), '--page', '1'],
                          capture_output=True, text=True, check=False).stdout
    out = []
    for line in dump.splitlines():
        m = RECORD.match(line.strip())
        if m:
            x0, y0, x1, y1 = (float(v) for v in m.groups())
            out.append((x0, y0, x1, y1))
    return out


def barofpie_radius(pdf):
    """R from the bar: a stack of same-width rectangles whose column is 0.5R by 1.0R."""
    cand = {}
    for (x0, y0, x1, y1) in bars(pdf):
        w = round(x1 - x0, 1)
        if w < 20:
            continue
        cand.setdefault((round(x0, 1), w), []).append((y0, y1))
    best = None
    for (x0, w), ys in cand.items():
        if len(ys) < 2:
            continue
        top = min(y for y, _ in ys)
        bot = max(y for _, y in ys)
        h = bot - top
        # the bar is 0.5R x 1.0R, so its height is twice its width
        if abs(h - 2 * w) / max(h, 1) < 0.06:
            if best is None or h > best[0]:
                best = (h, w, x0, top, bot)
    return best


def main():
    work = pathlib.Path(sys.argv[1]).resolve()
    work.mkdir(parents=True, exist_ok=True)
    profile = work / 'prof'
    rows = []
    for name, where, edit in VARIANTS:
        d = work / name
        if d.exists():
            shutil.rmtree(d)
        d.mkdir(parents=True)
        doc = d / f'{name}.docx'
        write(P028, doc, edit, where)
        pdf = render(doc, d, profile)
        if pdf is None:
            rows.append((name, 'no pdf', '', '', ''))
            continue
        got = barofpie_radius(pdf)
        if got is None:
            rows.append((name, 'no bar', '', '', ''))
            continue
        h, w, x0, top, bot = got
        rows.append((name, f'{h:.2f}', f'{w:.2f}', f'{h / 2:.2f}', f'{x0:.2f},{top:.2f}-{bot:.2f}'))
    print('variant\tbar_h=R\tbar_w=R/2\tmain_r=R*2/3\tbar_box')
    for r in rows:
        print('\t'.join(r))


if __name__ == '__main__':
    main()
