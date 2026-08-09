#!/usr/bin/env python3
"""How far apart are the *drawn* table row heights, over the whole words track?

A table row's height is not stated in a PDF; what is stated is the cell edges that bound
it. A vertical stroke inside a table is one cell's left or right edge and its length is
that cell's drawn height, so the multiset of vertical stroke lengths on a page is the
multiset of drawn row heights (merged rows included, borderless rows excluded).

Comparing multisets rather than pairing by position is deliberate: a row height error one
third of the way down a page displaces every rule under it, so a positional pairing would
report the cascade rather than the defect. Sorted lengths are translation invariant.

    row-height-census.py <ours-dir> <ref-dir> [out.tsv]

Reads the PDFs a `batch-check.sh` sweep has already written, so it renders nothing.

What it can and cannot see, stated because a census that does not say so has misled this
project twice:

  * It sees only rows whose cell edges are **stroked**. A borderless table states no rule
    and is invisible here, in both formats equally.
  * It is blind to any page beyond the shorter of the two renderings, and to documents
    whose page counts differ badly — the row on page 7 of one is not the row on page 7 of
    the other.
  * It counts a *drawn* difference, not a declared one, so it is a reach and not a ceiling.
    Unlike a markup census it reads `.doc` and `.docx` identically, because it reads
    neither.
"""
import os, re, subprocess, sys, collections

OPS = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                   '../../../.claude/skills/render-comparison/scripts/pdf-ops.py')
OPS = os.environ.get('PDF_OPS', os.path.normpath(OPS))

STROKE = re.compile(r'\s*stroke\s+p(\d+)\s+\(\s*([\d.-]+),\s*([\d.-]+)\)-\(\s*([\d.-]+),\s*([\d.-]+)\)')

MIN_LEN = 3.0        # shorter than this is a tick, a hairline or a curve segment
FLAT    = 0.6        # a stroke this thin in x is vertical
TOL     = 1.0        # a difference under this is rounding, not a row height


def verticals(pdf):
    """{page: sorted list of vertical stroke lengths}"""
    out = collections.defaultdict(list)
    txt = subprocess.run([OPS, 'dump', pdf], capture_output=True, text=True).stdout
    for line in txt.splitlines():
        m = STROKE.match(line)
        if not m:
            continue
        pg, x0, y0, x1, y1 = int(m.group(1)), *map(float, m.groups()[1:])
        if abs(x1 - x0) < FLAT and abs(y1 - y0) >= MIN_LEN:
            out[pg].append(round(abs(y1 - y0), 2))
    for pg in out:
        out[pg].sort()
    return out


def compare(ours, ref):
    """(paired, differing, max diff, sum of |diff|, unpairable pages)"""
    paired = differing = 0
    worst = 0.0
    total = 0.0
    unpairable = 0
    for pg in sorted(set(ours) & set(ref)):
        a, b = ours[pg], ref[pg]
        if len(a) != len(b):
            unpairable += 1
            continue
        for u, v in zip(a, b):
            paired += 1
            d = abs(u - v)
            total += d
            if d > TOL:
                differing += 1
                worst = max(worst, d)
    return paired, differing, worst, total, unpairable


def main():
    ours_dir, ref_dir = sys.argv[1], sys.argv[2]
    out = open(sys.argv[3], 'w') if len(sys.argv) > 3 else sys.stdout
    print('document\tpaired\tdiffering\tworst\tsum_abs\tunpairable_pages', file=out)
    docs = sorted(f for f in os.listdir(ours_dir) if f.endswith('.pdf'))
    n_any = n_ten = 0
    for f in docs:
        o, r = os.path.join(ours_dir, f), os.path.join(ref_dir, f)
        if not os.path.exists(r):
            continue
        paired, differing, worst, total, unp = compare(verticals(o), verticals(r))
        print(f'{f[:-4]}\t{paired}\t{differing}\t{worst:.2f}\t{total:.2f}\t{unp}', file=out)
        if differing:
            n_any += 1
        if worst >= 10:
            n_ten += 1
    print(f'# {len(docs)} documents; {n_any} with a paired row height differing by >{TOL} pt;'
          f' {n_ten} with one differing by >=10 pt', file=out)
    if out is not sys.stdout:
        out.close()


if __name__ == '__main__':
    main()
