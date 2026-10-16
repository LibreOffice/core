#!/usr/bin/env python3
"""Bisect the box height at which each renderer changes its autofit answer, and read
the block height it must have measured out of that threshold.

    block(row) = H_min(row) + 1        (see make-fitedge-probe.py for why)

One deck per refinement round, holding every pending probe for every case, so the whole
sweep costs a handful of conversions rather than one per sample.  Both renderers see the
identical deck.

    fitedge.py --out <dir> [--cases a,b,c] [--lo 1500] [--hi 8000] [--grid 28] [--rounds 12]
               [--ppt]

`--ppt` converts the deck to .ppt with 26.2.4.2 first and probes that instead, which is the
format of every document in this column.
"""
import argparse, importlib.util, os, shutil, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import tfz


def _load(name, filename):
    spec = importlib.util.spec_from_file_location(name, os.path.join(HERE, filename))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


_probe = _load('fitedge_probe', 'make-fitedge-probe.py')
CASES, build, BOX_W = _probe.CASES, _probe.build, _probe.BOX_W

SOFFICE = '/opt/libreoffice26.2/program/soffice'
CLI = os.environ.get('CLI', '/home/user/r103-slidefw/cli-base/XRay.Office.Cli')


def render_ref(deck, outdir, fmt='pdf'):
    env = dict(os.environ, SOURCE_DATE_EPOCH='1700000000')
    subprocess.run([SOFFICE, '--headless', '--norestore',
                    '-env:UserInstallation=file:///home/user/r103-slidefw/lo',
                    '--convert-to', fmt, '--outdir', outdir, deck],
                   check=False, capture_output=True, timeout=1800, env=env)
    stem = os.path.splitext(os.path.basename(deck))[0]
    return os.path.join(outdir, stem + '.' + fmt)


def render_ours(deck, outdir):
    env = dict(os.environ, SOURCE_DATE_EPOCH='1700000000')
    subprocess.run([CLI, 'render', deck, '--format', 'pdf', '--outdir', outdir],
                   check=False, capture_output=True, timeout=1800, env=env)
    stem = os.path.splitext(os.path.basename(deck))[0]
    return os.path.join(outdir, stem + '.pdf')


def signatures(pdf):
    """(size, count, span) of the fitted box on every page, from the content stream."""
    out = {}
    for page, _h, shows in tfz.read(pdf):
        ys = [(sz, y) for _fn, sz, _x, y in shows if y < 500.0]
        if not ys:
            out[page] = ('none', 0, 0.0)
            continue
        size = round(max(sz for sz, _ in ys), 1)
        span = round(max(y for _, y in ys) - min(y for _, y in ys), 1)
        out[page] = (size, len(ys), span)
    return out


def sweep(cases, lo, hi, grid, rounds, outdir, as_ppt, vary='h', fixed=8000):
    sampled = {c: {} for c in cases}          # case -> {H: {'ref': sig, 'ours': sig}}
    pending = {c: sorted(set(round(lo + (hi - lo) * i / (grid - 1)) for i in range(grid)))
               for c in cases}
    work = tempfile.mkdtemp(prefix='fitedge-', dir='/home/user/r103-slidefw')
    log = []
    other = 'w' if vary == 'h' else 'h'
    for rnd in range(rounds):
        spec = [{'case': c, vary: h, other: fixed} for c in cases for h in pending[c]]
        if not spec:
            break
        deck = os.path.join(work, f'edge{rnd}.pptx')
        build(deck, spec)
        probe = deck
        if as_ppt:
            probe = render_ref(deck, work, 'ppt')
            if not os.path.exists(probe):
                raise SystemExit('no .ppt came back from the reference')
        ref = render_ref(probe, work)
        ours = render_ours(probe, work)
        rs, os_ = signatures(ref), signatures(ours)
        for i, item in enumerate(spec, 1):
            sampled[item['case']][item[vary]] = {'ref': rs.get(i), 'ours': os_.get(i)}
        for f in (ref, ours, deck, probe):
            if os.path.exists(f) and f != deck:
                os.remove(f)
        # next round: bisect every adjacent pair whose answer differs for either renderer
        nxt = {}
        total = 0
        for c in cases:
            hs = sorted(sampled[c])
            want = set()
            for a, b in zip(hs, hs[1:]):
                if b - a < 2:
                    continue
                if (sampled[c][a]['ref'] != sampled[c][b]['ref']
                        or sampled[c][a]['ours'] != sampled[c][b]['ours']):
                    want.add((a + b) // 2)
            nxt[c] = sorted(want)
            total += len(want)
        log.append(f'round {rnd}: {len(spec)} slides, {total} boundaries still open')
        print(log[-1], flush=True)
        pending = nxt
    shutil.rmtree(work, ignore_errors=True)

    os.makedirs(outdir, exist_ok=True)
    with open(os.path.join(outdir, 'fitedge-samples.tsv'), 'w') as fh:
        fh.write('case\th\tleg\tsize\tshows\tspan\n')
        for c in cases:
            for h in sorted(sampled[c]):
                for leg in ('ref', 'ours'):
                    s = sampled[c][h][leg]
                    fh.write(f'{c}\t{h}\t{leg}\t{s[0]}\t{s[1]}\t{s[2]}\n')

    lines = []
    for c in cases:
        hs = sorted(sampled[c])
        for leg in ('ref', 'ours'):
            edges = []
            for a, b in zip(hs, hs[1:]):
                if sampled[c][a][leg] != sampled[c][b][leg] and b - a == 1:
                    # b is the smallest sampled height carrying b's answer
                    edges.append((b, sampled[c][b][leg], sampled[c][a][leg]))
            for h, above, below in edges:
                lines.append((c, leg, h + 1, above, below))
    with open(os.path.join(outdir, 'fitedge-blocks.tsv'), 'w') as fh:
        fh.write('case\tleg\tblock\tsize\tshows\tspan\tnext_size\n')
        for c, leg, block, above, below in lines:
            fh.write(f'{c}\t{leg}\t{block}\t{above[0]}\t{above[1]}\t{above[2]}\t{below[0]}\n')
    print('\n'.join(log))
    return lines


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('--out', required=True)
    ap.add_argument('--cases', default=','.join(CASES))
    ap.add_argument('--lo', type=int, default=1500)
    ap.add_argument('--hi', type=int, default=8000)
    ap.add_argument('--grid', type=int, default=28)
    ap.add_argument('--rounds', type=int, default=12)
    ap.add_argument('--ppt', action='store_true')
    ap.add_argument('--vary', default='h', choices=('h', 'w'))
    ap.add_argument('--fixed', type=int, default=8000)
    a = ap.parse_args()
    sweep(a.cases.split(','), a.lo, a.hi, a.grid, a.rounds, a.out, a.ppt, a.vary, a.fixed)
