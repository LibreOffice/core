#!/usr/bin/env python3
"""How many .ppt empty paragraphs sit on a character run our Runs() loop never reaches?

PptTextBody.Runs walks the character runs accumulating `position`, records `atStart` when
`start >= position && start < runEnd`, and breaks when `position >= end`.  For an EMPTY
paragraph start == end, so the loop breaks at the end of the run that *ends* at `start` --
one run before the run that *contains* `start`.  atStart is therefore never found for any
empty paragraph other than one at text position 0, and the blank line falls back to the
master level's character height.

This counts the population that changes, and -- the part that decides whether it matters --
how many of those runs state a font height of their own.
"""
import glob, os, struct, sys, collections
import olefile
sys.path.insert(0, "/c/sandbox/workdir/wt-slides-r50/dotnet/probes/slides-r53")
import importlib.util
spec = importlib.util.spec_from_file_location("d", "/c/sandbox/workdir/wt-slides-r50/dotnet/probes/slides-r53/ppt-style-dump.py")
d = importlib.util.module_from_spec(spec); spec.loader.exec_module(d)

TEXT_HEADER_ATOM, TEXT_CHARS_ATOM, TEXT_BYTES_ATOM, STYLE_TEXT_PROP = 3999, 4000, 4008, 4001

docs = sorted(glob.glob("/c/sandbox/workdir/sample-files/slides/*/ppt/*.ppt"))
tot = collections.Counter()
perdoc = collections.Counter()
heights = collections.Counter()
for path in docs:
    try:
        ole = olefile.OleFileIO(path)
        data = ole.openstream("PowerPoint Document").read()
    except Exception as e:
        print("SKIP", os.path.basename(path), e); continue
    tot["documents"] += 1
    for body, stop in d.slides(data):
        text = None
        for ver, inst, rtype, b, e, depth in d.walk(data, body, stop):
            if rtype == TEXT_CHARS_ATOM:
                text = data[b:e].decode("utf-16-le", "replace")
            elif rtype == TEXT_BYTES_ATOM:
                text = data[b:e].decode("latin-1", "replace")
            elif rtype == STYLE_TEXT_PROP and text is not None:
                try:
                    paras, pos = d.read_paras(data[b:e], len(text))
                    chars = d.read_chars(data[b:e], pos, len(text))
                except Exception:
                    tot['unparsable style atoms'] += 1
                    continue
                if not chars:
                    continue
                # real paragraphs are the \r-delimited segments of the text
                segs, s = [], 0
                for i, ch in enumerate(text):
                    if ch == '\r':
                        segs.append((s, i)); s = i + 1
                segs.append((s, len(text)))
                # our reader drops one trailing empty paragraph
                if len(segs) > 1 and segs[-1][0] == segs[-1][1]:
                    segs = segs[:-1]
                # character run extents
                ext, p = [], 0
                for c in chars:
                    ext.append((p, p + max(c["count"], 0), c)); p += max(c["count"], 0)
                for (a, bnd) in segs:
                    if a != bnd:
                        continue
                    tot["empty paragraphs"] += 1
                    if a == 0:
                        tot["at position 0 (already found)"] += 1
                        continue
                    tot["atStart never found"] += 1
                    cover = next((c for (lo, hi, c) in ext if lo <= a < hi), None)
                    if cover is None:
                        tot["no covering run"] += 1
                        continue
                    if "fontHeight" in cover:
                        tot["covering run states a height"] += 1
                        heights[cover["fontHeight"]] += 1
                        perdoc[os.path.basename(path)] += 1
                    else:
                        tot["covering run states no height"] += 1
for k, v in tot.items():
    print(f"{v:7d}  {k}")
print("\nstated heights:", dict(sorted(heights.items())))
print(f"\ndocuments with at least one: {len(perdoc)}")
for k, v in perdoc.most_common(25):
    print(f"  {v:5d}  {k}")
