#!/usr/bin/env python3
"""The ceiling for "a list label raises the line-spacing base height".

    label-spacing-census.py <corpus-root> [glob]

Round 46's `list-label-population.py` counted DOCX that resolve some paragraph to
proportional spacing above 100% *and* carry a numbering level stating its own `w:sz`, and
got 51 of 134. That was written for the rule round 46 believed — that a label behaves like
an as-character picture. The probes in this directory refute that: the label **does** raise
the base, so the population is every paragraph whose label is *taller than its item*, and a
label is taller for reasons `w:sz` cannot see.

So this prints two bands rather than one:

    outer   spacing > 100% anywhere in the chain, and the body numbers some paragraph.
            This is the true upper bound: any numbered paragraph can have a taller label,
            because a level in Symbol or Wingdings at the *same* point size still has a
            different line box from the item's Latin face.
    inner   the same, restricted to a level that states its own `w:sz`. Round 46's count,
            reproduced here so the two are read side by side.

**What neither can see**, said before the numbers:

- **the 66 `.doc`**, whose levels live in the WW8 `LSTF`/`LVLF` structures. The fix is in
  `XRay.Office.Text` and both readers build the same `PageLabel`, so the binary half is
  reachable and invisible to this script. Round 45 had to go through LibreOffice's own flat
  ODF export to see that half at all.
- **whether the label is really taller.** That needs the level's face and the item's
  resolved face and size, both through their style chains, and then the two line boxes.
- **whether a taller label on a spaced line moves any break.** A first line 2 pt taller
  moves a page break only when the page was within 2 pt of full.

Quote it as a ceiling, and measure the reach by rendering.
"""
from __future__ import annotations

import re
import sys
import zipfile
from pathlib import Path


def attr(element: str, name: str) -> str | None:
    m = re.search(rf'\bw:{name}="([^"]*)"', element)
    return m.group(1) if m else None


def spaced_above_hundred(xml: str) -> bool:
    """Any `w:spacing` stating proportional line spacing above single."""
    for element in re.findall(r"<w:spacing\b[^>]*/?>", xml):
        rule = attr(element, "lineRule") or "auto"
        line = attr(element, "line")
        if rule == "auto" and line and line.lstrip("-").isdigit() and int(line) > 240:
            return True
    return False


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    root = Path(sys.argv[1])
    glob = sys.argv[2] if len(sys.argv) > 2 else "words/batch-*"

    docs = sorted(p for p in root.glob(glob + "/*/*") if p.is_file())
    docx = [p for p in docs if p.suffix.lower() in (".docx", ".docm")]

    outer, inner = [], []
    for path in docx:
        try:
            with zipfile.ZipFile(path) as z:
                names = set(z.namelist())
                if "word/document.xml" not in names:
                    continue
                body = z.read("word/document.xml").decode("utf8", "replace")
                styles = (z.read("word/styles.xml").decode("utf8", "replace")
                          if "word/styles.xml" in names else "")
                numbering = (z.read("word/numbering.xml").decode("utf8", "replace")
                             if "word/numbering.xml" in names else "")
        except Exception:
            continue

        if not spaced_above_hundred(body + styles):
            continue
        if "<w:numPr" not in body and "<w:numPr" not in styles:
            continue
        outer.append(path)
        if re.search(r"<w:lvl\b.*?<w:sz\b", numbering, re.S):
            inner.append(path)

    print(f"documents in {glob}: {len(docs)}   docx read {len(docx)}   "
          f"binary and unread {len(docs) - len(docx)}")
    print(f"  outer ceiling — spacing > 100% and a numbered paragraph : {len(outer)}")
    print(f"  inner         — and a level stating its own w:sz        : {len(inner)}")
    print()
    for p in outer:
        print(f"  {'sz' if p in inner else '  '}  {p.relative_to(root)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
