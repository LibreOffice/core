#!/usr/bin/env python3
"""Census the corpus for the (family-name, family-class) pairs that make
LibreOffice's fontconfig pre-match answer differently for the measuring font
than for the drawing font.

The measuring font carries the OOXML pitchFamily's family class, which
vcl/unx/generic/font/fontconfig.cxx:1076-1087 turns into an extra 'serif' or
'sans' FC_FAMILY entry; the drawing font, rebuilt from a drawinglayer
FontAttribute, has no family class at all.  So the split happens exactly when

    fc-match "<name>"          !=  fc-match "<name>,<generic>"

usage: reach-census.py <corpus-root>
"""
import sys, os, re, zipfile, subprocess, collections, json

root = sys.argv[1]
FAM = {1: "serif", 2: "sans"}          # vcl maps ROMAN and SWISS only
PITCH_FIXED = 1

_cache = {}
def fcmatch(pattern):
    if pattern not in _cache:
        out = subprocess.run(["fc-match", "-f", "%{file}", pattern],
                             capture_output=True, text=True).stdout.strip()
        _cache[pattern] = out
    return _cache[pattern]

def splits(name, generic):
    """True when adding the generic family changes fontconfig's answer."""
    for style in ("", ":bold"):
        if fcmatch(name + style) != fcmatch(f"{name},{generic}" + style):
            return True
    return False

rx = re.compile(rb'<a:(?:latin|cs|ea)\s[^>]*typeface="([^"]*)"[^>]*?pitchFamily="(\d+)"')
rx2 = re.compile(rb'<a:(?:latin|cs|ea)\s[^>]*pitchFamily="(\d+)"[^>]*?typeface="([^"]*)"')

docs = collections.defaultdict(set)      # track -> set(paths that split)
pairs = collections.Counter()            # (name, generic) -> documents
scanned = 0
for dirpath, _, files in os.walk(root):
    for fn in sorted(files):
        p = os.path.join(dirpath, fn)
        if not zipfile.is_zipfile(p):
            continue
        rel = os.path.relpath(p, root)
        track = rel.split(os.sep)[0]
        scanned += 1
        hit = set()
        try:
            with zipfile.ZipFile(p) as z:
                for n in z.namelist():
                    if not n.endswith(".xml"):
                        continue
                    try:
                        data = z.read(n)
                    except Exception:
                        continue
                    if b"pitchFamily" not in data:
                        continue
                    for m in rx.finditer(data):
                        hit.add((m.group(1).decode("utf-8", "replace"), int(m.group(2))))
                    for m in rx2.finditer(data):
                        hit.add((m.group(2).decode("utf-8", "replace"), int(m.group(1))))
        except Exception:
            continue
        for name, pf in hit:
            if not name or name.startswith("+"):
                continue
            fam, pitch = (pf >> 4) & 0xF, pf & 0xF
            if pitch == PITCH_FIXED:      # monospaced survives the round trip
                continue
            generic = FAM.get(fam)
            if generic and splits(name, generic):
                docs[track].add(rel)
                pairs[(name, generic)] += 1

print(f"zip documents scanned: {scanned}")
tot = 0
for t in sorted(docs):
    print(f"  {t:8s} {len(docs[t]):4d}")
    tot += len(docs[t])
print(f"  {'TOTAL':8s} {tot:4d}")
print("\nfamily names that split, by document count:")
for (name, generic), n in pairs.most_common(40):
    print(f"  {n:4d}  {name!r} + {generic}   {os.path.basename(fcmatch(name))} -> "
          f"{os.path.basename(fcmatch(name + ',' + generic))}")
json.dump({t: sorted(v) for t, v in docs.items()},
          open(sys.argv[2], "w") if len(sys.argv) > 2 else open(os.devnull, "w"), indent=1)
