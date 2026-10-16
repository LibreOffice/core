#!/usr/bin/env python3
"""How tall an *empty* paragraph is, in the body and in a running head.

`probes/words-margin-print-area/results.md` §4 filed a 1.90 pt shortfall on an empty
header paragraph and left it. This isolates it: the same empty paragraph is put in the
body, where its height can be read straight off the gap between two text lines, and in a
header, where it shows as the body's own top.

The variables are the ones that decide which font an empty paragraph is as tall as: a run
with an empty `w:t`, a run with no `w:t` at all, no run at all, and a size stated on the
paragraph *mark* (`w:pPr/w:rPr`) rather than on a run.

Usage:  python3 makeprobe.py <outdir>
"""
import os, sys, zipfile

CT_HEAD = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
           '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
           '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
           '<Default Extension="xml" ContentType="application/xml"/>'
           '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>')
CT_HDR = ('<Override PartName="/word/header1.xml" '
          'ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>')
CT_STY = ('<Override PartName="/word/styles.xml" '
          'ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>')

RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>'
        '</Relationships>')

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"')


def rpr(face='Liberation Serif', half=24):
    return (f'<w:rPr><w:rFonts w:ascii="{face}" w:hAnsi="{face}"/>'
            f'<w:sz w:val="{half}"/></w:rPr>')


def line(text, face='Liberation Serif', half=24):
    return f'<w:p><w:r>{rpr(face, half)}<w:t>{text}</w:t></w:r></w:p>'


# The five spellings of "an empty paragraph" a real document actually uses.
FORMS = {
    # A run carrying formatting and an empty `w:t` — what `words-margin-print-area` used.
    'run-empty-t': lambda face, half: f'<w:p><w:r>{rpr(face, half)}<w:t></w:t></w:r></w:p>',
    # A run carrying formatting and nothing at all — the catalogue's own header shape.
    'run-no-t': lambda face, half: f'<w:p><w:r>{rpr(face, half)}</w:r></w:p>',
    # No run: the size can only come from the paragraph mark's own `w:rPr`.
    'mark-only': lambda face, half: f'<w:p><w:pPr>{rpr(face, half)}</w:pPr></w:p>',
    # Nothing whatever: the document default decides.
    'bare': lambda face, half: '<w:p/>',
    # Both, agreeing — the shape Word writes.
    'mark-and-run': lambda face, half:
        f'<w:p><w:pPr>{rpr(face, half)}</w:pPr><w:r>{rpr(face, half)}</w:r></w:p>',
}


def build(path, body, header=None, defaults=None):
    parts = {}
    ct = CT_HEAD
    drels = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">']
    refs = ''

    if header is not None:
        ct += CT_HDR
        drels.append('<Relationship Id="rIdH" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/header" Target="header1.xml"/>')
        refs += '<w:headerReference w:type="default" r:id="rIdH"/>'
        parts['word/header1.xml'] = (
            f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:hdr {NS}>{header}</w:hdr>')

    if defaults is not None:
        ct += CT_STY
        drels.append('<Relationship Id="rIdS" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>')
        parts['word/styles.xml'] = (
            f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:styles {NS}>'
            f'<w:docDefaults><w:rPrDefault><w:rPr>{defaults}</w:rPr></w:rPrDefault>'
            f'<w:pPrDefault/></w:docDefaults></w:styles>')

    drels.append('</Relationships>')
    ct += '</Types>'

    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {NS}><w:body>'
        + body
        + f'<w:sectPr>{refs}<w:pgSz w:w="11906" w:h="16838"/>'
          '<w:pgMar w:top="708" w:right="1440" w:bottom="1440" w:left="1440" '
          'w:header="708" w:footer="708" w:gutter="0"/></w:sectPr>'
        '</w:body></w:document>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', ct)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/document.xml', document)
        z.writestr('word/_rels/document.xml.rels', ''.join(drels))
        for name, content in parts.items():
            z.writestr(name, content)


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)
    P = lambda n: os.path.join(out, n + '.docx')

    # The body control: two lines and nothing between them.
    build(P('body-none'), line('TOPLINE') + line('BOTLINE'))
    # And one line between them, so an empty paragraph can be scored against a full one.
    build(P('body-full'), line('TOPLINE') + line('MIDLINE') + line('BOTLINE'))

    for name, form in FORMS.items():
        build(P(f'body-{name}'), line('TOPLINE') + form('Liberation Serif', 24) + line('BOTLINE'))

    # Size and face, on the spelling the earlier probe used.
    for half in (16, 20, 24, 40):
        build(P(f'body-sz{half}'),
              line('TOPLINE') + FORMS['run-empty-t']('Liberation Serif', half) + line('BOTLINE'))
    for face, tag in (('Carlito', 'carlito'), ('Liberation Sans', 'libsans')):
        build(P(f'body-{tag}'), line('TOPLINE') + FORMS['run-empty-t'](face, 24) + line('BOTLINE'))

    # The header half: the body's own top says how tall the running head came out.
    build(P('hdr-none'), line('BODYLINE'))
    build(P('hdr-full'), line('BODYLINE'), header=line('HEADERA'))
    for name, form in FORMS.items():
        build(P(f'hdr-{name}'), line('BODYLINE'), header=form('Liberation Serif', 24))
    # What `w:docDefaults/w:rPrDefault` does to the same empty paragraph. LibreOffice starts a
    # DOCX at Calibri 11 pt (`DomainMapper.cxx`:182-193, tdf#108350) and resets to Times New
    # Roman 10 pt the moment a `w:rPrDefault` exists at all (`StyleSheetTable.cxx`:2161-2167),
    # so the presence of the element matters and not only its content.
    empty = FORMS['run-empty-t']('Liberation Serif', 24)
    for tag, dd in (('dd-bare', ''),
                    ('dd-sz28', '<w:sz w:val="28"/>'),
                    ('dd-carlito', '<w:rFonts w:ascii="Carlito" w:hAnsi="Carlito"/>'),
                    ('dd-both', '<w:rFonts w:ascii="Carlito" w:hAnsi="Carlito"/><w:sz w:val="28"/>')):
        build(P(f'body-{tag}'), line('TOPLINE') + empty + line('BOTLINE'), defaults=dd)

    print('built in', out)
