#!/usr/bin/env python3
"""The right indent at which each title stops keeping its page number on its own line.

One-point resolution: the title is fixed and `w:ind w:right` is swept in 20-twip steps, so
the boundary is located to a point. `build.py`'s sweep steps the title by two characters
instead — 16.28 pt of the reference's own advance — against right-indent steps of 6.5 to
9 pt, and a boundary located to ±16 pt in a variable moving 9 pt at a time cannot tell one
rule from another. That is why `results.md`'s arithmetic readings of the coarse table
disagreed with each other.
"""
import os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import importlib.util

spec = importlib.util.spec_from_file_location("bf", os.path.join(HERE, "build-fine.py"))
bf = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bf)


def table(pdf):
    text = subprocess.run(["pdftotext", "-nopgbrk", pdf, "-"], capture_output=True, text=True).stdout
    out = {}
    for line in text.splitlines():
        m = re.search(r'\br(\d+)n(\d+)\b', line)
        if m:
            # `2-?123`: where the number overprints the leader the extractor loses the hyphen.
            out[(int(m.group(1)), int(m.group(2)))] = bool(re.search(r'2-?123', line))
    return out


if __name__ == "__main__":
    print(f"{'arm':<6} {'title':>6} {'last fitting':>26} {'first failing':>26}")
    for arm, pdf in (("ref", "fine-ref/toc-fine.pdf"), ("ours", "fine-ours/toc-fine.pdf")):
        t = table(os.path.join(HERE, pdf))
        for n in bf.TITLES:
            fits = [r for r in bf.RIGHTS if t.get((r, n)) is True]
            fails = [r for r in bf.RIGHTS if t.get((r, n)) is False]
            edge = lambda r: f"right {r:>4} (edge {540 - r / 20:7.2f})" if r is not None else "none in range"
            print(f"{arm:<6} {n:>6} {edge(max(fits) if fits else None):>26} "
                  f"{edge(min(fails) if fails else None):>26}")
