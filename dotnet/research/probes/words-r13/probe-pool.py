#!/usr/bin/env python3
"""What upper/lower spacing does each Writer pool style still carry when a DOCX child style
stating one half of w:spacing is imported before the parent's own definition?

For each candidate parent name P, one document with two children based on P — one stating
only w:after, one stating only w:before — both declared before P.
"""
import os, re, subprocess, zipfile

SRC = '/workspace/sample-files/words/batch-007/docx/final-technical-report-template.docx'
SP = '/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/pprobe'
os.makedirs(SP, exist_ok=True)
W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'

# w:name values that LibreOffice maps onto one of its own pool paragraph styles
PARENTS = [
    ('heading 1', None), ('heading 2', None), ('heading 3', None), ('heading 9', None),
    ('Body Text', None), ('Body Text 2', None), ('Title', None), ('Subtitle', None),
    ('List', None), ('List Paragraph', None), ('caption', None), ('header', None),
    ('footer', None), ('Quote', None), ('Normal', None), ('MyOwnStyle', None),
]


def sty(sid, name, based, spacing, extra=''):
    b = f'<w:basedOn w:val="{based}"/>' if based else ''
    return (f'<w:style w:type="paragraph" w:styleId="{sid}"><w:name w:val="{name}"/>{b}'
            f'<w:next w:val="Normal"/><w:qFormat/><w:pPr>{spacing}</w:pPr>'
            f'<w:rPr><w:sz w:val="22"/><w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/>'
            f'</w:rPr>{extra}</w:style>')


def build(name, parent_name):
    defs = [
        sty('KidAfter', 'Kid After', 'Parent', '<w:spacing w:after="240"/>'),
        sty('KidBefore', 'Kid Before', 'Parent', '<w:spacing w:before="240"/>'),
        sty('Parent', parent_name, 'Normal', ''),
    ]
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles {W}><w:docDefaults><w:rPrDefault><w:rPr>'
              '<w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/><w:sz w:val="22"/>'
              '<w:lang w:val="en-US"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>'
              '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
              '<w:name w:val="Normal"/><w:qFormat/></w:style>' + ''.join(defs) + '</w:styles>')
    body = ('<w:p><w:r><w:t>alpha</w:t></w:r></w:p>'
            '<w:p><w:pPr><w:pStyle w:val="KidAfter"/></w:pPr><w:r><w:t>AAA</w:t></w:r></w:p>'
            '<w:p><w:r><w:t>beta</w:t></w:r></w:p>'
            '<w:p><w:pPr><w:pStyle w:val="KidBefore"/></w:pPr><w:r><w:t>BBB</w:t></w:r></w:p>'
            '<w:p><w:r><w:t>gamma</w:t></w:r></w:p>')
    doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           f'<w:document {W}><w:body>' + body +
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


def run(parent_name):
    tag = re.sub(r'\W+', '_', parent_name)
    p = build(tag, parent_name)
    d = os.path.join(SP, tag)
    subprocess.run(['rm', '-rf', d]); os.makedirs(d)
    subprocess.run(['soffice', '-env:UserInstallation=file://' + SP + '/prof', '--headless',
                    '--convert-to', 'fodt', '--outdir', d, p], capture_output=True, timeout=300)
    fodt = os.path.join(d, tag + '.fodt')
    if not os.path.exists(fodt):
        return f'{parent_name:16} EXPORT FAILED'
    s = open(fodt, encoding='utf8').read()
    res = []
    for nm in ('Kid_20_After', 'Kid_20_Before'):
        m = re.search(r'<style:style style:name="%s".*?</style:style>' % nm, s, re.S)
        if not m:
            res.append('-/-'); continue
        t = re.search(r'fo:margin-top="([^"]*)"', m.group(0))
        b = re.search(r'fo:margin-bottom="([^"]*)"', m.group(0))
        res.append(f'top={t.group(1) if t else "-":9} bot={b.group(1) if b else "-":9}')
    return f'{parent_name:16} after-only: {res[0]}   before-only: {res[1]}'


for p, _ in PARENTS:
    print(run(p), flush=True)
