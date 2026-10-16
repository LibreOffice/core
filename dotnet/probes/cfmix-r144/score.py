"""Score the movers three ways against 26.2.4.2: pages, alphanumeric characters, and summed
unsigned ink, at the round's base and after it.

The three legs were rendered on the same day and **without** `SOURCE_DATE_EPOCH`, deliberately:
several of these workbooks state `TODAY()`, which the reference recalculates on load and this
tree reads from the cache, so pinning our half alone would score a date difference as ink."""
import glob
import os
import subprocess
import sys

import pymupdf

W = '/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad/r144'
DIFF = ('/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/'
        'pdf-image-diff.py')


def only(directory):
    found = glob.glob(os.path.join(glob.escape(directory), '*.pdf'))
    return found[0] if found else None


def counts(path):
    document = pymupdf.open(path)
    text = ''.join(page.get_text() for page in document)
    return document.page_count, sum(1 for c in text if c.isalnum())


def ink(ours, reference):
    work = os.path.join(W, 'inkwork')
    subprocess.run(['rm', '-rf', work], check=False)
    os.makedirs(work, exist_ok=True)
    result = subprocess.run(
        ['python3', DIFF, ours, reference, '--outdir', work],
        capture_output=True, text=True, timeout=1800, check=False)
    # The columns are `page  diff%  ink%  |ink|%  regions  verdict`, so the summed unsigned ink
    # is index 3 and the MAJOR count is the verdict. Reading the wrong column is silent: every
    # row raises `ValueError`, every row is skipped, and the total comes out a plausible 0.00.
    total, major, pages = 0.0, 0, 0
    for line in result.stdout.splitlines():
        parts = line.split('\t')
        if len(parts) < 6 or not parts[0].strip().isdigit():
            continue
        try:
            total += abs(float(parts[3].strip().rstrip('%')))
        except ValueError:
            continue
        if parts[5].strip().upper() == 'MAJOR':
            major += 1
        pages += 1
    subprocess.run(['rm', '-rf', work], check=False)
    if pages == 0:
        raise SystemExit('pdf-image-diff produced no scored rows for %s' % ours)
    return total, major


def main():
    print('\t'.join(['document', 'pages_base', 'pages_after', 'pages_ref',
                     'alnum_base', 'alnum_after', 'alnum_ref',
                     'ink_base', 'ink_after', 'major_base', 'major_after']))
    sums = [0.0, 0.0]
    for line in open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'movers.txt')):
        rel = line.strip()
        if not rel:
            continue
        key = rel.replace('/', '_')
        base = only(os.path.join(W, 'ours-base', key))
        after = only(os.path.join(W, 'ours-after', key))
        reference = only(os.path.join(W, 'ref', key))
        if not (base and after and reference):
            print('%s\tMISSING' % rel, file=sys.stderr)
            continue
        pb, ab = counts(base)
        pa, aa = counts(after)
        pr, ar = counts(reference)
        ib, mb = ink(base, reference)
        ia, ma = ink(after, reference)
        sums[0] += ib
        sums[1] += ia
        print('%s\t%d\t%d\t%d\t%d\t%d\t%d\t%.2f\t%.2f\t%d\t%d'
              % (rel.split('/')[-1], pb, pa, pr, ab, aa, ar, ib, ia, mb, ma))
    print('TOTAL\t\t\t\t\t\t\t%.2f\t%.2f' % (sums[0], sums[1]))


if __name__ == '__main__':
    main()
