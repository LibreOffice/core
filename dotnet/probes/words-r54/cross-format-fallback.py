#!/usr/bin/env python3
"""The same question — an unrecognised family, which DejaVu? — asked of every filter.

    python3 cross-format-fallback.py <outdir>

`font-fallback-rule.py` establishes that LibreOffice 26.2.4.2 answers an unrecognised family
**DejaVu Serif** through the DOCX filter and **fontconfig's own generic** through the ODF one.
That is a difference between *filters*, not a property of the font resolver, so the rule cannot
be transplanted to the other tracks by argument — each filter has to be asked.

This asks four more: RTF (`\\fnil` / `\\froman` / `\\fswiss` / `\\fmodern` in the font table),
XLSX, PPTX, and flat ODF spreadsheet/presentation. Three discriminating families, chosen because
`45-latin.conf` files them under three different generics and none of them is installed here:

    Candara   filed sans-serif   `fc-match` → DejaVuSans.ttf
    Consolas  filed monospace    `fc-match` → DejaVuSansMono.ttf
    Aptos     filed nothing      `fc-match` → DejaVuSans.ttf (49-sansserif.conf's default)

If a filter answers DejaVu Serif for all three, it defaults the family class to roman the way the
DOCX one does. If it tracks the `fc-match` column, it does not.
"""
import os
import re
import subprocess
import sys
import zipfile

PKG_R = 'http://schemas.openxmlformats.org/package/2006/relationships'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
TEXT = 'Handgloves quick brown fox 12345'

FAMILIES = ['Aptos', 'Candara', 'Consolas', 'Garamond']


def rels(body):
    return f'<?xml version="1.0"?><Relationships xmlns="{PKG_R}">{body}</Relationships>'


def rtf(path, family, family_code):
    with open(path, 'w', encoding='ascii', errors='replace') as handle:
        handle.write('{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0' + family_code + '\\fcharset0 '
                     + family + ';}}\\f0\\fs24 ' + TEXT + '\\par}')


def xlsx(path, family):
    styles = ('<?xml version="1.0"?><styleSheet xmlns="http://schemas.openxmlformats.org/'
              'spreadsheetml/2006/main">'
              f'<fonts count="1"><font><sz val="11"/><name val="{family}"/></font></fonts>'
              '<fills count="1"><fill><patternFill patternType="none"/></fill></fills>'
              '<borders count="1"><border/></borders>'
              '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/>'
              '</cellStyleXfs><cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" '
              'borderId="0" applyFont="1"/></cellXfs></styleSheet>')
    sheet = ('<?xml version="1.0"?><worksheet xmlns="http://schemas.openxmlformats.org/'
             'spreadsheetml/2006/main"><sheetData><row r="1">'
             f'<c r="A1" s="0" t="inlineStr"><is><t>{TEXT}</t></is></c>'
             '</row></sheetData></worksheet>')
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


A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
P = 'http://schemas.openxmlformats.org/presentationml/2006/main'


def pptx(path, family):
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
    slide = (f'<?xml version="1.0"?><p:sld xmlns:a="{A}" xmlns:p="{P}" xmlns:r="{R}">'
             '<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/>'
             '<p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>'
             '<p:sp><p:nvSpPr><p:cNvPr id="2" name="t"/><p:cNvSpPr txBox="1"/><p:nvPr/>'
             '</p:nvSpPr><p:spPr><a:xfrm><a:off x="457200" y="457200"/>'
             '<a:ext cx="7000000" cy="1000000"/></a:xfrm>'
             '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></p:spPr>'
             f'<p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang="en-US" sz="2400">'
             f'<a:latin typeface="{family}"/></a:rPr><a:t>{TEXT}</a:t></a:r></a:p>'
             '</p:txBody></p:sp></p:spTree></p:cSld></p:sld>')
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
        z.writestr('_rels/.rels', rels(f'<Relationship Id="rId1" Type="{R}/officeDocument" '
                                       'Target="ppt/presentation.xml"/>'))
        z.writestr('ppt/_rels/presentation.xml.rels',
                   rels(f'<Relationship Id="rId1" Type="{R}/slideMaster" '
                        'Target="slideMasters/slideMaster1.xml"/>'
                        f'<Relationship Id="rId2" Type="{R}/slide" Target="slides/slide1.xml"/>'
                        f'<Relationship Id="rId3" Type="{R}/theme" Target="theme/theme1.xml"/>'))
        z.writestr('ppt/slideMasters/_rels/slideMaster1.xml.rels',
                   rels(f'<Relationship Id="rId1" Type="{R}/slideLayout" '
                        'Target="../slideLayouts/slideLayout1.xml"/>'
                        f'<Relationship Id="rId2" Type="{R}/theme" Target="../theme/theme1.xml"/>'))
        z.writestr('ppt/slideLayouts/_rels/slideLayout1.xml.rels',
                   rels(f'<Relationship Id="rId1" Type="{R}/slideMaster" '
                        'Target="../slideMasters/slideMaster1.xml"/>'))
        z.writestr('ppt/slides/_rels/slide1.xml.rels',
                   rels(f'<Relationship Id="rId1" Type="{R}/slideLayout" '
                        'Target="../slideLayouts/slideLayout1.xml"/>'))
        z.writestr('ppt/presentation.xml', pres)
        z.writestr('ppt/slideMasters/slideMaster1.xml', master)
        z.writestr('ppt/slideLayouts/slideLayout1.xml', layout)
        z.writestr('ppt/slides/slide1.xml', slide)
        z.writestr('ppt/theme/theme1.xml', theme)


