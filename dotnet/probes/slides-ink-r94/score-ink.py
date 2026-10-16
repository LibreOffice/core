#!/usr/bin/env python3
"""Sum |ink|% per document for two of our renderings against one banked reference.

    score-ink.py <movers.txt> <ref-dir> <a-dir> <b-dir> [jobs]

Prints  id  pagesA/pagesB/pagesRef  inkA  inkB  delta.

The row parser requires a leading integer page number and five further numeric columns,
so the tool's own trailing "N pages, M with major differences" summary cannot be eaten
as a data row -- the defect that inverted a stored aggregate two rounds running.
"""
import re, subprocess, sys, pathlib, concurrent.futures as cf

# The worktree is a sparse checkout with no `.claude`; the skills live in the primary one.
S = pathlib.Path('/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py')
ROW = re.compile(r'^(\d+)\t([\d.]+)\t(-?[\d.]+)\t([\d.]+)\t(\d+)\t(\S+)')


def ink(ours, ref):
    """(pages, summed |ink|%) or None when the pair cannot be compared."""
    r = subprocess.run([sys.executable, str(S), str(ours), str(ref)],
                       capture_output=True, text=True, timeout=900)
    total, pages = 0.0, 0
    for line in r.stdout.splitlines():
        m = ROW.match(line)
        if not m:
            continue
        pages += 1
        total += float(m.group(4))
    if pages == 0:
        return None
    return pages, total


def main():
    ids = [x.strip() for x in pathlib.Path(sys.argv[1]).read_text().split('\n') if x.strip()]
    ref, a, b = (pathlib.Path(p) for p in sys.argv[2:5])
    jobs = int(sys.argv[5]) if len(sys.argv) > 5 else 4

    def one(i):
        return i, ink(a / f'{i}.pdf', ref / f'{i}.pdf'), ink(b / f'{i}.pdf', ref / f'{i}.pdf')

    rows = []
    with cf.ThreadPoolExecutor(jobs) as ex:
        for i, ra, rb in ex.map(one, ids):
            rows.append((i, ra, rb))

    sa = sb = 0.0
    print('id\tpages\tink_before\tink_after\tdelta')
    for i, ra, rb in sorted(rows):
        if ra is None or rb is None:
            print(f'{i}\t-\tUNSCOREABLE\t\t')
            continue
        sa += ra[1]; sb += rb[1]
        print(f'{i}\t{ra[0]}/{rb[0]}\t{ra[1]:.2f}\t{rb[1]:.2f}\t{rb[1]-ra[1]:+.2f}')
    print(f'TOTAL\t\t{sa:.2f}\t{sb:.2f}\t{sb-sa:+.2f}')


main()
