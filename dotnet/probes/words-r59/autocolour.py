#!/usr/bin/env python3
"""An automatic font colour on a shaded background, and what makes a background in the first place.

    python3 autocolour.py <outdir> [workers]

Round 58 pinned the threshold — white when `Color::IsDark()`, and `IsDark()` is
`GetWCAGLuminance() <= 87`, confirmed to the single sRGB step over 22 fills.  Three things it did
not measure, and each one decides a branch of the implementation:

  **A. The one colour where the two luminance functions disagree.**  `Color::IsDark()` is not one
  formula: `tools/source/generic/color.cxx:52` special-cases `COL_DEFAULT_SHAPE_FILLING`
  (`0x729FCF`) and asks `GetLuminance() <= 62` for it instead.  That colour's WCAG luminance is 83
  — dark, so white text — and its perceived luminance is 151 — bright, so black text.  It is the
  only input in the whole domain that separates the two, and a probe that does not include it
  cannot tell which function was ported.

  **B. What counts as "the background" when several disagree.**  `SwDrawTextInfo::ApplyAutoColor`
  (`fntcache.cxx`:2374) asks the *font's* own back colour first and falls back to
  `GetBackgroundBrush(..., bConsiderTextBox=true)`, which walks the frame chain.  So a character
  highlight beats a paragraph shade beats a cell fill, and the probe states all three against each
  other rather than only the cell.

  **C. `w:shd` is a pattern and not a fill.**  `CellColorHandler::getProperties` turns `w:val` into
  a per-mille weight — `clear` 0, `solid` 1000, `pctN` N x 10, every striped value 333 — and blends
  `w:color` over `w:fill` at it; `w:color="auto"` is **black** and `w:fill="auto"` is **white**.
  So `<w:shd w:val="solid" w:color="auto" w:fill="auto"/>` is a black cell, and we read its
  `w:fill="auto"` as no fill at all and draw nothing.  That is three of the eight rectangles
  `AFS-050-004-F2_0i` page 2 fills, and 156 elements in 15 corpus documents.

Refuses to print unless every package produced both halves.
"""
import os
import re
import shutil
import subprocess
import sys
import zipfile
from concurrent.futures import ThreadPoolExecutor

sys.path.insert(0, "/c/sandbox/workdir/wt-words-r50/dotnet/research/probes/slides-r15")
from pdfops import objects, pages, content  # noqa: E402

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PKG_R = 'http://schemas.openxmlformats.org/package/2006/relationships'


def zipup(path, parts):
    over = ''.join(
        f'<Override PartName="/word/{n}" ContentType="application/vnd.openxmlformats-'
        f'officedocument.wordprocessingml.{k}+xml"/>' for n, k in parts.keys())
    ctypes = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
              '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
              'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
              + over + '</Types>')
    rels = ''.join(
        f'<Relationship Id="rId{i + 8}" Type="{R}/{k}" Target="{n}"/>'
        for i, (n, k) in enumerate(parts.keys()) if n != 'document.xml')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ctypes)
        z.writestr('_rels/.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}"><Relationship Id="rId1" Type="{R}'
                   '/officeDocument" Target="word/document.xml"/></Relationships>')
        z.writestr('word/_rels/document.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}">{rels}</Relationships>')
        for (name, _), body in parts.items():
            z.writestr('word/' + name, body)


STYLES = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
          f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
          '<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
          '<w:sz w:val="40"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>'
          '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
          '<w:name w:val="Normal"/></w:style></w:styles>')

SECT = ('<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/></w:sectPr>')


def package(path, body):
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document xmlns:w="{W}"><w:body>{body}<w:p/>{SECT}</w:body></w:document>')
    zipup(path, {('document.xml', 'document.main'): doc, ('styles.xml', 'styles'): STYLES})


def cell(shd, runProperties='', paragraphShd=''):
    return ('<w:tbl><w:tblPr><w:tblW w:w="9000" w:type="dxa"/></w:tblPr>'
            '<w:tblGrid><w:gridCol w:w="9000"/></w:tblGrid>'
            f'<w:tr><w:tc><w:tcPr><w:tcW w:w="9000" w:type="dxa"/>{shd}</w:tcPr>'
            f'<w:p><w:pPr>{paragraphShd}</w:pPr>'
            f'<w:r><w:rPr>{runProperties}</w:rPr><w:t>Reversed out</w:t></w:r></w:p>'
            '</w:tc></w:tr></w:tbl>')


