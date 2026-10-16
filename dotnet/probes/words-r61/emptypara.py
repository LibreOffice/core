#!/usr/bin/env python3
"""What is a body paragraph between two tables worth, and how much of it is the line?

    python3 emptypara.py <outdir> [workers]

`097_Business_Case_Template` fails the gate at 1 page against 2 and round 59 established that the
reference's page 2 is *empty* — a trailing empty paragraph that does not fit on page 1. Measured on
the corpus document itself, the whole 3.36 pt by which the reference's last table rule sits lower
than ours is spent in **four body paragraphs between tables**: +0.95, +1.00, +1.05, +1.00, against
-0.65 on the one body paragraph that holds an image. Two of the four are empty and two hold a
`<w:br/>`, i.e. two lines — and the deficit is the *same* on both, which is the discriminator: a
line-height deficit would be twice as large on the two-line paragraphs and it is not.

So this probe separates three quantities that a corpus measurement cannot:

  * the **marginal line** — the cost of one more line in the same paragraph;
  * the **per-paragraph constant** — what a paragraph costs beyond its lines;
  * whether either depends on a **table** being on one side of it.

Every case is one authored package rendered by both sides, and the measured quantity is the
baseline of a marker glyph in the last block, which is a text origin in the PDF and needs no
border-geometry reading at all. Slopes are taken across k, so any constant offset — page origin,
first table's own height, border widths — cancels exactly.
"""
import os
import re
import shutil
import subprocess
import sys
import zipfile
from concurrent.futures import ThreadPoolExecutor

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PKG_R = 'http://schemas.openxmlformats.org/package/2006/relationships'


def table(text):
    return ('<w:tbl><w:tblPr><w:tblW w:w="9000" w:type="dxa"/><w:tblBorders>'
            '<w:top w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
            '<w:bottom w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
            '</w:tblBorders></w:tblPr><w:tblGrid><w:gridCol w:w="9000"/></w:tblGrid>'
            '<w:tr><w:tc><w:tcPr><w:tcW w:w="9000" w:type="dxa"/></w:tcPr>'
            f'<w:p><w:r><w:t>{text}</w:t></w:r></w:p></w:tc></w:tr></w:tbl>')


def para(kind, font, size=None):
    """`kind`: empty | br | text."""
    rpr = '<w:rFonts w:ascii="%s" w:hAnsi="%s"/>' % (font, font)
    if size:
        rpr += '<w:sz w:val="%d"/>' % size
    inner = {'empty': '',
             'br': f'<w:r><w:rPr>{rpr}</w:rPr><w:br/></w:r>',
             'brbr': f'<w:r><w:rPr>{rpr}</w:rPr><w:br/><w:br/></w:r>',
             'text': f'<w:r><w:rPr>{rpr}</w:rPr><w:t>x</w:t></w:r>'}[kind]
    return f'<w:p><w:pPr><w:rPr>{rpr}</w:rPr></w:pPr>{inner}</w:p>'


def package(path, body, defaults):
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
              '<w:rFonts w:ascii="Cambria" w:hAnsi="Cambria"/><w:sz w:val="22"/>'
              '</w:rPr></w:rPrDefault><w:pPrDefault><w:pPr>'
              f'{defaults}</w:pPr></w:pPrDefault></w:docDefaults>'
              '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
              '<w:name w:val="Normal"/></w:style></w:styles>')
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document xmlns:w="{W}"><w:body>{body}'
           '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
           '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>'
           '</w:sectPr></w:body></w:document>')
    parts = [('document.xml', 'document.main'), ('styles.xml', 'styles')]
    over = ''.join(
        f'<Override PartName="/word/{n}" ContentType="application/vnd.openxmlformats-'
        f'officedocument.wordprocessingml.{k}+xml"/>' for n, k in parts)
    ctypes = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
              '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.'
              'relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
              + over + '</Types>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ctypes)
        z.writestr('_rels/.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}"><Relationship Id="rId1" Type="{R}'
                   '/officeDocument" Target="word/document.xml"/></Relationships>')
        z.writestr('word/_rels/document.xml.rels',
                   '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                   f'<Relationships xmlns="{PKG_R}"><Relationship Id="rId8" Type="{R}/styles" '
                   'Target="styles.xml"/></Relationships>')
        z.writestr('word/document.xml', doc)
        z.writestr('word/styles.xml', styles)


# (family, defaults, before-marker builder, after) — each family is measured as a slope in k.
D097 = '<w:spacing w:after="160" w:line="259" w:lineRule="auto"/>'
DPLAIN = '<w:spacing w:after="0" w:line="240" w:lineRule="auto"/>'
DAFTER = '<w:spacing w:after="160" w:line="240" w:lineRule="auto"/>'

FAMILIES = [
    # name, defaults, how the k paragraphs are built, what surrounds them
    ('tbl-empty-097', D097, 'empty', 'tbl'),
    ('tbl-br-097', D097, 'br', 'tbl'),
    ('tbl-brbr-097', D097, 'brbr', 'tbl'),
    ('tbl-text-097', D097, 'text', 'tbl'),
    ('par-empty-097', D097, 'empty', 'par'),
    ('par-br-097', D097, 'br', 'par'),
    ('tbl-empty-plain', DPLAIN, 'empty', 'tbl'),
    ('tbl-br-plain', DPLAIN, 'br', 'tbl'),
    ('par-empty-plain', DPLAIN, 'empty', 'par'),
    ('tbl-empty-after', DAFTER, 'empty', 'tbl'),
    ('tbl-br-after', DAFTER, 'br', 'tbl'),
]
KS = (0, 1, 2, 3)


