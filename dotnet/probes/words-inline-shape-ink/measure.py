#!/usr/bin/env python3
"""Where an inline drawing's ink lands down the page, and where its text lands.

Reports, in PDF points from the page top, per fixture and per renderer:

  TOP / BOT   the `TOPLINE` and `BOTLINE` runs, so the line box can be read;
  inkT / inkB the rows carrying the drawing's own fill;
  INSIDE      the run inside a `wps:txbx`, where the fixture has one.

Renders through `/usr/bin/soffice`, `/opt/libreoffice26.2/program/soffice` and
`$XRAY_OFFICE_CLI`, so the version question is re-checked on every run.
"""
import subprocess, sys, os, re, glob

SOF = {'24.2': '/usr/bin/soffice', '26.2': '/opt/libreoffice26.2/program/soffice'}


def words(pdf):
    x = subprocess.run(['pdftotext', '-bbox', pdf, '-'], capture_output=True, text=True).stdout
    out = {}
    for m in re.finditer(
            r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>', x):
        out.setdefault(m.group(5), tuple(float(m.group(i)) for i in (1, 2, 3, 4)))
    return out


def band(pdf, dpi=288):
    """The rows carrying the drawing's fill: a sixth of the page's width of ink.

    Thresholded at 200 so a `C0C0C0` text-box fill counts, and at a sixth of the page
    width so no 12 pt text line can make a row while the 144 pt drawing — 24% of A4's
    595 pt — still does. A quarter was the first threshold tried and it found nothing
    at all, which is the shape of instrument error this file is meant to avoid.
    """
    stem = pdf[:-4] + f'.{dpi}'
    subprocess.run(['pdftoppm', '-r', str(dpi), '-png', '-singlefile', pdf, stem],
                   check=True, capture_output=True)
    from PIL import Image
    im = Image.open(stem + '.png').convert('L')
    w, h = im.size
    px = im.load()
    rows = [y for y in range(h) if sum(1 for x in range(w) if px[x, y] < 200) > w * 0.16]
    return (rows[0] * 72.0 / dpi, (rows[-1] + 1) * 72.0 / dpi) if rows else None


def render(who, doc, outdir):
    os.makedirs(outdir, exist_ok=True)
    if who == 'ours':
        subprocess.run([os.environ['XRAY_OFFICE_CLI'], 'render', doc, '--format', 'pdf',
                        '--outdir', outdir], capture_output=True, timeout=300)
    else:
        subprocess.run([SOF[who], f'-env:UserInstallation=file://{outdir}/prof', '--headless',
                        '--norestore', '--convert-to', 'pdf', '--outdir', outdir, doc],
                       capture_output=True, timeout=300)
    return os.path.join(outdir, os.path.basename(doc)[:-5] + '.pdf')


if __name__ == '__main__':
    fx, work = sys.argv[1], sys.argv[2]
    print(f"{'fixture':<13} {'who':<6} {'TOP':>7} {'inkT':>8} {'inkB':>8} {'BOT':>8} "
          f"{'INSIDE':>8} {'gap':>7}")
    for doc in sorted(glob.glob(os.path.join(fx, '*.docx'))):
        name = os.path.basename(doc)[:-5]
        for who in ('24.2', '26.2', 'ours'):
            pdf = render(who, doc, os.path.join(work, who, name))
            if not os.path.exists(pdf):
                print(f'{name:<13} {who:<6}  NO OUTPUT'); continue
            w = words(pdf)
            b = band(pdf)
            b0, b1 = b if b else (0.0, 0.0)
            g = lambda k, i=1: f'{w[k][i]:8.2f}' if k in w else '       -'
            gap = (w['BOTLINE'][1] - w['TOPLINE'][3]) if 'BOTLINE' in w and 'TOPLINE' in w else 0.0
            print(f'{name:<13} {who:<6} {w.get("TOPLINE",(0,0,0,0))[3]:7.2f} {b0:8.2f} {b1:8.2f} '
                  f'{g("BOTLINE")} {g("INSIDE")} {gap:7.2f}')
        print()
