#!/usr/bin/env python3
"""Run `first-divergence.py`'s analysis over PDFs a sweep has already rendered.

`first-divergence.py --corpus` renders both sides itself, which is right when nothing else has.
After a `batch-check.sh` sweep both renderings are already on disk, and re-rendering 400 PDFs to
re-classify them costs the same again for no new information — and, worse, invites doing the
classification on a *different* build from the one the sweep measured.

    divergence-from-pdfs.py --rows rows.tsv --ours-dir OUT/ours --ref-dir OUT/ref --out div.tsv
"""
from __future__ import annotations

import argparse
import importlib.util
import sys
from pathlib import Path

SPEC = importlib.util.spec_from_file_location(
    "firstdiv",
    "/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/first-divergence.py")
fd = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(fd)


def ident(rel: str) -> str:
    name = rel.rsplit("/", 1)[-1]
    stem, _, ext = name.rpartition(".")
    return f"{stem}__{ext.lower()}"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--rows", type=Path, required=True)
    ap.add_argument("--ours-dir", type=Path, action="append", required=True)
    ap.add_argument("--ref-dir", type=Path, action="append", required=True)
    ap.add_argument("--out", type=Path)
    args = ap.parse_args()

    lines = ["path\tverdict\tpages\tfirst_page\tof\tink\tdominant\tgdelta\tonly_ours\tonly_ref"
             "\tblank_ref\tcounts"]
    rows = [l.rstrip("\n").split("\t") for l in args.rows.read_text().splitlines() if l.strip()]
    rows = [r for r in rows if r[0] != "path"]
    for i, r in enumerate(rows, 1):
        pid = ident(r[0])
        o = next((d / f"{pid}.pdf" for d in args.ours_dir if (d / f"{pid}.pdf").exists()), None)
        rf = next((d / f"{pid}.pdf" for d in args.ref_dir if (d / f"{pid}.pdf").exists()), None)
        if not o or not rf:
            print(f"  [{i}/{len(rows)}] {r[0]} — no rendering", flush=True)
            continue
        a = fd.analyse(o, rf)
        counts = ",".join(f"{k}={v}" for k, v in (a.get("counts") or {}).items() if v)
        lines.append("\t".join(str(x) for x in [
            r[0], r[-1], f"{a['pages_ours']}/{a['pages_ref']}", a["first"] or "",
            min(a["pages_ours"], a["pages_ref"]), f"{a['ink']:.2f}", a["dominant"],
            a["gdelta"], a["only_ours"], a["only_ref"], a["blank_ref"], counts]))
        print(f"  [{i}/{len(rows)}] {r[0]} first={a['first']} {a['dominant']} {counts}", flush=True)
    text = "\n".join(lines)
    if args.out:
        args.out.write_text(text)
        print(f"wrote {args.out}")
    else:
        print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
