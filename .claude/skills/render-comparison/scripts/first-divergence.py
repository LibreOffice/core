#!/usr/bin/env python3
"""Find the first page where a rendering diverges, and say what differs on it.

Rendering errors cascade: one wrong measurement shifts everything after it, so a document
with a defect on page 3 has ninety wrong pages and one cause. Comparing page N against page N
across the whole document therefore measures the cascade, not the fault — and the further in
you look, the less the comparison means, because by then the two renderers are drawing
different content in the same place.

This walks forward instead and stops at the first page that materially differs. That page is
where the cause lives; everything after it is consequence. It then runs the operator diff on
*that page alone* and classifies what differs, so the output answers "where did it start, and
was it a size, a face, a position or a missing element".

    first-divergence.py ours.pdf ref.pdf
    first-divergence.py --corpus rows.tsv --root /workspace/sample-files --out div.tsv

The second form sweeps a track: one row per document, with the first divergent page and the
dominant kind of difference on it. Aggregating that column is the point — it answers "what do
these failures have in common", which no per-document reading can.

Page counts that differ are handled by comparing only the common prefix. `pdf-image-diff.py`
refuses a mismatched pair outright and is right to: page 3 against a different page 3 produces
a plausible and meaningless report.
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
IMAGE_DIFF = HERE / "pdf-image-diff.py"
OPS = HERE / "pdf-ops.py"

# A page has to differ by more than the shifted-by-antialiasing floor to count as the start of
# the divergence. `ink%` is the signed share of ink neither side accounts for; a reflow leaves
# it near nought and a genuinely different page does not.
INK_THRESHOLD = 0.35

NOTE_KINDS = ("size", "width", "face", "glyphs", "shows")


def run(cmd, **kw):
    return subprocess.run([str(c) for c in cmd], capture_output=True, text=True,
                          env=dict(os.environ, SOURCE_DATE_EPOCH="1700000000"), **kw)


def page_count(pdf: Path) -> int:
    m = re.search(r"^Pages:\s+(\d+)", run(["pdfinfo", pdf]).stdout, re.M)
    return int(m.group(1)) if m else 0


def prefix(src: Path, dst: Path, last: int) -> Path:
    run(["pdftocairo", "-pdf", "-f", 1, "-l", last, src, dst])
    return dst if dst.exists() else src


def per_page_ink(ours: Path, ref: Path, tmp: Path):
    """[(page, |ink|%, verdict)] over the common prefix, in page order."""
    no, nr = page_count(ours), page_count(ref)
    if not no or not nr:
        return [], no, nr
    common = min(no, nr)
    a = ours if no == common else prefix(ours, tmp / "a.pdf", common)
    b = ref if nr == common else prefix(ref, tmp / "b.pdf", common)
    out = run([IMAGE_DIFF, a, b, "--outdir", tmp / "cmp"]).stdout
    rows = []
    for line in out.splitlines():
        f = line.split("\t")
        if len(f) >= 6 and f[0].isdigit():
            try:
                rows.append((int(f[0]), float(f[3]), f[5]))
            except ValueError:
                pass
    return rows, no, nr


def classify(ours: Path, ref: Path, page: int):
    """What differs on one page, as counts by kind."""
    out = run([OPS, "diff", ours, ref, "--page", page]).stdout
    counts = {k: 0 for k in NOTE_KINDS}
    only_ours = only_ref = 0
    section = None
    for line in out.splitlines():
        if line.startswith("=== only in ours"):
            section = "ours"
        elif line.startswith("=== only in the reference"):
            section = "ref"
        elif line.startswith("=== drawn by both"):
            section = "both"
        elif line.strip().startswith(("text ", "fill ", "stroke", "image")):
            if section == "ours":
                only_ours += 1
            elif section == "ref":
                only_ref += 1
        else:
            for k in NOTE_KINDS:
                if re.search(rf"\b{k} [\d.]+ vs |\b{k} \S+ vs ", line):
                    counts[k] += 1
    dominant = max(counts, key=lambda k: counts[k]) if any(counts.values()) else ""
    if not dominant and (only_ours or only_ref):
        dominant = "one-sided"
    return counts, only_ours, only_ref, dominant


def first_text(pdf: Path, page: int, limit: int = 90) -> str:
    txt = run(["pdftotext", "-f", page, "-l", page, pdf, "-"]).stdout
    return " ".join(txt.split())[:limit]


def analyse(ours: Path, ref: Path):
    tmp = Path(tempfile.mkdtemp())
    try:
        rows, no, nr = per_page_ink(ours, ref, tmp)
        first = next((p for p, ink, _ in rows if ink >= INK_THRESHOLD), None)
        if first is None and no != nr:
            # every common page agrees, so the divergence is the page one side does not have
            first = min(no, nr) + 1
        result = {"pages_ours": no, "pages_ref": nr, "first": first,
                  "ink": next((i for p, i, _ in rows if p == first), 0.0),
                  "compared": len(rows)}
        if first and first <= min(no, nr):
            counts, oo, orr, dom = classify(ours, ref, first)
            result.update(counts=counts, only_ours=oo, only_ref=orr, dominant=dom,
                          text=first_text(ref, first))
        else:
            result.update(counts={}, only_ours=0, only_ref=0,
                          dominant="extra page" if first else "",
                          text=first_text(ref, min(first or 1, nr)) if nr else "")
        return result
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


def render_pair(src: Path, tmp: Path, cli: Path):
    (tmp / "o").mkdir(parents=True, exist_ok=True)
    (tmp / "r").mkdir(parents=True, exist_ok=True)
    run([cli, "render", "--outdir", tmp / "o", src])
    run(["soffice", "--headless", "--convert-to", "pdf", "--outdir", tmp / "r", src])
    o = next(iter(sorted((tmp / "o").glob("*.pdf"))), None)
    r = next(iter(sorted((tmp / "r").glob("*.pdf"))), None)
    return o, r


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("ours", nargs="?", type=Path)
    ap.add_argument("ref", nargs="?", type=Path)
    ap.add_argument("--corpus", type=Path, help="a batch-check rows.tsv; sweeps every row")
    ap.add_argument("--root", type=Path, default=Path("/workspace/sample-files"))
    ap.add_argument("--cli", type=Path,
                    default=Path("dotnet/tools/Paperless.Cli/bin/Debug/net10.0/linux-x64/Paperless.Cli"))
    ap.add_argument("--out", type=Path)
    args = ap.parse_args()

    if args.corpus:
        rows = [l.rstrip("\n").split("\t") for l in args.corpus.read_text().splitlines() if l.strip()]
        rows = [r for r in rows if r[0] != "path"]
        lines = ["path\tpages\tfirst_page\tof\tink\tdominant\tonly_ours\tonly_ref\ttext"]
        for i, r in enumerate(rows, 1):
            src = args.root / r[0]
            tmp = Path(tempfile.mkdtemp())
            try:
                o, rf = render_pair(src, tmp, args.cli)
                if not o or not rf:
                    print(f"  [{i}/{len(rows)}] {r[0]} — render failed", flush=True)
                    continue
                a = analyse(o, rf)
                lines.append("\t".join(str(x) for x in [
                    r[0], f"{a['pages_ours']}/{a['pages_ref']}", a["first"] or "",
                    min(a["pages_ours"], a["pages_ref"]), f"{a['ink']:.2f}",
                    a["dominant"], a["only_ours"], a["only_ref"], a["text"]]))
                print(f"  [{i}/{len(rows)}] {r[0]} first={a['first']} {a['dominant']}", flush=True)
            finally:
                shutil.rmtree(tmp, ignore_errors=True)
        text = "\n".join(lines)
        if args.out:
            args.out.write_text(text)
            print(f"wrote {args.out}")
        else:
            print(text)
        return 0

    if not args.ours or not args.ref:
        ap.error("give two PDFs, or --corpus")
    a = analyse(args.ours, args.ref)
    print(f"pages          {a['pages_ours']} ours / {a['pages_ref']} reference")
    print(f"compared       {a['compared']} common pages")
    if a["first"] is None:
        print("first diverge  none — every common page agrees")
        return 0
    print(f"first diverge  page {a['first']}  (|ink| {a['ink']:.2f}%)")
    print(f"dominant       {a['dominant']}")
    if a["counts"]:
        print("               " + ", ".join(f"{k} {v}" for k, v in a["counts"].items() if v))
    print(f"one-sided      {a['only_ours']} ours, {a['only_ref']} reference")
    if a["text"]:
        print(f"page text      {a['text']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
