#!/usr/bin/env python3
"""How 26.2.4.2's of-pie radius moves with the size of its data labels.

`ofpie-fit.py` shows the reference draws `028`'s of-pie at R = 135.40 with its labels and at
R = 184.90 without, so the shrink is `VDiagram::adjustInnerSize` charged for what the labels
consumed.  This maps the function: the legend is removed (its entries are the categories, so
varying the label text would otherwise move the available rectangle too) and the category
strings are rewritten to a ladder of lengths.  R is read off the bar, which is exactly
0.5R by 1.0R (`PieChart::getBarRect`, `PieChart.cxx`:381-397).

    ofpie-ladder.py <workdir>
"""
import pathlib, re, shutil, subprocess, sys

sys.path.insert(0, str(pathlib.Path(__file__).parent))
from importlib import import_module
fit = import_module('ofpie-fit'.replace('-', '_')) if False else None

SOFFICE = '/opt/libreoffice26.2/program/soffice'
P028 = ('/home/user/sample-files/words/chartset-010/docx/'
        '028_Unit_Circle_Chart_Optimized_Graph_83d9c756.docx')
OPS = '/home/user/wt-chartgeom/.claude/skills/render-comparison/scripts/pdf-ops.py'
RECORD = re.compile(
    r"fill\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)")

NO_LEGEND = lambda t: re.sub(r'<c:legend>.*?</c:legend>', '', t, flags=re.S)


def cats(t, word):
    return re.sub(r'<c:v>(?:Leaf|Stem|Branch) \d+</c:v>', f'<c:v>{word}</c:v>', t)


def write(src, dst, edit):
    import zipfile
    zin = zipfile.ZipFile(src)
    with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename.endswith('charts/chart1.xml'):
                data = edit(data.decode('utf-8')).encode('utf-8')
            zout.writestr(item, data)
    zin.close()


def render(path, outdir, profile):
    subprocess.run([SOFFICE, '--headless', f'-env:UserInstallation=file://{profile}',
                    '--convert-to', 'pdf', '--outdir', str(outdir), str(path)],
                   capture_output=True, check=False, timeout=300)
    pdfs = sorted(outdir.glob('*.pdf'))
    return pdfs[0] if pdfs else None


def radius(pdf):
    dump = subprocess.run(['python3', OPS, 'dump', str(pdf), '--page', '1'],
                          capture_output=True, text=True, check=False).stdout
    cand = {}
    for line in dump.splitlines():
        m = RECORD.match(line.strip())
        if not m:
            continue
        x0, y0, x1, y1 = (float(v) for v in m.groups())
        w = round(x1 - x0, 1)
        if w < 20:
            continue
        cand.setdefault((round(x0, 1), w), []).append((y0, y1))
    best = None
    for (x0, w), ys in cand.items():
        if len(ys) < 2:
            continue
        h = max(y for _, y in ys) - min(y for y, _ in ys)
        if abs(h - 2 * w) / max(h, 1) < 0.06 and (best is None or h > best):
            best = h
    return best


def main():
    work = pathlib.Path(sys.argv[1]).resolve()
    work.mkdir(parents=True, exist_ok=True)
    profile = work / 'prof'
    print('label\tchars\tR\tside=2R')
    for n in (1, 2, 3, 4, 6, 8, 10, 12, 14, 16, 20, 24, 30, 40):
        word = 'W' * n
        d = work / f'n{n:02d}'
        if d.exists():
            shutil.rmtree(d)
        d.mkdir(parents=True)
        doc = d / f'n{n:02d}.docx'
        write(P028, doc, lambda t, w=word: cats(NO_LEGEND(t), w))
        pdf = render(doc, d, profile)
        r = radius(pdf) if pdf else None
        print(f'{word[:12]}\t{n}\t{r}\t{None if r is None else round(2 * r, 2)}', flush=True)


if __name__ == '__main__':
    main()
