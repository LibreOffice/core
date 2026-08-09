#!/usr/bin/env python3
"""The same ceiling for the 66 `.doc` the zip census cannot read, via LibreOffice's own export.

A `.doc` states its inline picture as a `sprmCPicLocation` on a run in the WW8 text stream and
its line spacing as a `sprmPDyaLine`; no zip-level census can see either.  Rather than write a
second binary reader for a count, this asks the reference to do the reading: each document is
exported to flat ODF, and the paragraphs are then read out of XML exactly as the OOXML census
reads them out of `word/document.xml`.

A paragraph counts when it holds a `draw:frame` with `text:anchor-type="as-char"` **and** its
style chain resolves `fo:line-height` to a percentage above 100.  Same two syntactic conditions
as the OOXML census, and the same missing third one — whether the object is taller than the
text — so this is a ceiling too.

The one thing to hold onto: this measures LibreOffice's *import* of the document, not the
document.  Where the two disagree the import is what our own reader is being compared against
anyway, so for estimating reach it is the more useful of the two.
"""

from __future__ import annotations

import os
import re
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

STYLE = "{urn:oasis:names:tc:opendocument:xmlns:style:1.0}"
FO = "{urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0}"
TEXT = "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}"
DRAW = "{urn:oasis:names:tc:opendocument:xmlns:drawing:1.0}"


def line_heights(root):
    """style name -> proportional line height in per cent, resolved through the parent chain."""
    own, parent = {}, {}
    for style in root.iter(f"{STYLE}style"):
        name = style.get(f"{STYLE}name")
        if not name:
            continue
        parent[name] = style.get(f"{STYLE}parent-style-name")
        props = style.find(f"{STYLE}paragraph-properties")
        value = props.get(f"{FO}line-height") if props is not None else None
        if value and value.endswith("%"):
            own[name] = int(round(float(value[:-1])))

    def resolve(name, depth=0):
        if name is None or depth > 12:
            return 100
        if name in own:
            return own[name]
        return resolve(parent.get(name), depth + 1)

    return resolve


def carriers(fodt: Path) -> int:
    root = ET.parse(fodt).getroot()
    resolve = line_heights(root)
    hits = 0
    for para in root.iter(f"{TEXT}p"):
        frames = [f for f in para.iter(f"{DRAW}frame")
                  if f.get(f"{TEXT}anchor-type") == "as-char"]
        if not frames:
            continue
        if resolve(para.get(f"{TEXT}style-name")) > 100:
            hits += 1
    return hits


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else "/workspace/sample-files/words")
    out = Path(sys.argv[2]) if len(sys.argv) > 2 else Path(tempfile.mkdtemp())
    out.mkdir(parents=True, exist_ok=True)
    profile = out / "prof"
    docs = found = paras = failed = 0
    for path in sorted(root.rglob("*")):
        if not path.is_file() or path.suffix.lower() != ".doc":
            continue
        docs += 1
        target = out / (path.stem + ".fodt")
        if not target.exists():
            subprocess.run(
                ["soffice", "--headless", f"-env:UserInstallation=file://{profile}",
                 "--convert-to", "fodt", "--outdir", str(out), str(path)],
                capture_output=True, timeout=300, check=False)
        if not target.exists():
            failed += 1
            continue
        try:
            n = carriers(target)
        except ET.ParseError:
            failed += 1
            continue
        if n:
            found += 1
            paras += n
            print(f"{n:>4}  {path.relative_to(root)}", flush=True)
    print(f"\n{found} of {docs} .doc carry the shape, {paras} paragraphs; "
          f"{failed} could not be exported or parsed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
