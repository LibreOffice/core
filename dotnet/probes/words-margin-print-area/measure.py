#!/usr/bin/env python3
"""Where the black band lands, and where the body's first line lands, per fixture."""
import subprocess, sys, os, re, glob

def words(pdf):
    x = subprocess.run(['pdftotext', '-bbox', pdf, '-'], capture_output=True, text=True).stdout
    out = {}
    for m in re.finditer(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>', x):
        out.setdefault(m.group(5), tuple(float(m.group(i)) for i in (1, 2, 3, 4)))
    return out

def band(pdf, dpi=288):
    """The rows carrying the 200 pt black rectangle, in points from the page top."""
    stem = pdf[:-4] + f'.{dpi}'
    subprocess.run(['pdftoppm', '-r', str(dpi), '-png', '-singlefile', pdf, stem],
                   check=True, capture_output=True)
    from PIL import Image
    im = Image.open(stem + '.png').convert('L')
    w, h = im.size
    px = im.load()
    # A quarter of the page's width of dark pixels: the 200 pt band is a third of A4's
    # 595 pt and no line of 12 pt text on these fixtures comes close.
    rows = [y for y in range(h) if sum(1 for x in range(w) if px[x, y] < 64) > w * 0.25]
    return (rows[0] * 72.0 / dpi, (rows[-1] + 1) * 72.0 / dpi) if rows else None

def render_lo(soffice, doc, outdir):
    prof = os.path.join(outdir, 'prof')
    subprocess.run([soffice, f'-env:UserInstallation=file://{prof}', '--headless',
                    '--norestore', '--convert-to', 'pdf', '--outdir', outdir, doc],
                   capture_output=True, timeout=300)
    return os.path.join(outdir, os.path.basename(doc)[:-5] + '.pdf')

def render_ours(cli, doc, outdir):
    subprocess.run([cli, 'render', doc, '--format', 'pdf', '--outdir', outdir],
                   capture_output=True, timeout=300)
    return os.path.join(outdir, os.path.basename(doc)[:-5] + '.pdf')

if __name__ == '__main__':
    fx, work = sys.argv[1], sys.argv[2]
    cli = os.environ['XRAY_OFFICE_CLI']
    sofs = {'24.2': '/usr/bin/soffice', '26.2': '/opt/libreoffice26.2/program/soffice'}
    print(f"{'fixture':<13} {'who':<6} {'bandTop':>8} {'bandBot':>8} {'centre':>8} "
          f"{'HEADERA':>8} {'BODYLINE':>9} {'FOOTERC':>8}")
    for doc in sorted(glob.glob(os.path.join(fx, '*.docx'))):
        name = os.path.basename(doc)[:-5]
        for who in ['24.2', '26.2', 'ours']:
            od = os.path.join(work, who, name)
            os.makedirs(od, exist_ok=True)
            pdf = (render_ours(cli, doc, od) if who == 'ours'
                   else render_lo(sofs[who], doc, od))
            if not os.path.exists(pdf):
                print(f'{name:<13} {who:<6}  NO OUTPUT'); continue
            w = words(pdf)
            b = band(pdf)
            b0, b1 = b if b else (0.0, 0.0)
            g = lambda k: f"{w[k][1]:8.2f}" if k in w else '       -'
            print(f'{name:<13} {who:<6} {b0:8.2f} {b1:8.2f} {(b0+b1)/2:8.2f} '
                  f'{g("HEADERA")} {g("BODYLINE"):>9} {g("FOOTERC")}')
        print()
