#!/usr/bin/env python3
"""24.2.7.2 audit — `w:pPr/w:rPr` is the *paragraph mark's* formatting, not the paragraph's.

`XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs`:777 states three things and says all three
were measured "against LibreOffice 24.2.7.2":

  1. a **bold** paragraph style whose mark says `<w:b w:val="0"/>` still draws its text bold;
  2. an unstyled paragraph whose mark says `<w:b/><w:sz w:val="48"/>` still draws 10 pt upright;
  3. an **empty** paragraph has nothing but its mark, so a mark stating `w:sz w:val="72"` gives it
     36 pt of height.

This is the last genuinely open site in this project that a corpus document can reach — the other
ten hits of `24.2.7` under `XRay.Office.WordProcessing` are either inside an existing audit marker or
are prose recording that a 24.2.7.2 measurement has *already* been superseded on 26.2.4.2 in the
same comment.

The measurement is on the reference alone: our renderer never runs. One package per arm, each with
a control paragraph whose answer is known before the probe, and every arm read out of the
reference's own content stream — the `/F` resource each show names, the `Tf` size it is set at, and
the baseline pitch between the paragraphs either side of the empty one.

    audit_markstyle.py <outdir>
"""
import os
import re
import subprocess
import sys
import zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

STYLES = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="{W}">
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Sans" w:hAnsi="Liberation Sans"/><w:sz w:val="20"/><w:szCs w:val="20"/>
</w:rPr></w:rPrDefault>
<w:pPrDefault><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr></w:pPrDefault>
</w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
<w:style w:type="paragraph" w:styleId="Heavy"><w:name w:val="Heavy"/>
  <w:rPr><w:b/></w:rPr>
</w:style>
</w:styles>
'''

PARTS = {
    '[Content_Types].xml': '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>''',
    '_rels/.rels': '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>''',
    'word/_rels/document.xml.rels': '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>''',
    'word/styles.xml': STYLES,
}


def para(text, style=None, mark='', run=''):
    ppr = ''
    if style:
        ppr += f'<w:pStyle w:val="{style}"/>'
    if mark:
        ppr += f'<w:rPr>{mark}</w:rPr>'
    ppr = f'<w:pPr>{ppr}</w:pPr>' if ppr else ''
    body = f'<w:r>{f"<w:rPr>{run}</w:rPr>" if run else ""}<w:t>{text}</w:t></w:r>' if text else ''
    return f'<w:p>{ppr}{body}</w:p>'


def document(body):
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            f'<w:document xmlns:w="{W}"><w:body>{body}'
            '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
            '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"'
            ' w:header="720" w:footer="720"/></w:sectPr></w:body></w:document>')


ARMS = {
    # 1. the mark cannot un-bold the paragraph's text.
    'mark-unbold': [
        para('CONTROLBOLD', style='Heavy'),
        para('MARKSAYSOFF', style='Heavy', mark='<w:b w:val="0"/>'),
        para('CONTROLPLAIN'),
        para('RUNSAYSOFF', style='Heavy', run='<w:b w:val="0"/>'),
    ],
    # 2. the mark cannot embolden or enlarge the paragraph's text.
    'mark-embolden': [
        para('CONTROLPLAIN'),
        para('MARKSAYSBOLD', mark='<w:b/><w:sz w:val="48"/>'),
        para('RUNSAYSBOLD', run='<w:b/><w:sz w:val="48"/>'),
    ],
    # 3. an empty paragraph's height is its mark's.
    'mark-height': [
        para('ABOVE'),
        para('', mark='<w:sz w:val="72"/>'),
        para('BELOW'),
    ],
    'mark-height-control': [
        para('ABOVE'),
        para(''),
        para('BELOW'),
    ],
}

SHOW = re.compile(rb'([\d.-]+)\s+([\d.-]+)\s+Td\s*/(F\d+)\s+([\d.]+)\s+Tf')


def read(pdf):
    """(y, face name, Tf size) for every text object on page 1, in stream order.

    Resource name to `/BaseFont` is resolved through the font objects themselves, because `/F1` is
    whichever face the exporter happened to number first and reading the arms by resource name
    would compare two different things between two arms.
    """
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from textcolour import page_streams
    data = open(pdf, 'rb').read()
    objs = {int(m.group(1)): m.group(2)
            for m in re.finditer(rb'(\d+)\s+0\s+obj(.*?)endobj', data, re.S)}
    faces = {}
    for m in re.finditer(rb'/(F\d+)\s+(\d+)\s+0\s+R', data):
        body = objs.get(int(m.group(2)), b'')
        name = re.search(rb'/BaseFont\s*/(?:[A-Z]{6}\+)?([-\w]+)', body)
        if name:
            faces[m.group(1).decode()] = name.group(1).decode()

    stream = page_streams(data, 1)
    out = []
    for m in SHOW.finditer(stream):
        out.append((float(m.group(2)), faces.get(m.group(3).decode(), m.group(3).decode()),
                    float(m.group(4))))
    return out, sorted(set(faces.values()))


def main(outdir):
    os.makedirs(outdir, exist_ok=True)
    profile = os.path.join(outdir, 'prof')
    env = dict(os.environ, SOURCE_DATE_EPOCH='1700000000', TZ='UTC')
    for name, paras in ARMS.items():
        path = os.path.join(outdir, name + '.docx')
        with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
            for part, text in PARTS.items():
                z.writestr(zipfile.ZipInfo(part, (2026, 1, 1, 0, 0, 0)), text)
            z.writestr(zipfile.ZipInfo('word/document.xml', (2026, 1, 1, 0, 0, 0)),
                       document(''.join(paras)))
        subprocess.run(['soffice', '--headless', f'-env:UserInstallation=file://{profile}',
                        '--convert-to', 'pdf', '--outdir', outdir, path],
                       check=True, capture_output=True, env=env, timeout=600)
        shows, faces = read(os.path.join(outdir, name + '.pdf'))
        print('%-20s faces %s' % (name, faces))
        previous = None
        for y, face, size in shows:
            pitch = '' if previous is None else '  pitch %6.2f' % (previous - y)
            print('      y %8.2f  %-24s %5.2f pt%s' % (y, face, size, pitch))
            previous = y


if __name__ == '__main__':
    main(sys.argv[1])
