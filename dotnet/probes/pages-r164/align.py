#!/usr/bin/env python3
"""Align two PDF renderings page by page on their text lines.

Usage: align.py REF.pdf OURS.pdf [--max N]
Prints, for each reference page, the our-page its content starts on, and the
running line offset.  Settles 'first diverging page' and 'does it resynchronise'.
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

def main():
    ref, ours = sys.argv[1], sys.argv[2]
    rp, op = pages(ref), pages(ours)
    rs, ro = flat(rp)
    os_, oo = flat(op)
    print(f"ref pages={len(rp)} lines={len(rs)}   ours pages={len(op)} lines={len(os_)}")
    sm = difflib.SequenceMatcher(None, rs, os_, autojunk=False)
    # map ref global index -> our global index for matched lines
    m = {}
    for a, b, n in sm.get_matching_blocks():
        for k in range(n):
            m[a + k] = b + k
    # first index of each ref page
    start = {}
    for i, p in enumerate(ro):
        start.setdefault(p, i)
    print(f"{'refpg':>5} {'reflines':>8} {'refstart':>8} {'ourstart':>8} {'ourpg':>5} {'off':>5}")
    prev = None
    firstdiff = None
    for p in sorted(start):
        i = start[p]
        # walk forward to the first matched line on this page
        j = i
        while j < len(rs) and ro[j] == p and j not in m:
            j += 1
        if j >= len(rs) or ro[j] != p:
            print(f"{p:5d} {len(rp[p-1]):8d} {i:8d} {'-':>8} {'-':>5} {'?':>5}")
            continue
        oj = m[j]
        off = oj - j
        mark = ""
        if oo[oj] != p and firstdiff is None:
            firstdiff = p
        if prev is not None and off != prev:
            mark = f"  <<< offset {prev} -> {off}"
        prev = off
        print(f"{p:5d} {len(rp[p-1]):8d} {i:8d} {oj:8d} {oo[oj]:5d} {off:5d}{mark}")
    print("first ref page whose content starts on a different our-page:", firstdiff)

main()