FODS = ('xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" '
        'xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" '
        'xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" '
        'xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" '
        'xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"')


def fods(path, family):
    with open(path, 'w', encoding='utf-8') as handle:
        handle.write(
            f'<?xml version="1.0"?><office:document {FODS} office:version="1.3" '
            'office:mimetype="application/vnd.oasis.opendocument.spreadsheet">'
            '<office:font-face-decls><style:font-face style:name="probe" '
            f'svg:font-family="&apos;{family}&apos;"/></office:font-face-decls>'
            '<office:automatic-styles><style:style style:name="ce1" style:family="table-cell">'
            '<style:text-properties style:font-name="probe"/></style:style>'
            '</office:automatic-styles><office:body><office:spreadsheet>'
            '<table:table table:name="S"><table:table-row>'
            f'<table:table-cell table:style-name="ce1" office:value-type="string">'
            f'<text:p>{TEXT}</text:p></table:table-cell>'
            '</table:table-row></table:table></office:spreadsheet></office:body></office:document>')


def faces(pdf):
    if not os.path.exists(pdf):
        return []
    text = subprocess.run(['pdffonts', pdf], capture_output=True).stdout.decode('utf-8', 'replace')
    return [re.sub(r'^[A-Z]{6}\+', '', line.split()[0])
            for line in text.splitlines()[2:] if line.strip()]


def main():
    out = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else '.')
    for sub in ('src', 'pdf', 'prof'):
        os.makedirs(os.path.join(out, sub), exist_ok=True)
    print(subprocess.run(['soffice', '--version'], capture_output=True).stdout.decode().strip())

    cases = []
    for family in FAMILIES:
        cases.append((f'rtf-fnil-{family}', '.rtf', lambda p, f=family: rtf(p, f, '\\fnil')))
        cases.append((f'rtf-froman-{family}', '.rtf', lambda p, f=family: rtf(p, f, '\\froman')))
        cases.append((f'rtf-fswiss-{family}', '.rtf', lambda p, f=family: rtf(p, f, '\\fswiss')))
        cases.append((f'rtf-fmodern-{family}', '.rtf', lambda p, f=family: rtf(p, f, '\\fmodern')))
        cases.append((f'xlsx-{family}', '.xlsx', lambda p, f=family: xlsx(p, f)))
        cases.append((f'pptx-{family}', '.pptx', lambda p, f=family: pptx(p, f)))
        cases.append((f'fods-{family}', '.fods', lambda p, f=family: fods(p, f)))

    env = dict(os.environ, SOURCE_DATE_EPOCH='1700000000', TZ='UTC')
    print(f"{'case':28s} {'fc-match':22s} 26.2.4.2 draws")
    for index, (name, ext, build) in enumerate(cases):
        safe = name.replace(' ', '_')
        source = os.path.join(out, 'src', safe + ext)
        build(source)
        subprocess.run(['soffice', '--headless',
                        '-env:UserInstallation=file://' + os.path.join(out, 'prof', f'p{index}'),
                        '--convert-to', 'pdf', '--outdir', os.path.join(out, 'pdf'), source],
                       capture_output=True, env=env, timeout=300)
        drawn = faces(os.path.join(out, 'pdf', safe + '.pdf'))
        family = name.rsplit('-', 1)[-1]
        fc = subprocess.run(['fc-match', family], capture_output=True).stdout.decode().split(':')[0]
        print(f'{name:28s} {fc:22s} {", ".join(drawn) or "(nothing embedded)"}')


if __name__ == '__main__':
    main()
