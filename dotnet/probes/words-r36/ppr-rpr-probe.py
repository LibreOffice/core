#!/usr/bin/env python3
"""Does `w:pPr/w:rPr` format a paragraph's runs, or only its paragraph mark?

ECMA-376 calls it "Run Properties for the Paragraph Mark". Word applies it to the pilcrow.
What LibreOffice does is the question that matters here, because LibreOffice made the
reference PDFs — so this authors a document, converts it with the installed binary, and reads
the answer out of the flat-ODF export rather than out of anybody's source.

The probe states its styles explicitly. A hand-built DOCX with no `word/styles.xml` lays out
in the application's fallback face, and every length measured against it is then a length in a
different document.

    ppr-rpr-probe.py /abs/scratch/dir
"""
import subprocess
import sys
import zipfile
from pathlib import Path

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>"""

RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""

DOC_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
</Relationships>"""

# Every face is named outright: Liberation Serif for the body, and a bold, 18 pt Liberation
# Sans for the styled heading, so the exported weight and size can only have come from where
# this probe put them.
STYLES = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {W}>
 <w:docDefaults><w:rPrDefault><w:rPr>
   <w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>
   <w:sz w:val="20"/></w:rPr></w:rPrDefault></w:docDefaults>
 <w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
 <w:style w:type="paragraph" w:styleId="BoldHead"><w:name w:val="BoldHead"/>
   <w:basedOn w:val="Normal"/>
   <w:rPr><w:rFonts w:ascii="Liberation Sans" w:hAnsi="Liberation Sans"/>
          <w:b/><w:sz w:val="36"/></w:rPr></w:style>
</w:styles>"""


def paragraph(style, mark_rpr, run_rpr, text):
    named = '<w:pStyle w:val="%s"/>' % style if style else ""
    ppr = f'<w:pPr>{named}{mark_rpr}</w:pPr>'
    return f'<w:p>{ppr}<w:r>{run_rpr}<w:t xml:space="preserve">{text}</w:t></w:r></w:p>'


MARK_OFF = '<w:rPr><w:b w:val="0"/></w:rPr>'
MARK_ON = '<w:rPr><w:b/><w:sz w:val="48"/></w:rPr>'
RUN_ON = '<w:rPr><w:b/></w:rPr>'

BODY = "".join([
    # 1. the corpus case: a bold style, the mark turning bold off, a run saying nothing
    paragraph("BoldHead", MARK_OFF, "", "MARKOFF"),
    # 2. control: the same style, nothing on the mark
    paragraph("BoldHead", "", "", "CONTROL"),
    # 3. the mark asserting a property the style does not have, on an unstyled paragraph
    paragraph(None, MARK_ON, "", "MARKON"),
    # 4. the mark turning bold off and the run turning it on — the run must win
    paragraph("BoldHead", MARK_OFF, RUN_ON, "RUNWINS"),
    # 5. an empty paragraph whose only size statement is on the mark
    '<w:p><w:pPr><w:rPr><w:sz w:val="72"/></w:rPr></w:pPr></w:p>',
    paragraph(None, "", "", "AFTEREMPTY"),
])

DOCUMENT = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {W}><w:body>{BODY}
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"/></w:sectPr>
</w:body></w:document>"""


def main():
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    docx = out / "ppr-rpr-probe.docx"
    with zipfile.ZipFile(docx, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        z.writestr("_rels/.rels", RELS)
        z.writestr("word/_rels/document.xml.rels", DOC_RELS)
        z.writestr("word/styles.xml", STYLES)
        z.writestr("word/document.xml", DOCUMENT)
    print(f"wrote {docx}")

    subprocess.run(["soffice", "--headless", f"-env:UserInstallation=file://{out}/prof",
                    "--convert-to", "fodt", "--outdir", str(out), str(docx)],
                   capture_output=True)
    fodt = (out / "ppr-rpr-probe.fodt").read_text()
    print("\n=== what LibreOffice made of each paragraph ===")
    import re
    for tag in ("MARKOFF", "CONTROL", "MARKON", "RUNWINS", "AFTEREMPTY"):
        i = fodt.find(">" + tag)
        if i < 0:
            print(f"{tag:<12} not found")
            continue
        start = fodt.rfind("<text:p", 0, i)
        print(f"{tag:<12} {fodt[start:i + len(tag) + 1]}")
    print("\n=== text styles ===")
    for m in re.finditer(r'<style:style style:name="(T\d+)" style:family="text">\s*'
                         r'<style:text-properties([^/]*)/>', fodt):
        print(m.group(1), " ".join(m.group(2).split())[:200])
    print("\n=== the empty paragraph ===")
    for m in re.finditer(r'<text:p text:style-name="[^"]*"( loext:marker-style-name="[^"]*")?/>',
                         fodt):
        print(m.group(0))


if __name__ == "__main__":
    main()
