#!/usr/bin/env python3
"""How many .rtf use a paragraph style whose name is one of Word's `heading N`
and whose \sbasedon does not resolve -- the case where 26.2.4.2 keeps Writer's
pool Heading 4 -> Heading chain (DocumentStylePoolManager.cxx:768-819)."""
import re, pathlib, collections, sys

CW = re.compile(r'\\[a-zA-Z]+-?\d*[ ]?')

def entries(d):
    i = d.find('{\\stylesheet')
    if i < 0: return []
    depth = 0; start = None; out = []
    for idx in range(i, len(d)):
        ch = d[idx]
        if ch == '{':
            depth += 1
            if depth == 2: start = idx
        elif ch == '}':
            if depth == 2 and start is not None: out.append(d[start:idx + 1]); start = None
            depth -= 1
            if depth == 0: break
    return out

def name_of(e):
    body = e[1:-1]
    # strip nested groups such as {\*\cs15 ...}
    body = re.sub(r'\{[^{}]*\}', '', body)
    last = None
    for m in CW.finditer(body): last = m
    nm = body[last.end():] if last else body
    return nm.strip().rstrip(';').strip()

docs = collections.Counter(); tot = collections.Counter()
files = 0; hit = set()
for p in sorted(pathlib.Path(sys.argv[1]).rglob('*.rtf')):
    files += 1
    d = p.read_bytes().decode('cp1252', 'replace')
    used = set(int(m.group(1)) for m in re.finditer(r'\\s(\d+)[\\ \n]', d))
    seen = set(); here = set()
    for e in entries(d):
        m = re.match(r'\{\\(\*\\)?(s|cs|ts)(\d+)?\b', e)
        kind = m.group(2) if m else 's'
        sid = int(m.group(3)) if m and m.group(3) else 0
        b = re.search(r'\\sbasedon(\d+)\b', e)
        ok = bool(b) and int(b.group(1)) != sid and int(b.group(1)) in seen
        seen.add(sid)
        if kind != 's' or ok or sid not in used: continue
        nm = name_of(e)
        if re.fullmatch(r'[Hh]eading [1-9]', nm):
            here.add(nm); tot[nm] += 1
    if here: hit.add(p.name)
    for n in here: docs[n] += 1
print(f"{files} files; {len(hit)} use a parentless `heading N` style")
for n, c in sorted(docs.items()): print(f"  {n}: {c} documents, {tot[n]} entries")
print()
for f in sorted(hit): print(' ', f)