def shd(val='clear', colour='auto', fill='auto'):
    return f'<w:shd w:val="{val}" w:color="{colour}" w:fill="{fill}"/>'


CASES = [
    # A. the one colour where GetLuminance and GetWCAGLuminance disagree.
    ('A/729FCF-the-discriminator', cell(shd(fill='729FCF')),
     'WCAG 83 -> dark -> white; perceived 151 -> bright -> black. Only this input separates them.'),
    ('A/6F9BCB-just-below', cell(shd(fill='6F9BCB')),
     'the same colour one step away, where the special case does not apply'),
    ('A/000000-control', cell(shd(fill='000000')), 'CONTROL: white'),
    ('A/FFFFFF-control', cell(shd(fill='FFFFFF')), 'CONTROL: black'),

    # B. which background wins.
    ('B/cell-dark-para-light', cell(shd(fill='000000'), paragraphShd='<w:shd w:val="clear" '
                                   'w:color="auto" w:fill="FFFFFF"/>'),
     'a white paragraph shade inside a black cell'),
    ('B/cell-light-para-dark', cell(shd(fill='FFFFFF'), paragraphShd='<w:shd w:val="clear" '
                                    'w:color="auto" w:fill="000000"/>'),
     'a black paragraph shade inside a white cell'),
    ('B/cell-dark-run-highlight-light',
     cell(shd(fill='000000'), runProperties='<w:highlight w:val="yellow"/>'),
     'a yellow highlight on a run in a black cell'),
    ('B/cell-light-run-highlight-dark',
     cell(shd(fill='FFFFFF'), runProperties='<w:highlight w:val="darkBlue"/>'),
     'a dark-blue highlight on a run in a white cell'),
    ('B/cell-dark-run-coloured',
     cell(shd(fill='000000'), runProperties='<w:color w:val="FF0000"/>'),
     'CONTROL: a run that states its own colour is not automatic and must stay red'),
    ('B/paragraph-only-dark',
     '<w:p><w:pPr><w:shd w:val="clear" w:color="auto" w:fill="000000"/></w:pPr>'
     '<w:r><w:t>Reversed out</w:t></w:r></w:p>',
     'a black paragraph shade with no table at all'),

    # C. w:shd is a pattern, not a fill.
    ('C/solid-auto-auto', cell(shd(val='solid')),
     'weight 1000 over w:color auto = black: three of AFS-050-004-F2_0i page 2\'s rectangles'),
    ('C/solid-red-auto', cell(shd(val='solid', colour='FF0000')), 'weight 1000 over red'),
    ('C/pct50-auto-auto', cell(shd(val='pct50')), 'half black over white = mid grey'),
    ('C/pct25-auto-auto', cell(shd(val='pct25')), 'a quarter black over white'),
    ('C/pct75-auto-auto', cell(shd(val='pct75')), 'three quarters black over white'),
    ('C/diagStripe-auto-auto', cell(shd(val='diagStripe')), 'every striped value is weight 333'),
    ('C/thinDiagCross-auto-auto', cell(shd(val='thinDiagCross')), 'the same 333'),
    ('C/pct50-red-blue', cell(shd(val='pct50', colour='FF0000', fill='0000FF')),
     'half red over blue'),
    ('C/clear-auto-auto', cell(shd()), 'CONTROL: clear + auto fill is no fill at all'),
    ('C/nil-black', cell(shd(val='nil', fill='000000')), 'CONTROL: nil draws nothing'),
]


def render_ref(src, outdir, slot):
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + os.path.join(outdir, f'prof{slot}'),
         '--headless', '--norestore', '--convert-to', 'pdf',
         '--outdir', os.path.join(outdir, 'ref'), src], capture_output=True, timeout=300)


def render_ours(cli, src, outdir):
    subprocess.run([cli, 'render', src, '--outdir', os.path.join(outdir, 'ours')],
                   capture_output=True, timeout=300)


GLYPH = re.compile(
    rb'(?:([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+rg)|(?:([-\d.]+)\s+g\b)'
    rb'|(\((?:\\.|[^\\()])*\))\s*Tj|(<[0-9A-Fa-f\s]*>)\s*Tj'
    rb'|(\[(?:\\.|[^\\\[\]])*\])\s*TJ')

FILL = re.compile(
    rb'(?:([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+rg)|(?:([-\d.]+)\s+g\b)'
    rb'|([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+re\b|\b(f\*?)\b')


