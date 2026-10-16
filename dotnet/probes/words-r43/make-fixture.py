#!/usr/bin/env python3
"""Author `tests/corpus/features/inherited-table-header.docx`.

The shape `UG.CAO.00133 … Language.docx` has and every authored probe of round 42 lacked: a first
section whose header is **a table with no paragraph outside it**, and a second section naming an
empty even and an empty first header and no default one — the two slots Word writes into a section
the user never gave a header of its own.

Committed as a fixture because it is the case `SectionInheritedHeaderTests` pins, in both
directions: §17.10.1 says the second section inherits the first's header, we do, and LibreOffice
24.2.7.2 does **not** — its own PDF of this file leaves page 2 bare. The deviation is deliberate
and the test says so, so that reproducing LibreOffice's import defect cannot happen quietly.

    make-fixture.py [outdir]
"""
from __future__ import annotations

import sys
import zipfile
from pathlib import Path

NS = ('xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"')

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
<Override PartName="/word/header1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
<Override PartName="/word/header2.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
<Override PartName="/word/header3.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

# The style is stated rather than left to the application's fallback, per the skill's note about
# authored files. This fixture asserts *presence*, but the habit costs three lines.
STYLES = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {NS}><w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="22"/>
</w:rPr></w:rPrDefault></w:docDefaults></w:styles>"""

SECT = ('<w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" '
        'w:header="708" w:footer="708"/>')

DOCUMENT = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {NS}><w:body>
<w:p><w:pPr><w:sectPr>
<w:headerReference w:type="default" r:id="rIdH1"/>{SECT}
</w:sectPr></w:pPr><w:r><w:t>First section body</w:t></w:r></w:p>
<w:p><w:r><w:t>Second section body</w:t></w:r></w:p>
<w:sectPr>
<w:headerReference w:type="even" r:id="rIdH2"/>
<w:headerReference w:type="first" r:id="rIdH3"/>{SECT}
</w:sectPr></w:body></w:document>"""

TABLE_HEADER = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:hdr {NS}><w:tbl>
<w:tblPr><w:tblW w:w="0" w:type="auto"/></w:tblPr>
<w:tblGrid><w:gridCol w:w="2000"/><w:gridCol w:w="6000"/></w:tblGrid>
<w:tr>
<w:tc><w:tcPr><w:tcW w:w="2000" w:type="dxa"/></w:tcPr>
<w:p><w:r><w:t>Running head</w:t></w:r></w:p></w:tc>
<w:tc><w:tcPr><w:tcW w:w="6000" w:type="dxa"/></w:tcPr>
<w:p><w:r><w:t>Rev 1</w:t></w:r></w:p></w:tc>
</w:tr></w:tbl></w:hdr>"""

EMPTY_HEADER = f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<w:hdr {NS}><w:p/></w:hdr>'


def main() -> int:
    outdir = Path(sys.argv[1] if len(sys.argv) > 1
                  else "/home/user/libreoffice-core/dotnet/tests/corpus/features")
    outdir.mkdir(parents=True, exist_ok=True)
    path = outdir / "inherited-table-header.docx"
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zf:
        zf.writestr("[Content_Types].xml", CONTENT_TYPES)
        zf.writestr("_rels/.rels", ROOT_RELS)
        zf.writestr("word/document.xml", DOCUMENT)
        zf.writestr("word/styles.xml", STYLES)
        zf.writestr("word/header1.xml", TABLE_HEADER)
        zf.writestr("word/header2.xml", EMPTY_HEADER)
        zf.writestr("word/header3.xml", EMPTY_HEADER)
        zf.writestr(
            "word/_rels/document.xml.rels",
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            '<Relationship Id="rIdS" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
            'relationships/styles" Target="styles.xml"/>'
            '<Relationship Id="rIdH1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
            'relationships/header" Target="header1.xml"/>'
            '<Relationship Id="rIdH2" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
            'relationships/header" Target="header2.xml"/>'
            '<Relationship Id="rIdH3" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
            'relationships/header" Target="header3.xml"/>'
            "</Relationships>")
    print(f"wrote {path} ({path.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
