#!/usr/bin/env python3
"""Page-level alignment, robust to per-page header/footer lines.

For each reference page, reports the our-page its first and last MATCHED
content line land on, and the page-level delta.  Lines that match nothing
(headers, footers, page numbers) are skipped rather than counted.
"""
import re, subprocess, sys, difflib

def pages(pdf):
    txt = subprocess.run(["pdftotext", "-q", pdf, "-"], capture_output=True, text=True).stdout
    out = []
    for pg in txt.split("\f"):
        lines = [re.sub(r"\s+", " ", l).strip() for l in pg.splitlines()]
        out.append([l for l in lines if l])
    if out and not out[-1]:
        out.pop()
    return out

def flat(pp):
    seq, owner = [], []
    for i, pg in enumerate(pp, 1):
        for l in pg:
            seq.append(l); owner.append(i)
    return seq, owner

ref, ours = sys.argv[1], sys.argv[2]
rp, op = pages(ref), pages(ours)
rs, ro = flat(rp); os_, oo = flat(op)
print(f"ref pages={len(rp)} lines={len(rs)}   ours pages={len(op)} lines={len(os_)}")
sm = difflib.SequenceMatcher(None, rs, os_, autojunk=False)
m = {}
for a, b, n in sm.get_matching_blocks():
    for k in range(n):
        m[a + k] = b + k
print(f"{'refpg':>5} {'nref':>5} {'nours':>5} | {'firstmatch->ourpg':>17} {'lastmatch->ourpg':>16} {'dpg':>4}  head")
prevd = 0
for p in range(1, len(rp) + 1):
    idx = [i for i in range(len(rs)) if ro[i] == p and i in m]
    if not idx:
        print(f"{p:5d} {len(rp[p-1]):5d} {'-':>5} |  (no matched line)"); continue
    f_, l_ = oo[m[idx[0]]], oo[m[idx[-1]]]
    d = f_ - p
    mark = "  <<<" if d != prevd else ""
    prevd = d
    print(f"{p:5d} {len(rp[p-1]):5d} {len(op[f_-1]):5d} | {f_:17d} {l_:16d} {d:4d}{mark}  {rp[p-1][0][:44]}")
