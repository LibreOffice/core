#!/usr/bin/env python3
"""Corpus census for round 59's two OOXML chart changes, over all three tracks.

Counts what a chart part *states*, per axis, not what a shape resolves to -- so it over-counts
reach wherever a chart is never drawn (a hidden slide, a chart on a page neither stack renders)
and under-counts nothing.  Both numbers are reported: parts, and the documents holding them.
"""
import collections, os, re, sys, zipfile

AX = re.compile(r'<c:(catAx|valAx|dateAx|serAx)>(.*?)</c:\1>', re.S)
CHART = re.compile(r'^(?:ppt/charts|xl/charts|word/charts)/chart\d+\.xml$')


def scan(path):
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return None
    out = collections.Counter()
    for n in z.namelist():
        if not CHART.match(n):
            continue
        try:
            d = z.read(n).decode("utf-8", "replace")
        except Exception:
            continue
        out["parts"] += 1
        for match in AX.finditer(d):
            body = match.group(2)
            out["axes"] += 1
            m = re.search(r'<c:majorTickMark val="(\w+)"', body)
            out["tick:" + (m.group(1) if m else "(absent)")] += 1
            if re.search(r'<c:delete val="1"', body):
                out["deleted"] += 1
            for grid in ("majorGridlines", "minorGridlines"):
                g = re.search(r'<c:%s\s*(/>|>(.*?)</c:%s>)' % (grid, grid), body, re.S)
                if not g:
                    continue
                inner = g.group(2) or ""
                out[grid + (":stated" if "<a:ln" in inner else ":automatic")] += 1
            # the axis line itself
            sp = re.search(r'<c:spPr>(.*?)</c:spPr>', body, re.S)
            out["axisline:" + ("stated" if sp and "<a:ln" in sp.group(1) else "automatic")] += 1
    return out


if __name__ == "__main__":
    totals = collections.Counter()
    docs = collections.Counter()
    seen = set()
    perdoc = {}
    for root in sys.argv[1:]:
        for dirpath, _, names in os.walk(root):
            for n in sorted(names):
                if not n.lower().endswith((".pptx", ".xlsx", ".docx", ".xlsm", ".pptm", ".docm")):
                    continue
                key = os.path.join(os.path.basename(root), n).lower()
                if key in seen:
                    continue
                seen.add(key)
                c = scan(os.path.join(dirpath, n))
                if not c or not c["parts"]:
                    continue
                perdoc[os.path.join(dirpath, n)] = c
                totals.update(c)
                for k in c:
                    docs[k] += 1
    print("documents with a chart part:", docs["parts"])
    for k in sorted(totals):
        print(f"  {k:34s} {totals[k]:6d}   in {docs[k]:4d} documents")
    print()
    print("# documents whose every axis reserves a tick today but should not")
    for p, c in sorted(perdoc.items()):
        n = c["tick:none"] + c["tick:in"]
        if n:
            print(f"   {n:3d}/{c['axes']:3d}  {p}")
