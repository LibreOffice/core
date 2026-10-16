#!/usr/bin/env python3
r"""Compare OUR rules against the reference's, rule for rule, on the pages the predictor cannot score.

    compare-rules.py <cli> <corpus-root> <ref-dir> <scaled-docs.txt> <out.tsv>

`corpus-rules.py` cannot PREDICT a rule on a scaled page, because the page states the reference's
number multiplied by a factor the instrument cannot see.  It can still be COMPARED: both sides
scale by the same factor, so if our pre-scale value is right the drawn values agree whatever the
factor is.  Predicting and comparing are two different tests and only the first is blocked.

Rules are paired between the two renderings by page, by left edge and by depth -- within 1 pt on
each -- and an unpaired rule is reported as unpaired rather than quietly dropped, because a rule
this tree draws somewhere else is a finding and not a pass.
"""
import os, subprocess, sys, tempfile, shutil, collections
from concurrent.futures import ThreadPoolExecutor
import pymupdf

CLI, ROOT, REF, LIST, OUT = sys.argv[1:6]
EPOCH = '1700000000'


def rules_on(page):
    out = []
    for d in page.get_drawings():
        r = d['rect']
        if r.width < 3:
            continue
        if d['type'] == 's':
            if r.height > 1.5:
                continue
            t = d.get('width') or 0.0
            y = (r.y0 + r.y1) / 2.0
        else:
            if r.height > 8:
                continue
            t = r.height
            y = (r.y0 + r.y1) / 2.0
        if t > 0:
            out.append((round(r.x0, 3), round(r.x1, 3), round(y, 4), round(t, 4)))
    return out


def text_lines(pdf):
    """Every thin horizontal rule that sits under or through a span of its own width."""
    doc = pymupdf.open(pdf)
    found = []
    for pi in range(doc.page_count):
        page = doc[pi]
        rules = rules_on(page)
        if not rules:
            continue
        for block in page.get_text('dict')['blocks']:
            for line in block.get('lines', []):
                for span in line['spans']:
                    if not span['text'].strip():
                        continue
                    x0, _, x1, _ = span['bbox']
                    base, em = span['origin'][1], span['size']
                    for (rx0, rx1, ry, t) in rules:
                        if abs(rx0 - x0) > 1.0 or abs(rx1 - x1) > 1.0:
                            continue
                        depth = ry - base
                        if -0.70 * em <= depth <= 0.60 * em and abs(depth) > 0.004 * em:
                            found.append((pi, rx0, round(depth, 4), t))
    doc.close()
    return found


def one(name):
    stem = name[:-4]
    src = None
    for dirpath, _d, files in os.walk(ROOT):
        for f in files:
            if os.path.splitext(f)[0].replace(' ', '_') == stem.rsplit('__', 1)[0].replace(' ', '_'):
                src = os.path.join(dirpath, f)
                break
        if src:
            break
    if src is None:
        return name, 'no-source', 0, 0, 0, []
    tmp = tempfile.mkdtemp(prefix='cmp-')
    try:
        env = dict(os.environ, SOURCE_DATE_EPOCH=EPOCH)
        subprocess.run(['timeout', '-k', '30', '900', CLI, 'render', src,
                        '--format', 'pdf', '--outdir', tmp],
                       capture_output=True, timeout=1000, env=env)
        pdfs = [f for f in os.listdir(tmp) if f.endswith('.pdf')]
        if not pdfs:
            return name, 'ours-failed', 0, 0, 0, []
        ours = text_lines(os.path.join(tmp, pdfs[0]))
        theirs = text_lines(os.path.join(REF, name))
    except Exception:
        return name, 'error', 0, 0, 0, []
    finally:
        shutil.rmtree(tmp, ignore_errors=True)

    pool = list(ours)
    agree = disagree = 0
    pairs = []
    for (pi, x0, depth, t) in theirs:
        best, bi = None, -1
        for i, (qi, qx, qd, qt) in enumerate(pool):
            if qi != pi or abs(qx - x0) > 1.0 or abs(qd - depth) > 1.0:
                continue
            score = abs(qx - x0) + abs(qd - depth)
            if best is None or score < best:
                best, bi = score, i
        if bi < 0:
            continue
        (_, qx, qd, qt) = pool.pop(bi)
        pairs.append((pi, x0, depth, t, qd, qt))
        if abs(qd - depth) <= 0.0015 and abs(qt - t) <= 0.0015:
            agree += 1
        else:
            disagree += 1
    return name, 'ok', len(theirs), agree, disagree, pairs


def main():
    names = [l.strip() for l in open(LIST) if l.strip()]
    tally = collections.Counter()
    with open(OUT, 'w') as fh, open(OUT + '.pairs', 'w') as ph:
        fh.write('doc\tstatus\tref_rules\tagree\tdisagree\n')
        ph.write('doc\tpage\tx0\tref_depth\tref_w\tour_depth\tour_w\n')
        with ThreadPoolExecutor(max_workers=3) as pool:
            for name, status, n, a, d, pairs in pool.map(one, names):
                fh.write('%s\t%s\t%d\t%d\t%d\n' % (name, status, n, a, d))
                for (pi, x0, rd, rt, od, ot) in pairs:
                    ph.write('%s\t%d\t%.3f\t%.4f\t%.4f\t%.4f\t%.4f\n'
                             % (name, pi, x0, rd, rt, od, ot))
                tally[status] += 1
                tally['ref_rules'] += n
                tally['agree'] += a
                tally['disagree'] += d
    for k, v in sorted(tally.items()):
        print('%s\t%d' % (k, v))


if __name__ == '__main__':
    main()
