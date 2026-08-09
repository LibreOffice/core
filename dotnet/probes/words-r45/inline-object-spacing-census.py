#!/usr/bin/env python3
"""How many documents can the proportional-spacing-over-an-inline-object rule reach?

The rule only bites where all three hold on one paragraph:

  * its line spacing is **proportional and at or above 100%** — below that Writer scales the
    whole line and we already agreed;
  * it holds an **as-character object** (an inline picture, an inline shape, an inline OLE);
  * that object is **taller than the paragraph's text**, which is what makes the line height
    and the text height differ at all.

The first two are syntactic and this counts them.  The third is not: it needs the picture's
declared extent against the run's resolved font, and a picture smaller than the line changes
nothing.  So this is a **ceiling**, and the number to publish beside it is what actually moved
when the corpus was rendered.

**It reads OOXML only.**  A `.doc` states its inline picture as a `sprmCPicLocation` on a run
inside the WW8 text stream and its spacing as a `sprmPDyaLine`, neither of which a zip-level
census can see — so the count below is over 134 of the track's 200 documents, and the 66 `.doc`
are invisible to it.  That matters here more than usual: the document the rule was derived from
is one of the 66.
"""

from __future__ import annotations

import re
import sys
import zipfile
from pathlib import Path

PARA = re.compile(rb"<w:p[ >].*?</w:p>", re.S)
SPACING = re.compile(rb'<w:spacing[^>]*w:lineRule="auto"[^>]*>')
LINE = re.compile(rb'w:line="(\d+)"')
INLINE = re.compile(rb"<wp:inline[ >]")


def carries(path: Path) -> tuple[bool, int]:
    try:
        with zipfile.ZipFile(path) as z:
            names = [n for n in z.namelist()
                     if n.startswith("word/") and n.endswith(".xml")]
            hits = 0
            for n in names:
                body = z.read(n)
                for para in PARA.findall(body):
                    if not INLINE.search(para):
                        continue
                    m = SPACING.search(para)
                    if not m:
                        continue
                    line = LINE.search(m.group(0))
                    if line and int(line.group(1)) > 240:
                        hits += 1
            return hits > 0, hits
    except (zipfile.BadZipFile, OSError):
        return False, 0


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else "/workspace/sample-files/words")
    docs = ooxml = carriers = paras = 0
    for path in sorted(root.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in (".doc", ".docx"):
            continue
        docs += 1
        if path.suffix.lower() != ".docx":
            continue
        ooxml += 1
        hit, n = carries(path)
        if hit:
            carriers += 1
            paras += n
            print(f"{n:>4}  {path.relative_to(root)}")
    print(f"\n{carriers} of {ooxml} OOXML documents, {paras} paragraphs; "
          f"{docs - ooxml} .doc unreadable by this census, of {docs} in the track")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
