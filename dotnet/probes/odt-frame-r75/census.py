#!/usr/bin/env python3
"""Census of draw:frame growth conditions over the converted .odt corpus.

The rule established at the seat (xmloff/source/text/XMLTextFrameContext.cxx:997-1010,
:654-661): a frame's SizeType is MIN iff the *text box* alternative states
fo:min-height, whatever svg:height says.  svg:height and fo:min-height write the
same nHeight, the box's attributes first and the frame's second, so a frame stating
both keeps svg:height as the *floor* and still grows.
"""
import os, re, sys, zipfile
CORPUS = "/home/user/corpus-odf/words"
FRAME = re.compile(r'<draw:frame\b(.*?)(/>|>)', re.S)
BOX = re.compile(r'<draw:text-box\b([^>]*)>', re.S)

rows = []
for root, _d, files in os.walk(CORPUS):
    for n in sorted(files):
        if not n.lower().endswith(".odt"):
            continue
        p = os.path.join(root, n)
        try:
            c = zipfile.ZipFile(p).read("content.xml").decode("utf-8", "replace")
        except Exception:
            continue
        # Walk frames by locating each <draw:frame ...> and the box that follows it
        # before the matching close; cheap approximation: pair each frame open tag
        # with the next <draw:text-box> if it occurs before the next <draw:frame.
        opens = [m for m in re.finditer(r'<draw:frame\b', c)]
        stats = dict(frames=0, box=0, boxmin=0, boxmin_h=0, boxmin_noh=0, noh=0, split=0)
        for i, m in enumerate(opens):
            end = c.index('>', m.start())
            attrs = c[m.start():end]
            nxt = opens[i + 1].start() if i + 1 < len(opens) else len(c)
            bm = BOX.search(c, end, nxt)
            stats['frames'] += 1
            has_h = 'svg:height' in attrs
            if not has_h:
                stats['noh'] += 1
            if bm:
                stats['box'] += 1
                if 'fo:min-height' in bm.group(1):
                    stats['boxmin'] += 1
                    if has_h:
                        stats['boxmin_h'] += 1
                    else:
                        stats['boxmin_noh'] += 1
                        if 'may-break-between-pages="true"' in attrs:
                            stats['split'] += 1
        if stats['boxmin']:
            rows.append((n, stats))

print("documents with a growing (fo:min-height) frame:", len(rows))
for k in ('frames', 'box', 'boxmin', 'boxmin_h', 'boxmin_noh', 'noh', 'split'):
    print(f"  total {k}: {sum(r[1][k] for r in rows)}")
print()
print("top by growing frames:")
for r in sorted(rows, key=lambda r: -r[1]['boxmin'])[:20]:
    print("  ", r[0][:70], r[1])
