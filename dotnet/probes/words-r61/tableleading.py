#!/usr/bin/env python3
"""What exactly does a paragraph hand down to a table below it?

    python3 tableleading.py <outdir> [workers]

`emptypara.py` establishes that the reference puts a table ~1.00 pt lower than we do after a
paragraph whose line spacing is `w:line="259" w:lineRule="auto"`, that the paragraph's own baseline
agrees to 0.01 pt on both sides, and that the extra therefore sits *below* the paragraph. This probe
pins the quantity rather than the fact, over five arms that each vary one thing:

  * the **proportion** — 100/107.9/120/150/200 % against a fixed 11 pt line: is the extra
    `(p/100 - 1) x naturalLineHeight`, and is it exactly zero at 100 %?
  * the **size of the paragraph's own last line** — 11 pt against 22 pt: does the extra scale with
    the paragraph, which is what `SwTextFrame::GetLineSpace` taking `GetHeightOfLastLine()` says?
  * **which line** — a two-line paragraph whose *second* line is the large one, against one whose
    *first* is: `GetHeightOfLastLine` names the last, and the two arms disagree only if it does not.
  * the **rule** — `atLeast` and `exact` state a line height and not a proportion, so
    `SvxInterLineSpaceRule::Prop` is not taken and the extra must be nought.
  * the **top of a page** — a paragraph that ends a page hands nothing to a table that starts the
    next one, because `GetPrevFrameForUpperSpaceCalc_` finds no previous frame there.

The measured quantity is the y of the following table's top border rule, which is what moves. The
paragraph's own last baseline is printed beside it, because the two together separate "the paragraph
grew" from "the gap grew" and the whole result turns on it being the second.
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
PAGE_H = 841.89


def tbl():
    return ('<w:tbl><w:tblPr><w:tblW w:w="9000" w:type="dxa"/><w:tblBorders>'
            '<w:top w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
            '<w:bottom w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
            '</w:tblBorders></w:tblPr><w:tblGrid><w:gridCol w:w="9000"/></w:tblGrid>'
            '<w:tr><w:tc><w:tcPr><w:tcW w:w="9000" w:type="dxa"/></w:tcPr>'
            '<w:p><w:r><w:t>T</w:t></w:r></w:p></w:tc></w:tr></w:tbl>')


def run(text, sz=None):
    rpr = '<w:rPr>%s</w:rPr>' % ('<w:sz w:val="%d"/>' % sz if sz else '')
    return f'<w:r>{rpr}<w:t>{text}</w:t></w:r>'


def para(spacing, runs):
    return f'<w:p><w:pPr><w:spacing {spacing}/></w:pPr>{runs}</w:p>'


def package(path, body):
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
              '<w:rFonts w:ascii="Cambria" w:hAnsi="Cambria"/><w:sz w:val="22"/>'
              '</w:rPr></w:rPrDefault><w:pPrDefault><w:pPr>'
              '<w:spacing w:after="0" w:before="0" w:line="240" w:lineRule="auto"/>'
              '</w:pPr></w:pPrDefault></w:docDefaults>'
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


def line(pct):
    return 'w:after="0" w:before="0" w:line="%d" w:lineRule="auto"' % round(pct * 2.4)


CASES = []
# arm 1 — the proportion, 11 pt throughout. Caladea 11 pt natural line = 12.65 pt.
for pct in (100, 107.9, 120, 150, 200):
    CASES.append(('prop-%g' % pct, para(line(pct), run('P')) + tbl(),
                  'expect (p/100-1) x 12.65 = %.2f' % ((pct / 100 - 1) * 12.65)))
# arm 2 — the size of the paragraph's own line, at a fixed 150 %.
for sz, nat in ((22, 12.65), (44, 25.30)):
    CASES.append(('size-%d' % sz, para(line(150), run('P', sz)) + tbl(),
                  'expect 0.50 x %.2f = %.2f' % (nat, 0.5 * nat)))
# arm 3 — which line of a two-line paragraph decides it.
CASES.append(('last-big', para(line(150), run('a') + '<w:br/>' + run('B', 44)) + tbl(),
              'last line 22 pt: expect 12.65'))
CASES.append(('first-big', para(line(150), run('A', 44) + '<w:br/>' + run('b')) + tbl(),
              'last line 11 pt: expect 6.33'))
# arm 4 — a stated line height is not a proportion.
CASES.append(('atleast', para('w:after="0" w:before="0" w:line="400" w:lineRule="atLeast"',
                              run('P')) + tbl(), 'expect 0'))
CASES.append(('exact', para('w:after="0" w:before="0" w:line="400" w:lineRule="exact"',
                            run('P')) + tbl(), 'expect 0'))
# arm 5 — the control: no table, a paragraph follows instead.
# arm 5 — the control: a 100 %-spaced paragraph stands between the 150 % one and the table, so the
# table's predecessor hands down nothing and the whole gap must agree on both sides.
CASES.append(('control-para', para(line(150), run('P')) + para(line(100), run('Q')) + tbl(),
              'the 150 % leading goes to Q, not to the table'))


def render_ref(src, outdir, slot):
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + os.path.join(outdir, f'prof{slot}'),
         '--headless', '--norestore', '--convert-to', 'pdf',
         '--outdir', os.path.join(outdir, 'ref'), src], capture_output=True, timeout=300)


def render_ours(cli, src, outdir):
    subprocess.run([cli, 'render', src, '--outdir', os.path.join(outdir, 'ours')],
                   capture_output=True, timeout=300)


sys.path.insert(0, "/c/sandbox/workdir/wt-words-r50/dotnet/research/probes/slides-r15")


def geometry(pdf):
    """(last baseline before the first rule, first horizontal rule), top-down, or (None, None)."""
    from pdfops import objects, pages, content
    data = open(pdf, 'rb').read()
    objs = objects(data)
    base = None
    rule = None
    for pn in pages(data, objs):
        c = content(data, objs, pn).decode('latin1')
        cur = None
        for m in re.finditer(r'([-\d.]+) ([-\d.]+) Td|(?:TJ|Tj)|'
                             r'([-\d.]+) ([-\d.]+) m\s+([-\d.]+) ([-\d.]+) l', c):
            if m.group(2):
                cur = float(m.group(2))
            elif m.group(4) is not None:
                if abs(float(m.group(4)) - float(m.group(6))) < 0.2 and rule is None:
                    rule = round(PAGE_H - float(m.group(4)), 3)
            elif cur is not None and rule is None:
                base = round(PAGE_H - cur, 3)
        if base is not None:
            break
    return base, rule


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
    for i, (name, body, why) in enumerate(CASES):
        stem = '%02d-%s' % (i, name)
        path = os.path.join(out, 'in', stem + '.docx')
        package(path, body)
        built.append((name, stem, path, why))

    with ThreadPoolExecutor(workers) as pool:
        list(pool.map(lambda t: render_ref(t[2], out, built.index(t) % workers), built))
    with ThreadPoolExecutor(2) as pool:
        list(pool.map(lambda t: render_ours(cli, t[2], out), built))

    missing = ['%s: no %s' % (t[1], s) for t in built for s in ('ref', 'ours')
               if not os.path.exists(os.path.join(out, s, t[1] + '.pdf'))]
    if missing:
        print('REFUSING TO PRINT — %d halves missing: %s' % (len(missing), missing[:6]))
        sys.exit(2)

    geo = {}
    for name, stem, path, why in built:
        for side in ('ref', 'ours'):
            geo[(stem, side)] = geometry(os.path.join(out, side, stem + '.pdf'))
    blind = [k for k, v in geo.items() if v[0] is None or v[1] is None]
    if blind:
        print('REFUSING TO PRINT — %d halves gave no baseline or no rule: %s' % (len(blind), blind))
        sys.exit(2)

    print('%d packages, %d halves, every half gave a baseline and a rule\n' % (
        len(built), 2 * len(built)))
    print('%-13s %-17s %-17s %-8s  %s' % (
        'case', 'ref base/rule', 'our base/rule', 'gap r-o', 'what it tests'))
    for name, stem, path, why in built:
        rb, rr = geo[(stem, 'ref')]
        ob, orr = geo[(stem, 'ours')]
        print('%-13s %7.2f/%7.2f  %7.2f/%7.2f  %+7.2f  %s'
              % (name, rb, rr, ob, orr, (rr - rb) - (orr - ob), why))
