#!/usr/bin/env python3
"""One DOCX, one arm per paragraph, for what a `TOC \\t` switch does to direct paragraph formatting.

Each arm is a paragraph whose style is or is not named by a `TOC` field's `\\t` switch, carrying a
direct property whose survival the reference's own `--convert-to fodt` then reports.  A separate
`TOC` field precedes each group so the arms cannot interfere; the last arm deliberately puts its
paragraph *before* its field, because position turns out not to matter.
"""
import sys, zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'


def style(sid, name, *, custom=False, outline=None, after=120):
    attrs = ' w:customStyle="1"' if custom else ''
    level = f'<w:outlineLvl w:val="{outline}"/>' if outline is not None else ''
    return (f'<w:style w:type="paragraph"{attrs} w:styleId="{sid}"><w:name w:val="{name}"/>'
            '<w:basedOn w:val="Normal"/>'
            f'<w:pPr><w:keepNext/><w:spacing w:after="{after}"/>'
            f'<w:jc w:val="center"/>{level}</w:pPr>'
            '<w:rPr><w:sz w:val="26"/></w:rPr></w:style>')


STYLES = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
          f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
          '<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
          '<w:sz w:val="20"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>'
          '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
          '<w:name w:val="Normal"/></w:style>'
          '<w:style w:type="paragraph" w:styleId="TOC1"><w:name w:val="toc 1"/>'
          '<w:basedOn w:val="Normal"/></w:style>'
          + style('Heading3', 'heading 3', outline=2)
          + style('Heading4', 'heading 4', outline=3)
          + style('Title', 'Title')
          + style('Custom3', 'heading 5', custom=True, outline=4)
          + style('Named', 'Bar Name', custom=True, outline=2)
          + '</w:styles>')


def toc(template):
    instruction = f' TOC \\h \\z \\t "{template}" ' if template else ' TOC \\h \\z \\o "1-3" '
    return ('<w:p><w:pPr><w:pStyle w:val="TOC1"/></w:pPr>'
            '<w:r><w:fldChar w:fldCharType="begin"/></w:r>'
            f'<w:r><w:instrText xml:space="preserve">{instruction}</w:instrText></w:r>'
            '<w:r><w:fldChar w:fldCharType="separate"/></w:r>'
            '<w:r><w:t>contents entry</w:t></w:r>'
            '<w:r><w:fldChar w:fldCharType="end"/></w:r></w:p>')


# (label, the \t template or None for \o, the paragraph's style, its direct w:pPr, before the field)
ARMS = [
    ('A-named-spacing',   'Heading 3,2',            'Heading3', '<w:spacing w:after="0"/>',   False),
    ('B-named-jc',        'Heading 3,2',            'Heading3', '<w:jc w:val="left"/>',       False),
    ('C-named-ind',       'Heading 3,2',            'Heading3', '<w:ind w:left="720"/>',      False),
    ('D-named-break',     'Heading 3,2',            'Heading3', '<w:pageBreakBefore/>',       False),
    ('E-named-before',    'Heading 3,2',            'Heading3', '<w:spacing w:after="0"/>',   True),
    ('G-other-style',     'Heading 3,2',            'Heading4', '<w:spacing w:after="0"/>',   False),
    ('I-custom-name',     'heading 5,1',            'Custom3',  '<w:spacing w:after="0"/>',   False),
    ('J-custom-outline',  'Bar Name,1',             'Named',    '<w:spacing w:after="0"/>',   False),
    ('K-builtin-title',   'Title,1',                'Title',    '<w:spacing w:after="0"/>',   False),
]

# F (a `\t` naming only Heading 2), H (a `\o` switch and no `\t`) and L (no TOC at all) are
# controls that CANNOT live in this file: the registration is document-wide, so the arms above
# would void them too -- which is itself the measurement that position does not matter. They are
# one document each, under `probes/tocstyle-r160/`, and reference-controls.txt holds their answers.

paras = []
for label, template, pstyle, direct, first in ARMS:
    field = toc(template) if template != '' else ''
    target = (f'<w:p><w:pPr><w:pStyle w:val="{pstyle}"/>{direct}</w:pPr>'
              f'<w:r><w:t>{label}</w:t></w:r></w:p>')
    paras.append('<w:p><w:r><w:t>lead in</w:t></w:r></w:p>')
    paras.extend([target, field] if first else [field, target])
    paras.append('<w:p><w:r><w:t>follower</w:t></w:r></w:p>')

DOCUMENT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:document xmlns:w="{W}"><w:body>' + ''.join(paras)
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
    'word/styles.xml': STYLES,
    'word/document.xml': DOCUMENT,
}


def main(path):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, text in PARTS.items():
            z.writestr(name, text)
    print(path)


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1
         else 'dotnet/tests/corpus/features/words-toc-template-styles.docx')
