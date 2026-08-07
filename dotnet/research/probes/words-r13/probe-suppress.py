#!/usr/bin/env python3
"""Does a later-declared parent's own w:spacing still reach the child, or was the child's
missing half already baked from the pool value?

Child states only w:after.  Parent, declared after it, states w:before="480" (24 pt).
Baking predicts the child keeps the parent's *pool* value (12 pt for a heading, 0 for a
custom style) and never sees the 480.
"""
import os, re, subprocess, zipfile

SRC = '/workspace/sample-files/words/batch-007/docx/final-technical-report-template.docx'
SP = '/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/sprobe'
os.makedirs(SP, exist_ok=True)
W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'


def sty(sid, name, based, spacing):
    b = f'<w:basedOn w:val="{based}"/>' if based else ''
    return (f'<w:style w:type="paragraph" w:styleId="{sid}"><w:name w:val="{name}"/>{b}'
            f'<w:next w:val="Normal"/><w:qFormat/><w:pPr>{spacing}</w:pPr>'
            f'<w:rPr><w:sz w:val="22"/></w:rPr></w:style>')


CASES = {
    # parent name, parent declared first?
    'heading2-later': ('heading 2', False),
    'heading2-first': ('heading 2', True),
    'custom-later':   ('My Base', False),
    'custom-first':   ('My Base', True),
}


def build(name, parent_name, parent_first):
    kid = sty('Kid', 'Kid', 'Parent', '<w:spacing w:after="240"/>')
    par = sty('Parent', parent_name, 'Normal', '<w:spacing w:before="480"/>')
    defs = [par, kid] if parent_first else [kid, par]
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles {W}><w:docDefaults><w:rPrDefault><w:rPr>'
              '<w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/><w:sz w:val="22"/>'
              '<w:lang w:val="en-US"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>'
              '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
              '<w:name w:val="Normal"/><w:qFormat/></w:style>' + ''.join(defs) + '</w:styles>')
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document {W}><w:body>'
           '<w:p><w:r><w:t>alpha</w:t></w:r></w:p>'
           '<w:p><w:pPr><w:pStyle w:val="Kid"/></w:pPr><w:r><w:t>KID</w:t></w:r></w:p>'
           '<w:p><w:r><w:t>omega</w:t></w:r></w:p>'
           '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
           '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"'
           ' w:header="720" w:footer="720" w:gutter="0"/></w:sectPr></w:body></w:document>')
    out = os.path.join(SP, name + '.docx')
    zin = zipfile.ZipFile(SRC)
    keep = ('[Content_Types].xml', '_rels/.rels', 'word/_rels/document.xml.rels',
            'word/settings.xml', 'word/fontTable.xml', 'word/theme/theme1.xml',
            'word/webSettings.xml', 'docProps/app.xml', 'docProps/core.xml')
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as zo:
        for n in zin.namelist():
            if n == 'word/document.xml':
                zo.writestr(n, doc)
            elif n == 'word/styles.xml':
                zo.writestr(n, styles)
            elif n in keep:
                data = zin.read(n)
                if n == 'word/_rels/document.xml.rels':
                    s = data.decode('utf8')
                    s = re.sub(r'<Relationship [^>]*Type="[^"]*/(header|footer|numbering|'
                               r'customXml|glossaryDocument|image)"[^>]*/>', '', s)
                    data = s.encode('utf8')
                zo.writestr(n, data)
    return out


for name, (pn, pf) in CASES.items():
    p = build(name, pn, pf)
    d = os.path.join(SP, name)
    subprocess.run(['rm', '-rf', d]); os.makedirs(d)
    subprocess.run(['soffice', '-env:UserInstallation=file://' + SP + '/prof', '--headless',
                    '--convert-to', 'pdf', '--outdir', d, p], capture_output=True, timeout=300)
    pdf = os.path.join(d, name + '.pdf')
    if not os.path.exists(pdf):
        print(f'{name:16} RENDER FAILED'); continue
    bb = subprocess.run(['pdftotext', '-bbox', pdf, '-'], capture_output=True, text=True).stdout
    ys = {t: float(y) for y, t in
          re.findall(r'<word xMin="[\d.]+" yMin="([\d.]+)"[^>]*>(.*?)</word>', bb)}
    print(f'{name:16} in={ys.get("KID",0)-ys.get("alpha",0):7.2f} '
          f'(13.45 = no space, 25.45 = 12pt, 37.45 = 24pt)', flush=True)
