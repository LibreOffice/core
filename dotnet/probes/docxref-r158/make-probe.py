#!/usr/bin/env python3
"""One DOCX, one arm per paragraph, for what 26.2.4.2 draws for a REF field.

Each arm is a sentence of the form `<label>: [REF field] .` whose field caches a deliberately
wrong result, so whatever the reference draws in its place is its own answer rather than the
file's.  A `word/settings.xml` is included because a hand-built DOCX without one does not get
LibreOffice's OOXML compatibility defaults, and then answers a question nobody asked.
"""
import pathlib, sys, zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'


def run(text, *, style=''):
    return (f'<w:r>{style}<w:t xml:space="preserve">{text}</w:t></w:r>')


def field(instruction, cached):
    """The fldChar form: begin, instruction, separate, the cached result, end."""
    return ('<w:r><w:fldChar w:fldCharType="begin"/></w:r>'
            f'<w:r><w:instrText xml:space="preserve"> {instruction} </w:instrText></w:r>'
            '<w:r><w:fldChar w:fldCharType="separate"/></w:r>'
            + (run(cached) if cached else '')
            + '<w:r><w:fldChar w:fldCharType="end"/></w:r>')


def para(*pieces):
    return '<w:p>' + ''.join(pieces) + '</w:p>'


def mark(i, name):
    return (f'<w:bookmarkStart w:id="{i}" w:name="{name}"/>', f'<w:bookmarkEnd w:id="{i}"/>')


ARMS = []


def arm(label, body):
    ARMS.append(para(run(f'{label}: '), *body, run(' .')))


# The targets every arm quotes, each in its own paragraph so a bookmark's node is unambiguous.
s, e = mark(1, 'A_more')
targets = [para(run('caption '), s, run('Table '), run('7'), e, run(' tail'))]

s2, e2 = mark(2, 'B_span')
targets.append(para(run('first '), s2, run('half one')))
targets.append(para(run('half two'), e2, run(' after')))

s3, e3 = mark(3, 'C_point')
targets.append(para(run('point '), s3, e3, run(' bookmark')))

s4, e4 = mark(4, '__RefHeading__5')
targets.append(para(run('Heading text here'), s4, e4))

s5, e5 = mark(5, 'E_break')
targets.append(para(s5, run('Line one'), '<w:r><w:br/></w:r>', run('Line two'), e5))

s6, e6 = mark(6, 'F_seq')
targets.append(para(
    s6, run('Figure '),
    '<w:r><w:fldChar w:fldCharType="begin"/></w:r>'
    '<w:r><w:instrText xml:space="preserve"> SEQ Figure \\* ARABIC </w:instrText></w:r>'
    '<w:r><w:fldChar w:fldCharType="separate"/></w:r>' + run('1')
    + '<w:r><w:fldChar w:fldCharType="end"/></w:r>',
    e6))

s7, e7 = mark(7, 'G_hyphen')
targets.append(para(s7, run('soft­hyphen and non‑breaking'), e7))

s8, e8 = mark(8, 'H_tab')
targets.append(para(s8, run('before'), '<w:r><w:tab/></w:r>', run('after'), e8))

arm('A more than the cache', [field('REF A_more \\h', '7')])
arm('B into the next paragraph', [field('REF B_span \\h', 'stale')])
arm('C a collapsed bookmark', [field('REF C_point \\h', 'stale')])
arm('D a collapsed cross-reference', [field('REF __RefHeading__5 \\h', 'stale')])
arm('E a line break inside', [field('REF E_break \\h', 'stale')])
arm('F a numbering field inside', [field('REF F_seq \\h', 'stale')])
arm('G the two hyphens', [field('REF G_hyphen \\h', 'stale')])
arm('H a tab inside', [field('REF H_tab \\h', 'stale')])
arm('I no such bookmark', [field('REF Z_missing \\h', 'stale')])
arm('J the page switch', [field('REF A_more \\p \\h', 'stale')])
arm('K a page reference', [field('PAGEREF A_more \\h', 'stale')])
arm('L the compact form', ['<w:fldSimple w:instr=" REF A_more \\h ">' + run('stale')
                          + '</w:fldSimple>'])

DOCUMENT = (
    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
    f'<w:document xmlns:w="{W}"><w:body>'
    + ''.join(ARMS) + ''.join(targets)
    + '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
      '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/></w:sectPr>'
    '</w:body></w:document>')

PARTS = {
    '[Content_Types].xml':
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
        '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
        '<Default Extension="xml" ContentType="application/xml"/>'
        '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
        '<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>'
        '<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>'
        '</Types>',
    '_rels/.rels':
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>'
        '</Relationships>',
    'word/_rels/document.xml.rels':
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>'
        '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'
        '</Relationships>',
    'word/settings.xml':
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        f'<w:settings xmlns:w="{W}"><w:defaultTabStop w:val="720"/></w:settings>',
    'word/styles.xml':
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
        '<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
        '<w:sz w:val="20"/></w:rPr></w:rPrDefault></w:docDefaults></w:styles>',
    'word/document.xml': DOCUMENT,
}


def main(path):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, text in PARTS.items():
            z.writestr(name, text)
    print(path, pathlib.Path(path).stat().st_size, 'bytes')


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1
         else 'dotnet/tests/corpus/features/words-reference-field.docx')
