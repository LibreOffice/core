#!/usr/bin/env python3
"""Census: which corpus chart axes could witness O43's two-word restart?

Runs the CLI over a list of corpus documents with `XRAY_OFFICE_AXIS_TRACE=1`, which a
temporary patch to `ChartAxisLabels.Resolve` (banked beside this file as
`axis-trace.diff`, NOT committed to src) makes emit one `AXTRACE` line per multi-word
axis label per resolve attempt:

    AXTRACE dir spacing size bold linebreak overlap rot count index words whole widest rest text

Rows are deduplicated -- `Resolve` runs once per layout attempt and per chart copy -- and
written with the document path prepended. The PDF is deleted as soon as the run returns;
nothing here needs it.

    axis-census.py --cli <XRay.Office.Cli> --out rows.tsv [--jobs 2] < paths
"""
import argparse
import concurrent.futures as cf
import hashlib
import os
import shutil
import subprocess
import sys
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument("--cli", required=True)
ap.add_argument("--out", required=True)
ap.add_argument("--results", required=True)
ap.add_argument("--corpus", default="/home/user/sample-files")
ap.add_argument("--work", default="/tmp/axis-census")
ap.add_argument("--jobs", type=int, default=2)
args = ap.parse_args()

CORPUS = Path(args.corpus)
WORK = Path(args.work)
WORK.mkdir(parents=True, exist_ok=True)
paths = [ln.strip() for ln in sys.stdin if ln.strip() and not ln.startswith("#")]

FIELDS = ("dir spacing size bold linebreak overlap rot count index words "
          "whole widest rest text").split()
RFIELDS = "dir spacing linebreak overlap statedrot rotation rhythm staggered broke attempt".split()


def one(rel):
    src = CORPUS / rel
    work = WORK / hashlib.sha1(rel.encode()).hexdigest()[:16]
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)
    env = dict(os.environ, XRAY_OFFICE_AXIS_TRACE="1")
    try:
        proc = subprocess.run(
            [args.cli, "render", "--outdir", str(work), str(src)],
            capture_output=True, timeout=900, env=env)
        err = proc.stderr.decode("utf-8", "replace")
        rc = proc.returncode
    except subprocess.TimeoutExpired:
        err, rc = "", -9
    finally:
        shutil.rmtree(work, ignore_errors=True)

    seen, rows, results = set(), [], []
    for line in err.splitlines():
        if not (line.startswith("AXTRACE\t") or line.startswith("AXRESULT\t")):
            continue
        tag, body = line.split("\t", 1)
        if line in seen:
            continue
        seen.add(line)
        (rows if tag == "AXTRACE" else results).append(rel + "\t" + body)
    return rel, rc, rows, results


with open(args.out, "w") as out, open(args.results, "w") as res:
    out.write("path\t" + "\t".join(FIELDS) + "\n")
    res.write("path\t" + "\t".join(RFIELDS) + "\n")
    done = 0
    with cf.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for rel, rc, rows, results in pool.map(one, paths):
            for r in rows:
                out.write(r + "\n")
            for r in results:
                res.write(r + "\n")
            out.flush()
            res.flush()
            done += 1
            print("%4d/%d rc=%d rows=%d res=%d %s"
                  % (done, len(paths), rc, len(rows), len(results), rel), file=sys.stderr)
print("wrote", args.out, args.results)
