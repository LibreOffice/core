#!/usr/bin/env python3
"""Minimal DOCX builder for probes."""
import zipfile, sys, os

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
{hdrct}
</Types>'''

ROOTRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''

DOCRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rIdS" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>
{rels}
</Relationships>'''

SETTINGS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">{body}</w:settings>'''

DOC = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
 xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006"
 xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:v="urn:schemas-microsoft-com:vml"
 xmlns:w10="urn:schemas-microsoft-com:office:word"
 mc:Ignorable="w14 wp14"><w:body>{body}</w:body></w:document>'''

HDR = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:hdr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
 xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006"
 xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:v="urn:schemas-microsoft-com:vml"
 xmlns:w10="urn:schemas-microsoft-com:office:word"
 mc:Ignorable="w14 wp14">{body}</w:hdr>'''

PGSZ = ('<w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"'
        ' w:header="708" w:footer="708" w:gutter="0"/>')


def para(text, extra=''):
    return f'<w:p><w:pPr>{extra}</w:pPr><w:r><w:t xml:space="preserve">{text}</w:t></w:r></w:p>'


def build(path, body, headers, settings_body=''):
    """headers: dict name -> hdr inner xml. Referenced as rIdH<name>."""
    rels = ''.join(
        f'<Relationship Id="rIdH{n}" Type="http://schemas.openxmlformats.org/'
        f'officeDocument/2006/relationships/header" Target="header{n}.xml"/>'
        for n in headers)
    hdrct = ''.join(
        f'<Override PartName="/word/header{n}.xml" ContentType="application/vnd.'
        f'openxmlformats-officedocument.wordprocessingml.header+xml"/>' for n in headers)
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT.format(hdrct=hdrct))
        z.writestr('_rels/.rels', ROOTRELS)
        z.writestr('word/_rels/document.xml.rels', DOCRELS.format(rels=rels))
        z.writestr('word/settings.xml', SETTINGS.format(body=settings_body))
        z.writestr('word/document.xml', DOC.format(body=body))
        for n, h in headers.items():
            z.writestr(f'word/header{n}.xml', HDR.format(body=h))