def body_for(kind, surround, k, font='Cambria'):
    mid = ''.join(para(kind, font) for _ in range(k))
    if surround == 'tbl':
        return table('A') + mid + table('MARKER')
    return para('text', font) + mid + '<w:p><w:r><w:t>MARKER</w:t></w:r></w:p>'


def render_ref(src, outdir, slot):
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + os.path.join(outdir, f'prof{slot}'),
         '--headless', '--norestore', '--convert-to', 'pdf',
         '--outdir', os.path.join(outdir, 'ref'), src], capture_output=True, timeout=300)


def render_ours(cli, src, outdir):
    subprocess.run([cli, 'render', src, '--outdir', os.path.join(outdir, 'ours')],
                   capture_output=True, timeout=300)


sys.path.insert(0, "/c/sandbox/workdir/wt-words-r50/dotnet/research/probes/slides-r15")


def marker_y(pdf, expected=2):
    """Top-down y of the LAST text origin, and the page it is on.

    The packages are subset-embedded, so the show strings are glyph indices and not readable
    characters. Every package draws exactly two strings — `A` in the first block and `MARKER` in
    the last — so the last text origin in drawing order *is* the marker, and no decoding is needed.
    Asserted rather than assumed: a package whose page holds other than two show operators is
    reported as a miss and the probe refuses to print.
    """
    from pdfops import objects, pages, content
    data = open(pdf, 'rb').read()
    objs = objects(data)
    shows = []
    for pageno, pn in enumerate(pages(data, objs), 1):
        c = content(data, objs, pn).decode('latin1')
        cur = None
        for m in re.finditer(r'([-\d.]+) ([-\d.]+) Td|'
                             r'([-\d.]+) ([-\d.]+) ([-\d.]+) ([-\d.]+) ([-\d.]+) ([-\d.]+) Tm|'
                             r'(TJ|Tj)', c):
            if m.group(2):
                cur = float(m.group(2))
            elif m.group(8):
                cur = float(m.group(8))
            elif cur is not None:
                shows.append((pageno, round(841.89 - cur, 3)))
    if len(shows) != expected:
        return None, None
    return shows[-1]


if __name__ == '__main__':
    out = os.path.abspath(sys.argv[1])
    workers = int(sys.argv[2]) if len(sys.argv) > 2 else 4
    cli = os.environ.get('XRAY_OFFICE_CLI') or (
        '/c/sandbox/workdir/wt-words-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/'
        'linux-x64/XRay.Office.Cli')
    if os.path.isdir(out):
        shutil.rmtree(out)
    for d in ('in', 'ref', 'ours'):
        os.makedirs(os.path.join(out, d), exist_ok=True)

    built = []
    for name, defaults, kind, surround in FAMILIES:
        for k in KS:
            stem = '%s-k%d' % (name, k)
            path = os.path.join(out, 'in', stem + '.docx')
            package(path, body_for(kind, surround, k), defaults)
            built.append((name, k, stem, path))

    with ThreadPoolExecutor(workers) as pool:
        list(pool.map(lambda t: render_ref(t[3], out, built.index(t) % workers), built))
    with ThreadPoolExecutor(2) as pool:
        list(pool.map(lambda t: render_ours(cli, t[3], out), built))

    missing = ['%s: no %s' % (t[2], side) for t in built for side in ('ref', 'ours')
               if not os.path.exists(os.path.join(out, side, t[2] + '.pdf'))]
    if missing:
        print('REFUSING TO PRINT — %d halves missing:' % len(missing))
        for m in missing:
            print('   ', m)
        sys.exit(2)

    ys = {}
    for name, k, stem, path in built:
        expected = 2 + (k if name.split('-')[1] == 'text' else 0)
        ys[(name, k, 'ref')] = marker_y(os.path.join(out, 'ref', stem + '.pdf'), expected)
        ys[(name, k, 'ours')] = marker_y(os.path.join(out, 'ours', stem + '.pdf'), expected)
    blind = [k for k, v in ys.items() if v[1] is None]
    if blind:
        print('REFUSING TO PRINT — %d markers not found: %s' % (len(blind), blind[:6]))
        sys.exit(2)

    print('%d packages, %d halves, all present and all markers found\n' % (len(built), 2 * len(built)))
    print('%-18s %-34s %-34s %s' % ('family', 'reference marker y by k', 'ours', 'slope ref / ours'))
    for name, defaults, kind, surround in FAMILIES:
        r = [ys[(name, k, 'ref')][1] for k in KS]
        o = [ys[(name, k, 'ours')][1] for k in KS]
        sr = [round(b - a, 2) for a, b in zip(r, r[1:])]
        so = [round(b - a, 2) for a, b in zip(o, o[1:])]
        print('%-18s %-34s %-34s %s / %s' % (name, r, o, sr, so))
