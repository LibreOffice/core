#!/usr/bin/env python3
"""Embedded-face count against the reference's, before and after.

The gate's third check counts the faces each PDF embeds. It is independent of both the
word count and the ink figure, and it is the one a symbol-bullet fix should move: drawing
U+2022 out of the body face embeds no OpenSymbol, and drawing the recoded glyph does.
"""
import os, sys

S = "/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad"

def fonts(name):
    out = {}
    for line in open(os.path.join(S, name, "rows.tsv"), encoding="utf-8"):
        f = line.rstrip("\n").split("\t")
        if len(f) < 7 or "/" not in f[4]:
            continue
        ours, ref = f[4].split("/")
        try:
            out[f[0]] = (int(ours), int(ref))
        except ValueError:
            pass
    return out

a, b = fonts(sys.argv[1]), fonts(sys.argv[2])
common = [p for p in a if p in b]

exact_a = sum(1 for p in common if a[p][0] == a[p][1])
exact_b = sum(1 for p in common if b[p][0] == b[p][1])
err_a = sum(abs(a[p][0] - a[p][1]) for p in common)
err_b = sum(abs(b[p][0] - b[p][1]) for p in common)

print("documents compared: %d" % len(common))
print("%-40s %8s %8s" % ("", "base", "after"))
print("%-40s %8d %8d" % ("embed exactly the reference's count", exact_a, exact_b))
print("%-40s %8d %8d" % ("total absolute face-count error", err_a, err_b))
print()

closer = [p for p in common if abs(b[p][0]-b[p][1]) < abs(a[p][0]-a[p][1])]
further = [p for p in common if abs(b[p][0]-b[p][1]) > abs(a[p][0]-a[p][1])]
print("closer to the reference: %d   further: %d" % (len(closer), len(further)))
for p in further:
    print("  FURTHER  %-52s %d/%d -> %d/%d"
          % (os.path.basename(p)[:52], a[p][0], a[p][1], b[p][0], b[p][1]))
