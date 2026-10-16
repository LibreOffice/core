#!/usr/bin/env python3
"""Test one hypothesis: a DOCX style stating only w:after gets a *direct* upper margin too,
read from whatever its parent chain resolved to at that moment — which for a heading style
based on another heading is Writer's own "Heading" pool style (12 pt above, 6 pt below).

Prediction: with w:after stated, Heading1 basedOn Heading2 gains 12 pt above; without it,
nothing.  And a non-heading style based on a non-heading style gains nothing either way.
"""
import os, re, subprocess, zipfile

SRC = '/workspace/sample-files/words/batch-007/docx/final-technical-report-template.docx'
SP = '/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/uprobe'
os.makedirs(SP, exist_ok=True)
W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'


def sty(sid, name, based, lvl, spacing):
    b = f'<w:basedOn w:val="{based}"/>' if based else ''
    o = f'<w:outlineLvl w:val="{lvl}"/>' if lvl is not None else ''
    return (f'<w:style w:type="paragraph" w:styleId="{sid}"><w:name w:val="{name}"/>{b}'
            f'<w:next w:val="Normal"/><w:qFormat/><w:pPr>{spacing}{o}</w:pPr>'
            f'<w:rPr><w:sz w:val="22"/></w:rPr></w:style>')


AFTER = '<w:spacing w:after="240"/>'

CASES = {
    'h1onh2-after':   ('Heading1', [sty('Heading1', 'heading 1', 'Heading2', 0, AFTER),
                                    sty('Heading2', 'heading 2', 'Normal', 1, '')]),
    'h1onh2-nospc':   ('Heading1', [sty('Heading1', 'heading 1', 'Heading2', 0, ''),
                                    sty('Heading2', 'heading 2', 'Normal', 1, '')]),
    'h1onh2-after-h2first': ('Heading1', [sty('Heading2', 'heading 2', 'Normal', 1, ''),
                                          sty('Heading1', 'heading 1', 'Heading2', 0, AFTER)]),
    'h1onnormal-after': ('Heading1', [sty('Heading1', 'heading 1', 'Normal', 0, AFTER)]),
    'h1onh3-after':   ('Heading1', [sty('Heading1', 'heading 1', 'Heading3', 0, AFTER),
                                    sty('Heading3', 'heading 3', 'Normal', 2, '')]),
    'mine-on-h2-after': ('Mine', [sty('Mine', 'My Head', 'Heading2', None, AFTER),
                                  sty('Heading2', 'heading 2', 'Normal', 1, '')]),
    'mine-on-mine-after': ('Mine', [sty('Mine', 'My Head', 'Base2', None, AFTER),
                                    sty('Base2', 'My Base', 'Normal', None, '')]),
    'h2onh3-after':   ('Heading2', [sty('Heading2', 'heading 2', 'Heading3', 1, AFTER),
                                    sty('Heading3', 'heading 3', 'Normal', 2, '')]),
    'h1onh2-before':  ('Heading1', [sty('Heading1', 'heading 1', 'Heading2', 0,
                                        '<w:spacing w:before="240"/>'),
                                    sty('Heading2', 'heading 2', 'Normal', 1, '')]),
}


def build(name, head, defs):
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles {W}><w:docDefaults><w:rPrDefault><w:rPr>'
              '<w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/><w:sz w:val="22"/>'
              '<w:lang w:val="en-US"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>'
              '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
              '<w:name w:val="Normal"/><w:qFormat/></w:style>'
              + ''.join(defs) + '</w:styles>')
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document {W}><w:body>'
           '<w:p><w:r><w:t>alpha</w:t></w:r></w:p>'
           f'<w:p><w:pPr><w:pStyle w:val="{head}"/></w:pPr><w:r><w:t>HEAD</w:t></w:r></w:p>'
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


def run(name, head, defs):
    p = build(name, head, defs)
    d = os.path.join(SP, name)
    subprocess.run(['rm', '-rf', d]); os.makedirs(d)
    subprocess.run(['soffice', '-env:UserInstallation=file://' + SP + '/prof', '--headless',
                    '--convert-to', 'pdf', '--outdir', d, p], capture_output=True, timeout=300)
    pdf = os.path.join(d, name + '.pdf')
    if not os.path.exists(pdf):
        return f'{name:24} RENDER FAILED'
    bb = subprocess.run(['pdftotext', '-bbox', pdf, '-'], capture_output=True, text=True).stdout
    ys = {t: float(y) for y, t in
          re.findall(r'<word xMin="[\d.]+" yMin="([\d.]+)"[^>]*>(.*?)</word>', bb)}
    gin = ys.get('HEAD', 0) - ys.get('alpha', 0)
    gout = ys.get('omega', 0) - ys.get('HEAD', 0)
    return f'{name:24} in={gin:7.2f} (base 13.45)   out={gout:7.2f}'


for k, (h, d) in CASES.items():
    print(run(k, h, d), flush=True)
