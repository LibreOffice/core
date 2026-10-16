#!/usr/bin/env python3
"""Ask 26.2.4.2 what size it resolves an anchored object to, by reading the flat ODT.

Rewrites one `wp:extent` (and the matching `a:ext`) at a time and converts to `.fodt`, which
prints the reference's own model without rendering anything.
"""
import pathlib, re, subprocess, sys, zipfile

REF = '/opt/libreoffice26.2/program/soffice'
EMU = 914400.0


def anchors(xml):
    return [m for m in re.finditer(r'<wp:(anchor|inline)\b.*?</wp:\1>', xml, re.S)]


def is_chart(a):
    return 'pic:pic' not in a and 'wps:txbx' not in a and 'chart' in a


def resize(xml, which, cx, cy):
    """`which` selects the anchor by index; rewrite its extent and its a:ext."""
    ms = anchors(xml)
    m = ms[which]
    a = m.group(0)
    a2 = re.sub(r'<wp:extent cx="\d+" cy="\d+"/>', f'<wp:extent cx="{cx}" cy="{cy}"/>', a, count=1)
    a2 = re.sub(r'<a:ext cx="\d+" cy="\d+"/>', f'<a:ext cx="{cx}" cy="{cy}"/>', a2, count=1)
    return xml[:m.start()] + a2 + xml[m.end():]


def write(src, blobs, names, xml, path):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for n in names:
            z.writestr(n, xml.encode('utf-8') if n == 'word/document.xml' else blobs[n])


def main():
    src = pathlib.Path(sys.argv[1]).resolve()
    out = pathlib.Path(sys.argv[2]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(src) as z:
        names = z.namelist()
        blobs = {n: z.read(n) for n in names}
    xml = blobs['word/document.xml'].decode('utf-8')

    ms = anchors(xml)
    chart = next(i for i, m in enumerate(ms) if is_chart(m.group(0)))
    pic = next((i for i, m in enumerate(ms) if 'pic:pic' in m.group(0)), None)
    stated = re.search(r'<wp:extent cx="(\d+)" cy="(\d+)"/>', ms[chart].group(0))
    ccx, ccy = int(stated.group(1)), int(stated.group(2))
    print(f'{src.name}: chart anchor {chart} states {ccx}x{ccy} EMU '
          f'= {ccx/EMU*72:.2f} x {ccy/EMU*72:.2f} pt')

    made = {}
    made['base'] = xml
    made['half'] = resize(xml, chart, ccx // 2, ccy // 2)
    made['wide'] = resize(xml, chart, 9000000, 2000000)     # 708.7 x 157.5 pt, only width over
    made['tall'] = resize(xml, chart, 4000000, 11500000)    # 315 x 905.5 pt, only height over
    made['both'] = resize(xml, chart, 9000000, 11500000)
    made['fit'] = resize(xml, chart, 5000000, 3000000)      # 393.7 x 236.2 pt, neither over
    if pic is not None:
        made['picbig'] = resize(xml, pic, ccx, ccy)

    paths = []
    for label, text in made.items():
        p = out / f'{label}.docx'
        write(src, blobs, names, text, p)
        paths.append(p)

    fodt = out / 'fodt'
    fodt.mkdir(exist_ok=True)
    subprocess.run(['timeout', '-k', '30', '900', REF, '--headless', '--norestore',
                    f'-env:UserInstallation=file://{out / "prof"}',
                    '--convert-to', 'fodt', '--outdir', str(fodt)] + [str(p) for p in paths],
                   capture_output=True, timeout=950)

    for p in paths:
        f = fodt / (p.stem + '.fodt')
        if not f.exists():
            print(f'  {p.stem:8s} NO OUTPUT')
            continue
        s = f.read_text(encoding='utf-8')
        pw = re.search(r'fo:page-width="([\d.]+)in"', s)
        ph = re.search(r'fo:page-height="([\d.]+)in"', s)
        print(f'  {p.stem:8s} page {float(pw.group(1))*72:.2f} x {float(ph.group(1))*72:.2f} pt')
        for fr in re.finditer(r'<draw:frame\b[^>]*>', s):
            t = fr.group(0)
            name = re.search(r'draw:name="([^"]*)"', t)
            w = re.search(r'svg:width="([\d.-]+)in"', t)
            h = re.search(r'svg:height="([\d.-]+)in"', t)
            x = re.search(r'svg:x="([\d.-]+)in"', t)
            y = re.search(r'svg:y="([\d.-]+)in"', t)
            def pt(m):
                return f'{float(m.group(1))*72:8.2f}' if m else '       -'
            print(f'      {name.group(1) if name else "?":12s} '
                  f'w {pt(w)} h {pt(h)}  at x {pt(x)} y {pt(y)}')


main()
