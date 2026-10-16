#!/usr/bin/env python3
"""The diagram rectangle 26.2.4.2 actually used, read out of its own resolved view.

`--convert-to fodt` writes `<chart:plot-area>` -- the rectangle the diagram was given,
`CreateShapeParam2D::maRemainingSpace` -- and `<chart:coordinate-region>`, the inner rectangle
`VDiagram::adjustInnerSize` settled on.  Both are exact and neither needs a rendering to be
measured, so this replaces reading the of-pie's bar out of a PDF.

Each variant rewrites `028`'s three category levels to a word of n 'W's, with the legend
removed so that the categories -- which are also the legend entries -- do not move the
available rectangle as they lengthen.

    ofpie-region.py <workdir> [n ...]
"""
import pathlib, re, shutil, subprocess, sys, zipfile

SOFFICE = '/opt/libreoffice26.2/program/soffice'
P028 = ('/home/user/sample-files/words/chartset-010/docx/'
        '028_Unit_Circle_Chart_Optimized_Graph_83d9c756.docx')
CM = re.compile(r'svg:(x|y|width|height)="([-\d.]+)cm"')
PT = 72.0 / 2.54

NO_LEGEND = lambda t: re.sub(r'<c:legend>.*?</c:legend>', '', t, flags=re.S)


def cats(t, word):
    return re.sub(r'<c:v>(?:Leaf|Stem|Branch) \d+</c:v>', f'<c:v>{word}</c:v>', t)


def write(dst, edit):
    zin = zipfile.ZipFile(P028)
    with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename.endswith('charts/chart1.xml'):
                data = edit(data.decode('utf-8')).encode('utf-8')
            zout.writestr(item, data)
    zin.close()


def rects(fodt):
    s = pathlib.Path(fodt).read_text(encoding='utf-8')
    out = {}
    for tag in ('chart:plot-area', 'chart:coordinate-region'):
        m = re.search(rf'<{tag}[^>]*>', s)
        out[tag] = ({k: float(v) * PT for k, v in CM.findall(m.group(0))} if m else None)
    return out


def main():
    work = pathlib.Path(sys.argv[1]).resolve()
    work.mkdir(parents=True, exist_ok=True)
    profile = work / 'prof'
    ns = sys.argv[2:] or ['W' * n for n in (1, 3, 4, 6, 8, 10, 12, 16, 20, 30, 40, 60)]
    print('word\tavail_x\tavail_y\tavail_w\tavail_h\tinner_x\tinner_y\tinner_w\tinner_h')
    for n in ns:
        d = work / f'w{len(n):03d}{n[:1]}'
        if d.exists():
            shutil.rmtree(d)
        d.mkdir(parents=True)
        doc = d / f'w{len(n):03d}{n[:1]}.docx'
        write(doc, lambda t, w=n: cats(NO_LEGEND(t), w))
        subprocess.run([SOFFICE, '--headless', f'-env:UserInstallation=file://{profile}',
                        '--convert-to', 'fodt', '--outdir', str(d), str(doc)],
                       capture_output=True, check=False, timeout=300)
        got = sorted(d.glob('*.fodt'))
        if not got:
            print(f'{n}\tno fodt')
            continue
        r = rects(got[0])
        a, i = r['chart:plot-area'], r['chart:coordinate-region']
        f = lambda q: '\t'.join('' if q is None else f'{q[k]:.2f}'
                                for k in ('x', 'y', 'width', 'height'))
        print(f'{n}\t{f(a)}\t{f(i)}', flush=True)
        shutil.rmtree(d, ignore_errors=True)


if __name__ == '__main__':
    main()
