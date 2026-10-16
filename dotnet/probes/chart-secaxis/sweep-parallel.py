#!/usr/bin/env python3
"""The same sweep as sweep.py, scored in parallel and with the rendering split out.

sweep.py renders and scores one document at a time, which on this four-core container shared
with three other agents came to about four rows a minute — three hours for the corpus. The
work is entirely subprocess waits (`pdfinfo`, `pdftotext`, `pdffonts`, and `xray-office render`),
so it parallelises exactly; `prerender.py` fills the `ours` half and this scores what is there.

The verdict rule is `batch-check.sh`:262-286 transcribed, identical to sweep.py's, and it is
validated against the gate's own two halves before anything is scored — the run refuses to
print a scoreboard unless it reproduces all 947 stored verdicts.

    sweep-parallel.py <out-dir> [--jobs N] [--validate-only]
"""
import os, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

CORPUS = Path('/home/user/sample-files')
GATE = Path('/home/user/gate-2f47')
CLI = os.environ.get(
    'XRAY_OFFICE_CLI',
    '/home/user/wt-secaxis/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli')


def identity(path): return f"{path.stem}__{path.suffix.lstrip('.').lower()}"


def counts(pdf):
    """Pages, alphanumeric-bearing tokens, raw tokens, alphanumeric characters, unembedded faces."""
    if not pdf.exists():
        return None
    info = subprocess.run(['pdfinfo', str(pdf)], capture_output=True, text=True).stdout
    pages = next((int(l.split()[1]) for l in info.splitlines() if l.startswith('Pages:')), 0)
    text = subprocess.run(['pdftotext', str(pdf), '-'], capture_output=True).stdout.decode(
        'utf-8', 'replace')
    tokens = text.split()
    words = sum(1 for t in tokens if any(c.isalnum() for c in t))
    glyphs = sum(1 for c in text if c.isalnum())
    fonts = subprocess.run(['pdffonts', str(pdf)], capture_output=True, text=True).stdout
    rows = [r for r in fonts.splitlines()[2:] if r.strip()]
    unembedded = sum(1 for r in rows if len(r.split()) >= 8 and r.split()[-5] == 'no')
    return pages, words, len(tokens), glyphs, len(rows), unembedded


def verdict(ours, ref):
    if ours is None and ref is None: return 'both-failed'
    if ref is None: return 'ref-failed'
    if ours is None: return 'ours-failed'
    op, _, _, og, _, un = ours
    rp, _, _, rg, _, _ = ref
    faults = []
    if op != rp: faults.append('pages')
    if rg > 0:
        if abs(og - rg) > rg * 0.02 and abs(og - rg) > 15: faults.append('words')
    elif og > 15:
        faults.append('words')
    if un: faults.append('unembedded')
    return ','.join(faults) if faults else 'match'


def stored():
    rows = {}
    for line in (GATE / 'parity.tsv').read_text().splitlines():
        if line.startswith('#') or line.startswith('path\t'): continue
        parts = line.split('\t')
        rows[parts[0]] = parts[6]
    return rows


def main():
    out = Path(sys.argv[1])
    jobs = int(sys.argv[sys.argv.index('--jobs') + 1]) if '--jobs' in sys.argv else 4
    validate_only = '--validate-only' in sys.argv

    want = stored()
    paths = sorted(want)

    def check(rel):
        key = identity(CORPUS / rel)
        return rel, verdict(counts(GATE / 'ours' / f'{key}.pdf'),
                            counts(GATE / 'ref' / f'{key}.pdf'))

    with ThreadPoolExecutor(max_workers=jobs) as pool:
        disagreed = [(rel, want[rel], got) for rel, got in pool.map(check, paths)
                     if got != want[rel]]
    print(f'# validation: {len(paths) - len(disagreed)} of {len(paths)} verdicts reproduced')
    for rel, was, now in disagreed:
        print(f'#   DISAGREES\t{rel}\t{was}\t{now}')
    sys.stdout.flush()
    if validate_only: return
    if disagreed:
        sys.exit('the verdict rule does not reproduce the gate; refusing to score against it')

    (out / 'ours').mkdir(parents=True, exist_ok=True)

    def score(i_rel):
        i, rel = i_rel
        src = CORPUS / rel
        key = identity(src)
        mine = out / 'ours' / f'{key}.pdf'
        if not mine.exists():
            # One directory per *document*, not per worker slot: a pool of N threads is not
            # working on N consecutive indices, so `i % jobs` hands the same directory to two
            # live renders and one `rm -rf`s the other's output. That cost 124 of 947 renders
            # the first time this was written.
            work = out / 'w' / str(i)
            subprocess.run(['rm', '-rf', str(work)])
            work.mkdir(parents=True, exist_ok=True)
            try:
                subprocess.run([CLI, 'render', str(src), '--format', 'pdf', '--outdir', str(work)],
                               capture_output=True, timeout=900)
            except subprocess.TimeoutExpired:
                pass
            made = work / (src.stem + '.pdf')
            if made.exists() and not mine.exists():
                os.replace(made, mine)
            subprocess.run(['rm', '-rf', str(work)])
        ours, ref = counts(mine), counts(GATE / 'ref' / f'{key}.pdf')
        got = verdict(ours, ref)
        o = ours or ('-',) * 6
        r = ref or ('-',) * 6
        return (f'{rel}\t{o[0]}/{r[0]}\t{o[1]}/{r[1]}\t{o[4]}/{r[4]}\t{o[5]}\t{got}'
                f'\t{o[2]}/{r[2]}\t{o[3]}/{r[3]}\t{want[rel]}')

    print('path\tpages\twords\tfonts\tunemb\tverdict\trawwords\tglyphs\twas')
    with ThreadPoolExecutor(max_workers=jobs) as pool:
        for line in pool.map(score, enumerate(paths)):
            print(line)
            sys.stdout.flush()


main()
