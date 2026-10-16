#!/usr/bin/env python3
"""Classify the `.rtf` rows that fail only in that spelling.

    rtf-classify.py <rows.tsv> <ours-only-list> [corpus-root]

The four constructs round 71 closed are tested first, so that what is left is genuinely
unclassified; then the residual is bucketed by measurable properties of the row itself —
which way the page count runs, whether the text is short or long — and by control words the
reader can be checked against.
"""
import collections, re, sys
from pathlib import Path

ROWS, LIST = Path(sys.argv[1]), Path(sys.argv[2])
ROOT = Path(sys.argv[3] if len(sys.argv) > 3 else '/home/user/corpus-odf')

# Round 71's four, in the spellings its census used.
KNOWN = {
    'list-label': re.compile(rb'\\listtext(?![A-Za-z])'),
    'positioned-table': re.compile(rb'\\tpos[a-z]+(?![A-Za-z])'),
    'turned-cell': re.compile(rb'\\cltx(?:btlr|tbrl)(?![A-Za-z])'),
}

# Candidates for the residual, each a control word LibreOffice's RTF importer dispatches.
PROBES = {
    'sbknone': rb'\\sbknone(?![A-Za-z])',
    'sect': rb'\\sect(?![A-Za-z])',
    'columns': rb'\\cols\d',
    'keepn': rb'\\keepn(?![A-Za-z])',
    'trkeep': rb'\\trkeep(?![A-Za-z])',
    'shape': rb'\\shp(?![A-Za-z])',
    'pict': rb'\\pict(?![A-Za-z])',
    'objemb': rb'\\objemb(?![A-Za-z])',
    'footnote': rb'\\footnote(?![A-Za-z])',
    'trhdr': rb'\\trhdr(?![A-Za-z])',
    'nestrow': rb'\\nestrow(?![A-Za-z])',
    'trgaph': rb'\\trgaph\d',
    'clvmgf': rb'\\clvmgf(?![A-Za-z])',
    'linegrid': rb'\\linex\d',
    'field': rb'\\field(?![A-Za-z])',
}


def main():
    rows = {}
    for line in ROWS.read_text().split('\n'):
        if not line or line.startswith('#') or line.startswith('path\t'):
            continue
        f = line.split('\t')
        rows[f[0]] = f

    paths = [p for p in LIST.read_text().split('\n') if p.strip()]
    tally = collections.Counter()
    residual = []

    for rel in paths:
        data = (ROOT / rel).read_bytes()
        hit = [name for name, pat in KNOWN.items() if pat.search(data)]
        if hit:
            tally['+'.join(sorted(hit))] += 1
            continue
        f = rows[rel]
        op, rp = (int(x) if x.isdigit() else 0 for x in f[2].split('/'))
        og, rg = (int(x) if x.isdigit() else 0 for x in f[8].split('/'))
        residual.append((rel, op, rp, og, rg,
                         [k for k, pat in PROBES.items() if re.search(pat, data)]))
        tally['unclassified'] += 1

    print('# of the rows that pass in their original spelling and fail as .rtf')
    for k, n in tally.most_common():
        print(f'{n:4}  {k}')

    print()
    print('# the residual, by what its numbers say')
    print('pages\tglyphs\tpagedir\ttextdir\twords\tprobes\tpath')
    shape = collections.Counter()
    for rel, op, rp, og, rg, probes in sorted(residual, key=lambda r: -abs(r[1] - r[2])):
        pd = 'long' if op > rp else 'short' if op < rp else 'same'
        td = ('short' if rg and og < rg * 0.98 else
              'long' if rg and og > rg * 1.02 else 'same')
        shape[(pd, td)] += 1
        print(f'{op}/{rp}\t{og}/{rg}\t{pd}\t{td}\t{rows[rel][6]}\t'
              f'{",".join(probes)}\t{rel}')

    print()
    print('# residual by shape (page direction, text direction)')
    for (pd, td), n in shape.most_common():
        print(f'{n:4}  pages {pd:5} text {td}')


if __name__ == '__main__':
    main()
