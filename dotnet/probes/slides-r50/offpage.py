#!/usr/bin/env python3
"""Words whose ink box falls outside the media box -- text we draw that no reader can see.

This is the *drawn* consequence of the wrap rule, not a declaration census: a wrap="none"
body whose text fits the page produces nothing here, which is most of them.
"""
import subprocess,re,sys,pathlib
d=pathlib.Path(sys.argv[1])
print("id\tpagew\toff_words\tworst_overhang\tsample")
for p in sorted(d.glob("*.pdf")):
    try:
        info=subprocess.run(["pdfinfo",str(p)],capture_output=True,text=True,timeout=60).stdout
        m=re.search(r"Page size:\s+([\d.]+) x ([\d.]+)",info)
        if not m: continue
        W=float(m.group(1))
        x=subprocess.run(["pdftotext","-bbox",str(p),"-"],capture_output=True,text=True,timeout=300).stdout
    except Exception: continue
    off=[];worst=0.0
    for w in re.finditer(r'<word xMin="([\d.-]+)" yMin="[\d.-]+" xMax="([\d.-]+)"[^>]*>([^<]*)</word>',x):
        xmax=float(w.group(2)); xmin=float(w.group(1))
        if xmax>W+0.5 or xmin<-0.5:
            off.append(w.group(3)); worst=max(worst,xmax-W,-xmin)
    if off:
        print(f"{p.stem}\t{W:.0f}\t{len(off)}\t{worst:.1f}\t{' '.join(off[:6])}")
