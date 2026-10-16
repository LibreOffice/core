#!/usr/bin/env python3
"""What size is the square a legacy FORMCHECKBOX draws, and what decides it?

    python3 formcheckbox.py <outdir> [workers]

The words track's record has said since round 38 that these are "established, deliberately not
implemented", because "the drawn square's size would not pin (9.0…15.9 pt, not following
`w:checkBox/w:size`)".  That is a claim about a superseded binary and it was measured against the
wrong candidate: `sw/source/core/text/portxt.cxx`:1492 sets the portion's width and height to
`rInf.GetTextHeight()` and its ascent to `rInf.GetAscent()`, and
`SwTextPaintInfo::DrawCheckBox` (`inftxt.cxx`:1247) then insets the drawn rectangle by a hard
`delta = 25` twips on every side.  So the size follows the **line's text height**, which varies
with the font size and the face — and 9.0…15.9 pt is exactly what a range of font sizes looks
like.  `w:checkBox/w:size` is expected to do nothing at all.

This measures it: one square per package, read straight out of the reference PDF's stroked
rectangles, over six font sizes, three faces, both `w:sizeAuto` and a stated `w:size` that
disagrees with the run's own, and checked as well as unchecked.

A *control runs first*: `size-12/serif` is rendered twice from byte-identical input, and the two
readings must agree exactly before any of the rest is believed.
"""
import os
import re
import subprocess
import sys
import zipfile
from concurrent.futures import ThreadPoolExecutor

sys.path.insert(0, "/c/sandbox/workdir/wt-words-r50/dotnet/research/probes/slides-r15")
from pdfops import objects, pages, content  # noqa: E402

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PKG_R = 'http://schemas.openxmlformats.org/package/2006/relationships'


def package(path, *, half_points=24, family='Liberation Serif',
            box_size=None, checked=False):
    """One paragraph: a FORMCHECKBOX field, then a word of ordinary text."""
    box = ('<w:checkBox>'
           + (f'<w:size w:val="{box_size}"/>' if box_size else '<w:sizeAuto/>')
           + f'<w:default w:val="{1 if checked else 0}"/></w:checkBox>')
    rpr = (f'<w:rPr><w:rFonts w:ascii="{family}" w:hAnsi="{family}"/>'
           f'<w:sz w:val="{half_points}"/><w:szCs w:val="{half_points}"/></w:rPr>')
    field = (f'<w:r>{rpr}<w:fldChar w:fldCharType="begin"><w:ffData>'
             '<w:name w:val="Check1"/><w:enabled/><w:calcOnExit w:val="0"/>'
             f'{box}</w:ffData></w:fldChar></w:r>'
             f'<w:r>{rpr}<w:instrText xml:space="preserve"> FORMCHECKBOX </w:instrText></w:r>'
             f'<w:r>{rpr}<w:fldChar w:fldCharType="end"/></w:r>')
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document xmlns:w="{W}"><w:body>'
           f'<w:p>{field}<w:r>{rpr}<w:t xml:space="preserve">Hx</w:t></w:r></w:p>'
           '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
           '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>'
           '</w:sectPr></w:body></w:document>')
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
              f'<w:rFonts w:ascii="{family}" w:hAnsi="{family}"/>'
              f'<w:sz w:val="{half_points}"/></w:rPr></w:rPrDefault>'
              '<w:pPrDefault/></w:docDefaults></w:styles>')
    settings = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                f'<w:settings xmlns:w="{W}"><w:documentProtection w:edit="forms" '
                'w:enforcement="0"/></w:settings>')
    parts = [('document.xml', 'document.main'), ('styles.xml', 'styles'),
             ('settings.xml', 'settings')]
    over = ''.join(
        f'<Override PartName="/word/{n}" ContentType="application/vnd.openxmlformats-'
        f'officedocument.wordprocessingml.{k}+xml"/>' for n, k in parts)
    ctypes = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
              '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
              'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
              + over + '</Types>')
    rels = [f'<Relationship Id="rId1" Type="{R}/officeDocument" Target="document.xml"/>',
            f'<Relationship Id="rId8" Type="{R}/styles" Target="styles.xml"/>',
            f'<Relationship Id="rId7" Type="{R}/settings" Target="settings.xml"/>']
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ctypes)
        z.writestr('_rels/.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}"><Relationship Id="rId1" Type="{R}'
                   '/officeDocument" Target="word/document.xml"/></Relationships>')
        z.writestr('word/_rels/document.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}">{"".join(rels)}</Relationships>')
        z.writestr('word/document.xml', doc)
        z.writestr('word/styles.xml', styles)
        z.writestr('word/settings.xml', settings)


