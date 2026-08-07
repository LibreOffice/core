#!/usr/bin/env python3
"""Is LibreOffice's DOCX line pitch always on a 300 dpi grid, or only with w:usePrinterMetrics?

Same eight-line paragraph in several faces and sizes, each package built twice — with the
flag and without — and the pitch read from both renderings plus ours.
"""
import os, re, subprocess, zipfile

SP = '/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/gprobe'
W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
CLI = ('/home/user/libreoffice-core/.claude/worktrees/agent-aacd64f37ea26a795/dotnet/'
       'tools/Paperless.Cli/bin/Debug/net10.0/linux-x64/Paperless.Cli')
os.makedirs(SP, exist_ok=True)

CT = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>"""
RR = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Target="word/document.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
</Relationships>"""
DR = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Target="styles.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"/>
<Relationship Id="rId2" Target="settings.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"/>
</Relationships>"""
LINE = ('The quick brown fox jumps over the lazy dog and keeps on running past the margin '
        'so the paragraph needs several lines to hold it. ')

CASES = [('Times New Roman', 20), ('Calibri', 22), ('Calibri', 20), ('Arial', 20),
         ('Arial', 24), ('Times New Roman', 24), ('Cambria', 22)]


def build(path, face, half, printer, mode15):
    styles = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:styles {W}>'
              f'<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="{face}" w:hAnsi="{face}"/>'
              f'<w:sz w:val="{half}"/><w:lang w:val="en-US"/></w:rPr></w:rPrDefault>'
              '<w:pPrDefault/></w:docDefaults>'
              '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
              '<w:name w:val="Normal"/><w:qFormat/></w:style></w:styles>')
    flag = '<w:usePrinterMetrics/>' if printer else ''
    m15 = ('<w:compatSetting w:name="compatibilityMode" '
           'w:uri="http://schemas.microsoft.com/office/word" w:val="15"/>') if mode15 else ''
    settings = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:settings {W}>'
                f'<w:defaultTabStop w:val="720"/><w:compat>{flag}{m15}</w:compat></w:settings>')
    doc = (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>'
           f'<w:p><w:r><w:t xml:space="preserve">{LINE * 6}</w:t></w:r></w:p>'
           '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/><w:pgMar w:top="1440" w:right="1440"'
           ' w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/>'
           '</w:sectPr></w:body></w:document>')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for n, d in (('[Content_Types].xml', CT), ('_rels/.rels', RR),
                     ('word/_rels/document.xml.rels', DR), ('word/document.xml', doc),
                     ('word/styles.xml', styles), ('word/settings.xml', settings)):
            z.writestr(n, d)


def pitch(pdf):
    if not os.path.exists(pdf):
        return None
    bb = subprocess.run(['pdftotext', '-bbox', pdf, '-'], capture_output=True, text=True).stdout
    ys = sorted({round(float(y), 2) for y in re.findall(r'yMin="([\d.]+)"', bb)})
    if len(ys) < 3:
        return None
    return round((ys[-1] - ys[0]) / (len(ys) - 1), 3)


print(f'{"face/size":24} {"ref off":>8} {"ref on":>8} {"ours off":>9} {"ours on":>8}')
for face, half in CASES:
    row = []
    for printer in (False, True):
        tag = re.sub(r'\W+', '', face) + str(half) + ('P' if printer else 'N')
        p = os.path.join(SP, tag + '.docx')
        build(p, face, half, printer, mode15=True)
        d = os.path.join(SP, tag)
        subprocess.run(['rm', '-rf', d]); os.makedirs(d)
        subprocess.run(['soffice', '-env:UserInstallation=file://' + SP + '/prof', '--headless',
                        '--convert-to', 'pdf', '--outdir', d, p], capture_output=True, timeout=300)
        subprocess.run([CLI, 'render', p, '--format', 'pdf', '--outdir', d + '-ours'],
                       capture_output=True, timeout=300)
        row.append((pitch(os.path.join(d, tag + '.pdf')),
                    pitch(os.path.join(d + '-ours', tag + '.pdf'))))
    print(f'{face + " " + str(half / 2) + "pt":24} {str(row[0][0]):>8} {str(row[1][0]):>8} '
          f'{str(row[0][1]):>9} {str(row[1][1]):>8}', flush=True)
