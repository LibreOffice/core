#!/usr/bin/env python3
"""Read one tab-probe rendering: per row, where the text is and what rule cover it carries.

    read-tabprobe.py <pdf> <manifest.tsv> <out.tsv>

Rows are found by their own drawn label (`L000` ... or the Mono group's `MMMMMMMMMM`), so a
page break or a reordering cannot mis-pair them.  Rule cover is measured on BOTH shapes -- a
fill and a stroke -- because C16 says a census that looks for one scores the other side's rules
as absent.
"""
import sys
import pymupdf

TOL_Y = 3.0


def rules(page):
    out = []
    for d in page.get_drawings():
        k = d.get('type', '')
        for it in d['items']:
            if it[0] == 're':
                r = it[1]
                if r.height <= 3.0 and r.width >= 0.5 and k in ('f', 'fs', 's'):
                    out.append((r.x0, r.x1, (r.y0 + r.y1) / 2))
            elif it[0] == 'l' and k in ('s', 'fs'):
                a, b = it[1], it[2]
                if abs(a.y - b.y) <= 3.0 and abs(a.x - b.x) >= 0.5:
                    out.append((min(a.x, b.x), max(a.x, b.x), (a.y + b.y) / 2))
    return out


def merge(spans):
    spans = sorted(spans)
    out = []
    for a, b in spans:
        if out and a <= out[-1][1] + 0.5:
            out[-1][1] = max(out[-1][1], b)
        else:
            out.append([a, b])
    return out


pdf, manifest, outpath = sys.argv[1:4]
rowinfo = {}
with open(manifest) as fh:
    head = fh.readline().rstrip('\n').split('\t')
    for line in fh:
        f = line.rstrip('\n').split('\t')
        rowinfo[f[1]] = dict(zip(head, f))

doc = pymupdf.open(pdf)
found = {}
for page in doc:
    rs = rules(page)
    for blk in page.get_text('dict')['blocks']:
        for line in blk.get('lines', []):
            spans = line['spans']
            if not spans:
                continue
            text = ''.join(s['text'] for s in spans)
            key = text.strip()
            label = None
            if key.startswith('MMMMMMMMMM'):
                label = 'MONO'
            elif key[:4] in rowinfo:
                label = key[:4]
            elif key == 'Q':
                label = 'CLIFF'
            elif key == 'Z':
                label = 'RESID'
            if label is None:
                continue
            x0 = min(s['bbox'][0] for s in spans)
            x1 = max(s['bbox'][2] for s in spans)
            base = line['bbox'][3]
            yc = (line['bbox'][1] + line['bbox'][3]) / 2
            # the 'R' after the tab: last span's own left edge
            rx = None
            for s in spans:
                if s['text'].strip().endswith('R'):
                    rx = s['bbox'][2]
            mine = merge([(a, b) for a, b, y in rs
                          if line["bbox"][1] - 1.5 <= y <= line["bbox"][3] + 3.0])
            found.setdefault(label, []).append(
                (page.number + 1, x0, x1, rx, base, mine))

# The Mono group shares a label, so pair its occurrences in document order with its manifest rows.
for grp, bucket in (('mono', 'MONO'), ('cliff', 'CLIFF'), ('resid', 'RESID')):
    keys = sorted((k for k, v in rowinfo.items() if v['group'] == grp),
                  key=lambda k: int(rowinfo[k]['row']))
    if bucket in found:
        for k, hit in zip(keys, found.pop(bucket)):
            found[k] = [hit]

with open(outpath, 'w') as fh:
    fh.write('label\tface\tsize\tkind\tgroup\ttabtwips\tpage\tx0\tx1\trx\tnrules\tcover\t'
             'contiguous\tspans\n')
    for label in sorted(rowinfo, key=lambda k: int(rowinfo[k]['row'])):
        info = rowinfo[label]
        hits = found.get(label)
        if not hits:
            fh.write('%s\t%s\t%s\t%s\t%s\t%s\tMISSING\n'
                     % (label, info['face'], info['size'], info['kind'], info['group'],
                        info['tabtwips']))
            continue
        page, x0, x1, rx, base, mine = hits[0]
        cover = sum(b - a for a, b in mine)
        contiguous = 1 if len(mine) == 1 else 0
        fh.write('%s\t%s\t%s\t%s\t%s\t%s\t%d\t%.2f\t%.2f\t%s\t%d\t%.2f\t%d\t%s\n'
                 % (label, info['face'], info['size'], info['kind'], info['group'],
                    info['tabtwips'], page, x0, x1,
                    '%.2f' % rx if rx else '', len(mine), cover, contiguous,
                    ';'.join('%.2f-%.2f' % (a, b) for a, b in mine)))
print('wrote', outpath)
