#!/usr/bin/env python3
"""Build tests/corpus/features/style-one-sided-spacing.docx.

Four paragraph styles, each stating one half of w:spacing and based on a different parent,
so the one file carries the rule and both of its controls:

  KidLateHeading  after only, based on a `heading 2` declared AFTER it   -> 12 pt above
  KidEarlyHeading after only, based on a `heading 3` declared BEFORE it  -> the parent's own
  KidLateCustom   after only, based on a custom style declared after it  -> nought above
  KidBoth         both stated                                            -> untouched

`word/settings.xml` is present and non-empty on purpose: a DOCX without one is imported
against a different set of defaults, so a fixture that omits it answers a question nobody
asked.
"""
import os
import sys
import zipfile

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
  <Default Extension="xml" ContentType="application/xml"/>
  <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
  <Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
  <Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Target="word/document.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
</Relationships>"""

DOC_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Target="styles.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"/>
  <Relationship Id="rId2" Target="settings.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"/>
</Relationships>"""

SETTINGS = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings {W} xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml"
  xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" mc:Ignorable="w14">
  <w:defaultTabStop w:val="720"/>
  <w:compat><w:compatSetting w:name="compatibilityMode"
    w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat>
</w:settings>"""


def style(sid, name, based, spacing):
    b = f'<w:basedOn w:val="{based}"/>' if based else ''
    return (f'<w:style w:type="paragraph" w:styleId="{sid}"><w:name w:val="{name}"/>{b}'
            f'<w:next w:val="Normal"/><w:qFormat/><w:pPr>{spacing}</w:pPr>'
            f'<w:rPr><w:sz w:val="22"/></w:rPr></w:style>')


STYLES = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {W}>
<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/>
<w:sz w:val="22"/><w:lang w:val="en-US"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:qFormat/></w:style>
{style('Heading3', 'heading 3', 'Normal', '<w:spacing w:before="480"/>')}
{style('KidEarlyHeading', 'Kid Early Heading', 'Heading3', '<w:spacing w:after="240"/>')}
{style('KidLateHeading', 'Kid Late Heading', 'Heading2', '<w:spacing w:after="240"/>')}
{style('KidLateCustom', 'Kid Late Custom', 'MyBase', '<w:spacing w:after="240"/>')}
{style('KidBoth', 'Kid Both', 'Heading2', '<w:spacing w:before="60" w:after="240"/>')}
{style('Heading2', 'heading 2', 'Normal', '<w:spacing w:before="480"/>')}
{style('MyBase', 'My Base', 'Normal', '<w:spacing w:before="480"/>')}
</w:styles>"""


def para(sid, text):
    return (f'<w:p><w:pPr><w:pStyle w:val="{sid}"/></w:pPr>'
            f'<w:r><w:t>{text}</w:t></w:r></w:p>')


DOCUMENT = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {W}><w:body>
<w:p><w:r><w:t>alpha</w:t></w:r></w:p>
{para('KidLateHeading', 'late heading parent')}
<w:p><w:r><w:t>beta</w:t></w:r></w:p>
{para('KidEarlyHeading', 'early heading parent')}
<w:p><w:r><w:t>gamma</w:t></w:r></w:p>
{para('KidLateCustom', 'late custom parent')}
<w:p><w:r><w:t>delta</w:t></w:r></w:p>
{para('KidBoth', 'both stated')}
<w:p><w:r><w:t>omega</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/>
</w:sectPr></w:body></w:document>"""

PARTS = {
    '[Content_Types].xml': CONTENT_TYPES,
    '_rels/.rels': ROOT_RELS,
    'word/_rels/document.xml.rels': DOC_RELS,
    'word/document.xml': DOCUMENT,
    'word/styles.xml': STYLES,
    'word/settings.xml': SETTINGS,
}

out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    os.path.dirname(__file__), '..', '..', '..',
    'tests', 'corpus', 'features', 'style-one-sided-spacing.docx')
os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
    for name, data in PARTS.items():
        z.writestr(name, data)
print('wrote', os.path.abspath(out))
