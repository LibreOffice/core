#!/usr/bin/env python3
"""Which of our renderings moved between two sweeps, and by how much.

Byte-compares the two halves with the PDF's own date stamps masked — `/CreationDate`,
`/ModDate` and the XMP `xmp:CreateDate`/`xmp:ModifyDate`, which carry the wall clock
and differ between any two runs — and reports the documents that really differ
together with their page and alphanumeric counts on each side.

    movers.py BEFORE_DIR AFTER_DIR
"""
import re
import subprocess
import sys
from pathlib import Path

DATE = re.compile(rb"(/CreationDate|/ModDate)\s*\([^)]*\)|"
                  rb"<xmp:(?:CreateDate|ModifyDate)>[^<]*</xmp:[^>]*>")


def masked(path: Path) -> bytes:
    return DATE.sub(b"", path.read_bytes())


def counts(path: Path):
    info = subprocess.run(["pdfinfo", str(path)], capture_output=True, text=True).stdout
    pages = next((l.split()[1] for l in info.splitlines() if l.startswith("Pages")), "-")
    text = subprocess.run(["pdftotext", str(path), "-"],
                          capture_output=True).stdout.decode("utf-8", "replace")
    return pages, sum(1 for c in text if c.isalnum())


def main():
    before, after = Path(sys.argv[1]), Path(sys.argv[2])
    names = sorted({p.name for p in before.glob("*.pdf")} |
                   {p.name for p in after.glob("*.pdf")})
    moved = []
    identical = 0
    for name in names:
        a, b = before / name, after / name
        if not a.is_file() or not b.is_file():
            moved.append((name, "missing on one side", "", ""))
            continue
        if masked(a) == masked(b):
            identical += 1
            continue
        moved.append((name, "differs", counts(a), counts(b)))

    print(f"{len(names)} documents; {identical} byte-identical; {len(moved)} moved")
    for name, why, a, b in moved:
        print(f"  {name}\t{why}\tbefore {a}\tafter {b}")


if __name__ == "__main__":
    main()
