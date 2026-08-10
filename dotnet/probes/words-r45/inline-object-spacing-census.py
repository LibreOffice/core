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

**It resolves the style chain, and the first version did not.**  That version searched each
`w:p` for a `w:spacing` of its own and reported **1** carrier where rendering the track moved
**11**.  Seven of the shortfall state the spacing in `word/styles.xml` and inherit it — including
`gpp-pr-top-7-office-markets-4q-2023.docx`, whose `w:line="288"` sits in a style and whose
178 pt picture is the largest movement the change made.  The usual warning on this project is
that a grep over what a file *declares* overstates what it *draws*; this is the same mistake
pointing the other way, and it is the more dangerous one, because a prediction built on it is
low and a low prediction that comes true reads as a good prediction.
"""

from __future__ import annotations

import re
import sys
import zipfile
from pathlib import Path

PARA = re.compile(rb"<w:p[ >].*?</w:p>", re.S)
SPACING = re.compile(rb'<w:spacing[^>]*w:lineRule="auto"[^>]*/?>')
LINE = re.compile(rb'w:line="(\d+)"')
INLINE = re.compile(rb"<wp:inline[ >]|<w:object[ >]|<w:pict[ >]")
STYLE = re.compile(rb'<w:style [^>]*w:styleId="([^"]+)".*?</w:style>', re.S)
DEFAULT_PARA = re.compile(
    rb'<w:style [^>]*w:type="paragraph"[^>]*w:default="1"[^>]*w:styleId="([^"]+)"'
    rb'|<w:style [^>]*w:default="1"[^>]*w:type="paragraph"[^>]*w:styleId="([^"]+)"')
DOCDEFAULTS = re.compile(rb"<w:docDefaults>.*?</w:docDefaults>", re.S)
BASED = re.compile(rb'<w:basedOn w:val="([^"]+)"')
PSTYLE = re.compile(rb'<w:pStyle w:val="([^"]+)"')


def style_spacing(z: zipfile.ZipFile):
    """styleId -> proportional line spacing in 240ths, resolved through `w:basedOn`."""
    try:
        styles = z.read("word/styles.xml")
    except KeyError:
        return lambda _: None

    own: dict[bytes, int | None] = {}
    parent: dict[bytes, bytes | None] = {}
    for m in STYLE.finditer(styles):
        sid = m.group(1)
        based = BASED.search(m.group(0))
        parent[sid] = based.group(1) if based else None
        spacing = SPACING.search(m.group(0))
        line = LINE.search(spacing.group(0)) if spacing else None
        own[sid] = int(line.group(1)) if line else None

    # A paragraph naming no style takes the default paragraph style, and below that
    # `w:docDefaults`. Leaving those out is what kept five of the eleven documents this change
    # moved out of the count: their spacing is stated once, at the top, and never repeated.
    fallback = None
    m = DEFAULT_PARA.search(styles)
    if m:
        fallback = m.group(1) or m.group(2)
    defaults = DOCDEFAULTS.search(styles)
    doc_default = None
    if defaults:
        spacing = SPACING.search(defaults.group(0))
        line = LINE.search(spacing.group(0)) if spacing else None
        doc_default = int(line.group(1)) if line else None

    def resolve(sid, depth=0):
        if sid is None or depth > 12:
            return doc_default
        if own.get(sid) is not None:
            return own[sid]
        return resolve(parent.get(sid), depth + 1)

    return lambda sid: resolve(sid if sid is not None else fallback)


def carries(path: Path) -> tuple[bool, int]:
    try:
        with zipfile.ZipFile(path) as z:
            resolve = style_spacing(z)
            names = [n for n in z.namelist()
                     if n.startswith("word/") and n.endswith(".xml")]
            hits = 0
            for n in names:
                if n == "word/styles.xml":
                    continue
                body = z.read(n)
                for para in PARA.findall(body):
                    if not INLINE.search(para):
                        continue

                    # Direct formatting first, then the style it names — the order the reader
                    # resolves them in, and the half the first version of this census skipped.
                    line = None
                    spacing = SPACING.search(para)
                    if spacing:
                        found = LINE.search(spacing.group(0))
                        line = int(found.group(1)) if found else None
                    if line is None:
                        named = PSTYLE.search(para)
                        line = resolve(named.group(1) if named else None)

                    if line is not None and line > 240:
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
