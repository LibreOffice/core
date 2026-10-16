#!/usr/bin/env python3
"""First page whose *word multiset* differs, ignoring draw order."""
import sys, pathlib, pymupdf
from collections import Counter

def pages(p):
    d = pymupdf.open(p); out=[]
    for pg in d:
        w = pg.get_text("words")
        out.append((Counter(x[4] for x in w), sorted(w, key=lambda x:(round(x[1],1), x[0])), pg.rect.height))
    return out

for name in sys.argv[1:]:
    try:
        a = pages(f"/home/user/gate-odf-r76/ours/{name}__rtf.pdf")
        b = pages(f"/home/user/gate-odf-r76/ref/{name}__rtf.pdf")
    except Exception as e:
        print(f"### {name}: {e}"); continue
    k = None
    for i in range(min(len(a),len(b))):
        if a[i][0] != b[i][0]: k = i; break
    print(f"### {name}  ours {len(a)}p ref {len(b)}p  first differing page {k+1 if k is not None else '-'}  H={a[0][2]:.1f}")
    if k is None: continue
    for tag, r in (("ours", a[k]), ("ref ", b[k])):
        ws = r[1]
        print(f"  {tag} {len(ws)}w  y {ws[0][1]:.2f}..{ws[-1][3]:.2f}  last: {' '.join(x[4] for x in ws[-8:])[:70]!r}")
    extra_ours = a[k][0] - b[k][0]; extra_ref = b[k][0] - a[k][0]
    print(f"  ours-only: {list(extra_ours.elements())[:12]}")
    print(f"  ref-only : {list(extra_ref.elements())[:12]}")
