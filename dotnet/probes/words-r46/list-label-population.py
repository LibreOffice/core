#!/usr/bin/env python3
"""How many words documents can the round-45 rule reach through a *list label*?

Round 45 established that proportional line spacing extends a line by (prop − 100)% of the line's
**text** height, and that an as-character object raises the line without taking a share. It left
this open in its own words: "the list-label population is unmeasured — three of the eleven
renderings that changed carry no inline object at all."

The mechanism is the same one. A list level states its own character formatting, Writer's label is
a `SwNumberPortion`, and `PortionType::Number` is *not* one of the portions that raise
`GetLineSpacingBaseHeight()` — so a numbering level taller than the item it labels makes the first
line taller and takes no share of the percentage, exactly as a picture does.

    list-label-population.py <corpus-root> [glob]

**What this counts.** A DOCX where some paragraph resolves, through its own `w:pPr` or its style
chain up through `w:docDefaults`, to a line spacing above 100%, *and* takes its numbering from a
`w:numbering` level whose `w:lvl/w:rPr/w:sz` is larger than the size that paragraph resolves to.
Both halves are needed: a taller label on a singly-spaced line changes nothing, and a spaced
paragraph with no label is round 45's other population.

**What it cannot see**, stated before the number rather than after it:

- the **66 `.doc`**, whose numbering lives in the `LSTF`/`LVLF` structures of the WW8 stream. Round
  45's own census had to go through LibreOffice's flat-ODF export to see that half at all, and the
  same is true here;
- a label whose height exceeds the item's for a reason other than `w:sz` — a different face at the
  same size, which happens whenever a level uses Symbol or Wingdings and the item does not;
- whether the taller label changes any *break*. This is a population, not a reach, and round 45's
  measured reach on the whole rule was 11 renderings against a 20-document ceiling.
"""
from __future__ import annotations

import re
import sys
import zipfile
from pathlib import Path

W = "{http://schemas.openxmlformats.org/wordprocessingml/2006/main}"


def attr(element: str, name: str) -> str | None:
    m = re.search(rf'\bw:{name}="([^"]*)"', element)
    return m.group(1) if m else None


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    root = Path(sys.argv[1])
    glob = sys.argv[2] if len(sys.argv) > 2 else "words/batch-*"

    docs = sorted(p for p in root.glob(glob + "/*/*") if p.is_file())
    docx = [p for p in docs if p.suffix.lower() in (".docx", ".docm")]

    spaced_and_labelled, spaced_only, labelled_only = [], [], []

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

        # Any proportional spacing above 100%, anywhere in the chain. `w:line` with `w:lineRule`
        # auto (or absent, which means auto) is 240 per single line.
        spaced = False
        for element in re.findall(r"<w:spacing\b[^>]*/?>", body + styles):
            rule = attr(element, "lineRule") or "auto"
            line = attr(element, "line")
            if rule == "auto" and line and line.isdigit() and int(line) > 240:
                spaced = True
                break

        # A numbering level that states a character size at all. Comparing it against the item's
        # resolved size needs the style chain per paragraph, so the looser test is used and the
        # count is quoted as the ceiling it is.
        labelled = bool(re.search(r"<w:lvl\b.*?<w:sz\b", numbering, re.S)) and "<w:numPr" in body

        if spaced and labelled:
            spaced_and_labelled.append(path)
        elif spaced:
            spaced_only.append(path)
        elif labelled:
            labelled_only.append(path)

    print(f"documents in {glob}: {len(docs)}   docx read {len(docx)}   "
          f"binary and unread {len(docs) - len(docx)}")
    print(f"  proportional spacing > 100% AND a sized numbering level : "
          f"{len(spaced_and_labelled)}")
    print(f"  proportional spacing > 100%, no sized level              : {len(spaced_only)}")
    print(f"  a sized numbering level, no spacing above 100%           : {len(labelled_only)}")
    print()
    print("the population the label half of round 45's rule can reach, named:")
    for p in spaced_and_labelled:
        print(f"  {p.relative_to(root)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
