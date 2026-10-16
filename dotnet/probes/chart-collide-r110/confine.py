#!/usr/bin/env python3
"""Render a list of corpus documents with one CLI and bank only what a verdict needs.

The PDF is deleted the moment its three numbers are out of it -- sha256, page count and
alphanumeric characters -- because this container has four gigabytes free and a chart-bearing
corpus is a thousand pages of workbook. `glyphs` is `batch-check.sh`'s column 9 rule verbatim:
`pdftotext` and then `sum(1 for c in text if c.isalnum())`, Unicode-aware, which is what the
gate scores.

    confine.py --out <dir> --cli <XRay.Office.Cli> --rows <tsv>   < paths
"""
import argparse
import concurrent.futures as cf
import hashlib
import shutil
import subprocess
import sys
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument("--out", required=True)
ap.add_argument("--cli", required=True)
ap.add_argument("--rows", required=True)
ap.add_argument("--corpus", default="/home/user/sample-files")
ap.add_argument("--jobs", type=int, default=3)
args = ap.parse_args()

CORPUS = Path(args.corpus)
OUT = Path(args.out)
OUT.mkdir(parents=True, exist_ok=True)
paths = [line.strip() for line in sys.stdin if line.strip() and not line.startswith("#")]


def key(rel):
    """One directory per document, keyed on the path -- never on a worker slot."""
    return hashlib.sha1(rel.encode("utf-8")).hexdigest()[:16]


def stats(pdf):
    data = pdf.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    text = subprocess.run(["pdftotext", str(pdf), "-"], capture_output=True).stdout
    text = text.decode("utf-8", "replace")
    pages = data.count(b"/Type /Page") or text.count("\f")
    pages = text.count("\f") or 1
    return digest, pages, sum(1 for c in text if c.isalnum())


def one(rel):
    src = CORPUS / rel
    work = OUT / key(rel)
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)
    proc = subprocess.run(
        [args.cli, "render", "--outdir", str(work), str(src)],
        capture_output=True, timeout=900)
    pdfs = sorted(work.glob("*.pdf"))
    if not pdfs:
        shutil.rmtree(work, ignore_errors=True)
        return "%s\tFAILED\t0\t0\t%s" % (rel, proc.returncode)
    row = "%s\t%s\t%d\t%d\t0" % ((rel,) + stats(pdfs[0]))
    shutil.rmtree(work, ignore_errors=True)
    return row


with open(args.rows, "w") as out:
    out.write("path\tsha256\tpages\tglyphs\trc\n")
    with cf.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for row in pool.map(one, paths):
            out.write(row + "\n")
            out.flush()
print("wrote", args.rows)
