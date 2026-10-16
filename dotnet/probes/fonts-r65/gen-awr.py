#!/usr/bin/env python3
"""What class is in force at `AWR OPS-AOC 044`'s `MS Gothic` runs, and what it draws.

That document draws 103 `U+2610` in runs naming `MS Gothic`, which its own font table files
`modern` — a code `FontTable::lcl_sprm` drops on the floor
(`sw/source/writerfilter/dmapper/FontTable.cxx`:127-141), so no `PROP_CHAR_FONT_FAMILY` is inserted
and the class is whatever an ancestor set. These cells reproduce the shape one variable at a time.
"""
import pathlib
import sys
import zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
      '<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>'
      '<Override PartName="/word/fontTable.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml"/></Types>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/></Relationships>')
DRELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
         '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'
         '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable" Target="fontTable.xml"/></Relationships>')


def styles(default_fonts):
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:styles xmlns:w="{W}">'
            f'<w:docDefaults><w:rPrDefault><w:rPr>{default_fonts}</w:rPr></w:rPrDefault></w:docDefaults>'
            f'<w:style w:type="paragraph" w:default="1" w:styleId="Normal">'
            f'<w:name w:val="Normal"/></w:style></w:styles>')


def table(entries):
    body = ''.join(f'<w:font w:name="{n}"><w:family w:val="{f}"/></w:font>' for n, f in entries)
    return f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:fonts xmlns:w="{W}">{body}</w:fonts>'


def document(family):
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:document xmlns:w="{W}"><w:body><w:p><w:r><w:rPr>'
            f'<w:rFonts w:ascii="{family}" w:hAnsi="{family}"/><w:sz w:val="40"/></w:rPr>'
            '<w:t>☐</w:t></w:r></w:p>'
            '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
            '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"/></w:sectPr>'
            '</w:body></w:document>')


THEME = '<w:rFonts w:asciiTheme="minorHAnsi" w:hAnsiTheme="minorHAnsi"/>'

CELLS = [
    ("awr-msgothic-modern-theme", THEME, [("MS Gothic", "modern"), ("Calibri", "swiss")], "MS Gothic"),
    ("awr-msgothic-modern-plain", '', [("MS Gothic", "modern")], "MS Gothic"),
    ("awr-msgothic-notable", '', [], "MS Gothic"),
    ("awr-msgothic-roman", '', [("MS Gothic", "roman")], "MS Gothic"),
    ("awr-msgothic-swiss", '', [("MS Gothic", "swiss")], "MS Gothic"),
    ("awr-segoe-swiss", '', [("Segoe UI Symbol", "swiss")], "Segoe UI Symbol"),
    ("awr-aptos-modern", '', [("Aptos", "modern")], "Aptos"),
]


def main():
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "src-awr")
    out.mkdir(parents=True, exist_ok=True)
    for stem, default_fonts, entries, family in CELLS:
        with zipfile.ZipFile(out / f"{stem}.docx", "w", zipfile.ZIP_DEFLATED) as z:
            z.writestr("[Content_Types].xml", CT)
            z.writestr("_rels/.rels", RELS)
            z.writestr("word/_rels/document.xml.rels", DRELS)
            z.writestr("word/document.xml", document(family))
            z.writestr("word/styles.xml", styles(default_fonts))
            z.writestr("word/fontTable.xml", table(entries))
    print(f"wrote {len(CELLS)} files to {out}")


if __name__ == "__main__":
    main()
