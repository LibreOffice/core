#!/usr/bin/env python3
"""Census: which w:* run-level elements the words corpus's DOCX actually contain."""
import sys, os, zipfile, re, collections

ROOT = "/workspace/sample-files/words"
TARGETS = ["sym", "noBreakHyphen", "softHyphen", "ptab", "separator", "continuationSeparator",
           "lnNumType", "ruby", "object"]

docs = collections.defaultdict(dict)
n_ooxml = 0
allfiles = []
for dp, _, fns in os.walk(ROOT):
    for fn in fns:
        allfiles.append(os.path.join(dp, fn))
if True:
    for p in sorted(allfiles):
        f = os.path.basename(p)
        if not os.path.isfile(p): continue
        try:
            z = zipfile.ZipFile(p)
        except Exception:
            continue
        n_ooxml += 1
        blob = b""
        for n in z.namelist():
            if n.startswith("word/") and n.endswith(".xml"):
                try: blob += z.read(n)
                except Exception: pass
        for t in TARGETS:
            c = len(re.findall((r'<w:%s[ />]' % t).encode(), blob))
            if c: docs[f][t] = c

print(f"zip-container documents scanned: {n_ooxml}")
tot = collections.Counter(); dc = collections.Counter()
for f, m in docs.items():
    for t, c in m.items():
        tot[t] += c; dc[t] += 1
print(f"{'element':24s} {'docs':>5s} {'elements':>9s}")
for t in TARGETS:
    print(f"w:{t:22s} {dc[t]:5d} {tot[t]:9d}")
print()
for t in TARGETS:
    names = sorted(f for f, m in docs.items() if t in m)
    if names and dc[t] <= 20:
        print(f"w:{t}: " + ", ".join(f"{n}({docs[n][t]})" for n in names))
