#!/usr/bin/env python3
"""Count forward \sbasedon references: a stylesheet entry based on a style whose
own entry appears later in the same {\stylesheet}. getStyleName (rtfdocumentimpl.cxx:873-885)
answers "" for those, so lcl_findParentStyle (:275-298) drops the inheritance."""
import re, pathlib, sys

def stylesheet(d):
    i = d.find('{\\stylesheet')
    if i < 0: return []
    depth = 0; start = None; out = []
    for idx in range(i, len(d)):
        ch = d[idx]
        if ch == '{':
            depth += 1
            if depth == 2: start = idx
        elif ch == '}':
            if depth == 2 and start is not None:
                out.append(d[start:idx + 1]); start = None
            depth -= 1
            if depth == 0: break
    return out

def sid(e):
    m = re.search(r'\\(?:s|cs|ts)(\d+)\b', e)
    return int(m.group(1)) if m else 0

rows = []
for p in sorted(pathlib.Path(sys.argv[1]).rglob('*.rtf')):
    d = p.read_bytes().decode('cp1252', 'replace')
    ents = stylesheet(d)
    seen = set(); fwd = 0; tot = 0
    for e in ents:
        i = sid(e)
        m = re.search(r'\\sbasedon(\d+)\b', e)
        if m:
            tot += 1
            b = int(m.group(1))
            if b != i and b not in seen and b != 0: fwd += 1
        seen.add(i)
    rows.append((fwd, tot, len(ents), p.name))
n = sum(1 for r in rows if r[0])
print(f"{len(rows)} files, {n} with at least one forward \\sbasedon")
for r in sorted(rows, reverse=True)[:25]:
    print(f"{r[0]:5d} forward of {r[1]:5d} based-on, {r[2]:5d} entries  {r[3]}")
