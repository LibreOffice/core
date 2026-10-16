#!/usr/bin/env python3
"""Render the original words track with two binaries and compare the two outputs byte for byte.

The original track is the `.docx`/`.doc`/`.rtf` corpus the ODF one was converted from, and
this is the confinement check: a gain on the converted column that costs a row on the
original track is not a gain.  Both halves are ours, so `soffice` is not involved at all and
`SOURCE_DATE_EPOCH` makes `xray-office render` byte-deterministic — the PDFs are then
comparable with nothing masked.

  original.py <before-cli> <after-cli> <workdir>
"""
import hashlib, os, shutil, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

CORPUS = "/home/user/sample-files/words"
WORKERS = 2


def files():
    out = []
    for root, _d, names in os.walk(CORPUS):
        for n in sorted(names):
            out.append(os.path.join(root, n))
    return sorted(out)


def render(path, cli, work, tag):
    d = hashlib.md5((tag + path).encode("utf-8")).hexdigest()[:16]
    tmp = os.path.join(work, f"t-{d}")
    shutil.rmtree(tmp, ignore_errors=True)
    os.makedirs(tmp, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH="0")
    try:
        subprocess.run([cli, "render", path, "--format", "pdf", "--outdir", tmp],
                       capture_output=True, timeout=300, env=env)
    except subprocess.TimeoutExpired:
        pass
    stem = os.path.splitext(os.path.basename(path))[0]
    made = os.path.join(tmp, stem + ".pdf")
    h = None
    if os.path.exists(made):
        h = hashlib.md5(open(made, "rb").read()).hexdigest()
    shutil.rmtree(tmp, ignore_errors=True)
    return h


def main():
    before, after, work = sys.argv[1], sys.argv[2], sys.argv[3]
    os.makedirs(work, exist_ok=True)
    docs = files()

    def one(p):
        return p, render(p, before, work, "b"), render(p, after, work, "a")

    with ThreadPoolExecutor(WORKERS) as ex:
        rows = list(ex.map(one, docs))

    moved = [r for r in rows if r[1] != r[2]]
    failed = [r for r in rows if r[1] is None or r[2] is None]
    with open(os.path.join(work, "original.tsv"), "w") as f:
        for p, b, a in rows:
            f.write(f"{os.path.relpath(p, CORPUS)}\t{b}\t{a}\t{'same' if b == a else 'MOVED'}\n")
    print(f"documents: {len(rows)}  identical: {len(rows) - len(moved)}  moved: {len(moved)}  "
          f"unrendered by one or both: {len(failed)}")
    for p, b, a in moved[:20]:
        print("  MOVED", os.path.relpath(p, CORPUS), b, a)


main()
