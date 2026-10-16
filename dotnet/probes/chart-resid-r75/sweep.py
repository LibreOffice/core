#!/usr/bin/env python3
"""Render a list of corpus documents with one X-Ray.Office CLI, one directory per document.

One directory per *document*, never per worker slot: a thread pool does not work
consecutive indices, so keying on a slot index lets two live renders share a directory
and one delete the other's output (dotnet/CLAUDE.md, "A parallel sweep must give each
document its own directory").

Usage: [CORPUS=<root>] sweep.py <cli> <census.tsv> <outdir> [workers]
"""
import concurrent.futures as cf
import hashlib
import os
import pathlib
import subprocess
import sys

CORPUS = pathlib.Path(os.environ.get('CORPUS', '/home/user/sample-files'))


def render(cli, rel, outroot):
    src = CORPUS / rel
    key = hashlib.md5(rel.encode()).hexdigest()[:16]
    out = outroot / key
    out.mkdir(parents=True, exist_ok=True)
    (out / 'name.txt').write_text(rel + '\n')
    try:
        r = subprocess.run([cli, 'render', '--quiet', '--outdir', str(out), str(src)],
                           capture_output=True, timeout=600)
        return rel, r.returncode
    except subprocess.TimeoutExpired:
        return rel, -1


def main():
    cli, census, outroot = sys.argv[1], pathlib.Path(sys.argv[2]), pathlib.Path(sys.argv[3])
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 2
    rels = [ln.split('\t')[0] for ln in census.read_text().splitlines()[1:] if ln.strip()]
    outroot.mkdir(parents=True, exist_ok=True)
    bad = 0
    with cf.ThreadPoolExecutor(jobs) as pool:
        for rel, code in pool.map(lambda r: render(cli, r, outroot), rels):
            if code != 0:
                bad += 1
                print(f'FAIL {code} {rel}', flush=True)
    print(f'TOTAL {len(rels)} FAILED {bad}')


if __name__ == '__main__':
    main()
