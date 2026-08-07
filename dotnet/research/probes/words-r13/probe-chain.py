#!/usr/bin/env python3
"""Does a DOCX heading style that states no spacing pick one up, and from where?

Clean carrier: a real corpus DOCX keeps its settings.xml/theme, but document.xml and
styles.xml are ours.  Each variant changes only which style Heading1 is basedOn.
"""
import os, re, subprocess, zipfile

SRC = '/workspace/sample-files/words/batch-007/docx/final-technical-report-template.docx'
SP = '/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/cprobe'
os.makedirs(SP, exist_ok=True)
W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'


def style(sid, name, based, lvl):
    b = f'<w:basedOn w:val="{based}"/>' if based else ''
    o = f'<w:outlineLvl w:val="{lvl}"/>' if lvl is not None else ''
    return (f'<w:style w:type="paragraph" w:styleId="{sid}"><w:name w:val="{name}"/>{b}'
            f'<w:next w:val="Normal"/><w:qFormat/><w:pPr>{o}</w:pPr>'
            f'<w:rPr><w:sz w:val="22"/></w:rPr></w:style>')


CASES = {
    # name: list of (styleId, w:name, basedOn, outlineLvl) defining the chain, in file order
    'h1-on-normal':   [('Heading1', 'heading 1', 'Normal', 0)],
    'h1-on-h2':       [('Heading1', 'heading 1', 'Heading2', 0),
                       ('Heading2', 'heading 2', 'Normal', 1)],
    'h1-on-h2-first': [('Heading2', 'heading 2', 'Normal', 1),
                       ('Heading1', 'heading 1', 'Heading2', 0)],
    'h1-on-h2-undef': [('Heading1', 'heading 1', 'Heading2', 0)],
    'h1-on-mine':     [('Heading1', 'heading 1', 'MyBase', 0),
                       ('MyBase', 'My Base', 'Normal', None)],
    'h1-no-based':    [('Heading1', 'heading 1', None, 0)],
    'mine-on-h2':     [('Mine', 'My Head', 'Heading2', 0),
                       ('Heading2', 'heading 2', 'Normal', 1)],
}


def docx(name, defs):
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
              f'<w:styles {W}><w:docDefaults><w:rPrDefault><w:rPr>'
              '<w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/><w:sz w:val="22"/>'
              '<w:lang w:val="en-US"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>'
              '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
              '<w:name w:val="Normal"/><w:qFormat/></w:style>'
              + ''.join(style(*d) for d in defs) + '</w:styles>')
    head = defs[0][0] if defs[0][0] != 'Heading2' else defs[1][0]
    body = ('<w:p><w:r><w:t>alpha</w:t></w:r></w:p>'
            f'<w:p><w:pPr><w:pStyle w:val="{head}"/></w:pPr><w:r><w:t>HEAD</w:t></w:r></w:p>'
            '<w:p><w:r><w:t>omega</w:t></w:r></w:p>')
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


def run(name, defs):
    p = docx(name, defs)
    d = os.path.join(SP, name)
    subprocess.run(['rm', '-rf', d]); os.makedirs(d)
    subprocess.run(['soffice', '-env:UserInstallation=file://' + SP + '/prof', '--headless',
                    '--convert-to', 'fodt', '--outdir', d, p], capture_output=True, timeout=300)
    subprocess.run(['soffice', '-env:UserInstallation=file://' + SP + '/prof', '--headless',
                    '--convert-to', 'pdf', '--outdir', d, p], capture_output=True, timeout=300)
    pdf = os.path.join(d, name + '.pdf')
    if not os.path.exists(pdf):
        return f'{name:16} RENDER FAILED'
    bb = subprocess.run(['pdftotext', '-bbox', pdf, '-'], capture_output=True, text=True).stdout
    ys = {t: float(y) for y, t in
          re.findall(r'<word xMin="[\d.]+" yMin="([\d.]+)"[^>]*>(.*?)</word>', bb)}
    gin = ys.get('HEAD', 0) - ys.get('alpha', 0)
    gout = ys.get('omega', 0) - ys.get('HEAD', 0)
    fodt = os.path.join(d, name + '.fodt')
    marg = ''
    if os.path.exists(fodt):
        s = open(fodt, encoding='utf8').read()
        m = re.search(r'<style:style style:name="Heading_20_1".*?</style:style>', s, re.S)
        if not m:
            m = re.search(r'<style:style style:name="My_20_Head".*?</style:style>', s, re.S)
        if m:
            pm = re.search(r'fo:margin-top="([^"]*)"', m.group(0))
            pb = re.search(r'fo:margin-bottom="([^"]*)"', m.group(0))
            par = re.search(r'style:parent-style-name="([^"]*)"', m.group(0))
            marg = f'  fodt: parent={par.group(1) if par else "-"} ' \
                   f'top={pm.group(1) if pm else "-"} bottom={pb.group(1) if pb else "-"}'
    return f'{name:16} in={gin:7.2f} out={gout:7.2f}{marg}'


for k, v in CASES.items():
    print(run(k, v), flush=True)
