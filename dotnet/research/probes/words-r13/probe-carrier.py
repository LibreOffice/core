#!/usr/bin/env python3
"""Narrow the carrier: keep the real styles.xml, shrink document.xml, then strip styles.xml
piece by piece until the unexplained 12 pt above Heading 1 disappears."""
import os, re, subprocess, zipfile

SRC = '/workspace/sample-files/words/batch-007/docx/final-technical-report-template.docx'
SP = '/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/kprobe'
os.makedirs(SP, exist_ok=True)
W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'

DOC = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       f'<w:document {W}><w:body>'
       '<w:p><w:r><w:t>alpha</w:t></w:r></w:p>'
       '<w:p><w:pPr><w:pStyle w:val="Heading1"/></w:pPr><w:r><w:t>HEAD</w:t></w:r></w:p>'
       '<w:p><w:r><w:t>omega</w:t></w:r></w:p>'
       '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
       '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"'
       ' w:header="720" w:footer="720" w:gutter="0"/></w:sectPr></w:body></w:document>')

RAW = zipfile.ZipFile(SRC).read('word/styles.xml').decode('utf8')


def drop_style(s, sid):
    return re.sub(r'<w:style [^>]*w:styleId="%s">.*?</w:style>' % sid, '', s, flags=re.S)


CASES = {
    'real-styles':      lambda s: s,
    'no-latent':        lambda s: re.sub(r'<w:latentStyles.*?</w:latentStyles>|<w:latentStyles[^>]*/>',
                                         '', s, flags=re.S),
    'no-tocheading':    lambda s: drop_style(s, 'TOCHeading'),
    'no-heading2':      lambda s: drop_style(s, 'Heading2'),
    'no-docdefaults':   lambda s: re.sub(r'<w:docDefaults>.*?</w:docDefaults>', '', s, flags=re.S),
    'h1-based-normal':  lambda s: s.replace('<w:name w:val="heading 1"/><w:basedOn w:val="Heading2"/>',
                                            '<w:name w:val="heading 1"/><w:basedOn w:val="Normal"/>'),
    'h2-before0':       lambda s: s.replace(
        '<w:name w:val="heading 2"/><w:basedOn w:val="Normal"/><w:next w:val="Normal"/><w:qFormat/><w:pPr><w:keepNext/>',
        '<w:name w:val="heading 2"/><w:basedOn w:val="Normal"/><w:next w:val="Normal"/><w:qFormat/><w:pPr><w:spacing w:before="0"/><w:keepNext/>'),
}


def run(name, fn):
    styles = fn(RAW)
    out = os.path.join(SP, name + '.docx')
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as zo:
        for n in zin.namelist():
            if n == 'word/document.xml':
                zo.writestr(n, DOC)
            elif n == 'word/styles.xml':
                zo.writestr(n, styles)
            else:
                zo.writestr(n, zin.read(n))
    d = os.path.join(SP, name)
    subprocess.run(['rm', '-rf', d]); os.makedirs(d)
    subprocess.run(['soffice', '-env:UserInstallation=file://' + SP + '/prof', '--headless',
                    '--convert-to', 'pdf', '--outdir', d, out], capture_output=True, timeout=300)
    pdf = os.path.join(d, name + '.pdf')
    if not os.path.exists(pdf):
        return f'{name:18} RENDER FAILED'
    bb = subprocess.run(['pdftotext', '-bbox', pdf, '-'], capture_output=True, text=True).stdout
    ys = {t: float(y) for y, t in
          re.findall(r'<word xMin="[\d.]+" yMin="([\d.]+)"[^>]*>(.*?)</word>', bb)}
    return (f'{name:18} alpha={ys.get("alpha", 0):7.2f} HEAD={ys.get("HEAD", 0):7.2f} '
            f'omega={ys.get("omega", 0):7.2f}  in={ys.get("HEAD",0)-ys.get("alpha",0):6.2f}')


for k, v in CASES.items():
    print(run(k, v), flush=True)
