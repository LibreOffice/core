#!/usr/bin/env python3
"""Render one track's documents with `xray-office render`, one directory per document."""
import os, subprocess, sys, shutil
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

CLI = os.environ["XRAY_OFFICE_CLI"]
ROOT = Path(sys.argv[1])
PATTERN = sys.argv[2]
OUT = Path(sys.argv[3]); OUT.mkdir(parents=True, exist_ok=True)
JOBS = int(sys.argv[4]) if len(sys.argv) > 4 else 3

docs = sorted(p for p in ROOT.glob(PATTERN) if p.is_file())
env = dict(os.environ, SOURCE_DATE_EPOCH="1700000000")

def ident(p):
    return f"{p.stem}__{p.suffix.lstrip('.').lower()}"

def run(p):
    d = OUT / "w" / ident(p)
    if d.exists(): shutil.rmtree(d)
    d.mkdir(parents=True)
    r = subprocess.run([CLI, "render", str(p), "--format", "pdf", "--outdir", str(d)],
                       capture_output=True, timeout=600, env=env)
    pdfs = list(d.glob("*.pdf"))
    if r.returncode != 0 or not pdfs:
        return (ident(p), "failed")
    tgt = OUT / (ident(p) + ".pdf")
    shutil.move(str(pdfs[0]), tgt)
    shutil.rmtree(d)
    return (ident(p), "ok")

ok = bad = 0
with ThreadPoolExecutor(JOBS) as ex:
    for name, st in ex.map(run, docs):
        if st == "ok": ok += 1
        else: bad += 1; print("FAILED", name, flush=True)
print(f"TOTAL {len(docs)}  OK {ok}  FAILED {bad}")
