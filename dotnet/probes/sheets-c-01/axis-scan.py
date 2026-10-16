#!/usr/bin/env python3
"""The value-axis tick sequence on each page of a PDF, read out of `pdftotext -bbox`.

An axis is a run of at least four numeric words that share an edge -- a common baseline for a
horizontal axis, a common *right* edge for a vertical one, because value-axis labels are
right-aligned and their left edges differ with their digit count -- and whose values step
evenly along that run.

Reading the top label alone is not enough: an axis labelled every second tick tops out below
its own maximum, so the whole sequence is reported and the caller can see the step.
"""
import subprocess, sys, re, collections

def words(path, page):
    out = subprocess.run(['pdftotext','-bbox','-f',str(page),'-l',str(page),path,'-'],
                         capture_output=True).stdout.decode('utf-8','replace')
    return [(float(a),float(b),float(c),float(d),t) for a,b,c,d,t in
            re.findall(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">(.*?)</word>', out)]

def runs(ws):
    found=[]
    # (bucket key, ordering key) pairs: vertical axis keyed on the right edge, horizontal on the
    # baseline.
    for keyfn, ordfn in ((lambda w: round(w[2],1), lambda w: w[1]),
                         (lambda w: round(w[3],1), lambda w: w[0])):
        buckets=collections.defaultdict(list)
        for w in ws:
            if not re.fullmatch(r'-?\d+(\.\d+)?', w[4]): continue
            buckets[keyfn(w)].append((ordfn(w), float(w[4])))
        for v in buckets.values():
            if len(v)<4: continue
            v.sort()
            vals=[t for _,t in v]
            for cand in (vals, vals[::-1]):
                steps={round(cand[i+1]-cand[i],6) for i in range(len(cand)-1)}
                if len(steps)==1 and steps.copy().pop()>0:
                    found.append(cand); break
    return found

path=sys.argv[1]; pages=int(sys.argv[2])
for p in range(1,pages+1):
    seen=set()
    for a in runs(words(path,p)):
        k=(a[0],a[-1],len(a))
        if k in seen: continue
        seen.add(k)
        print(f"p{p}\t{a[0]:g}..{a[-1]:g} step {a[1]-a[0]:g}\t({len(a)} labels)")
