#!/usr/bin/env python3
"""Bisect what makes LibreOffice give final-technical-report-template's Heading1 a 12 pt
space above that the DOCX never states.  Each variant edits only word/styles.xml."""
import os, re, subprocess, zipfile

SRC = '/workspace/sample-files/words/batch-007/docx/final-technical-report-template.docx'
SP = '/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/bprobe'
os.makedirs(SP, exist_ok=True)

H1 = ('<w:style w:type="paragraph" w:styleId="Heading1"><w:name w:val="heading 1"/>'
      '<w:basedOn w:val="Heading2"/><w:next w:val="Normal"/><w:autoRedefine/><w:qFormat/>'
      '<w:rsid w:val="004F71C5"/><w:pPr><w:numPr><w:numId w:val="11"/></w:numPr>'
      '<w:spacing w:after="240"/><w:outlineLvl w:val="0"/></w:pPr>')

VARIANTS = {
    'baseline':      H1,
    'no-autoredef':  H1.replace('<w:autoRedefine/>', ''),
    'based-normal':  H1.replace('w:basedOn w:val="Heading2"', 'w:basedOn w:val="Normal"'),
    'no-numpr':      H1.replace('<w:numPr><w:numId w:val="11"/></w:numPr>', ''),
    'renamed':       H1.replace('<w:name w:val="heading 1"/>', '<w:name w:val="MyHead1"/>'),
    'no-outlinelvl': H1.replace('<w:outlineLvl w:val="0"/>', ''),
}


def run(name, h1):
    out = os.path.join(SP, name + '.docx')
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as zo:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == 'word/styles.xml':
                s = data.decode('utf8')
                assert H1 in s, 'H1 literal not found'
                data = s.replace(H1, h1).encode('utf8')
            zo.writestr(item, data)
    d = os.path.join(SP, name)
    subprocess.run(['rm', '-rf', d]); os.makedirs(d)
    subprocess.run(['soffice', '-env:UserInstallation=file://' + SP + '/prof', '--headless',
                    '--convert-to', 'pdf', '--outdir', d, out], capture_output=True, timeout=300)
    pdf = os.path.join(d, name + '.pdf')
    if not os.path.exists(pdf):
        return f'{name:15} RENDER FAILED'
    info = subprocess.run(['pdfinfo', pdf], capture_output=True, text=True).stdout
    pages = re.search(r'^Pages:\s+(\d+)', info, re.M).group(1)
    bb = subprocess.run(['pdftotext', '-bbox', '-f', '5', '-l', '5', pdf, '-'],
                        capture_output=True, text=True).stdout
    ys = sorted({round(float(y), 1) for y in re.findall(r'yMin="([\d.]+)"', bb)})
    return f'{name:15} pages={pages} page5 y={ys[:4]}'


for k, v in VARIANTS.items():
    print(run(k, v), flush=True)
