#!/usr/bin/env python3
r"""Which of the 338 converted `.rtf` this round's change actually moves, byte for byte.

`reach-census.py` counts the documents that *declare and apply* a name the round adds. That is
not reach: round 95 closed O9 with two documents applying `Body Text` and **neither moving**,
because each entry states its own `\fs` and the reset writes RTF's default back over it. So the
number that belongs in a write-up is the one this script produces — documents whose rendering
changes — and the census is only the shortlist it explains.

Each document is rendered with `XRay.Office.Cli render --format pdf`, hashed, and the PDF deleted
immediately, so the sweep's disk cost is one PDF at a time rather than 338. `SOURCE_DATE_EPOCH`
is pinned, so two runs of the same binary over the same file are byte-identical (checked before
the sweep, not assumed) and a hash difference between two legs is a real difference.

    XRAY_OFFICE_CLI=<abs path> reach-sweep.py <corpus root> <out.tsv> [workers]

Run it once at the base commit and once at HEAD, then `diff` the two `.tsv`.
"""
import hashlib
import os
import pathlib
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor


def main():
    root = pathlib.Path(sys.argv[1])
    out = pathlib.Path(sys.argv[2])
    workers = int(sys.argv[3]) if len(sys.argv) > 3 else 3
    cli = os.environ['XRAY_OFFICE_CLI']
    work = out.parent / (out.stem + '-work')

    files = sorted(root.rglob('*.rtf'))
    print(f'{len(files)} documents through {cli}', flush=True)

    def one(item):
        index, src = item
        # One directory per document, not per worker: a pool does not work consecutive indices,
        # and two renders sharing a directory silently delete each other's output.
        here = work / f'd{index}'
        if here.exists():
            shutil.rmtree(here)
        here.mkdir(parents=True)
        env = dict(os.environ, SOURCE_DATE_EPOCH='1700000000')
        code = subprocess.run(['timeout', '-k', '30', '240', cli, 'render', str(src),
                               '--format', 'pdf', '--outdir', str(here)],
                              capture_output=True, env=env).returncode
        made = here / (src.stem + '.pdf')
        digest = (hashlib.sha256(made.read_bytes()).hexdigest() if made.exists()
                  else f'NONE-exit{code}')
        size = made.stat().st_size if made.exists() else 0
        shutil.rmtree(here)
        return f'{src.name}\t{digest}\t{size}'

    with ThreadPoolExecutor(max_workers=workers) as pool:
        rows = list(pool.map(one, enumerate(files)))
    shutil.rmtree(work, ignore_errors=True)
    out.write_text('\n'.join(sorted(rows)) + '\n', encoding='utf-8')
    missing = sum(1 for r in rows if '\tNONE' in r)
    print(f'wrote {len(rows)} rows to {out}; {missing} produced no PDF')


main()
