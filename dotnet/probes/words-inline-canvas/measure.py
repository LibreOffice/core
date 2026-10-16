#!/usr/bin/env python3
"""Where each coloured member of an inline canvas or group landed, in PDF points.

Each member has its own primary colour, so its box is read straight out of the raster
with no pairing: the fixtures need no text inside the shapes and nothing has to be
matched between the two renderings.
"""
import subprocess, sys, os, glob

SOF = {'24.2': '/usr/bin/soffice', '26.2': '/opt/libreoffice26.2/program/soffice'}
COLOURS = {'red': (255, 0, 0), 'green': (0, 255, 0), 'blue': (0, 0, 255)}


def boxes(pdf, dpi=150):
    stem = pdf[:-4] + f'.{dpi}'
    subprocess.run(['pdftoppm', '-r', str(dpi), '-png', '-singlefile', pdf, stem],
                   check=True, capture_output=True)
    from PIL import Image
    im = Image.open(stem + '.png').convert('RGB')
    w, h = im.size
    px = im.load()
    found = {}
    for name, (r, g, b) in COLOURS.items():
        xs, ys = [], []
        for y in range(h):
            for x in range(w):
                pr, pg, pb = px[x, y]
                # Generous, because the raster antialiases the edges.
                if abs(pr - r) < 60 and abs(pg - g) < 60 and abs(pb - b) < 60:
                    xs.append(x); ys.append(y)
        if xs:
            k = 72.0 / dpi
            found[name] = (min(xs) * k, min(ys) * k, (max(xs) + 1) * k, (max(ys) + 1) * k)
    return found


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
    print(f"{'fixture':<9} {'who':<6} {'member':<6} {'x0':>8} {'y0':>8} {'x1':>8} {'y1':>8}")
    for doc in sorted(glob.glob(os.path.join(fx, '*.docx'))):
        name = os.path.basename(doc)[:-5]
        for who in ('24.2', '26.2', 'ours'):
            pdf = render(who, doc, os.path.join(work, who, name))
            if not os.path.exists(pdf):
                print(f'{name:<9} {who:<6}  NO OUTPUT'); continue
            got = boxes(pdf)
            for member in ('red', 'green', 'blue'):
                if member not in got:
                    print(f'{name:<9} {who:<6} {member:<6}   ABSENT'); continue
                x0, y0, x1, y1 = got[member]
                print(f'{name:<9} {who:<6} {member:<6} {x0:8.2f} {y0:8.2f} {x1:8.2f} {y1:8.2f}')
        print()
