#!/usr/bin/env python3
"""Render the corpus to PDF with headless LibreOffice, one worker pool, one profile each.

soffice exits 0 even when it converts nothing, so the PDF's existence is the only
signal. Each worker gets a private UserInstallation so concurrent runs cannot collide.
"""
import concurrent.futures as cf
import csv, json, os, pathlib, shutil, subprocess, sys, tempfile, threading, time

ROOT = pathlib.Path("/home/user/sample-files")
OUT = pathlib.Path("/data/bench/lo")
MANIFEST = pathlib.Path("/data/bench/manifest.tsv")
TIMEOUT = 180
WORKERS = 4

profiles = {}
lock = threading.Lock()

def profile_for_thread():
    tid = threading.get_ident()
    with lock:
        if tid not in profiles:
            profiles[tid] = tempfile.mkdtemp(prefix="loprof-")
        return profiles[tid]

def convert(row):
    src = ROOT / row["path"]
    dest = OUT / row["id"]
    pdf = dest / "out.pdf"
    if pdf.exists() and pdf.stat().st_size > 0:
        return row["id"], "cached", 0.0
    dest.mkdir(parents=True, exist_ok=True)
    prof = profile_for_thread()
    t0 = time.time()
    # Convert into a private scratch dir: soffice names the output after the input
    # stem, and two inputs in one output dir would overwrite each other.
    scratch = tempfile.mkdtemp(prefix="loout-", dir="/data/bench/lo")
    try:
        cmd = ["soffice", "--headless", "--norestore", "--nolockcheck", "--nodefault",
               "--nofirststartwizard", "-env:UserInstallation=file://" + prof,
               "--convert-to", "pdf", "--outdir", scratch, str(src)]
        try:
            subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                           timeout=TIMEOUT)
        except subprocess.TimeoutExpired:
            subprocess.run(["pkill", "-f", prof], stdout=subprocess.DEVNULL,
                           stderr=subprocess.DEVNULL)
            return row["id"], "timeout", time.time() - t0
        made = list(pathlib.Path(scratch).glob("*.pdf"))
        if not made:
            return row["id"], "failed", time.time() - t0
        shutil.move(str(made[0]), str(pdf))
        return row["id"], "ok", time.time() - t0
    finally:
        shutil.rmtree(scratch, ignore_errors=True)

def main():
    rows = list(csv.DictReader(MANIFEST.open(), delimiter="\t"))
    OUT.mkdir(parents=True, exist_ok=True)
    results = {}
    done = 0
    t0 = time.time()
    with cf.ThreadPoolExecutor(WORKERS) as ex:
        for rid, status, secs in ex.map(convert, rows):
            results[rid] = {"status": status, "seconds": round(secs, 2)}
            done += 1
            if done % 25 == 0 or done == len(rows):
                el = time.time() - t0
                print(f"{done}/{len(rows)}  {el:.0f}s elapsed, "
                      f"eta {el / done * (len(rows) - done):.0f}s", flush=True)
    (pathlib.Path("/data/bench/lo-status.json")).write_text(json.dumps(results, indent=1))
    from collections import Counter
    print(Counter(v["status"] for v in results.values()))

if __name__ == "__main__":
    sys.exit(main())
