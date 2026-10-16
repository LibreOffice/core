#!/usr/bin/env python3
"""How many corpus .rtf declare a \\deff that is not font 0, and whether the two faces differ."""
import re, pathlib, collections
root = pathlib.Path("/home/user/corpus-odf")
def fonttbl(s):
    i = s.find('{\\fonttbl')
    if i < 0: return {}
    # crude: split on {\fN...;}
    out = {}
    for m in re.finditer(r'\{\\f(\d+)[^;{}]*?(?:\{[^{}]*\}[^;{}]*)*?\s([^;{}]*);', s[i:i+8000]):
        out.setdefault(int(m.group(1)), m.group(2).strip())
    return out
n=0; nodeff=0; diff=0; same=0
rows=[]
for f in sorted(root.rglob("*.rtf")):
    s = f.read_bytes()[:20000].decode('latin-1')
    m = re.search(r'\\deff(\d+)', s)
    n += 1
    if not m: nodeff += 1; continue
    d = int(m.group(1)); ft = fonttbl(s)
    a, b = ft.get(0), ft.get(d)
    if a is None or b is None: continue
    if a != b: diff += 1; rows.append((f.stem, d, a, b))
    else: same += 1
print(f"rtf files {n}; no \\deff {nodeff}; \\deff names a different face from \\f0: {diff}; same face {same}")
for r in rows[:20]: print("   ", r)
