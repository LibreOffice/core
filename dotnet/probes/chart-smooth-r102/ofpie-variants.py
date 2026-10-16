#!/usr/bin/env python3
"""One-attribute variants of the corpus's two of-pie documents, put through 26.2.4.2.

Each variant differs from its original in exactly one element.  Two questions are asked of
each: does the reference's own `--convert-to fodt` carry a `loext:sub-bar` / `loext:sub-pie`
(the resolved model), and does its PDF draw a second plot (the rendering)?  The two are
different code paths and this round found them answering differently.

Run: python3 ofpie-variants.py <workdir>     (writes ofpie.txt)
"""
import pymupdf, re, subprocess, sys, zipfile, pathlib

SOFFICE = '/opt/libreoffice26.2/program/soffice'
P029 = ('/home/user/sample-files/words/chartset-011/docx/'
        '029_Unit_Circle_Chart_Pie_Theme_8a922142.docx')
P028 = ('/home/user/sample-files/words/chartset-010/docx/'
        '028_Unit_Circle_Chart_Optimized_Graph_83d9c756.docx')

NO_DPT = lambda t: re.sub(r'<c:dPt>.*?</c:dPt>', '', t, flags=re.S)
NO_EXPLODE = lambda t: re.sub(r'<c:explosion val="\d+"/>', '', t)


def points(t, n, old):
    cats = ''.join(f'<c:pt idx="{i}"><c:v>Q{i + 1}</c:v></c:pt>' for i in range(n))
    vals = ''.join(f'<c:pt idx="{i}"><c:v>{50 - 4 * i}</c:v></c:pt>' for i in range(n))
    t = re.sub(rf'<c:strCache><c:ptCount val="{old}"/>.*?</c:strCache>',
               f'<c:strCache><c:ptCount val="{n}"/>{cats}</c:strCache>', t, flags=re.S)
    return re.sub(rf'<c:numCache><c:formatCode>General</c:formatCode><c:ptCount val="{old}"/>'
                  r'.*?</c:numCache>',
                  f'<c:numCache><c:formatCode>General</c:formatCode>'
                  f'<c:ptCount val="{n}"/>{vals}</c:numCache>', t, flags=re.S)


VARIANTS = [
    ('029-asis', P029, lambda t: t),
    ('029-swap-to-bar', P029, lambda t: t.replace('val="pie"', 'val="bar"')),
    ('029-no-explosion', P029, NO_EXPLODE),
    ('029-no-dPt', P029, NO_DPT),
    ('029-neither', P029, lambda t: NO_EXPLODE(NO_DPT(t))),
    ('029-neither-bar', P029,
     lambda t: NO_EXPLODE(NO_DPT(t)).replace('val="pie"', 'val="bar"')),
    ('029-neither-n5', P029, lambda t: points(NO_EXPLODE(NO_DPT(t)), 5, 4)),
    ('029-neither-n7', P029, lambda t: points(NO_EXPLODE(NO_DPT(t)), 7, 4)),
    ('028-asis', P028, lambda t: t),
    ('028-swap-to-pie', P028, lambda t: t.replace('val="bar"', 'val="pie"')),
    ('028-no-dPt', P028, NO_DPT),
    ('028-explode', P028, lambda t: t.replace('</c:tx>', '</c:tx><c:explosion val="1"/>', 1)),
]


def build(out, src, fn):
    zin = zipfile.ZipFile(src)
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        for name in zin.namelist():
            data = zin.read(name)
            if name == 'word/charts/chart1.xml':
                data = fn(data.decode('utf8')).encode('utf8')
            z.writestr(name, data)


def convert(path, fmt, outdir):
    subprocess.run(['timeout', '-k', '30', '240', SOFFICE, '--headless',
                    '-env:UserInstallation=file:///tmp/lo-ofpie-probe',
                    '--convert-to', fmt, '--outdir', str(outdir), str(path)],
                   capture_output=True, check=False)


def drawn(pdf):
    """How many spatially separate groups of filled marks the reference drew.

    A plain pie is one group; an of-pie is two, the second being the right-hand pie or the
    stacked bar.  Filled paths are clustered by their x centre with a 30 pt gap, and only
    marks are counted — white fills (the page and the text boxes behind the body copy) and
    anything shorter than twelve points (the legend keys) are dropped.
    """
    pg = pymupdf.open(pdf)[0]
    centres = []
    for dr in pg.get_drawings():
        fill = dr.get('fill')
        if fill is None or min(fill) > 0.97:
            continue          # the page's and the text boxes' white backgrounds
        r = pymupdf.Rect(dr['rect'])
        if r.width > 400 or r.height > 400 or r.width < 2 or r.height < 2:
            continue          # the page and chart backgrounds
        if r.height < 12:
            continue          # legend keys and text underlines
        centres.append((r.x0 + r.x1) / 2)
    centres.sort()
    groups, run = [], []
    for c in centres:
        if run and c - run[-1] > 30:
            groups.append(run)
            run = []
        run.append(c)
    if run:
        groups.append(run)
    return (f"{len(groups)} group(s) of filled marks: "
            + ', '.join(f'{len(g)} at x {g[0]:.0f}-{g[-1]:.0f}' for g in groups))


def main(work):
    work = pathlib.Path(work)
    work.mkdir(parents=True, exist_ok=True)
    print(f'{"variant":20s} {"model (fodt)":34s} rendering')
    for name, src, fn in VARIANTS:
        docx = work / f'{name}.docx'
        build(docx, src, fn)
        convert(docx, 'fodt', work)
        convert(docx, 'pdf', work)
        fodt = (work / f'{name}.fodt').read_text(encoding='utf8', errors='replace')
        model = ' '.join(re.findall(r'loext:sub-\w+="[^"]*"|loext:split-position="[^"]*"', fodt))
        print(f'{name:20s} {model or "(no of-pie in the model)":34s} '
              f'{drawn(work / f"{name}.pdf")}')


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '/tmp/ofpie-probe')
