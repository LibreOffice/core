#!/usr/bin/env python3
"""Build `features/words-object-duplicate.docx`, the O90 fixture.

One `w:object` carrying `w:dxaOrig`/`w:dyaOrig` and holding a `v:shape` whose
own `style` states the box, with a `v:imagedata` inside it. That is the shape
the defect needs and the smallest one that shows it:

  * `DocxVmlFrames.TopLevel` offered EVERY descendant of the `w:object` to
    `One`, because its filter read `IsShape(child) is not false` against a
    predicate that never answered false;
  * `One` falls back to the OBJECT's `dxaOrig`/`dyaOrig` when the element it is
    given states no size, and that is the same pair for every descendant;
  * `DocxPictures.ReadVml` searches `DescendantsAndSelf`, so the `v:imagedata`
    element resolves ITSELF.

So the replacement picture was drawn twice -- once for the `v:shape` and once
for the `v:imagedata` inside it. A `v:shapetype` is included too: it is what the
class' own remark claimed was "excluded deliberately" while nothing excluded it.
"""
import struct, zlib, zipfile, pathlib, sys

def png(w, h, rgb):
    def chunk(tag, data):
        c = tag + data
        return struct.pack('>I', len(data)) + c + struct.pack('>I', zlib.crc32(c) & 0xffffffff)
    ihdr = struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0)
    raw = b''.join(b'\x00' + bytes(rgb) * w for _ in range(h))
    return (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', ihdr)
            + chunk(b'IDAT', zlib.compress(raw)) + chunk(b'IEND', b''))

R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'

PARTS = {
'[Content_Types].xml': '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Default Extension="png" ContentType="image/png"/>
<Default Extension="bin" ContentType="application/vnd.openxmlformats-officedocument.oleObject"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>''',
'_rels/.rels': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/officeDocument" Target="word/document.xml"/>
</Relationships>''',
'word/_rels/document.xml.rels': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/settings" Target="settings.xml"/>
<Relationship Id="rId2" Type="{R}/styles" Target="styles.xml"/>
<Relationship Id="rId3" Type="{R}/image" Target="media/image1.png"/>
<Relationship Id="rId4" Type="{R}/oleObject" Target="embeddings/oleObject1.bin"/>
</Relationships>''',
# An empty settings part is load-bearing: without one the importer takes a different
# set of OOXML compatibility defaults and the fixture answers a question nobody asked.
# See `.claude/skills/office-corpus/SKILL.md`.
'word/settings.xml': '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
            xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml">
<w:compat><w:compatSetting w:name="compatibilityMode"
  w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat>
</w:settings>''',
'word/styles.xml': '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/><w:sz w:val="22"/>
</w:rPr></w:rPrDefault></w:docDefaults>
</w:styles>''',
'word/document.xml': '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
            xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
            xmlns:v="urn:schemas-microsoft-com:vml"
            xmlns:o="urn:schemas-microsoft-com:office:office">
<w:body>
<w:p><w:r><w:t>Before.</w:t></w:r></w:p>
<w:p><w:r><w:object w:dxaOrig="1540" w:dyaOrig="997">
<v:shapetype id="_x0000_t75" coordsize="21600,21600" o:spt="75" o:preferrelative="t"
             path="m@4@5l@4@11@9@11@9@5xe" filled="f" stroked="f">
  <v:stroke joinstyle="miter"/>
  <v:path o:extrusionok="f" gradientshapeok="t" o:connecttype="rect"/>
</v:shapetype>
<v:shape id="_x0000_i1025" type="#_x0000_t75" style="width:77.25pt;height:49.5pt" o:ole="">
  <v:imagedata r:id="rId3" o:title=""/>
</v:shape>
<o:OLEObject Type="Embed" ProgID="Package" ShapeID="_x0000_i1025"
             DrawAspect="Icon" ObjectID="_1234567890" r:id="rId4"/>
</w:object></w:r></w:p>
<w:p><w:r><w:t>After.</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"
         w:header="708" w:footer="708" w:gutter="0"/></w:sectPr>
</w:body></w:document>''',
}

def main():
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1
                       else 'dotnet/tests/corpus/features/words-object-duplicate.docx')
    out.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, text in PARTS.items():
            z.writestr(name, text)
        z.writestr('word/media/image1.png', png(2, 2, (0x30, 0x60, 0xC0)))
        z.writestr('word/embeddings/oleObject1.bin', b'probe')
    print(f'wrote {out} ({out.stat().st_size} bytes)')

if __name__ == '__main__':
    main()
