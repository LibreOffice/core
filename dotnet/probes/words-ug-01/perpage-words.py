#!/usr/bin/env python3
"""Per-page gate-word counts for two PDFs, side by side."""
import subprocess, sys

def pages(pdf):
    n = int(subprocess.run(["pdfinfo", pdf], capture_output=True, text=True)
            .stdout.split("Pages:")[1].split()[0])
    out = []
    for p in range(1, n + 1):
        t = subprocess.run(["pdftotext", "-f", str(p), "-l", str(p), pdf, "-"],
                           capture_output=True).stdout.decode("utf-8", "replace").split()
        out.append((sum(1 for w in t if any(c.isalnum() for c in w)), len(t)))
    return out

ours, ref = sys.argv[1], sys.argv[2]
a, b = pages(ours), pages(ref)
assert a, "no pages from ours"
assert b, "no pages from ref"
print(f"pages: ours {len(a)}  ref {len(b)}")
print("page\tours\tref\tdelta\touRAW\trefRAW")
ta = tb = 0
for i in range(max(len(a), len(b))):
    x = a[i][0] if i < len(a) else 0
    y = b[i][0] if i < len(b) else 0
    xr = a[i][1] if i < len(a) else 0
    yr = b[i][1] if i < len(b) else 0
    ta += x; tb += y
    print(f"{i+1}\t{x}\t{y}\t{x-y:+d}\t{xr}\t{yr}")
print(f"TOT\t{ta}\t{tb}\t{ta-tb:+d}")
