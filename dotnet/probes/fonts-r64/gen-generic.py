#!/usr/bin/env python3
"""Which installed face draws a character the run's own face cannot, and what decides it.

One DOCX per (declared family class, character), so `pdffonts` attributes the answer without
ambiguity. The class is declared in `word/fontTable.xml`, because 26.2's
`FontConfigManager::Substitute` appends `serif` for `FAMILY_ROMAN` and `sans` for `FAMILY_SWISS`
as a *second* `FC_FAMILY` — and the glyph-fallback hook goes through the same function, so the
declaration decides which of fontconfig's generic preference lists ranks the candidates.

The requested family is one whose own coverage is small (Calibri, which resolves to Carlito here),
so that every character below actually falls back.
"""
import pathlib
import sys
import zipfile

FAMILY = "Calibri"
CLASSES = ["none", "roman", "swiss", "modern", "script", "decorative"]
CHARS = ["2713", "27A2", "2714", "2611", "05D0", "4E00", "0E01", "2500", "25CF", "2022", "2190", "263A"]

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
      '<Override PartName="/word/fontTable.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml"/>'
      '<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/></Types>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/></Relationships>')
SET = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
       '<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>')
W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'


def drels(with_table: bool) -> str:
    rels = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>',
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">',
            '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>']
    if with_table:
        rels.append('<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable" Target="fontTable.xml"/>')
    rels.append('</Relationships>')
    return ''.join(rels)


def font_table(kind: str) -> str:
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:fonts xmlns:w="{W}">'
            f'<w:font w:name="{FAMILY}"><w:family w:val="{kind}"/></w:font></w:fonts>')


def document(ch: str) -> str:
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:document xmlns:w="{W}"><w:body>'
            f'<w:p><w:r><w:rPr><w:rFonts w:ascii="{FAMILY}" w:hAnsi="{FAMILY}" w:cs="{FAMILY}"/>'
            f'<w:sz w:val="48"/></w:rPr><w:t>{ch}</w:t></w:r></w:p>'
            '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
            '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"/></w:sectPr>'
            '</w:body></w:document>')


def main() -> None:
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "src-generic")
    out.mkdir(parents=True, exist_ok=True)
    index = []
    for kind in CLASSES:
        for cp in CHARS:
            stem = f"{kind}__{cp}"
            with zipfile.ZipFile(out / f"{stem}.docx", "w", zipfile.ZIP_DEFLATED) as z:
                z.writestr("[Content_Types].xml", CT)
                z.writestr("_rels/.rels", RELS)
                z.writestr("word/_rels/document.xml.rels", drels(kind != "none"))
                z.writestr("word/document.xml", document(chr(int(cp, 16))))
                z.writestr("word/settings.xml", SET)
                if kind != "none":
                    z.writestr("word/fontTable.xml", font_table(kind))
            index.append(f"{stem}\t{kind}\tU+{cp}")
    (out / "index.tsv").write_text("\n".join(index) + "\n")
    print(f"wrote {len(index)} files to {out}")


if __name__ == "__main__":
    main()
