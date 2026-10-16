#!/usr/bin/env python3
"""Render the corpus to PDF with the X-Ray.Office CLI.

X-Ray.Office exits 0 only when every file rendered; 65 means the format is not laid
out yet, 1 means the file could not be read. Both are recorded as-is: an engine
that declines a document is a result, not an error to hide.
"""
import concurrent.futures as cf
import csv, json, pathlib, shutil, subprocess, sys, tempfile, time

ROOT = pathlib.Path("/home/user/sample-files")
OUT = pathlib.Path("/data/bench/pl")
MANIFEST = pathlib.Path("/data/bench/manifest.tsv")
CLI = "/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli"
TIMEOUT = 300
WORKERS = 4

def render(row):
    src = ROOT / row["path"]
    dest = OUT / row["id"]
    pdf = dest / "out.pdf"
    if pdf.exists() and pdf.stat().st_size > 0:
        return row["id"], "cached", 0.0, ""
    dest.mkdir(parents=True, exist_ok=True)
    scratch = tempfile.mkdtemp(prefix="plout-", dir="/data/bench/pl")
    t0 = time.time()
    try:
        try:
            r = subprocess.run([CLI, "render", "--format", "pdf", "--outdir", scratch,
                                str(src)],
                               capture_output=True, text=True, timeout=TIMEOUT)
        except subprocess.TimeoutExpired:
            return row["id"], "timeout", time.time() - t0, ""
        made = list(pathlib.Path(scratch).glob("*.pdf"))
        err = (r.stderr or r.stdout or "").strip().replace("\n", " ")[:300]
        if not made:
            status = {65: "unsupported", 1: "failed", 2: "usage"}.get(r.returncode, "failed")
            return row["id"], status, time.time() - t0, err
        shutil.move(str(made[0]), str(pdf))
        return row["id"], "ok", time.time() - t0, ""
    finally:
        shutil.rmtree(scratch, ignore_errors=True)

def main():
    rows = list(csv.DictReader(MANIFEST.open(), delimiter="\t"))
    OUT.mkdir(parents=True, exist_ok=True)
    results = {}
    done = 0
    t0 = time.time()
    with cf.ThreadPoolExecutor(WORKERS) as ex:
        for rid, status, secs, err in ex.map(render, rows):
            results[rid] = {"status": status, "seconds": round(secs, 2), "error": err}
            done += 1
            if done % 25 == 0 or done == len(rows):
                el = time.time() - t0
                print(f"{done}/{len(rows)}  {el:.0f}s elapsed, "
                      f"eta {el / done * (len(rows) - done):.0f}s", flush=True)
    pathlib.Path("/data/bench/pl-status.json").write_text(json.dumps(results, indent=1))
    from collections import Counter
    print(Counter(v["status"] for v in results.values()))

if __name__ == "__main__":
    sys.exit(main())
