#!/usr/bin/env python3
"""How many words documents hold a paragraph whose whole content is an inline picture?

The population the picture-alone descent rule can reach at all. A ceiling, and a partial one.

    picture-only-paragraph-census.py <corpus-root> [glob]

**What it can see.** DOCX only, and only a `w:p` that carries a `w:drawing` or a `w:pict` and no
`w:t` with any non-blank text in it. That is exactly the "picture alone on its line" shape for the
OOXML reader, which emits an anchor character for the drawing and nothing else.

**What it cannot see, all of which are reached by the change:**

- the **66 `.doc`** of the 200, whose pictures live in the WW8 text stream behind a `chPicture`
  and in an `FSPA`. The change is format-neutral and reaches them;
- a picture alone on a line that is *not* alone in its paragraph — a paragraph long enough to wrap
  where the picture ends up on a line of its own;
- an inline picture inside a header, footer, text box or table cell, all of which are `w:p` too
  and are counted here, but whose effect on the page is a different question;
- and the direction that matters most: **whether the extra 2.6 pt ever changed a page break.**
  A logo alone above a heading is 2.6 pt too tall on every document that has one and moves
  nothing on nearly all of them.

So read this as "the shape occurs in N documents", never as "N documents will change".
"""
from __future__ import annotations

import re
import sys
import zipfile
from pathlib import Path


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    root = Path(sys.argv[1])
    glob = sys.argv[2] if len(sys.argv) > 2 else "words/batch-*"

    docs = sorted(p for p in root.glob(glob + "/*/*") if p.is_file())
    docx = [p for p in docs if p.suffix.lower() in (".docx", ".docm")]

    paragraph = re.compile(r"<w:p[ >].*?</w:p>", re.S)
    text = re.compile(r"<w:t[ >]([^<]*)<", re.S)

    carriers, total = [], 0
    for p in docx:
        try:
            with zipfile.ZipFile(p) as z:
                parts = [n for n in z.namelist()
                         if re.fullmatch(r"word/(document|header\d*|footer\d*)\.xml", n)]
                found = 0
                for n in parts:
                    body = z.read(n).decode("utf8", "replace")
                    for m in paragraph.finditer(body):
                        block = m.group(0)
                        if "<w:drawing" not in block and "<w:pict" not in block:
                            continue
                        if any(t.strip() for t in text.findall(block)):
                            continue
                        found += 1
        except Exception:
            continue
        if found:
            carriers.append((p, found))
            total += found

    print(f"documents in {glob}: {len(docs)}  docx read: {len(docx)}  "
          f"(the {len(docs) - len(docx)} binary ones are reached and not counted)")
    print(f"docx with a picture-only paragraph: {len(carriers)}, "
          f"{total} such paragraphs in all")
    for p, n in sorted(carriers, key=lambda r: -r[1])[:15]:
        print(f"  {n:>4}  {p.relative_to(root)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
