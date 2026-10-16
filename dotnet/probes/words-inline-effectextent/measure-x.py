#!/usr/bin/env python3
"""Read the horizontal geometry of a `make-x-fixture.py` page out of a rendering.

Reports, in PDF points: where `LEFT` ends, where the drawing's ink starts and
ends, where `RIGHT` starts, and — when the shape carries a text box — the box of
the `INSIDE` run. `adv` is how much room the drawing took on the line, which is
`RIGHT.xMin - LEFT.xMax` less the two spaces.
"""
import subprocess, sys, os, re, glob

def words(pdf):
    x = subprocess.run(['pdftotext', '-bbox', pdf, '-'], capture_output=True, text=True).stdout
    out = {}
    for m in re.finditer(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>', x):
        out[m.group(5)] = tuple(float(m.group(i)) for i in (1, 2, 3, 4))
    return out

def ink_columns(pdf, dpi=288):
    """Columns of the page carrying the shape's fill, in PDF points from the left.

    Thresholded at 200 so the `tbx-*` fixtures' C0C0C0 fill counts, and required
    on 5% of the page rows so a glyph stem cannot make a column."""
    stem = pdf[:-4] + f'.{dpi}'
    subprocess.run(['pdftoppm', '-r', str(dpi), '-png', '-singlefile', pdf, stem],
                   check=True, capture_output=True)
    from PIL import Image
    im = Image.open(stem + '.png').convert('L')
    w, h = im.size
    px = im.load()
    cols = [x for x in range(w) if sum(1 for y in range(h) if px[x, y] < 200) > h * 0.05]
    if not cols:
        return None
    return (cols[0] * 72.0 / dpi, (cols[-1] + 1) * 72.0 / dpi)

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
    hdr = (f"{'fixture':<14} {'who':<6} {'LEFTend':>8} {'inkL':>8} {'inkR':>8} "
           f"{'RIGHTx':>8} {'adv':>8} {'INSIDEx':>8} {'INSIDEy':>8}")
    print(hdr)
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
            band = ink_columns(pdf)
            le = w.get('LEFT', (0, 0, 0, 0))[2]
            rx = w.get('RIGHT', (0, 0, 0, 0))[0]
            ins = w.get('INSIDE')
            b0, b1 = band if band else (0.0, 0.0)
            insx = f'{ins[0]:8.2f}' if ins else '       -'
            insy = f'{ins[1]:8.2f}' if ins else '       -'
            print(f'{name:<14} {who:<6} {le:8.2f} {b0:8.2f} {b1:8.2f} {rx:8.2f} '
                  f'{rx-le:8.2f} {insx} {insy}')
        print()
