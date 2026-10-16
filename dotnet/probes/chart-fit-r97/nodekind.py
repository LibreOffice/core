#!/usr/bin/env python3
"""Which oversize anchored objects 26.2.4.2 cuts down to the page, and how.

`framesize.py` reads the resolved model out of the flat ODT, which only shows a squeeze for an
OLE node — `SwFlyFreeFrame::CheckClip` writes the new size back into the frame format for those
alone (`sw/source/core/layout/flylay.cxx`:638-648). This one renders instead and measures what
is drawn, so a picture and a text box can be scored too.

Four variants of `023_Unit_Circle_Chart_Circular_Percentage`, one attribute changed at a time:

    picthrough  the picture given the chart's 682.10 x 493.50 pt extent, wrap left at wrapNone
    picsquare   the same, wrap changed to wrapSquare
    txtsquare   the text box given 708.66 x 905.51 pt, wrap left at wrapSquare
    chartnone   the chart's wrap changed from wrapSquare to wrapNone, extent left alone
"""
import pathlib, re, subprocess, sys, zipfile

REF = '/opt/libreoffice26.2/program/soffice'


def anchors(xml):
    return list(re.finditer(r'<wp:(anchor|inline)\b.*?</wp:\1>', xml, re.S))


def edit(xml, which, cx=None, cy=None, wrap=None):
    ms = anchors(xml)
    m = ms[which]
    a = m.group(0)
    if cx is not None:
        a = re.sub(r'<wp:extent cx="\d+" cy="\d+"/>', f'<wp:extent cx="{cx}" cy="{cy}"/>', a, count=1)
        a = re.sub(r'<a:ext cx="\d+" cy="\d+"/>', f'<a:ext cx="{cx}" cy="{cy}"/>', a, count=1)
    if wrap is not None:
        a = re.sub(r'<wp:wrapNone/>|<wp:wrapSquare[^>]*/>', wrap, a, count=1)
    return xml[:m.start()] + a + xml[m.end():]


def main():
    src = pathlib.Path(sys.argv[1]).resolve()
    out = pathlib.Path(sys.argv[2]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(src) as z:
        names = z.namelist()
        blobs = {n: z.read(n) for n in names}
    xml = blobs['word/document.xml'].decode('utf-8')

    ms = anchors(xml)
    kinds = []
    for m in ms:
        a = m.group(0)
        kinds.append('chart' if 'chart' in a and 'pic:pic' not in a
                     else 'pic' if 'pic:pic' in a else 'text')
    pic = kinds.index('pic')
    chart = kinds.index('chart')
    text = kinds.index('text')
    print('anchors:', kinds, f'(pic={pic} chart={chart} text={text})')

    CX, CY = 8662670, 6267450          # 682.10 x 493.50 pt, the chart's own extent
    WX, WY = 9000000, 11500000         # 708.66 x 905.51 pt
    SQ = '<wp:wrapSquare wrapText="bothSides"/>'

    made = {
        'picthrough': edit(xml, pic, CX, CY),
        'picsquare': edit(xml, pic, CX, CY, SQ),
        'txtsquare': edit(xml, text, WX, WY),
        'chartnone': edit(xml, chart, wrap='<wp:wrapNone/>'),
    }

    paths = []
    for label, t in made.items():
        p = out / f'{label}.docx'
        with zipfile.ZipFile(p, 'w', zipfile.ZIP_DEFLATED) as z:
            for n in names:
                z.writestr(n, t.encode('utf-8') if n == 'word/document.xml' else blobs[n])
        paths.append(p)

    pdfs = out / 'pdf'
    subprocess.run(['timeout', '-k', '30', '900', REF, '--headless', '--norestore',
                    f'-env:UserInstallation=file://{out / "prof"}',
                    '--convert-to', 'pdf', '--outdir', str(pdfs)] + [str(p) for p in paths],
                   capture_output=True, timeout=950)

    import pymupdf
    for p in paths:
        f = pdfs / (p.stem + '.pdf')
        if not f.exists():
            print(f'{p.stem:12s} NO OUTPUT')
            continue
        page = pymupdf.open(f)[0]
        print(f'{p.stem:12s} page {page.rect.width:.2f} x {page.rect.height:.2f}')
        for im in page.get_image_info():
            b = im['bbox']
            print(f'   image  x {b[0]:8.2f}..{b[2]:8.2f}  y {b[1]:8.2f}..{b[3]:8.2f}'
                  f'   w {b[2]-b[0]:7.2f} h {b[3]-b[1]:7.2f}')
        for d in page.get_drawings():
            r = d['rect']
            if r.width > 250 and r.height > 90:
                print(f'   path   x {r.x0:8.2f}..{r.x1:8.2f}  y {r.y0:8.2f}..{r.y1:8.2f}'
                      f'   w {r.width:7.2f} h {r.height:7.2f}')


main()
