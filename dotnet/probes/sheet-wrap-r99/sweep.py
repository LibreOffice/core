#!/usr/bin/env python3
"""Render a list of documents with one CLI, under SOURCE_DATE_EPOCH, one output
directory per document, and print an md5 per document. Deletes the PDF after hashing so a
whole track fits on a tight disk.

usage: XRAY_OFFICE_CLI=... sweep.py <list-of-paths> <workdir> [jobs]
"""
import concurrent.futures, hashlib, os, pathlib, subprocess, sys

CLI = os.environ["XRAY_OFFICE_CLI"]
EPOCH = os.environ.get("SOURCE_DATE_EPOCH", "1757462400")

def one(args):
    src, out = args
    ident = pathlib.PurePosixPath(src).stem + "__" + pathlib.PurePosixPath(src).suffix.lstrip('.')
    work = pathlib.Path(out) / ident
    work.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH=EPOCH)
    try:
        r = subprocess.run([CLI, "render", src, "--outdir", str(work)],
                           capture_output=True, timeout=900, env=env, check=False)
    except subprocess.TimeoutExpired:
        return ident, "timeout", 0
    pdfs = sorted(work.glob("*.pdf"))
    if r.returncode != 0 or not pdfs:
        return ident, "failed", 0
    data = pdfs[0].read_bytes()
    pages = data.count(b"/Type /Page\n") or data.count(b"/Type/Page")
    if b"%%EOF" not in data[-2048:]:
        return ident, "truncated", 0
    h = hashlib.md5(data).hexdigest()
    for p in pdfs: p.unlink()
    work.rmdir()
    return ident, h, len(data)

paths = [l.strip() for l in open(sys.argv[1]) if l.strip()]
out = sys.argv[2]
jobs = int(sys.argv[3]) if len(sys.argv) > 3 else 2
print("identity\tmd5\tbytes")
with concurrent.futures.ThreadPoolExecutor(jobs) as pool:
    for ident, h, n in pool.map(one, [(p, out) for p in paths]):
        print("%s\t%s\t%d" % (ident, h, n), flush=True)
