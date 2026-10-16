#!/usr/bin/env python3
"""Does a paragraph's direct `w:spacing w:after` override its style's, on a heading?

`24-25_FAA_Holdover_Tables.docx` says `<w:pStyle w:val="Heading3"/><w:spacing w:after="0"/>` and
26.2.4.2's own `--convert-to fodt` gives that paragraph no `fo:margin-bottom` at all, so it keeps
the style's 6 pt.  One arm per variant, each read out of the reference's resolved view rather than
off a page.
"""
import sys, zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

# (label, pStyle, the paragraph's own <w:spacing …> or '')
ARMS = [
    ('A-h3-nothing',        'H3', ''),
    ('B-h3-after0',         'H3', '<w:spacing w:after="0"/>'),
    ('C-h3-after0-before0', 'H3', '<w:spacing w:after="0" w:before="0"/>'),
    ('D-h3-after40',        'H3', '<w:spacing w:after="40"/>'),
    ('E-h3-after0-line',    'H3', '<w:spacing w:after="0" w:line="240" w:lineRule="auto"/>'),
    ('F-plain-nothing',     'PL', ''),
    ('G-plain-after0',      'PL', '<w:spacing w:after="0"/>'),
    ('H-plain-after40',     'PL', '<w:spacing w:after="40"/>'),
    ('I-h3-contextual',     'H3', '<w:spacing w:after="0"/><w:contextualSpacing/>'),
    ('J-named-nothing',     'NM', ''),
    ('K-named-after0',      'NM', '<w:spacing w:after="0"/>'),
]

paras = []
for label, style, spacing in ARMS:
    paras.append(f'<w:p><w:pPr><w:pStyle w:val="{style}"/>{spacing}</w:pPr>'
                 f'<w:r><w:t>{label}</w:t></w:r></w:p>')
    paras.append('<w:p><w:r><w:t>follower</w:t></w:r></w:p>')

DOCUMENT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:document xmlns:w="{W}"><w:body>' + ''.join(paras)
            + '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
              '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/></w:sectPr>'
            '</w:body></w:document>')

# H3 is the witness's shape: a built-in heading name, an outline level and an `after` of 120.
# PL is the same properties under a name Writer has no pool row for, and NM is the built-in name
# with no outline level -- so the three separate "is it the name", "is it the level" and "is it
# the spacing" readings.
STYLES = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
          f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
          '<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
          '<w:sz w:val="20"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>'
          '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
          '<w:name w:val="Normal"/></w:style>'
          '<w:style w:type="paragraph" w:styleId="H3"><w:name w:val="heading 3"/>'
          '<w:basedOn w:val="Normal"/><w:next w:val="Normal"/>'
          '<w:pPr><w:keepNext/><w:spacing w:after="120"/><w:outlineLvl w:val="2"/></w:pPr>'
          '<w:rPr><w:sz w:val="26"/></w:rPr></w:style>'
          '<w:style w:type="paragraph" w:customStyle="1" w:styleId="PL">'
          '<w:name w:val="Plain Block"/><w:basedOn w:val="Normal"/>'
          '<w:pPr><w:keepNext/><w:spacing w:after="120"/></w:pPr>'
          '<w:rPr><w:sz w:val="26"/></w:rPr></w:style>'
          '<w:style w:type="paragraph" w:styleId="NM"><w:name w:val="heading 4"/>'
          '<w:basedOn w:val="Normal"/><w:next w:val="Normal"/>'
          '<w:pPr><w:keepNext/><w:spacing w:after="120"/></w:pPr>'
          '<w:rPr><w:sz w:val="26"/></w:rPr></w:style>'
          '</w:styles>')

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
    main(sys.argv[1] if len(sys.argv) > 1 else '/tmp/claude-0/o110/headspace.docx')
