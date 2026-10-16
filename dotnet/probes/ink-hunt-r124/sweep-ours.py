#!/usr/bin/env python3
"""Render our half of the corpus, one output directory per DOCUMENT.

Only our side moves this round: the reference leg is the bank at `gate-r122/ref`, which was
drawn by 26.2.4.2 at 0fa883d0c, and HEAD adds probe files only. So this renders 947 documents
and nothing else.

One directory per *document*, never per worker slot -- two live renders in one directory is the
silent failure `CLAUDE.md` records.

Usage: sweep-ours.py <corpus-root> <rows.tsv> <outdir> [jobs]
"""
import concurrent.futures
import os
import pathlib
import subprocess
import sys
import time

CLI = os.environ["XRAY_OFFICE_CLI"]
EPOCH = os.environ.get("SOURCE_DATE_EPOCH", "1757462400")


def identity(rel, ext):
    return f"{pathlib.PurePosixPath(rel).stem}__{ext}"


def render(args):
    root, rel, ext, outdir = args
    src = pathlib.Path(root) / rel
    ident = identity(rel, ext)
    work = pathlib.Path(outdir) / "work" / ident
    work.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH=EPOCH)
    try:
        r = subprocess.run([CLI, "render", str(src), "--outdir", str(work)],
                           capture_output=True, timeout=900, env=env, check=False)
    except subprocess.TimeoutExpired:
        status = "timeout"
    else:
        pdfs = sorted(work.glob("*.pdf"))
        if r.returncode != 0 or not pdfs:
            status = "ours-failed"
        else:
            out = pathlib.Path(outdir) / "ours" / f"{ident}.pdf"
            out.parent.mkdir(parents=True, exist_ok=True)
            out.write_bytes(pdfs[0].read_bytes())
            status = "ok"
    for f in work.iterdir():
        f.unlink()
    work.rmdir()
    return ident, rel, status


def main():
    root, rowsfile, outdir = sys.argv[1], sys.argv[2], sys.argv[3]
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 3

    items = []
    for line in pathlib.Path(rowsfile).read_text(encoding="utf-8").splitlines():
        p = line.split("\t")
        if len(p) < 2:
            continue
        items.append((root, p[0], p[1], outdir))

    pathlib.Path(outdir).mkdir(parents=True, exist_ok=True)
    started, done = time.time(), 0
    with open(pathlib.Path(outdir) / "render.tsv", "w", encoding="utf-8") as out:
        out.write("identity\tpath\tstatus\n")
        with concurrent.futures.ThreadPoolExecutor(jobs) as pool:
            for ident, rel, status in pool.map(render, items):
                out.write(f"{ident}\t{rel}\t{status}\n")
                out.flush()
                done += 1
                if done % 50 == 0:
                    print(f"{done}/{len(items)}  {time.time() - started:.0f}s", flush=True)
    print(f"TOTAL {done} of {len(items)} in {time.time() - started:.0f}s")


main()
