#!/usr/bin/env python3
"""Which of Writer's three character-font items a run's glyph fallback is ranked by.

Writer keeps `RES_CHRATR_FONT`, `RES_CHRATR_CJK_FONT` and `RES_CHRATR_CTL_FONT`, and
`SwScriptInfo::WhichFont` picks one per script item of the text (`sw/source/core/text/porlay.cxx`
:893-901).  `FontConfigManager::Substitute` appends a generic for the *selected* item's family
type only (`vcl/unx/generic/font/fontconfig.cxx`:1075-1088), so which item is selected decides
which of fontconfig's preference lists ranks the faces that cover a missing character.

One DOCX per cell so `pdffonts` attributes the answer without ambiguity.  The family is always
`Calibri`, which resolves to Carlito here and covers none of the probe characters, so every cell
actually falls back.
"""
import pathlib
import sys
import zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
FAMILY = "Calibri"

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


def drels(with_table):
    rels = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>',
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">',
            '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>']
    if with_table:
        rels.append('<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable" Target="fontTable.xml"/>')
    rels.append('</Relationships>')
    return ''.join(rels)


def font_table(kind):
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:fonts xmlns:w="{W}">'
            f'<w:font w:name="{FAMILY}"><w:family w:val="{kind}"/></w:font></w:fonts>')


def document(slots, hint, text):
    attrs = ' '.join(f'w:{k}="{v}"' for k, v in slots.items())
    if hint:
        attrs += f' w:hint="{hint}"'
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:document xmlns:w="{W}"><w:body>'
            f'<w:p><w:r><w:rPr><w:rFonts {attrs}/>'
            f'<w:sz w:val="48"/><w:szCs w:val="48"/></w:rPr><w:t>{text}</w:t></w:r></w:p>'
            '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
            '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"/></w:sectPr>'
            '</w:body></w:document>')


A = {"ascii": FAMILY, "hAnsi": FAMILY}
AE = {"ascii": FAMILY, "hAnsi": FAMILY, "eastAsia": FAMILY}
AC = {"ascii": FAMILY, "hAnsi": FAMILY, "cs": FAMILY}
E = {"eastAsia": FAMILY}
C = {"cs": FAMILY}

# stem, slots, hint, table class (None = no font table), code point
CELLS = [
    # The western item: the roman default and the one class that moves it.
    ("west-none-2610", A, None, None, "2610"),
    ("west-roman-2610", A, None, "roman", "2610"),
    ("west-swiss-2610", A, None, "swiss", "2610"),
    ("west-none-2713", A, None, None, "2713"),
    ("west-swiss-2713", A, None, "swiss", "2713"),
    # The CJK item, reached by a `w:hint="eastAsia"` on a weak character.
    ("cjk-hint-none-2610", AE, "eastAsia", None, "2610"),
    ("cjk-hint-roman-2610", AE, "eastAsia", "roman", "2610"),
    ("cjk-hint-swiss-2610", AE, "eastAsia", "swiss", "2610"),
    ("cjk-hint-none-2713", AE, "eastAsia", None, "2713"),
    ("cjk-hint-eaonly-2610", E, "eastAsia", None, "2610"),
    ("cjk-nohint-2610", AE, None, None, "2610"),
    # An asian character selects the CJK item with no hint at all.
    ("cjk-char-none-4E00", A, None, None, "4E00"),
    ("cjk-char-ea-4E00", AE, None, None, "4E00"),
    ("cjk-char-swiss-4E00", A, None, "swiss", "4E00"),
    # The CTL item, reached by a complex-script character.
    ("ctl-cs-none-05D0", AC, None, None, "05D0"),
    ("ctl-cs-roman-05D0", AC, None, "roman", "05D0"),
    ("ctl-cs-swiss-05D0", AC, None, "swiss", "05D0"),
    ("ctl-nocs-none-05D0", A, None, None, "05D0"),
    ("ctl-cs-none-0E01", AC, None, None, "0E01"),
    ("ctl-cs-swiss-0E01", AC, None, "swiss", "0E01"),
    ("ctl-nocs-none-0E01", A, None, None, "0E01"),
    # A `w:hint="cs"` puts a weak character on the CTL item.
    ("ctl-hint-none-2610", AC, "cs", None, "2610"),
    ("ctl-hint-swiss-2610", AC, "cs", "swiss", "2610"),
    # Arabic, the CTL item's own default language.
    ("ctl-cs-none-0627", AC, None, None, "0627"),
    ("ctl-nocs-none-0627", A, None, None, "0627"),
]


def main():
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "src-scriptitem")
    out.mkdir(parents=True, exist_ok=True)
    index = []
    for stem, slots, hint, kind, cp in CELLS:
        with zipfile.ZipFile(out / f"{stem}.docx", "w", zipfile.ZIP_DEFLATED) as z:
            z.writestr("[Content_Types].xml", CT)
            z.writestr("_rels/.rels", RELS)
            z.writestr("word/_rels/document.xml.rels", drels(kind is not None))
            z.writestr("word/document.xml", document(slots, hint, chr(int(cp, 16))))
            z.writestr("word/settings.xml", SET)
            if kind is not None:
                z.writestr("word/fontTable.xml", font_table(kind))
        index.append(f"{stem}\t{','.join(slots)}\t{hint or '-'}\t{kind or '-'}\tU+{cp}")
    (out / "index.tsv").write_text("\n".join(index) + "\n")
    print(f"wrote {len(index)} files to {out}")


if __name__ == "__main__":
    main()
