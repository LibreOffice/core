#!/usr/bin/env python3
"""Build `features/words-style-char-scale.docx`, the O94 fixture.

`w:w` is character width scaling: the glyphs narrow and the line does not get
shorter in height. It is read through the style chain like any other run
property -- and a paragraph whose runs state NO `w:rPr` at all is uniform by
every test the run splitter makes, so it reaches the layout with no runs and
was measured and drawn at 100 per cent whatever its style said.

Three paragraphs, all the same text at the same size, differing in one thing:

  A  the style states `w:w="60"` and the runs state nothing
        -- the defect: uniform, so the scale had nowhere to go;
  B  the runs state `w:w="60"` and the style states nothing
        -- the control that already worked;
  C  neither states anything
        -- the control that must not move;
  D  the style states it and the text is long enough to WRAP
        -- because the scale has to reach the line breaker as well as the pen,
        and a one-line arm cannot tell the two apart: this tree carried the
        scale in neither, and carrying it in only one would break the line at
        one width and draw it at another;
  E  the same long text with no scale at all, the wrap control.

60 rather than the corpus's commonest 99 because the question is whether the
scale is applied at all, and 99 is within a percent of not being applied.
"""
import zipfile, pathlib, sys

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
TEXT = 'Hamburgefonstiv 12345'
WRAP = ('alpha bravo charlie delta echo foxtrot golf hotel india '
        'juliett kilo lima mike november oscar papa quebec romeo')

PARTS = {
'[Content_Types].xml': '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>''',
'_rels/.rels': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/officeDocument" Target="word/document.xml"/>
</Relationships>''',
'word/_rels/document.xml.rels': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/styles" Target="styles.xml"/>
<Relationship Id="rId2" Type="{R}/settings" Target="settings.xml"/>
</Relationships>''',
# An empty settings part is load-bearing: without one the importer takes a
# different set of OOXML compatibility defaults and the fixture answers a
# question nobody asked. See the `office-corpus` skill.
'word/settings.xml': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="{W}"><w:compat><w:compatSetting w:name="compatibilityMode" w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat></w:settings>''',
'word/styles.xml': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="{W}">
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="48"/>
</w:rPr></w:rPrDefault></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
<w:style w:type="paragraph" w:customStyle="1" w:styleId="Scaled">
<w:name w:val="Scaled"/><w:basedOn w:val="Normal"/>
<w:rPr><w:w w:val="60"/></w:rPr>
</w:style>
</w:styles>''',
'word/document.xml': f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="{W}"><w:body>
<w:p><w:pPr><w:pStyle w:val="Scaled"/></w:pPr><w:r><w:t>{TEXT}</w:t></w:r></w:p>
<w:p><w:r><w:rPr><w:w w:val="60"/></w:rPr><w:t>{TEXT}</w:t></w:r></w:p>
<w:p><w:r><w:t>{TEXT}</w:t></w:r></w:p>
<w:p><w:pPr><w:pStyle w:val="Scaled"/></w:pPr><w:r><w:t>{WRAP}</w:t></w:r></w:p>
<w:p><w:r><w:t>{WRAP}</w:t></w:r></w:p>
</w:body></w:document>''',
}


def main():
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1
                       else 'dotnet/tests/corpus/features/words-style-char-scale.docx')
    out.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, text in PARTS.items():
            z.writestr(name, text)
    print(f'wrote {out} ({out.stat().st_size} bytes)')


if __name__ == '__main__':
    main()