def hexof(triple):
    return '#' + ''.join('%02X' % round(float(v) * 255) for v in triple)


def glyph_colours(pdf):
    data = open(pdf, 'rb').read()
    objs = objects(data)
    out = {}
    for pnum in pages(data, objs):
        colour = '#000000'
        for m in GLYPH.finditer(content(data, objs, pnum)):
            if m.group(1) is not None:
                colour = hexof(m.group(1, 2, 3))
            elif m.group(4) is not None:
                colour = hexof((m.group(4),) * 3)
            else:
                body = m.group(5) or m.group(6) or m.group(7)
                n = 0
                parts = (re.finditer(rb'\((?:\\.|[^\\()])*\)|<[0-9A-Fa-f\s]*>', body)
                         if m.group(7) else [m])
                for part in parts:
                    s = part.group(0) if m.group(7) else (m.group(5) or m.group(6))
                    n += (len(re.sub(rb'\\(\d{1,3}|.)', b'x', s[1:-1])) if s[:1] == b'('
                          else len(re.sub(rb'\s', b'', s[1:-1])) // 2)
                out[colour] = out.get(colour, 0) + n
    return out


def fills(pdf):
    """Filled rectangles bigger than a hairline, as colour -> count."""
    data = open(pdf, 'rb').read()
    objs = objects(data)
    out = {}
    for pnum in pages(data, objs):
        colour = '#000000'
        pending = None
        for m in FILL.finditer(content(data, objs, pnum)):
            if m.group(1) is not None:
                colour = hexof(m.group(1, 2, 3))
            elif m.group(4) is not None:
                colour = hexof((m.group(4),) * 3)
            elif m.group(5) is not None:
                w, h = abs(float(m.group(7))), abs(float(m.group(8)))
                pending = (w, h) if w > 4 and h > 4 else None
            elif m.group(9) is not None and pending:
                out[colour] = out.get(colour, 0) + 1
                pending = None
    return out


def fmt(d):
    return ','.join(f'{k}:{v}' for k, v in sorted(d.items())) or '-'


if __name__ == '__main__':
    out = os.path.abspath(sys.argv[1])
    workers = int(sys.argv[2]) if len(sys.argv) > 2 else 6
    cli = os.environ.get('XRAY_OFFICE_CLI') or (
        '/c/sandbox/workdir/wt-words-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/'
        'linux-x64/XRay.Office.Cli')
    if os.path.isdir(out):
        shutil.rmtree(out)
    for d in ('in', 'ref', 'ours'):
        os.makedirs(os.path.join(out, d), exist_ok=True)

    built = []
    for index, (name, body, why) in enumerate(CASES):
        stem = '%02d-%s' % (index, re.sub(r'[^A-Za-z0-9]+', '-', name))
        path = os.path.join(out, 'in', stem + '.docx')
        package(path, body)
        built.append((name, stem, path, why))

    with ThreadPoolExecutor(workers) as pool:
        list(pool.map(lambda t: render_ref(t[2], out, built.index(t) % workers), built))
    for t in built:
        render_ours(cli, t[2], out)

    missing = [f'{t[1]}: no {side}' for t in built for side in ('ref', 'ours')
               if not os.path.exists(os.path.join(out, side, t[1] + '.pdf'))]
    if missing:
        print('REFUSING TO PRINT — %d of %d halves missing:' % (len(missing), 2 * len(built)))
        for m in missing:
            print('   ', m)
        sys.exit(2)
    print('%d packages, %d halves, all present\n' % (len(built), 2 * len(built)))

    print('%-32s %-26s %-26s %-18s %-18s' % ('case', 'ref glyphs', 'our glyphs',
                                             'ref fills', 'our fills'))
    for name, stem, path, why in built:
        rg = glyph_colours(os.path.join(out, 'ref', stem + '.pdf'))
        og = glyph_colours(os.path.join(out, 'ours', stem + '.pdf'))
        rf = fills(os.path.join(out, 'ref', stem + '.pdf'))
        of = fills(os.path.join(out, 'ours', stem + '.pdf'))
        flag = '' if (rg == og and rf == of) else '   <-- differs'
        print('%-32s %-26s %-26s %-18s %-18s%s'
              % (name, fmt(rg), fmt(og), fmt(rf), fmt(of), flag))
    print()
    for name, stem, path, why in built:
        print('%-32s %s' % (name, why))
