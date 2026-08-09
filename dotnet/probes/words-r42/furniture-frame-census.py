#!/usr/bin/env python3
"""Count the DOCX in a track whose running heads could exercise either of round 42's two fixes.

A ceiling, and labelled as one — it counts what a part *declares*, not what a page *draws*, and this
project has measured that gap at two-fold, six-fold and fortyfold. It is also blind to the binary
half of the track: a `.doc` keeps its header stories in the WW8 text stream, where no zip-level
census can see them, and that is 66 of the words track's 200 documents.

Two populations, matching the two fixes:

  furniture-table-frame   a header or footer part holding a `w:tbl` with a `w:drawing`/`w:pict`
                          inside it. `FrameLayout.FlowsOn` walked the body's tables and not the
                          furniture's, so a picture in a header table was never placed.
  furniture-frame-field   a header or footer part holding a `PAGE`/`NUMPAGES` field instruction
                          inside a `txbxContent`. `PageFields` descended into table cells and not
                          into a paragraph's frames, so such a footer was laid out once and every
                          page got page one's copy.
"""
from __future__ import annotations

import re
import sys
import zipfile
from pathlib import Path

FIELD = re.compile(r"<w:instrText[^>]*>([^<]*)</w:instrText>", re.S)
PAGE_FIELD = re.compile(r"\b(page|numpages)\b", re.I)


def furniture_parts(zf: zipfile.ZipFile):
    for name in zf.namelist():
        base = name.rsplit("/", 1)[-1]
        if re.fullmatch(r"(header|footer)\d*\.xml", base):
            yield name


def table_with_drawing(xml: str) -> bool:
    """A `w:tbl` somewhere above a `w:drawing` or a `w:pict`, by span rather than by parse."""
    for table in re.finditer(r"<w:tbl>", xml):
        end = xml.find("</w:tbl>", table.end())
        if end < 0:
            end = len(xml)
        inner = xml[table.end():end]
        if "<w:drawing>" in inner or "<w:pict" in inner:
            return True
    return False


def field_in_textbox(xml: str) -> bool:
    for box in re.finditer(r"<w:txbxContent>", xml):
        end = xml.find("</w:txbxContent>", box.end())
        if end < 0:
            end = len(xml)
        for instr in FIELD.finditer(xml[box.end():end]):
            if PAGE_FIELD.search(instr.group(1)):
                return True
    return False


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else "/workspace/sample-files/words")
    docx = tables = fields = both = binary = 0
    hits = {"table": [], "field": []}
    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        if path.suffix.lower() in (".doc", ".dot"):
            binary += 1
            continue
        if path.suffix.lower() not in (".docx", ".docm", ".dotx", ".dotm"):
            continue
        docx += 1
        try:
            with zipfile.ZipFile(path) as zf:
                xml = "".join(zf.read(name).decode("utf-8", "replace")
                              for name in furniture_parts(zf))
        except (OSError, zipfile.BadZipFile):
            continue
        t = table_with_drawing(xml)
        f = field_in_textbox(xml)
        tables += t
        fields += f
        both += t and f
        if t:
            hits["table"].append(path.name)
        if f:
            hits["field"].append(path.name)

    print(f"docx read                     {docx}")
    print(f"binary .doc, invisible here   {binary}")
    print(f"furniture-table-frame         {tables}")
    print(f"furniture-frame-field         {fields}")
    print(f"both                          {both}")
    for kind, names in hits.items():
        print(f"\n{kind}:")
        for name in names:
            print(f"  {name}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
