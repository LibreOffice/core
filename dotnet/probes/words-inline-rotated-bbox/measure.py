#!/usr/bin/env python3
"""The rotated drawing's ink rectangle, the line it sits on, and the room it took."""
import subprocess, sys, os, re, glob

def words(pdf):
    x = subprocess.run(['pdftotext', '-bbox', pdf, '-'], capture_output=True, text=True).stdout
    out = {}
    for m in re.finditer(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>', x):
        out.setdefault(m.group(5), tuple(float(m.group(i)) for i in (1, 2, 3, 4)))
    return out

def ink(pdf, dpi=288):
    """The bounding box of the black rectangle, in points.

    The fixtures set every text run to 0x909090, so thresholding at 64 leaves the
    rectangle and nothing else — which matters because the drawing shares its line with
    `LEFT` and `RIGHT`, and a row-based reading that let those in returned the whole
    line's width as the drawing's."""
    stem = pdf[:-4] + f'.{dpi}'
    subprocess.run(['pdftoppm', '-r', str(dpi), '-png', '-singlefile', pdf, stem],
                   check=True, capture_output=True)
    from PIL import Image
    im = Image.open(stem + '.png').convert('L')
    w, h = im.size
    px = im.load()
    rows, xs = [], []
    for y in range(h):
        row = [x for x in range(w) if px[x, y] < 64]
        if not row:
            continue
        rows.append(y)
        xs.append(row[0])
        xs.append(row[-1])
    if not rows:
        return None
    return (rows[0] * 72.0 / dpi, (rows[-1] + 1) * 72.0 / dpi,
            min(xs) * 72.0 / dpi, (max(xs) + 1) * 72.0 / dpi)

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
    print(f"{'fixture':<12} {'who':<6} {'gap':>7} {'inkTop':>8} {'inkBot':>8} {'inkH':>7} "
          f"{'inkL':>8} {'inkR':>8} {'inkW':>7} {'adv':>7}")
    for doc in sorted(glob.glob(os.path.join(fx, '*.docx'))):
        name = os.path.basename(doc)[:-5]
        for who in ['24.2', '26.2', 'ours']:
            od = os.path.join(work, who, name)
            os.makedirs(od, exist_ok=True)
            pdf = (render_ours(cli, doc, od) if who == 'ours'
                   else render_lo(sofs[who], doc, od))
            if not os.path.exists(pdf):
                print(f'{name:<12} {who:<6}  NO OUTPUT'); continue
            w = words(pdf)
            box = ink(pdf)
            t = w.get('TOPLINE', (0, 0, 0, 0))[3]
            b = w.get('BOTLINE', (0, 0, 0, 0))[3]
            le = w.get('LEFT', (0, 0, 0, 0))[2]
            rx = w.get('RIGHT', (0, 0, 0, 0))[0]
            if box:
                y0, y1, x0, x1 = box
                print(f'{name:<12} {who:<6} {b-t:7.2f} {y0:8.2f} {y1:8.2f} {y1-y0:7.2f} '
                      f'{x0:8.2f} {x1:8.2f} {x1-x0:7.2f} {rx-le:7.2f}')
            else:
                print(f'{name:<12} {who:<6} {b-t:7.2f} {"-":>8} {"-":>8} {"-":>7} '
                      f'{"-":>8} {"-":>8} {"-":>7} {rx-le:7.2f}')
        print()
