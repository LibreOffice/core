#!/usr/bin/env python3
"""The same claim as `fallback-oblique.py`, in the OOXML formats the other two tracks are made of.

    python3 fallback-oblique-ooxml.py <outdir> [workers]

`fallback-oblique.py` establishes the rule through four filters, of which two — `.fodp` and
`.fods` — reach the slides and sheets layouts.  Neither extension occurs in either corpus: the
slides track is 251 `.pptx` and 51 `.ppt`, the sheets track is `.xlsx`/`.xls`.  So the ODF arms
prove the *rule* and prove nothing about the formats a cross-track sweep would actually move,
and this probe closes that gap by authoring the same two-run paragraph as `.pptx` and `.xlsx`.

The package skeletons are round 54's (`probes/words-r54/cross-format-fallback.py`), which are
known to be read correctly by 26.2.4.2 — including the trap recorded in `TODO.24-2-7-audit.md`
that an `.xlsx` with no `<cellStyles>` has its `cellXf` font discarded.  Two runs per file, one
upright and one carrying the varied property, so the second run's sheared-glyph count is the
only quantity that can move.

Refuses to print unless every package produced both halves.
"""
import importlib.util
import os
import re
import subprocess
import sys
import zipfile
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location(
    "shearfaces", os.path.join(HERE, "..", "words-r56", "shear-faces.py"))
shearfaces = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(shearfaces)

R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
P = 'http://schemas.openxmlformats.org/presentationml/2006/main'

PLAIN = 'Upright aaaa '
CJK = '手机免提系统'
SYM = '☐☒➢✦'
HEB = 'אבגד'
LATIN = 'Slanted bbbb'
NONESUCH = 'Zqxwv Nonesuch'


def rels(body):
    return ('<?xml version="1.0"?><Relationships xmlns="http://schemas.openxmlformats.org/'
            'package/2006/relationships">' + body + '</Relationships>')


def cases():
    """(name, family, text, italic, bold, expected)"""
    return [
        ('latin-italic',    'Arial',    LATIN, True,  False, 'nothing shears (control)'),
        ('cjk-upright',     'Arial',    CJK,   False, False, 'nothing shears (control)'),
        ('cjk-italic',      'Arial',    CJK,   True,  False, 'shears run 2'),
        ('sym-italic',      'Arial',    SYM,   True,  False, 'shears run 2'),
        ('heb-italic',      'Carlito',  HEB,   True,  False, 'shears run 2, in DejaVu Sans'),
        ('cjk-italic-none', NONESUCH,   CJK,   True,  False, 'shears run 2'),
        ('cjk-bold-italic', 'Arial',    CJK,   True,  True,  'shears run 2'),
        ('latin-italic-none', NONESUCH, LATIN, True,  False, 'shears run 2 (round 56 control)'),
    ]


# ---------------------------------------------------------------------------- XLSX

