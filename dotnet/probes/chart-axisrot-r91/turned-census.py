"""Count turned text lines and glyph-sized filled paths per document, both sides.

A category axis the reference turns 45 degrees leaves one of two traces: turned TEXT
(dir off the horizontal) when the chart was not sheared, or glyph-sized FILLED PATHS
when it was -- the shear rule. Counting only the first misses every sheared chart,
which is most of the interesting ones.
"""
import os, sys, glob, math, pymupdf, json
BANK = "/home/user/gate-orig-r83"
out = {}
paths = sorted(glob.glob(BANK + "/ours/*.pdf"))
shard = sys.argv[1]
if ":" in shard:
    k,n = (int(x) for x in shard.split(":"))
    paths = [p for j,p in enumerate(paths) if j % n == k]
else:
    paths = [p for p in paths if shard in os.path.basename(p)]
for i, op in enumerate(paths):
    name = os.path.basename(op)
    rp = os.path.join(BANK, "ref", name)
    if not os.path.exists(rp): continue
    row = {}
    for side, p in (("ours", op), ("ref", rp)):
        try: d = pymupdf.open(p)
        except Exception: row[side] = None; continue
        turned = 0; glyphs = 0
        for page in d:
            for b in page.get_text('dict')['blocks']:
                if b['type'] != 0: continue
                for l in b['lines']:
                    dy = l['dir'][1]
                    if abs(dy) > 0.05 and abs(abs(dy) - 1.0) > 0.05:
                        turned += 1
            for dr in page.get_drawings():
                if dr['fill'] is None: continue
                r = dr['rect']
                if 0.3 < r.width < 15 and 0.3 < r.height < 15: glyphs += 1
        row[side] = (turned, glyphs, d.page_count)
        d.close()
    out[name] = row
    if i % 50 == 0: print(f"  ...{i}/{len(paths)}", file=sys.stderr, flush=True)
json.dump(out, open(sys.argv[2] if len(sys.argv) > 2 else "/dev/stdout", "w"))
