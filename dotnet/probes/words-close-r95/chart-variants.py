#!/usr/bin/env python3
"""One-attribute variants of a `Unit_Circle` document's chart anchor, rendered by the reference.

The 93 fixtures of `make-capture.py` establish that 26.2.4.2 *does* pull a non-wrap-through fly
back inside the sheet in both axes. The three `Unit_Circle` documents contradict that: each states
a chart 682 pt wide at `positionH relativeFrom="margin" posOffset="-221.35pt"` on a 595 pt page,
so the clamp would move it 165 pt right — and the reference draws it where the file states it.

This asks the reference which half of that is wrong, by changing one attribute at a time:

    base      the document as authored
    offset    the chart's `wp:posOffset` alone, -221.35 pt -> -100 pt
    wrapnone  the chart's wrap alone, `wrapSquare`/`wrapTight` -> `wrapNone`
    page      the chart's `relativeFrom` alone, `margin` -> `page`

If `offset` moves the drawn chart by the whole 121.35 pt the chart is not being clamped; if it
moves by less, it is, and the clamp's own arithmetic is what the difference measures.

  chart-variants.py <document.docx> <outdir>
"""
import pathlib
import re
import shutil
import subprocess
import sys
import zipfile

REF = '/opt/libreoffice26.2/program/soffice'


def variants(xml):
    """The document part, once per variant, with exactly one attribute changed."""
    # The chart's own anchor is the one holding a `graphicFrame`-less chart reference: it is the
    # only `wp:anchor` in these three documents that holds neither `pic:pic` nor `wps:txbx`.
    anchors = [m for m in re.finditer(r'<wp:anchor\b.*?</wp:anchor>', xml, re.S)]
    chart = next(m for m in anchors if 'pic:pic' not in m.group(0) and 'wps:txbx' not in m.group(0))
    a = chart.group(0)

    def swap(new):
        return xml[:chart.start()] + new + xml[chart.end():]

    out = {'base': xml}

    m = re.search(r'(<wp:positionH relativeFrom="[^"]*"><wp:posOffset>)(-?\d+)(</wp:posOffset>)', a)
    out['offset'] = swap(a[:m.start(2)] + str(-100 * 12700) + a[m.end(2):])
    out['page'] = swap(a.replace('<wp:positionH relativeFrom="margin">',
                                 '<wp:positionH relativeFrom="page">', 1))
    out['wrapnone'] = swap(re.sub(r'<wp:wrap(Square|Tight)\b.*?(/>|</wp:wrap\1>)', '<wp:wrapNone/>',
                                  a, count=1, flags=re.S))
    return out


def main():
    src = pathlib.Path(sys.argv[1])
    out = pathlib.Path(sys.argv[2]).resolve()
    out.mkdir(parents=True, exist_ok=True)

    with zipfile.ZipFile(src) as z:
        names = z.namelist()
        blobs = {n: z.read(n) for n in names}
    xml = blobs['word/document.xml'].decode('utf-8')

    made = []
    for label, text in variants(xml).items():
        path = out / f'{label}.docx'
        with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
            for n in names:
                z.writestr(n, text.encode('utf-8') if n == 'word/document.xml' else blobs[n])
        made.append(path)

    pdfs = out / 'pdf'
    pdfs.mkdir(exist_ok=True)
    subprocess.run(['timeout', '-k', '30', '900', REF, '--headless',
                    f'-env:UserInstallation=file://{out / "prof"}',
                    '--convert-to', 'pdf', '--outdir', str(pdfs)] + [str(p) for p in made],
                   capture_output=True, timeout=950)

    print(f'{src.name}')
    for path in made:
        pdf = pdfs / (path.stem + '.pdf')
        if not pdf.exists():
            print(f'  {path.stem:<10} NO PDF')
            continue
        text = subprocess.run(['pdftotext', '-bbox', str(pdf), '-'],
                              capture_output=True, text=True).stdout
        xs = [float(m.group(1)) for m in re.finditer(r'<word xMin="([\d.]+)"', text)]
        print(f'  {path.stem:<10} leftmost word x {min(xs):8.2f}   words {len(xs)}'
              if xs else f'  {path.stem:<10} no text')


main()
