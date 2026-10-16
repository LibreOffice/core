#!/usr/bin/env python3
"""Run `first-divergence.py`'s analysis over a sweep's *already rendered* PDFs.

    divergence-from-sweep.py <ours-dir> <ref-dir> > slides-divergence.tsv

`first-divergence.py --corpus` renders both sides itself, once per document — a whole extra
sweep plus a whole extra soffice pass over the track, which is hours on a loaded machine and
which a round has already had to skip for that reason. A sweep has both PDFs on disk when it
finishes, so the analysis is the only part that needs doing; this reuses them.

It is the same code — `analyse()` is imported, not reimplemented — so the columns and the
thresholds are whatever the shared script currently says they are, including the fixes the
words track made to separate `box` from `size` and to treat a page the image diff refuses to
compare as the strongest signal rather than as agreement.
"""
import importlib.util, os, sys
from pathlib import Path

SCRIPT = (Path(__file__).resolve().parents[3] /
          ".claude/skills/render-comparison/scripts/first-divergence.py")

spec = importlib.util.spec_from_file_location("first_divergence", SCRIPT)
fd = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fd)


def main(ours_dir, ref_dir):
    ours, ref = Path(ours_dir), Path(ref_dir)
    ids = sorted(p.stem for p in ours.glob("*.pdf"))

    print("doc\tpages\tfirst_page\tof\tink\tdominant\tgdelta\tonly_ours\tonly_ref\tblank_ref\ttext")
    for n, i in enumerate(ids, 1):
        o, r = ours / f"{i}.pdf", ref / f"{i}.pdf"
        if not r.exists():
            continue
        a = fd.analyse(o, r)
        print("\t".join(str(x) for x in [
            i, f"{a['pages_ours']}/{a['pages_ref']}", a["first"] or "",
            min(a["pages_ours"], a["pages_ref"]), f"{a['ink']:.2f}",
            a["dominant"], a["gdelta"], a["only_ours"], a["only_ref"],
            a["blank_ref"], a["text"]]), flush=True)
        print(f"[{n}/{len(ids)}] {i}", file=sys.stderr, flush=True)


if __name__ == "__main__":
    main(*sys.argv[1:])