def xlsx(path, *, family, text, italic, bold):
    fonts = (f'<font><sz val="20"/><name val="{family}"/></font>'
             f'<font><sz val="20"/><name val="{family}"/>'
             + ('<i/>' if italic else '') + ('<b/>' if bold else '') + '</font>')
    styles = ('<?xml version="1.0"?><styleSheet xmlns="http://schemas.openxmlformats.org/'
              'spreadsheetml/2006/main">'
              f'<fonts count="2">{fonts}</fonts>'
              '<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
              '<borders count="1"><border/></borders>'
              '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/>'
              '</cellStyleXfs><cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" '
              'borderId="0" applyFont="1"/></cellXfs>'
              # `cellStyles` is not decoration: without it LibreOffice discards the cellXf font
              # entirely and every case reads back at 10 pt.  See TODO.24-2-7-audit.md.
              '<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/>'
              '</cellStyles></styleSheet>')
    run2 = ('<rPr><sz val="20"/><rFont val="' + family + '"/>'
            + ('<i/>' if italic else '') + ('<b/>' if bold else '') + '</rPr>')
    sheet = ('<?xml version="1.0"?><worksheet xmlns="http://schemas.openxmlformats.org/'
             'spreadsheetml/2006/main"><sheetData><row r="1">'
             '<c r="A1" s="0" t="inlineStr"><is>'
             f'<r><rPr><sz val="20"/><rFont val="{family}"/></rPr>'
             f'<t xml:space="preserve">{PLAIN}</t></r>'
             f'<r>{run2}<t xml:space="preserve">{text}</t></r>'
             '</is></c></row></sheetData></worksheet>')
    book = ('<?xml version="1.0"?><workbook xmlns="http://schemas.openxmlformats.org/'
            f'spreadsheetml/2006/main" xmlns:r="{R}"><sheets>'
            '<sheet name="S" sheetId="1" r:id="rId1"/></sheets></workbook>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml',
                   '<?xml version="1.0"?><Types xmlns="http://schemas.openxmlformats.org/'
                   'package/2006/content-types">'
                   '<Default Extension="rels" ContentType="application/vnd.openxmlformats-'
                   'package.relationships+xml"/>'
                   '<Default Extension="xml" ContentType="application/xml"/>'
                   '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.'
                   'openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
                   '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.'
                   'openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
                   '<Override PartName="/xl/styles.xml" ContentType="application/vnd.'
                   'openxmlformats-officedocument.spreadsheetml.styles+xml"/></Types>')
        z.writestr('_rels/.rels',
                   rels(f'<Relationship Id="rId1" Type="{R}/officeDocument" '
                        'Target="xl/workbook.xml"/>'))
        z.writestr('xl/_rels/workbook.xml.rels',
                   rels(f'<Relationship Id="rId1" Type="{R}/worksheet" '
                        f'Target="worksheets/sheet1.xml"/><Relationship Id="rId2" '
                        f'Type="{R}/styles" Target="styles.xml"/>'))
        z.writestr('xl/workbook.xml', book)
        z.writestr('xl/worksheets/sheet1.xml', sheet)
        z.writestr('xl/styles.xml', styles)


# ---------------------------------------------------------------------------- PPTX

