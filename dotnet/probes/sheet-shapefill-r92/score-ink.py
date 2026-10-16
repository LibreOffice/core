#!/usr/bin/env python3
"""Summed unsigned ink between one rendering and the banked reference, per document.

The instrument is `render-comparison/scripts/pdf-image-diff.py`; this only drives it over a
list of identities and totals the `|ink|%` column, because a per-page table is what that
script prints and a ranking is what a round needs.

Usage: score-ink.py <ours-dir> <ref-dir> <identity...>   (or - to read identities on stdin)
"""
import pathlib
import subprocess
import sys

DIFF = "/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py"


def score(ours, ref):
    r = subprocess.run(
        ["python3", DIFF, str(ours), str(ref)],
        capture_output=True, text=True, timeout=1800, check=False)
    total = 0.0
    pages = 0
    major = 0
    for line in r.stdout.splitlines():
        parts = line.split("\t")
        if len(parts) < 6 or not parts[0].strip().isdigit():
            continue
        try:
            total += float(parts[3])
        except ValueError:
            continue
        pages += 1
        if parts[5].strip() == "MAJOR":
            major += 1
    return total, pages, major


def main():
    ours_dir = pathlib.Path(sys.argv[1])
    ref_dir = pathlib.Path(sys.argv[2])
    names = sys.argv[3:]
    if names == ["-"] or not names:
        names = [line.strip() for line in sys.stdin if line.strip()]

    print("identity\tsum|ink|%\tpages\tmajor")
    for ident in names:
        ours = ours_dir / f"{ident}.pdf"
        ref = ref_dir / f"{ident}.pdf"
        if not ours.exists() or not ref.exists():
            print(f"{ident}\t-\t-\tmissing")
            continue
        total, pages, major = score(ours, ref)
        print(f"{ident}\t{total:.2f}\t{pages}\t{major}", flush=True)


if __name__ == "__main__":
    main()
