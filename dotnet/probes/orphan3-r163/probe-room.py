#!/usr/bin/env python3
"""Sweep the room left on page 1 and watch the reference's master part height.

after=40 (2 pt) throughout, filler fixed, bottom margin swept in 10-twip (0.5 pt) steps.
If LibreOffice charges the finished cell's space-after unconditionally the master part is
16.90 whenever it splits at all; if it drops that spacing when the room is tight, a band of
margins will show 14.90 instead.
"""
import os, re, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkdocx3 import write
from probe_common import rules, render

OUT = os.path.abspath("fx2"); os.makedirs(OUT, exist_ok=True)
BORDERS = ('<w:tblBorders>' + "".join(
    f'<w:{e} w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
    for e in ("top","left","bottom","right","insideH","insideV")) + '</w:tblBorders>')
def para(b,a,t):
    return (f'<w:p><w:pPr><w:spacing w:before="{b}" w:after="{a}" w:line="233" w:lineRule="auto"/>'
            f'</w:pPr><w:r><w:t xml:space="preserve">{t}</w:t></w:r></w:p>')
def doc(after, filler):
    pre = "".join(f'<w:p><w:pPr><w:spacing w:before="0" w:after="0"/></w:pPr>'
                  f'<w:r><w:t>Filler {i}</w:t></w:r></w:p>' for i in range(filler))
    tc = lambda w,p: f'<w:tc><w:tcPr><w:tcW w:w="{w}" w:type="dxa"/></w:tcPr>{p}</w:tc>'
    rows = ''.join('<w:tr>' + tc(1460, para(40, after, f"Anchor {i}"))
                   + tc(1109, para(40, after, "short")) + '</w:tr>' for i in range(2))
    rows += ('<w:tr>' + tc(1460, para(40, after, "Tag"))
             + tc(1109, para(40, after, "05/2010")) + '</w:tr>')
    return (pre + '<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/>' + BORDERS + '</w:tblPr>'
            '<w:tblGrid><w:gridCol w:w="1460"/><w:gridCol w:w="1109"/></w:tblGrid>'
            + rows + '</w:tbl><w:p/>')

print(f"{'mb(tw)':>7} {'bodyBot':>8} {'who':>5} {'prevRule':>9} {'lastRule':>9} {'masterH':>8} {'p1txt':>6} {'p2txt':>6} {'room':>7}")
import sys as _s
AFTER=int(_s.argv[1]); FILLER=int(_s.argv[2]); LO=int(_s.argv[3]); HI=int(_s.argv[4])
for mb in range(LO, HI, 10):
    src = os.path.join(OUT, f"mb{mb}.docx"); write(src, doc(AFTER, FILLER), mb=mb)
    line = []
    for who in ("ref","ours"):
        pdf = render(src, who, f"mb{mb}", OUT)
        if not pdf: line.append((who,"FAIL")); continue
        n = int(subprocess.run(["pdfinfo",pdf],capture_output=True,text=True).stdout.split("Pages:")[1].split()[0])
        r1,t1 = rules(pdf,1)
        r2,t2 = (rules(pdf,2) if n>=2 else ([],[]))
        bodybot = mb*72/1440
        prev = r1[-2] if len(r1)>=2 else None
        last = r1[-1] if r1 else None
        h = round(prev-last,2) if prev is not None else None
        room = round(prev-bodybot,2) if prev is not None else None
        line.append((who, prev, last, h, len(t1), len(t2), room, bodybot))
    for v in line:
        if v[1]=="FAIL": print(f"{mb:>7} {'':>8} {v[0]:>5}  FAIL"); continue
        who,prev,last,h,n1,n2,room,bb = v
        print(f"{mb:>7} {bb:8.2f} {who:>5} {prev:9.2f} {last:9.2f} {str(h):>8} {n1:>6} {n2:>6} {str(room):>7}")
    print()
