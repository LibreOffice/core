#!/usr/bin/env python3
"""Nine authored variants that isolate what a `w:br` is worth, measured against 26.2.4.2.

    python3 br-paragraph-probe.py <outdir>

Each document is `AAA`, one paragraph under test, `BBB`, all Cambria on A4 with 1 inch
margins, and what is measured is the y of `BBB` against the y it has when the paragraph
under test is absent. Both sides are rendered at `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

Measured 2026-08-20 (points added by the paragraph under test):

    case         reference     ours
    b-empty          12.65    11.50
    a-br             25.30     0.00   <-- a paragraph whose whole content is a break
    c-two            25.30    23.00
    d-brbr           37.95     0.00
    f-xbry           25.30    23.00
    g-brtext         25.30    23.00
    h-textbr         25.30    23.00
    i-spacebr        25.30    23.00

So the reference gives a paragraph with N breaks N+1 lines, and we agree in every case
where the paragraph holds any other content at all — one space is enough. The paragraph
whose *whole* content is breaks contributes nothing on our side. `xray-office extract`
already reports the two blank lines for `a-br`, so the reader is right and the seat is in
layout.

469 such paragraphs exist in 66 of the 271 distinct words documents.
"""
import os, subprocess, sys, re, zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
      'officedocument.wordprocessingml.document.main+xml"/></Types>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
        'relationships/officeDocument" Target="word/document.xml"/></Relationships>')
F = '<w:rPr><w:rFonts w:ascii="Cambria" w:hAnsi="Cambria"/></w:rPr>'


def para(inner=""):
    return f'<w:p><w:pPr>{F}</w:pPr>{inner}</w:p>'


def run(text):
    return f'<w:r>{F}<w:t>{text}</w:t></w:r>'


BR = f'<w:r>{F}<w:br/></w:r>'
CASES = {
    'e-none': '',
    'b-empty': para(),
    'a-br': para(BR),
    'c-two': para() + para(),
    'd-brbr': para(BR + BR),
    'f-xbry': para(f'<w:r>{F}<w:t>X</w:t><w:br/><w:t>Y</w:t></w:r>'),
    'g-brtext': para(BR + run('Y')),
    'h-textbr': para(run('Y') + BR),
    'i-spacebr': para(run(' ') + BR),
}


def build(out):
    for name, middle in CASES.items():
        body = para(run('AAA')) + middle + para(run('BBB'))
        doc = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
               f'<w:document xmlns:w="{W}"><w:body>{body}'
               '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
               '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>'
               '</w:sectPr></w:body></w:document>')
        with zipfile.ZipFile(os.path.join(out, name + '.docx'), 'w', zipfile.ZIP_DEFLATED) as z:
            z.writestr('[Content_Types].xml', CT)
            z.writestr('_rels/.rels', RELS)
            z.writestr('word/document.xml', doc)


def y_of(pdf, word='BBB'):
    if not pdf or not os.path.exists(pdf):
        return None
    text = subprocess.run(['pdftotext', '-bbox', pdf, '-'],
                          capture_output=True).stdout.decode('utf-8', 'replace')
    found = re.search(r'<word xMin="[\d.]+" yMin="([\d.]+)"[^>]*>' + word + '</word>', text)
    return float(found.group(1)) if found else None


def main():
    out = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else '.')
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        sys.exit('set XRAY_OFFICE_CLI to the tree you mean to measure')

    os.makedirs(os.path.join(out, 'ref'), exist_ok=True)
    os.makedirs(os.path.join(out, 'ours'), exist_ok=True)
    build(out)

    env = dict(os.environ, SOURCE_DATE_EPOCH='1700000000', TZ='UTC')
    for name in CASES:
        source = os.path.join(out, name + '.docx')
        subprocess.run(['soffice', '--headless',
                        '-env:UserInstallation=file://' + os.path.join(out, 'prof'),
                        '--convert-to', 'pdf', '--outdir', os.path.join(out, 'ref'), source],
                       capture_output=True, env=env)
        subprocess.run([cli, 'render', source, '--outdir', os.path.join(out, 'ours')],
                       capture_output=True, env=env)

    base_ref = y_of(os.path.join(out, 'ref', 'e-none.pdf'))
    base_our = y_of(os.path.join(out, 'ours', 'e-none.pdf'))
    print(f"{'case':10s} {'reference':>10s} {'ours':>8s}")
    for name in CASES:
        ref = y_of(os.path.join(out, 'ref', name + '.pdf'))
        our = y_of(os.path.join(out, 'ours', name + '.pdf'))
        r = f'{ref - base_ref:10.2f}' if ref is not None else '         -'
        o = f'{our - base_our:8.2f}' if our is not None else '       -'
        print(f'{name:10s} {r} {o}')


if __name__ == '__main__':
    main()
