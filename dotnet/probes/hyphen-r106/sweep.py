#!/usr/bin/env python3
"""Render every corpus document with our own CLI, hash the result, and keep no PDF.

Two legs from ONE binary: XRAY_OFFICE_HYPHEN_DICTS=0 is the no-dictionary state, which is
byte-for-byte the behaviour before this seat because ChartAxisLabels asks Hyphenators.For,
which answers null when the variable turns the shipped file off. Running one binary twice
removes the rebuild-under-a-sweep trap dotnet/CLAUDE.md records, at the cost of needing a
separate check that the off leg really equals a base-built binary -- which is done on three
documents by hand.

Disk is the reason nothing is kept: 947 PDFs twice does not fit in what is free. Each
render is hashed and deleted; the movers are re-rendered afterwards, and only those.
"""
import hashlib, os, re, shutil, subprocess, sys, tempfile
from concurrent.futures import ThreadPoolExecutor

CLI = sys.argv[1]
LIST = sys.argv[2]
OUT = sys.argv[3]
DICTS = sys.argv[4]          # "0" for the off leg, "" for the shipped default
JOBS = int(sys.argv[5]) if len(sys.argv) > 5 else 4
ROOT = "/home/user/sample-files"

env = dict(os.environ)
env["SOURCE_DATE_EPOCH"] = "1700000000"
env["XRAY_OFFICE_HYPHEN_DICTS"] = DICTS

rels = [l.strip() for l in open(LIST) if l.strip()]

def ident(rel):
    stem, ext = os.path.splitext(rel)
    return stem.replace("/", "_").replace(" ", "_") + "__" + ext[1:]

def one(rel):
    src = os.path.join(ROOT, rel)
    tmp = tempfile.mkdtemp(prefix="r106-", dir="/home/user/r106-work/tmp")
    try:
        subprocess.run([CLI, "render", "--quiet", "--outdir", tmp, src],
                       env=env, capture_output=True, timeout=600)
        pdfs = [f for f in os.listdir(tmp) if f.endswith(".pdf")]
        if not pdfs:
            return rel, "FAILED", 0
        data = open(os.path.join(tmp, pdfs[0]), "rb").read()
        return rel, hashlib.sha256(data).hexdigest(), len(data)
    except Exception as exc:                                   # noqa: BLE001
        return rel, "ERROR:" + type(exc).__name__, 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)

os.makedirs("/home/user/r106-work/tmp", exist_ok=True)
with ThreadPoolExecutor(max_workers=JOBS) as pool, open(OUT, "w") as fh:
    for rel, digest, size in pool.map(one, rels):
        fh.write(f"{rel}\t{digest}\t{size}\n")
        fh.flush()
print("done", len(rels))
