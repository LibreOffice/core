#!/usr/bin/env python3
r"""How many .rtf state a REF field, and how many occurrences each has.

Counted on the `{\*\fldinst … REF …}` group rather than on the string `REF `, which
matches PAGEREF, a heading called REFERENCES and any word ending in it.

  refcensus.py <corpus root>
"""
import pathlib
import re
import sys

FIELD = re.compile(r'fldinst[^}]*?\bREF\s', re.I)

rows = []
files = 0
for path in sorted(pathlib.Path(sys.argv[1]).rglob('*.rtf')):
    files += 1
    text = path.read_bytes().decode('cp1252', 'replace')
    hits = len([m for m in FIELD.finditer(text) if 'PAGEREF' not in m.group(0).upper()])
    if hits:
        rows.append((hits, len(re.findall(r'bkmkstart', text)), path.name))

rows.sort(reverse=True)
print(f"{files} files; {len(rows)} state a REF field")
for hits, marks, name in rows:
    print(f"  {hits:5d} REF  {marks:5d} bkmkstart  {name}")
