#!/usr/bin/env python3
"""Summed |ink ratio − 1| over a document's pages, against a 26.2.4.2 reference.

    inkscore.py <listfile> <ref26-dir> <before-ours-dir> <after-ours-dir> [dpi]

Uses `compare-images.py`'s own `ink_delta`-family metric rather than a hand-rolled
threshold — a threshold over-ranks a page dense with thin vector strokes; see
`probes/vision-r68/results.md`. Refuses to score a document whose three renderings do not
have the same page count on both of our halves, and says so.
"""
import importlib.util, shutil, subprocess, sys, tempfile
from pathlib import Path

SKILL = Path('/home/user/wt-words69/.claude/skills/render-comparison/scripts/compare-images.py')
spec = importlib.util.spec_from_file_location('ci', SKILL)
ci = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ci)

LIST, REF, BEFORE, AFTER = (Path(a) for a in sys.argv[1:5])
DPI = sys.argv[5] if len(sys.argv) > 5 else '100'


def raster(pdf, into):
    into.mkdir(parents=True, exist_ok=True)
    subprocess.run(['pdftoppm', '-r', DPI, '-png', str(pdf), str(into / 'page')], check=True)
    return sorted(into.glob('*.png'))


def ink(pngs_ref, pngs_ours):
    """Summed |ink ratio - 1| over the pages both sides have."""
    total = 0.0
    for r, o in zip(pngs_ref, pngs_ours):
        m = ci.compare(ci.read_png(o), ci.read_png(r))
        total += abs(m['ink_ratio'] - 1.0)
    return total, len(pngs_ref), len(pngs_ours)


print(f"{'document':58} {'pages ref/before/after':>22} {'|ink| before':>12} {'|ink| after':>12}")
for line in LIST.read_text().split('\n'):
    doc = line.strip()
    if not doc:
        continue
    p = Path(doc)
    stem, ext = p.stem, p.suffix[1:].lower()
    ref = REF / f'{stem}.pdf'
    ours_b = BEFORE / f'{stem}__{ext}.pdf'
    ours_a = AFTER / f'{stem}__{ext}.pdf'
    if not (ref.exists() and ours_b.exists() and ours_a.exists()):
        print(f'{stem[:58]:58} MISSING A RENDERING')
        continue
    with tempfile.TemporaryDirectory(dir=Path(tempfile.gettempdir())) as tmp:
        t = Path(tmp)
        pr = raster(ref, t / 'r')
        pb = raster(ours_b, t / 'b')
        pa = raster(ours_a, t / 'a')
        vb, nr, nb = ink(pr, pb)
        va, _, na = ink(pr, pa)
    print(f'{stem[:58]:58} {nr:6}/{nb}/{na:<14} {vb:12.3f} {va:12.3f}')
