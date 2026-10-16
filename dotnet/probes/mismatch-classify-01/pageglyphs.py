#!/usr/bin/env python3
"""Per-page alphanumeric-character counts for one banked rendering pair.

Prints ours and reference side by side so a page that exists on one side only,
or a page that is blank on one side, is visible without opening the page.
"""
import subprocess
import sys


def counts(pdf):
    n = int(subprocess.run(["pdfinfo", pdf], capture_output=True, text=True)
            .stdout.split("Pages:")[1].split()[0])
    out = []
    for p in range(1, n + 1):
        t = subprocess.run(["pdftotext", "-f", str(p), "-l", str(p), pdf, "-"],
                           capture_output=True, text=True).stdout
        out.append(sum(1 for c in t if c.isalnum()))
    return out


ident = sys.argv[1]
refdir = sys.argv[2] if len(sys.argv) > 2 else "/home/user/gate-2f47/ref"
o = counts(f"/home/user/gate-2f47/ours/{ident}.pdf")
r = counts(f"{refdir}/{ident}.pdf")
print(f"{'page':>5} {'ours':>8} {'ref':>8}")
for i in range(max(len(o), len(r))):
    a = o[i] if i < len(o) else None
    b = r[i] if i < len(r) else None
    mark = "" if a == b else "   <<<"
    print(f"{i+1:5d} {a if a is not None else '-':>8} {b if b is not None else '-':>8}{mark}")
