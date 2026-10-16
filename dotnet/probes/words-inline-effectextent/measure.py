#!/usr/bin/env python3
"""Render each fixture through a LibreOffice binary and through X-Ray.Office, and
report the y of TOPLINE, the shape's ink band, and the y of BOTLINE."""
import subprocess, sys, os, re, glob

def words(pdf):
    x = subprocess.run(['pdftotext', '-bbox', pdf, '-'], capture_output=True, text=True).stdout
    out = {}
    for m in re.finditer(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>', x):
        out[m.group(5)] = (float(m.group(2)), float(m.group(4)))
    return out

def ink_band(pdf):
    """Rows of the page holding the black shape, in PDF points from the top."""
    png = pdf + '.png'
    subprocess.run(['pdftoppm', '-r', '72', '-png', '-singlefile', pdf, png[:-4]],
                   check=True, capture_output=True)
    from PIL import Image
    im = Image.open(png).convert('L')
    w, h = im.size
    px = im.load()
    rows = [y for y in range(h) if sum(1 for x in range(w) if px[x, y] < 64) > w * 0.3]
    return (min(rows), max(rows)) if rows else None

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
    print(f"{'fixture':<14} {'who':<6} {'TOP':>8} {'inkTop':>8} {'inkBot':>8} {'BOT':>8} {'gap':>8}")
    for doc in sorted(glob.glob(os.path.join(fx, '*.docx'))):
        name = os.path.basename(doc)[:-5]
        for who in ['24.2', '26.2', 'ours']:
            od = os.path.join(work, who, name)
            os.makedirs(od, exist_ok=True)
            pdf = (render_ours(cli, doc, od) if who == 'ours'
                   else render_lo(sofs[who], doc, od))
            if not os.path.exists(pdf):
                print(f'{name:<14} {who:<6}  NO OUTPUT'); continue
            w = words(pdf)
            band = ink_band(pdf)
            top = w.get('TOPLINE', (0, 0))[1]      # baseline-ish: yMax of TOPLINE
            bot = w.get('BOTLINE', (0, 0))[1]
            b0, b1 = band if band else (0, 0)
            print(f'{name:<14} {who:<6} {top:8.2f} {b0:8.2f} {b1:8.2f} {bot:8.2f} {bot-top:8.2f}')
        print()
