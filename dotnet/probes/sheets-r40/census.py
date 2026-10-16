#!/usr/bin/env python3
"""How many sheets state a closed <col> run past their last data column.

The condition the print-area change keys on, counted over the formats a zip-level census can
read. Prints one row per document that has at least one such sheet, and a total.

    census.py /workspace/sample-files/sheets
"""
import re
import sys
import zipfile
from pathlib import Path

MAXCOL = 16383


def last_data_column(sheet: str) -> int:
    last = -1
    for m in re.finditer(r'<c r="([A-Z]+)\d+"', sheet):
        col = 0
        for ch in m.group(1):
            col = col * 26 + (ord(ch) - 64)
        if col - 1 > last:
            last = col - 1
    return last


def allocated(sheet: str) -> int:
    last = -1
    for m in re.finditer(r'<col ([^>]*)/>', sheet):
        a = m.group(1)
        lo = re.search(r'min="(\d+)"', a)
        hi = re.search(r'max="(\d+)"', a)
        if not lo:
            continue
        first = int(lo.group(1)) - 1
        end = (int(hi.group(1)) - 1) if hi else first
        reach = end if end < MAXCOL else first - 1
        last = max(last, reach)
    return last


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else "/workspace/sample-files/sheets")
    hits = sheets = docs = 0
    for path in sorted(root.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in (".xlsx", ".xlsm", ".xltx"):
            continue
        docs += 1
        try:
            z = zipfile.ZipFile(path)
        except Exception:
            continue
        wide = []
        for name in z.namelist():
            if not re.match(r"xl/worksheets/sheet\d+\.xml$", name):
                continue
            sheets += 1
            s = z.read(name).decode("utf8", "replace")
            d, a = last_data_column(s), allocated(s)
            if a > d:
                wide.append((name.split("/")[-1], d, a))
        if wide:
            hits += 1
            print(f"{path.name}: " + ", ".join(f"{n} data {d} allocated {a}" for n, d, a in wide))
    print(f"# {hits} of {docs} documents, over {sheets} sheets")
    return 0


if __name__ == "__main__":
    sys.exit(main())