# `re S` only: a bare `re` is also how the page's own clip and its background are written, and
# taking the first one read the A4 media box on every case.  The checkbox is the only *stroked*
# rectangle these fixtures draw.
RECT = re.compile(rb"(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+re\s+S\b")
LINE = re.compile(rb"(-?[\d.]+)\s+(-?[\d.]+)\s+m\s+(-?[\d.]+)\s+(-?[\d.]+)\s+l")
TD = re.compile(rb"(-?[\d.]+)\s+(-?[\d.]+)\s+Td")


def squares(pdf):
    data = open(pdf, "rb").read()
    objs = objects(data)
    out = []
    baselines = []
    for pnum in pages(data, objs):
        stream = content(data, objs, pnum)
        for m in RECT.finditer(stream):
            x, y, w, h = (float(m.group(i)) for i in range(1, 5))
            out.append((x, y, w, h))
        for m in TD.finditer(stream):
            baselines.append((float(m.group(1)), float(m.group(2))))
        diagonals = len(LINE.findall(stream))
    return out, baselines, diagonals


def render(docx, outdir, slot):
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + os.path.join(outdir, f'prof{slot}'),
         '--headless', '--norestore', '--convert-to', 'pdf',
         '--outdir', os.path.join(outdir, 'ref'), docx],
        capture_output=True, timeout=300)


def cases():
    out = [('control-a', dict(half_points=24)), ('control-b', dict(half_points=24))]
    for hp in (16, 20, 24, 28, 36, 48, 80):
        out.append((f'serif/{hp/2:g}pt', dict(half_points=hp)))
    for fam in ('Liberation Sans', 'Liberation Mono', 'DejaVu Sans', 'Carlito'):
        out.append((f'{fam}/12pt', dict(half_points=24, family=fam)))
    # `w:checkBox/w:size` stated, and stated to disagree with the run's own size.
    for sz in (10, 20, 40, 80):
        out.append((f'boxsize-{sz}/12pt', dict(half_points=24, box_size=sz)))
    out.append(('checked/12pt', dict(half_points=24, checked=True)))
    out.append(('checked/24pt', dict(half_points=48, checked=True)))
    return out


if __name__ == '__main__':
    out = os.path.abspath(sys.argv[1])
    workers = int(sys.argv[2]) if len(sys.argv) > 2 else 4
    os.makedirs(os.path.join(out, 'in'), exist_ok=True)
    os.makedirs(os.path.join(out, 'ref'), exist_ok=True)
    built = []
    for i, (name, kw) in enumerate(cases()):
        safe = re.sub(r'[^A-Za-z0-9]+', '-', name).strip('-')
        path = os.path.join(out, 'in', safe + '.docx')
        package(path, **kw)
        built.append((name, safe, path, kw, i))
    with ThreadPoolExecutor(workers) as pool:
        list(pool.map(lambda t: render(t[2], out, t[4] % workers), built))

    print(f"{'case':22s} {'pt':>6} {'squares':>8} {'side pt':>9} {'x':>9} {'y':>9} "
          f"{'baseline':>9} {'top-base':>9} {'text-x':>9} {'lines':>6}")
    for name, safe, path, kw, _ in built:
        pdf = os.path.join(out, 'ref', safe + '.pdf')
        if not os.path.exists(pdf):
            print(f'{name:22s}  !! no reference rendering')
            continue
        rects, baselines, diag = squares(pdf)
        pt = kw.get('half_points', 24) / 2
        if not rects:
            print(f'{name:22s} {pt:6g} {0:8d}')
            continue
        x, y, w, h = rects[0]
        bx, by = baselines[0] if baselines else (float('nan'), float('nan'))
        print(f'{name:22s} {pt:6g} {len(rects):8d} {w:9.3f} {x:9.3f} {y:9.3f} '
              f'{by:9.3f} {y + h - by:9.3f} {bx - x:9.3f} {diag:6d}')
