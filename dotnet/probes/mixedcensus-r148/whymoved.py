#!/usr/bin/env python3
"""For a document, diff the horizontal band segments of our BEFORE and AFTER renderings."""
import collections, glob, os, sys
import pymupdf
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import importlib.util
spec = importlib.util.spec_from_file_location('mc', os.path.join(os.path.dirname(os.path.abspath(__file__)), 'mixed-census.py'))
mc = importlib.util.module_from_spec(spec); spec.loader.exec_module(mc)

S = '/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad'
BEF = S + '/r146/after/pdf'
AFT = S + '/r147/after2/pdf'


def find(root, rel):
    d = os.path.join(root, rel.replace('/', '_'))
    g = glob.glob(os.path.join(d, '*.pdf'))
    return g[0] if len(g) == 1 else None


def bands(path):
    doc = pymupdf.open(path)
    out = []
    for p in range(doc.page_count):
        for top, x0, x1, th in mc.segs(doc[p]):
            out.append((p, round(top, 3), round(x0, 3), round(x1, 3), round(th, 3)))
    doc.close()
    return out


for rel in [l.rstrip('\n') for l in open(sys.argv[1])]:
    a, b = find(BEF, rel), find(AFT, rel)
    if not a or not b:
        print('MISSING', rel); continue
    A, B = bands(a), bands(b)
    ca, cb = collections.Counter(A), collections.Counter(B)
    gone = sorted((ca - cb).elements())
    new = sorted((cb - ca).elements())
    print('%s\n   horizontal bands before=%d after=%d   changed: -%d +%d' % (rel, len(A), len(B), len(gone), len(new)))
    # pair them up by (page, x0, x1, th)
    byk = collections.defaultdict(list)
    for t in new:
        byk[(t[0], t[2], t[3], t[4])].append(t[1])
    shifts = collections.Counter()
    unpaired = 0
    for t in gone:
        k = (t[0], t[2], t[3], t[4])
        if byk.get(k):
            shifts[round(byk[k].pop() - t[1], 3)] += 1
        else:
            unpaired += 1
    print('   top-edge shifts:', dict(shifts.most_common(6)), ' unpaired gone:', unpaired,
          ' unpaired new:', sum(len(v) for v in byk.values()))
    for t in gone[:4]:
        print('      - p%d top=%.3f x=%.2f..%.2f th=%.3f' % t)
    for t in new[:4]:
        print('      + p%d top=%.3f x=%.2f..%.2f th=%.3f' % t)
