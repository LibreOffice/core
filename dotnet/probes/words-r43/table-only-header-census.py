#!/usr/bin/env python3
"""How many corpus documents have a section inheriting a header made only of tables?

`header-inherit-bisect.py` and `header-inherit-content-shape.py` establish the rule: LibreOffice
copies a section's header into a following section that names none of its own **only when the
source header holds at least one top-level `w:p`**. A header whose content is nothing but tables
copies as empty, and every page of the inheriting sections then has no running head.

This counts the population, so the finding can be quoted as a reach rather than as one document.

    table-only-header-census.py /workspace/sample-files/words

**What it can and cannot see.** It reads the OOXML parts, so it counts DOCX only. The words track
is 200 documents of which **66 are `.doc`**, whose header stories live in the WW8 text stream where
no zip-level census can look. Round 42 predicted 18–30 renderings and measured 37 for exactly this
reason: 11 of its 37 were `.doc`. Read the number below as "of the 134 the census can read".

It is also a **ceiling**, not a reach, for the ordinary reason: it counts what a package declares,
not what a page resolves to. A document is counted when *some* section inherits *some* header whose
part has no top-level paragraph; whether that section owns any page depends on the layout.
"""
from __future__ import annotations

import re
import sys
import zipfile
from pathlib import Path
from xml.etree import ElementTree

W = "{http://schemas.openxmlformats.org/wordprocessingml/2006/main}"
R = "{http://schemas.openxmlformats.org/officeDocument/2006/relationships}"
SECTPR = re.compile(r"<w:sectPr\b.*?</w:sectPr>", re.S)


def header_parts(zf: zipfile.ZipFile) -> dict[str, str]:
    """rId -> part name, for header relationships."""
    try:
        rels = zf.read("word/_rels/document.xml.rels").decode("utf8", "replace")
    except KeyError:
        return {}
    out = {}
    for rid, kind, target in re.findall(r'Id="([^"]+)"[^>]*Type="([^"]+)"[^>]*Target="([^"]+)"', rels):
        if kind.endswith("/header"):
            out[rid] = "word/" + target.lstrip("/")
    return out


def has_top_level_paragraph(data: bytes) -> bool | None:
    try:
        root = ElementTree.fromstring(data)
    except ElementTree.ParseError:
        return None
    return any(child.tag == f"{W}p" for child in root)


def refs(sectpr: str) -> dict[str, str]:
    return {t: rid for t, rid in
            re.findall(r'<w:headerReference[^>]*w:type="(\w+)"[^>]*r:id="(\w+)"', sectpr)
            or re.findall(r'<w:headerReference[^>]*r:id="(\w+)"[^>]*w:type="(\w+)"', sectpr)}


def examine(path: Path):
    """(inheriting sections, of which from a table-only header, the parts involved)."""
    try:
        with zipfile.ZipFile(path) as zf:
            if "word/document.xml" not in zf.namelist():
                return None
            document = zf.read("word/document.xml").decode("utf8", "replace")
            parts = header_parts(zf)
            table_only = {}
            for rid, name in parts.items():
                try:
                    table_only[rid] = has_top_level_paragraph(zf.read(name)) is False
                except KeyError:
                    table_only[rid] = False
    except (zipfile.BadZipFile, OSError):
        return None

    effective = None          # the rId a section's default header resolves to
    inheriting = affected = 0
    for sectpr in SECTPR.findall(document):
        own = refs(sectpr).get("default")
        if own:
            effective = own
            continue
        if effective is not None:
            inheriting += 1
            if table_only.get(effective):
                affected += 1
    return inheriting, affected


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else "/workspace/sample-files/words")
    docs = sorted(p for p in root.rglob("*") if p.suffix.lower() in (".docx", ".docm", ".dotx"))
    binary = sum(1 for p in root.rglob("*") if p.suffix.lower() in (".doc", ".dot"))
    hits = []
    inheriting_any = 0
    for path in docs:
        result = examine(path)
        if not result:
            continue
        inheriting, affected = result
        if inheriting:
            inheriting_any += 1
        if affected:
            hits.append((affected, path))

    print(f"DOCX read                              {len(docs)}")
    print(f".doc in the track, invisible here      {binary}")
    print(f"with a section inheriting a header     {inheriting_any}")
    print(f"…inheriting a *table-only* header      {len(hits)}")
    for affected, path in sorted(hits, key=lambda h: -h[0]):
        print(f"    {affected:3d} sections   {path.relative_to(root)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
