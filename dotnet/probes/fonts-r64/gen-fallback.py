#!/usr/bin/env python3
"""One DOCX per (family, missing character), so `pdffonts` attributes the fallback face without
ambiguity.

The question is *which* of the several installed faces that cover a character LibreOffice draws it
in, and whether the answer depends on the family the run asked for. Characters are chosen so that
neither the requested family's own face nor its metric-compatible stand-in covers them, and so that
both OpenSymbol (head of LibreOffice's generic glyph-fallback list) and DejaVu Sans (further down
it) do — which is exactly the case where the two stages of the search disagree.
"""
import pathlib
import sys
import zipfile

FAMILIES = ["Arial", "Times New Roman", "Courier New", "Calibri", "Cambria",
            "Wingdings", "Zzzz Nonexistent Family", "Georgia"]
# hex, note
CHARS = ["2713", "2714", "27A2", "2611"]

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
      '<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/></Types>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/></Relationships>')
DRELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
         '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/></Relationships>')
SET = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>')


def document(family: str, ch: str) -> str:
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:body>'
            f'<w:p><w:r><w:rPr><w:rFonts w:ascii="{family}" w:hAnsi="{family}" w:cs="{family}"/>'
            f'<w:sz w:val="48"/></w:rPr><w:t>{ch}</w:t></w:r></w:p>'
            '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
            '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"/></w:sectPr>'
            '</w:body></w:document>')


def main() -> None:
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "src-fallback")
    out.mkdir(parents=True, exist_ok=True)
    index = []
    for family in FAMILIES:
        for cp in CHARS:
            stem = f"{family.replace(' ', '_')}__{cp}"
            path = out / f"{stem}.docx"
            with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
                z.writestr("[Content_Types].xml", CT)
                z.writestr("_rels/.rels", RELS)
                z.writestr("word/_rels/document.xml.rels", DRELS)
                z.writestr("word/document.xml", document(family, chr(int(cp, 16))))
                z.writestr("word/settings.xml", SET)
            index.append(f"{stem}\t{family}\tU+{cp}")
    (out / "index.tsv").write_text("\n".join(index) + "\n")
    print(f"wrote {len(index)} files to {out}")


if __name__ == "__main__":
    main()