def pptx(path, *, family, text, italic, bold):
    theme = (f'<?xml version="1.0"?><a:theme xmlns:a="{A}" name="t"><a:themeElements>'
             '<a:clrScheme name="c">' +
             ''.join(f'<a:{k}><a:srgbClr val="000000"/></a:{k}>' for k in
                     ('dk1', 'lt1', 'dk2', 'lt2', 'accent1', 'accent2', 'accent3', 'accent4',
                      'accent5', 'accent6', 'hlink', 'folHlink')) +
             '</a:clrScheme>'
             f'<a:fontScheme name="f"><a:majorFont><a:latin typeface="{family}"/>'
             '<a:ea typeface=""/><a:cs typeface=""/></a:majorFont>'
             f'<a:minorFont><a:latin typeface="{family}"/><a:ea typeface=""/>'
             '<a:cs typeface=""/></a:minorFont></a:fontScheme>'
             '<a:fmtScheme name="s"><a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/>'
             '</a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
             '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>'
             '<a:lnStyleLst><a:ln><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>'
             '<a:ln><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>'
             '<a:ln><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln></a:lnStyleLst>'
             '<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle>'
             '<a:effectStyle><a:effectLst/></a:effectStyle>'
             '<a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>'
             '<a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
             '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill>'
             '<a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst>'
             '</a:fmtScheme></a:themeElements></a:theme>')
    master = (f'<?xml version="1.0"?><p:sldMaster xmlns:a="{A}" xmlns:p="{P}" xmlns:r="{R}">'
              '<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/>'
              '<p:nvPr/></p:nvGrpSpPr><p:grpSpPr/></p:spTree></p:cSld>'
              '<p:clrMap bg1="lt1" tx1="dk1" bg2="lt2" tx2="dk2" accent1="accent1" '
              'accent2="accent2" accent3="accent3" accent4="accent4" accent5="accent5" '
              'accent6="accent6" hlink="hlink" folHlink="folHlink"/>'
              '<p:sldLayoutIdLst><p:sldLayoutId id="2147483649" r:id="rId1"/></p:sldLayoutIdLst>'
              '</p:sldMaster>')
    layout = (f'<?xml version="1.0"?><p:sldLayout xmlns:a="{A}" xmlns:p="{P}" xmlns:r="{R}" '
              'type="blank"><p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/>'
              '<p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/></p:spTree></p:cSld>'
              '<p:clrMapOvr><a:overrideClrMapping bg1="lt1" tx1="dk1" bg2="lt2" tx2="dk2" '
              'accent1="accent1" accent2="accent2" accent3="accent3" accent4="accent4" '
              'accent5="accent5" accent6="accent6" hlink="hlink" folHlink="folHlink"/>'
              '</p:clrMapOvr></p:sldLayout>')
    attrs = ' sz="2000"' + (' b="1"' if bold else '') + (' i="1"' if italic else '')
    faces = (f'<a:latin typeface="{family}"/><a:ea typeface="{family}"/>'
             f'<a:cs typeface="{family}"/>')
    slide = (f'<?xml version="1.0"?><p:sld xmlns:a="{A}" xmlns:p="{P}" xmlns:r="{R}">'
             '<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/>'
             '<p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>'
             '<p:sp><p:nvSpPr><p:cNvPr id="2" name="t"/><p:cNvSpPr txBox="1"/><p:nvPr/>'
             '</p:nvSpPr><p:spPr><a:xfrm><a:off x="457200" y="457200"/>'
             '<a:ext cx="8000000" cy="1400000"/></a:xfrm>'
             '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></p:spPr>'
             '<p:txBody><a:bodyPr/><a:lstStyle/><a:p>'
             f'<a:r><a:rPr lang="en-US" sz="2000">{faces}</a:rPr>'
             f'<a:t>{PLAIN}</a:t></a:r>'
             f'<a:r><a:rPr lang="en-US"{attrs}>{faces}</a:rPr>'
             f'<a:t>{text}</a:t></a:r>'
             '</a:p></p:txBody></p:sp></p:spTree></p:cSld></p:sld>')
    pres = (f'<?xml version="1.0"?><p:presentation xmlns:a="{A}" xmlns:p="{P}" xmlns:r="{R}">'
            '<p:sldMasterIdLst><p:sldMasterId id="2147483648" r:id="rId1"/></p:sldMasterIdLst>'
            '<p:sldIdLst><p:sldId id="256" r:id="rId2"/></p:sldIdLst>'
            '<p:sldSz cx="9144000" cy="6858000"/><p:notesSz cx="6858000" cy="9144000"/>'
            '</p:presentation>')
    ct = ('<?xml version="1.0"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/'
          'content-types"><Default Extension="rels" ContentType="application/vnd.'
          'openxmlformats-package.relationships+xml"/>'
          '<Default Extension="xml" ContentType="application/xml"/>'
          '<Override PartName="/ppt/presentation.xml" ContentType="application/vnd.'
          'openxmlformats-officedocument.presentationml.presentation.main+xml"/>'
          '<Override PartName="/ppt/slides/slide1.xml" ContentType="application/vnd.'
          'openxmlformats-officedocument.presentationml.slide+xml"/>'
          '<Override PartName="/ppt/slideLayouts/slideLayout1.xml" ContentType="application/vnd.'
          'openxmlformats-officedocument.presentationml.slideLayout+xml"/>'
          '<Override PartName="/ppt/slideMasters/slideMaster1.xml" ContentType="application/vnd.'
          'openxmlformats-officedocument.presentationml.slideMaster+xml"/>'
          '<Override PartName="/ppt/theme/theme1.xml" ContentType="application/vnd.'
          'openxmlformats-officedocument.theme+xml"/></Types>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ct)
        z.writestr('_rels/.rels',
                   rels(f'<Relationship Id="rId1" Type="{R}/officeDocument" '
                        'Target="ppt/presentation.xml"/>'))
        z.writestr('ppt/_rels/presentation.xml.rels',
                   rels(f'<Relationship Id="rId1" Type="{R}/slideMaster" '
                        f'Target="slideMasters/slideMaster1.xml"/>'
                        f'<Relationship Id="rId2" Type="{R}/slide" '
                        f'Target="slides/slide1.xml"/>'))
        z.writestr('ppt/slideMasters/_rels/slideMaster1.xml.rels',
                   rels(f'<Relationship Id="rId1" Type="{R}/slideLayout" '
                        f'Target="../slideLayouts/slideLayout1.xml"/>'
                        f'<Relationship Id="rId2" Type="{R}/theme" '
                        f'Target="../theme/theme1.xml"/>'))
        z.writestr('ppt/slideLayouts/_rels/slideLayout1.xml.rels',
                   rels(f'<Relationship Id="rId1" Type="{R}/slideMaster" '
                        f'Target="../slideMasters/slideMaster1.xml"/>'))
        z.writestr('ppt/slides/_rels/slide1.xml.rels',
                   rels(f'<Relationship Id="rId1" Type="{R}/slideLayout" '
                        f'Target="../slideLayouts/slideLayout1.xml"/>'))
        z.writestr('ppt/presentation.xml', pres)
        z.writestr('ppt/slideMasters/slideMaster1.xml', master)
        z.writestr('ppt/slideLayouts/slideLayout1.xml', layout)
        z.writestr('ppt/slides/slide1.xml', slide)
        z.writestr('ppt/theme/theme1.xml', theme)


BUILDERS = [('pptx', pptx), ('xlsx', xlsx)]


def render_ref(src, outdir, slot):
    profile = os.path.join(outdir, f'prof{slot}')
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + profile, '--headless', '--norestore',
         '--convert-to', 'pdf', '--outdir', os.path.join(outdir, 'ref'), src],
        capture_output=True, timeout=300)


