#!/usr/bin/env python3
"""Rank documents on SHAPE disagreement rather than on ink.

    rank-shape.py <docs.tsv...> --mentions <m.tsv> --out <ranking.tsv>

Round 124's ranking summed |ink|%, and the two documents it turned out to want scored 0.71
and 1.67 because the frames it draws wrongly are pale.  Every column here is cheap, exact and
blind to how dark a thing is:

    dpages    |pages_ours - pages_ref|, relative
    dsize     pages whose two media boxes differ -- the row `pdf-image-diff.py` prints
              non-numerically and every ink-summing tool silently drops (r124 s5)
    dchars    relative disagreement in the text layer's character count
    ddraws    relative disagreement in the number of path constructions
    dimages   relative disagreement in the number of image placements

Each is normalised to [0,1] as |a-b| / max(a,b,1) so that no column's units decide the rank,
and the score is the WORST of them, not their sum -- a document wrong in one respect and
right in four is exactly what this is for, and a sum would bury it.

`fills` and `strokes` are carried in the table and deliberately NOT scored: C16 records that
this tree fills every text rule where 26.2.4.2 strokes it, so those two columns disagree
by construction on any document with an underline in it.
"""
import pathlib
import sys

args = sys.argv[1:]
mentions = args[args.index('--mentions') + 1]
out = args[args.index('--out') + 1]
tables = args[:args.index('--mentions')]

seen = {}
for line in pathlib.Path(mentions).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    seen[p[1]] = (int(p[0]), p[2], p[3], p[5], p[6])

rows = []
head = None
for t in tables:
    for i, line in enumerate(pathlib.Path(t).read_text(encoding='utf-8').splitlines()):
        p = line.split('\t')
        if i == 0:
            head = p
            continue
        rows.append(p)


def rel(a, b):
    a, b = float(a), float(b)
    return abs(a - b) / max(a, b, 1.0)


scored = []
for p in rows:
    if p[3] != 'ok':
        scored.append((p, None))
        continue
    po, pr = int(p[4]), int(p[5])
    cols = {
        'dpages': rel(po, pr),
        'dsize': int(p[6]) / max(po, pr, 1),
        'dchars': rel(p[7], p[8]),
        'ddraws': rel(p[9], p[10]),
        'dimages': rel(p[15], p[16]),
    }
    scored.append((p, cols))

scored.sort(key=lambda x: -(max(x[1].values()) if x[1] else -1))
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('\t'.join(head) + '\tprior\ttrack\tstatus\tkind\tworst\t'
             'dpages\tdsize\tdchars\tddraws\tdimages\n')
    for p, c in scored:
        n, ext, track, status, kind = seen.get(
            p[0].rsplit('__', 1)[0], (-1, '', '', '', ''))
        if c is None:
            fh.write('\t'.join(p) + f'\t{n}\t{track}\t{status}\t{kind}\tunscored\n')
            continue
        fh.write('\t'.join(p) + f'\t{n}\t{track}\t{status}\t{kind}\t'
                 f'{max(c.values()):.4f}\t' +
                 '\t'.join(f'{c[k]:.4f}' for k in
                           ('dpages', 'dsize', 'dchars', 'ddraws', 'dimages')) + '\n')
ok = [x for x in scored if x[1]]
print(f'{len(ok)} scored, {len(scored) - len(ok)} unscored; '
      f'{sum(1 for _, c in ok if max(c.values()) > 0)} disagree in at least one column')
