#!/usr/bin/env python3
"""Do two `w:num` over one `w:abstractNumId` share a counter, and when does an override fire?

One arm per group of paragraphs.  Every paragraph carries its own label in its text, so the
number 26.2.4.2 draws beside it can be read out of the PDF and attributed without guessing.
`word/settings.xml` is present because a hand-built DOCX without one does not get LibreOffice's
OOXML compatibility defaults.
"""
import sys, zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

# (arm, [(abstractId, numId, startOverride or None), …], [(numId or None, …paragraphs…)])
ARMS = [
    # Two instances over one abstract, neither overriding.
    ('A', [(10, 101, None), (10, 102, None)], [101, 101, 102, 102]),
    # The FAA shape: the first instance restarts, the second inherits the running count.
    ('B', [(11, 111, 1), (11, 112, None)], [111, 111, 112, 112]),
    # The override on the *second* instance, which should restart the shared counter once.
    ('C', [(12, 121, None), (12, 122, 1)], [121, 121, 122, 122]),
    # The same with a value other than one, so a restart cannot be confused with a start.
    ('D', [(13, 131, None), (13, 132, 5)], [131, 131, 132, 132]),
    # Does the override fire again the second time its instance is used?
    ('E', [(14, 141, None), (14, 142, 1)], [141, 141, 142, 142, 141, 141, 142, 142]),
    # The control: two instances over two *different* abstracts must not share anything.
    ('F', [(15, 151, None), (16, 161, None)], [151, 151, 161, 161]),
    # The style route: paragraphs with no `w:numPr` take the style's instance.
    ('G', [(17, 171, None), (17, 172, 1)], [172, 172, None, None]),
]


def lvl(start=1):
    return (f'<w:lvl w:ilvl="0"><w:start w:val="{start}"/><w:numFmt w:val="decimal"/>'
            '<w:lvlText w:val="%1."/><w:lvlJc w:val="left"/>'
            '<w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr></w:lvl>')


abstracts, nums, paras = [], [], []
seen = set()
for arm, definitions, sequence in ARMS:
    for abstract, num, override in definitions:
        if abstract not in seen:
            seen.add(abstract)
            abstracts.append(f'<w:abstractNum w:abstractNumId="{abstract}">'
                             '<w:multiLevelType w:val="singleLevel"/>' + lvl() + '</w:abstractNum>')
        body = f'<w:abstractNumId w:val="{abstract}"/>'
        if override is not None:
            body += ('<w:lvlOverride w:ilvl="0">'
                     f'<w:startOverride w:val="{override}"/></w:lvlOverride>')
        nums.append(f'<w:num w:numId="{num}">{body}</w:num>')

    paras.append(f'<w:p><w:r><w:t>ARM {arm}</w:t></w:r></w:p>')
    for i, num in enumerate(sequence, 1):
        if num is None:
            style = '<w:pPr><w:pStyle w:val="ArmStyle"/></w:pPr>'
            tail = 'style'
        else:
            style = ('<w:pPr><w:numPr><w:ilvl w:val="0"/>'
                     f'<w:numId w:val="{num}"/></w:numPr></w:pPr>')
            tail = str(num)
        paras.append(f'<w:p>{style}<w:r><w:t>{arm}{i} via {tail}</w:t></w:r></w:p>')

DOCUMENT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:document xmlns:w="{W}"><w:body>' + ''.join(paras)
            + '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
              '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/></w:sectPr>'
            '</w:body></w:document>')

NUMBERING = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
             f'<w:numbering xmlns:w="{W}">' + ''.join(abstracts) + ''.join(nums) + '</w:numbering>')

STYLES = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
          f'<w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>'
          '<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>'
          '<w:sz w:val="20"/></w:rPr></w:rPrDefault></w:docDefaults>'
          '<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
          '<w:name w:val="Normal"/></w:style>'
          '<w:style w:type="paragraph" w:customStyle="1" w:styleId="ArmStyle">'
          '<w:name w:val="Arm Style"/><w:basedOn w:val="Normal"/>'
          '<w:pPr><w:numPr><w:numId w:val="171"/></w:numPr></w:pPr></w:style>'
          '</w:styles>')

PARTS = {
    '[Content_Types].xml':
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
        '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
        '<Default Extension="xml" ContentType="application/xml"/>'
        '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
        '<Override PartName="/word/numbering.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"/>'
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
        '<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering" Target="numbering.xml"/>'
        '</Relationships>',
    'word/settings.xml':
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        f'<w:settings xmlns:w="{W}"><w:defaultTabStop w:val="720"/></w:settings>',
    'word/styles.xml': STYLES,
    'word/numbering.xml': NUMBERING,
    'word/document.xml': DOCUMENT,
}


def main(path):
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, text in PARTS.items():
            z.writestr(name, text)
    print(path)


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1
         else 'dotnet/tests/corpus/features/words-list-instance-share.docx')
