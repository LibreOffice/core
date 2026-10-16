#!/usr/bin/env python3
"""Build the round's ranking tables from the per-document ink scores.

    rank.py <per-doc.tsv> <mentions.tsv> <out-ranking.tsv>

Emits one banked ranking, sorted by summed |ink|%, carrying beside it the per-page figure (the
column a long document otherwise buries), the worst page and its hint, the gate's own track and
extension, and whether any earlier round's write-up has ever named the document -- so a page
three rounds have already opened can be told apart from one nobody has opened.
"""
import pathlib
import sys

doc, mentions, out = sys.argv[1:4]
seen = {}
for line in pathlib.Path(mentions).read_text(encoding='utf-8').splitlines()[1:]:
    p = line.split('\t')
    if len(p) >= 4:
        seen[p[1]] = (int(p[0]), p[3])

rows = []
head = None
for i, line in enumerate(pathlib.Path(doc).read_text(encoding='utf-8').splitlines()):
    p = line.split('\t')
    if i == 0:
        head = p
        continue
    if p[10] != 'ok':
        continue
    stem = p[0].rsplit('__', 1)[0]
    n, where = seen.get(stem, (0, ''))
    rows.append(p + [str(n), where])

rows.sort(key=lambda r: -float(r[5]))
with open(out, 'w', encoding='utf-8') as fh:
    fh.write('\t'.join(head + ['prior_mentions', 'prior_rounds']) + '\n')
    for r in rows:
        fh.write('\t'.join(r) + '\n')
print(f'{len(rows)} scored documents; total |ink|% {sum(float(r[5]) for r in rows):.2f}')
