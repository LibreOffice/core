#!/usr/bin/env python3
"""Summed diff% and |ink|% between a rendering and the banked reference, per document.

Two columns rather than one: `|ink|%` is a per-region *signed* mean and therefore cancels
inside a region, so a change that removes ink we wrongly drew can raise it. `diff%` is the
raw count of disagreeing pixels and cannot cancel. Round 97 (`probes/clip-seats-r97` §2.2)
is why both are reported.

Usage: score-both.py <ours-dir> <ref-dir> [identity...]   (or read identities on stdin)
"""
import os, pathlib, shutil, subprocess, sys, tempfile

DIFF = os.environ.get(
    "PDF_IMAGE_DIFF",
    "/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py")


def score(ours, ref):
    work = tempfile.mkdtemp(prefix="score-both-")
    try:
        r = subprocess.run(["python3", DIFF, str(ours), str(ref), "--outdir", work],
                           capture_output=True, text=True, timeout=1800, check=False)
    finally:
        shutil.rmtree(work, ignore_errors=True)
    diff = ink = 0.0
    pages = major = 0
    for line in r.stdout.splitlines():
        p = line.split("\t")
        if len(p) < 6 or not p[0].strip().isdigit():
            continue
        try:
            diff += float(p[1]); ink += float(p[3])
        except ValueError:
            continue
        pages += 1
        if p[5].strip() == "MAJOR":
            major += 1
    return diff, ink, pages, major


def main():
    ours_dir, ref_dir = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
    names = sys.argv[3:]
    if not names or names == ["-"]:
        names = [l.strip() for l in sys.stdin if l.strip()]
    print("identity\tsum diff%\tsum|ink|%\tpages\tmajor")
    for ident in names:
        o, r = ours_dir / f"{ident}.pdf", ref_dir / f"{ident}.pdf"
        if not o.exists() or not r.exists():
            print(f"{ident}\t-\t-\t-\tmissing"); continue
        d, i, pg, mj = score(o, r)
        print(f"{ident}\t{d:.2f}\t{i:.2f}\t{pg}\t{mj}", flush=True)


main()
