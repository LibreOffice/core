#!/usr/bin/env python3
"""What the rule PAINTS: the per-page left-edge shift, measured, over two groups.

For each document, pair spans by (page, y, text) exactly as odfpad-r142's
invoice-bands.py does -- an instrument checked on the witness at 71 of 71 with no
duplicate keys -- and report, per document, the modal per-page x offset and how many
of its printed pages carry a non-zero one.

The control group is 81 .ods sampled (seed 149) from the 226 that state NO
style:table-centering. It is what makes the treatment figure mean anything: a
left-edge offset that also shows up at the same rate on documents the rule cannot
reach is not this rule.
"""
import sys, glob, pathlib, collections, statistics, pymupdf

def spans(path):
    out = {}
    try: doc = pymupdf.open(path)
    except Exception: return None
    for pn, pg in enumerate(doc):
        for b in pg.get_text('dict')['blocks']:
            for l in b.get('lines', []):
                for s in l['spans']:
                    t = s['text'].strip()
                    if t:
                        out.setdefault((pn, round(s['bbox'][1], 1), t), []).append(s['bbox'][0])
    return out

def score(ours, ref):
    """-> (pages compared, pages with |modal dx| > 0.5, worst |modal dx|, matched, total)"""
    o, r = spans(ours), spans(ref)
    if o is None or r is None: return None
    per = collections.defaultdict(list)
    for k in set(o) & set(r):
        for a, b in zip(o[k], r[k]):
            per[k[0]].append(a - b)
    if not per: return None
    moved, worst, signs = 0, 0.0, set()
    for pn, d in per.items():
        m = statistics.median(d)
        if abs(m) > 0.5:
            moved += 1
            signs.add('left' if m < 0 else 'RIGHT')
        worst = max(worst, abs(m))
    matched = sum(len(v) for v in per.values())
    total = sum(len(v) for v in o.values())
    # Centring can only ever put the reference to the RIGHT of an un-centred
    # left-flush block, so every displaced page must read negative. A positive one
    # is some other cause and is counted separately rather than folded in.
    return len(per), moved, worst, matched, total, ','.join(sorted(signs)) or '-'

def run(listfile, out, label):
    rows = []
    for line in pathlib.Path(listfile).read_text().splitlines():
        stem = pathlib.Path(line).stem
        a, b = f'{out}/ours/{stem}.pdf', f'{out}/ref/{stem}.pdf'
        if not (pathlib.Path(a).exists() and pathlib.Path(b).exists()):
            rows.append((stem, 'unrendered', '', '', '', '', '')); continue
        s = score(a, b)
        if s is None: rows.append((stem, 'unpaired', '', '', '', '', '')); continue
        pages, moved, worst, matched, total, signs = s
        rows.append((stem, 'ok', pages, moved, f'{worst:.2f}', f'{matched}/{total}', signs))
    scored = [r for r in rows if r[1] == 'ok']
    movers = [r for r in scored if r[3] > 0]
    print(f'--- {label}: {len(rows)} listed, {len(scored)} scored, '
          f'{sum(1 for r in rows if r[1]=="unrendered")} unrendered, '
          f'{sum(1 for r in rows if r[1]=="unpaired")} unpaired')
    print(f'    documents with at least one displaced page: {len(movers)} of {len(scored)}')
    if movers:
        ws = sorted(float(r[4]) for r in movers)
        print(f'    worst |dx| median {statistics.median(ws):.2f} pt, max {ws[-1]:.2f} pt')
        left = sum(1 for r in movers if r[6] == 'left')
        print(f'    every displaced page reads LEFT (our block left of the reference): '
              f'{left} of {len(movers)}; mixed or rightward: {len(movers)-left}')
    return rows, len(scored), len(movers)

if __name__ == '__main__':
    out = sys.argv[1]
    a = run('list-states.txt', out, 'STATES style:table-centering')
    b = run('list-control.txt', out, 'CONTROL states none (n=81, seed 149)')
    with open('reach.tsv', 'w') as f:
        f.write('group\tdocument\tstatus\tpages\tpages_displaced\tworst_dx_pt\tspans_matched\tsign\n')
        for g, (rows, _, _) in (('states', a), ('control', b)):
            for r in rows:
                f.write(g + '\t' + '\t'.join(str(x) for x in r) + '\n')
