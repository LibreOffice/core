#!/usr/bin/env python3
"""Render our half of the corpus and record a hash per document.

Only our side moves in this round — the diff is confined to `dotnet/src`, which cannot reach
`soffice` — so the reference half is taken from the bank and only this is re-run. One output
directory per *document*, never per worker slot: two live renders in one directory is the
silent failure `CLAUDE.md` records.

Usage: sweep-ours.py <corpus-root> <manifest.tsv> <outdir> [jobs] [path-prefix-filter]
"""
import concurrent.futures
import hashlib
import os
import pathlib
import subprocess
import sys
import time

CLI = os.environ["XRAY_OFFICE_CLI"]
EPOCH = os.environ.get("SOURCE_DATE_EPOCH", "1757462400")


def identity(rel):
    p = pathlib.PurePosixPath(rel)
    return f"{p.stem}__{p.suffix.lstrip('.')}"


def render(args):
    root, rel, outdir = args
    src = pathlib.Path(root) / rel
    ident = identity(rel)
    work = pathlib.Path(outdir) / "work" / ident
    work.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH=EPOCH)
    try:
        r = subprocess.run(
            [CLI, "render", str(src), "--outdir", str(work)],
            capture_output=True, timeout=600, env=env, check=False)
    except subprocess.TimeoutExpired:
        return ident, rel, "timeout", ""
    pdfs = sorted(work.glob("*.pdf"))
    if r.returncode != 0 or not pdfs:
        return ident, rel, "ours-failed", ""
    out = pathlib.Path(outdir) / "ours" / f"{ident}.pdf"
    out.parent.mkdir(parents=True, exist_ok=True)
    data = pdfs[0].read_bytes()
    out.write_bytes(data)
    for f in work.iterdir():
        f.unlink()
    work.rmdir()
    return ident, rel, "ok", hashlib.md5(data).hexdigest()


def main():
    root, manifest, outdir = sys.argv[1], sys.argv[2], sys.argv[3]
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 3
    prefix = sys.argv[5] if len(sys.argv) > 5 else ""

    rows = []
    with open(manifest, encoding="utf-8") as fh:
        header = fh.readline().rstrip("\n").split("\t")
        col = header.index("path") if "path" in header else 0
        for line in fh:
            parts = line.rstrip("\n").split("\t")
            if len(parts) <= col:
                continue
            if prefix and not parts[col].startswith(prefix):
                continue
            rows.append(parts[col])

    pathlib.Path(outdir).mkdir(parents=True, exist_ok=True)
    started = time.time()
    done = 0
    with open(pathlib.Path(outdir) / "hashes.tsv", "w", encoding="utf-8") as out:
        out.write("identity\tpath\tstatus\tmd5\n")
        with concurrent.futures.ThreadPoolExecutor(jobs) as pool:
            for ident, rel, status, digest in pool.map(
                    render, [(root, r, outdir) for r in rows]):
                out.write(f"{ident}\t{rel}\t{status}\t{digest}\n")
                out.flush()
                done += 1
                if done % 50 == 0:
                    print(f"{done}/{len(rows)}  {time.time() - started:.0f}s", flush=True)
    print(f"TOTAL {done} of {len(rows)} in {time.time() - started:.0f}s")


if __name__ == "__main__":
    main()
