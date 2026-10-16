#!/usr/bin/env python3
"""Render every document of a corpus track with one X-Ray.Office build.

    render-track.py <corpus-root> <glob> <outdir> [workers]

`SOURCE_DATE_EPOCH` is set here rather than left to the caller, because reach is measured by
diffing two of these runs byte for byte and a document that prints the date draws different ink
on a different day.
"""
import os, subprocess, sys, concurrent.futures

ROOT, GLOB, OUT = sys.argv[1], sys.argv[2], sys.argv[3]
WORKERS = int(sys.argv[4]) if len(sys.argv) > 4 else 6
CLI = os.environ["XRAY_OFFICE_CLI"]
os.makedirs(OUT, exist_ok=True)
ENV = dict(os.environ, SOURCE_DATE_EPOCH="1700000000")

EXT = (".doc", ".docx", ".rtf", ".odt", ".ott", ".xls", ".xlsx", ".ods", ".csv",
       ".ppt", ".pptx", ".odp", ".otp")

files = []
for d in sorted(subprocess.run(["bash", "-c", f"cd {ROOT} && ls -d {GLOB}"],
                               capture_output=True, text=True).stdout.split()):
    for root, _, names in os.walk(os.path.join(ROOT, d)):
        for n in sorted(names):
            if n.lower().endswith(EXT):
                files.append(os.path.join(root, n))
files.sort()
print(f"{len(files)} documents", flush=True)

def one(path):
    stem, ext = os.path.splitext(os.path.basename(path))
    ident = f"{stem}__{ext[1:].lower()}"
    tmp = os.path.join(OUT, ".t", ident)
    os.makedirs(tmp, exist_ok=True)
    try:
        subprocess.run([CLI, "render", path, "--format", "pdf", "--outdir", tmp],
                       capture_output=True, env=ENV, timeout=240)
    except subprocess.TimeoutExpired:
        pass
    made = os.path.join(tmp, stem + ".pdf")
    if os.path.exists(made):
        os.replace(made, os.path.join(OUT, ident + ".pdf"))
        return ident, True
    return ident, False

failed = []
with concurrent.futures.ThreadPoolExecutor(WORKERS) as pool:
    for ident, ok in pool.map(one, files):
        if not ok:
            failed.append(ident)
print(f"rendered {len(files) - len(failed)} of {len(files)}; failed: {failed}")

# The batch each document came from, so a `done-*` verdict can be taken out of a whole-track run
# without rendering the track twice. Written beside the PDFs and keyed the way verdict.py keys them.
with open(os.path.join(OUT, "manifest.tsv"), "w") as fh:
    for path in files:
        stem, ext = os.path.splitext(os.path.basename(path))
        rel = os.path.relpath(path, ROOT)
        fh.write(f"{stem}__{ext[1:].lower()}\t{rel}\n")