def render_ours(cli, src, outdir):
    subprocess.run([cli, 'render', src, '--outdir', os.path.join(outdir, 'ours')],
                   capture_output=True, timeout=300)


def lean_of(pdf):
    lean, flat = shearfaces.census(pdf)
    return (sum(lean.values()), sum(flat.values()),
            {shearfaces.strip(k): v for k, v in lean.items() if v})


if __name__ == '__main__':
    out = os.path.abspath(sys.argv[1])
    workers = int(sys.argv[2]) if len(sys.argv) > 2 else 6
    cli = os.environ.get('XRAY_OFFICE_CLI') or (
        '/c/sandbox/workdir/wt-words-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/'
        'linux-x64/XRay.Office.Cli')
    for d in ('in', 'ref', 'ours'):
        os.makedirs(os.path.join(out, d), exist_ok=True)

    built = []
    n = 0
    for name, family, text, italic, bold, expect in cases():
        for ext, build in BUILDERS:
            stem = '%02d-%s-%s' % (n, re.sub(r'[^A-Za-z0-9]+', '-', name), ext)
            path = os.path.join(out, 'in', stem + '.' + ext)
            build(path, family=family, text=text, italic=italic, bold=bold)
            built.append((name, ext, stem, path, expect))
            n += 1

    with ThreadPoolExecutor(workers) as pool:
        list(pool.map(lambda t: render_ref(t[3], out, built.index(t) % workers), built))
    for t in built:
        render_ours(cli, t[3], out)

    missing = [f'{stem}: no {side}'
               for _, _, stem, _, _ in built for side in ('ref', 'ours')
               if not os.path.exists(os.path.join(out, side, stem + '.pdf'))]
    if missing:
        print('REFUSING TO PRINT — %d of %d halves missing:' % (len(missing), 2 * len(built)))
        for m in missing:
            print('   ', m)
        sys.exit(2)

    print('%d packages, %d halves, all present\n' % (len(built), 2 * len(built)))
    print(f"{'case':20s} {'fmt':5s} {'ref lean':>8} {'our lean':>8} {'ref flat':>8} "
          f"{'our flat':>8}  {'ref leaning faces':30s} {'our leaning faces':30s} expected")
    for name, ext, stem, path, expect in built:
        rl, rf, rfaces = lean_of(os.path.join(out, 'ref', stem + '.pdf'))
        ol, of, ofaces = lean_of(os.path.join(out, 'ours', stem + '.pdf'))
        fr = ','.join(f'{k}:{v}' for k, v in sorted(rfaces.items())) or '-'
        fo = ','.join(f'{k}:{v}' for k, v in sorted(ofaces.items())) or '-'
        print(f'{name:20s} {ext:5s} {rl:8d} {ol:8d} {rf:8d} {of:8d}  {fr:30s} {fo:30s} {expect}')
