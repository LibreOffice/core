#!/usr/bin/env python3
"""Apply batch-check.sh's verdict rule to a bank of rendered pairs.

Reference bytes come from a bank; ours may come from a second directory, so a change
confined to `dotnet/src` can be scored without re-rendering the reference half.
Usage: score.py <ours-dir> <ref-dir> <ext> > rows.tsv
"""
import sys, pathlib, subprocess, concurrent.futures as cf

OURS, REF, EXT = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2]), sys.argv[3]

def counts(p):
    try:
        t = subprocess.run(["pdftotext", str(p), "-"], capture_output=True, timeout=180).stdout.decode("utf-8", "replace")
    except Exception:
        return None
    toks = t.split()
    words = sum(1 for w in toks if any(c.isalnum() for c in w))
    glyphs = sum(1 for c in t if c.isalnum())
    try:
        info = subprocess.run(["pdfinfo", str(p)], capture_output=True, timeout=120).stdout.decode("utf-8", "replace")
        pages = int(next(l.split()[1] for l in info.splitlines() if l.startswith("Pages")))
    except Exception:
        return None
    fonts = subprocess.run(["pdffonts", str(p)], capture_output=True, timeout=120).stdout.decode("utf-8", "replace").splitlines()[2:]
    fonts = [l for l in fonts if l.strip()]
    unemb = sum(1 for l in fonts if len(l.split()) >= 8 and l.split()[-5] == "no")
    return pages, words, glyphs, len(fonts), unemb

def one(name):
    o, r = OURS / f"{name}__{EXT}.pdf", REF / f"{name}__{EXT}.pdf"
    a = counts(o) if o.exists() else None
    b = counts(r) if r.exists() else None
    if a is None and b is None: return (name, "-", "-", "-", "-", "-", "both-failed")
    if b is None: return (name, "-", "-", "-", "-", "-", "ref-failed")
    if a is None: return (name, "-", str(b[0]), "-", str(b[2]), "-", "ours-failed")
    v = []
    if a[0] != b[0]: v.append("pages")
    if b[2] > 0:
        d = abs(a[2] - b[2])
        if d > b[2] * 0.02 and d > 15: v.append("words")
    elif a[2] > 15: v.append("words")
    if a[4] != 0: v.append("unembedded")
    return (name, str(a[0]), str(b[0]), str(a[2]), str(b[2]), str(a[4]), ",".join(v) or "match")

names = sorted(p.name[: -len(f"__{EXT}.pdf")] for p in REF.glob(f"*__{EXT}.pdf"))
print("name\tours_pages\tref_pages\tours_glyphs\tref_glyphs\tunemb\tverdict")
with cf.ThreadPoolExecutor(max_workers=4) as ex:
    for row in ex.map(one, names):
        print("\t".join(row))
