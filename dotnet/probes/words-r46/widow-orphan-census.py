#!/usr/bin/env python3
"""How much of the words track does the `w:pPrDefault` widow/orphan default reach?

The rule established by `widow-orphan-default.py` against the installed 24.2.7.2:

> A DOCX paragraph gets widow and orphan control of two lines when `word/styles.xml` carries a
> `w:docDefaults/w:pPrDefault` element — **empty or not** — unless the docDefaults' own `w:pPr`,
> the paragraph's style chain or the paragraph itself turns `w:widowControl` off.

    widow-orphan-census.py <corpus-root> [glob]

**What this census can see, and what it cannot.** It reads `word/styles.xml` out of the zip, so:

  * it is blind to the **66 `.doc`** of the 200, whose widow flag lives in `sprmPFWidowControl`
    in the WW8 stream. That half is *not* affected by this change at all — `Ww8LayoutFormat`
    already defaults `HasWidowControl` to true — so the blindness costs nothing here, which is
    the opposite of the usual situation and is why it is worth saying;
  * it counts documents whose paragraphs *would gain* the property, not documents whose
    pagination *changes*. A paragraph only pays for widow/orphan control when it straddles a page
    break with fewer than two lines on one side, which no census can see;
  * it cannot see a paragraph that never splits because it is one line long, which is most of
    them, nor one inside a table row that breaks by a different rule.

So the count below is an upper bound on the reach and a lower bound on nothing.
"""
from __future__ import annotations

import re
import sys
import zipfile
from pathlib import Path

W = "{http://schemas.openxmlformats.org/wordprocessingml/2006/main}"


def styles_of(path: Path) -> str | None:
    try:
        with zipfile.ZipFile(path) as z:
            return z.read("word/styles.xml").decode("utf8", "replace")
    except Exception:
        return None


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    root = Path(sys.argv[1])
    glob = sys.argv[2] if len(sys.argv) > 2 else "words/batch-*"

    docs = sorted(p for p in root.glob(glob + "/*/*") if p.is_file())
    docx = [p for p in docs if p.suffix.lower() in (".docx", ".docm")]
    other = [p for p in docs if p not in docx]

    has_ppr_default = []
    no_ppr_default = []
    dd_turns_off = []
    unreadable = []

    for p in docx:
        s = styles_of(p)
        if s is None:
            unreadable.append(p)
            continue
        m = re.search(r"<w:docDefaults\b.*?</w:docDefaults>", s, re.S)
        dd = m.group(0) if m else ""
        if "<w:pPrDefault" not in dd:
            no_ppr_default.append(p)
            continue
        ppr = re.search(r"<w:pPrDefault\b.*?(/>|</w:pPrDefault>)", dd, re.S)
        body = ppr.group(0) if ppr else ""
        off = re.search(r'<w:widowControl\s+w:val="(0|false|off)"', body)
        (dd_turns_off if off else has_ppr_default).append(p)

    print(f"documents in {glob}: {len(docs)}  docx {len(docx)}  other {len(other)}")
    print(f"  word/styles.xml carries w:docDefaults/w:pPrDefault : {len(has_ppr_default)}")
    print(f"    …and that pPrDefault turns widowControl off       : {len(dd_turns_off)}")
    print(f"  no w:pPrDefault (no default control, unchanged)     : {len(no_ppr_default)}")
    print(f"  styles.xml unreadable                               : {len(unreadable)}")
    print()
    print("not reached by this change, and why:")
    print(f"  {len(other)} binary/other documents — the WW8 reader already defaults the flag on")
    for p in no_ppr_default:
        print(f"  no pPrDefault: {p.relative_to(root)}")
    for p in dd_turns_off:
        print(f"  pPrDefault turns it off: {p.relative_to(root)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
