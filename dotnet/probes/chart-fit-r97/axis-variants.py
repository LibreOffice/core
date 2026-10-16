#!/usr/bin/env python3
"""One-attribute variants of `048_Expense_trends_budget`'s value axis, rendered by 26.2.4.2.

Ours steps the 0…500 value axis by 50 where the reference steps by 100, and the interval-cap
model of `probes/chart-axis-r87` predicts 50 for any axis length over ten label heights — which
this one is, at 151 pt of drawn axis against a 9.04 pt label. So the cap is not what the
reference is lowering here, and this asks it which attribute is.

    fmt       `c:numFmt formatCode` alone, "#,##0;;" -> "#,##0"      (tdf#48041's arm)
    nozoom    the sheet's `fitToPage`/`pageSetup@scale` alone, removed
    both      the two together
"""
import pathlib, re, shutil, subprocess, sys, zipfile

REF = '/opt/libreoffice26.2/program/soffice'


def main():
    src = pathlib.Path(sys.argv[1]).resolve()
    out = pathlib.Path(sys.argv[2]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(src) as z:
        names = z.namelist()
        blobs = {n: z.read(n) for n in names}

    chart = next(n for n in names if re.fullmatch(r'xl/charts/chart\d+\.xml', n))
    sheets = [n for n in names if re.fullmatch(r'xl/worksheets/sheet\d+\.xml', n)]

    def nofmt(b):
        c = b[chart].decode('utf-8').replace('formatCode="#,##0;;"', 'formatCode="#,##0"')
        return dict(b, **{chart: c.encode('utf-8')})

    def nozoom(b):
        d = dict(b)
        for s in sheets:
            t = d[s].decode('utf-8')
            t = re.sub(r'<sheetPr[^>]*>.*?</sheetPr>', lambda m: re.sub(
                r'<pageSetUpPr[^>]*/>', '', m.group(0)), t, flags=re.S)
            t = re.sub(r'<pageSetUpPr[^>]*/>', '', t)
            t = re.sub(r'(<pageSetup\b[^>]*?)\sscale="\d+"', r'\1 scale="100"', t)
            d[s] = t.encode('utf-8')
        return d

    made = {'base': blobs, 'fmt': nofmt(blobs), 'nozoom': nozoom(blobs),
            'both': nofmt(nozoom(blobs))}

    paths = []
    for label, b in made.items():
        p = out / f'{label}.xlsx'
        with zipfile.ZipFile(p, 'w', zipfile.ZIP_DEFLATED) as z:
            for n in names:
                z.writestr(n, b[n])
        paths.append(p)

    pdfs = out / 'pdf'
    pdfs.mkdir(exist_ok=True)
    subprocess.run(['timeout', '-k', '30', '900', REF, '--headless', '--norestore',
                    f'-env:UserInstallation=file://{out / "prof"}',
                    '--convert-to', 'pdf', '--outdir', str(pdfs)] + [str(p) for p in paths],
                   capture_output=True, timeout=950)
    for p in paths:
        f = pdfs / (p.stem + '.pdf')
        print(f'-- {p.stem}: {"ok" if f.exists() else "NO OUTPUT"}')
        if f.exists():
            r = subprocess.run(['python3', str(pathlib.Path(__file__).parent / 'axisticks.py'),
                                str(f)], capture_output=True, text=True)
            for line in r.stdout.splitlines():
                if 'step=0' not in line and line.strip().startswith('p'):
                    print('   ', line.strip())


main()
