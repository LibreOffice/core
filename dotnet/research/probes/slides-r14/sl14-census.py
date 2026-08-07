#!/usr/bin/env python3
"""Census: which slides-track documents ask for a bullet from a recodeable symbol face.

pptx: <a:buFont typeface="Wingdings"/> beside <a:buChar char="..."/>
ppt : the face name appears in the font collection with the symbol charset; a raw
      UTF-16 name search is a cheap over-approximation, so it is reported separately.
"""
import os, re, sys, zipfile, collections

ROOT = "/workspace/sample-files/slides"

# The faces fontcvt.cxx's aStarSymbolRecodeTable names, after GetEnglishSearchFontName
# (lowercase, spaces removed).
RECODEABLE = {
    "starbats", "starmath", "symbol", "standardsymbols", "standardsymbolsl",
    "monotypesorts", "zapfdingbats", "itczapfdingbats", "dingbats",
    "webdings", "wingdings", "wingdings2", "wingdings3", "mtextra",
}

def norm(name):
    return re.sub(r"[^a-z0-9]", "", (name or "").lower())

pptx_hits = collections.Counter()
pptx_faces = collections.defaultdict(collections.Counter)
ppt_hits = collections.Counter()
ppt_faces = collections.defaultdict(collections.Counter)

for dirpath, _, files in os.walk(ROOT):
    for fn in sorted(files):
        p = os.path.join(dirpath, fn)
        ext = fn.rsplit(".", 1)[-1].lower()
        if ext in ("pptx", "pptm", "potx", "ppsx"):
            try:
                z = zipfile.ZipFile(p)
            except Exception:
                continue
            for n in z.namelist():
                if not (n.endswith(".xml") and ("slide" in n or "Master" in n or "theme" in n)):
                    continue
                try:
                    b = z.read(n).decode("utf-8", "replace")
                except Exception:
                    continue
                for m in re.finditer(r'<a:buFont[^>]*typeface="([^"]*)"', b):
                    if norm(m.group(1)) in RECODEABLE:
                        pptx_hits[fn] += 1
                        pptx_faces[fn][m.group(1)] += 1
            z.close()
        elif ext in ("ppt", "pot", "pps"):
            try:
                d = open(p, "rb").read()
            except Exception:
                continue
            for m in re.finditer(rb"(?:[\x20-\x7e]\x00){3,40}", d):
                s = m.group().decode("utf-16-le", "ignore").strip()
                if norm(s) in RECODEABLE:
                    ppt_hits[fn] += 1
                    ppt_faces[fn][s] += 1

print("=== pptx: a:buFont naming a recodeable symbol face ===")
for fn, c in pptx_hits.most_common():
    print("%5d  %-62s %s" % (c, fn[:62], dict(pptx_faces[fn])))
print("  documents: %d" % len(pptx_hits))

print()
print("=== ppt: recodeable symbol face named anywhere in the file ===")
for fn, c in ppt_hits.most_common():
    print("%5d  %-62s %s" % (c, fn[:62], dict(ppt_faces[fn])))
print("  documents: %d" % len(ppt_hits))
