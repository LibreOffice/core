#!/usr/bin/env python3
"""Compare two renderings a *line* at a time, and say what the first difference is.

    line-anatomy.py ours.pdf ref.pdf [--page N] [--all]
    line-anatomy.py --corpus rows.tsv --root /workspace/sample-files --cli … --out anat.tsv

WHY NOT `pdf-ops.py diff`
─────────────────────────
`pdf-ops.py` pairs **show operators** by position. That is the right unit for asking "is this
fill the same colour", and the wrong unit for asking anything about text, because the two
renderers do not agree on how a line is cut into operators: LibreOffice writes one show per
*character* on printer-metric text (`shows 1 vs 77` is routine) and starts a new operator
wherever it resets kerning. A line identical on both sides therefore pairs one of our records
against one of seventy-seven of theirs, and the leftovers land in the one-sided lists.

Measured, and this is why the file exists: over **thirty documents that match the corpus gate
exactly** — same page count, same word count, fonts embedded — `pdf-ops.py`'s `glyphs a vs b`
note fires on **26 of 30**, is the *dominant* difference on **22 of 30**, and the worst
per-page glyph delta is **20 or more on 15 of 30**, reaching 97. Those pages agree. A signature
that appears on three quarters of the documents with nothing wrong with them cannot rank the
documents with something wrong with them, and a round was spent treating it as the track's
largest cluster.

WHAT THIS DOES INSTEAD
──────────────────────
It asks poppler for the lines — `pdftotext -bbox-layout`, which has poppler's own decoder and
line assembly behind it — and aligns the two line lists by their text. That makes the unit of
comparison the thing the layout engine actually decides:

    text        a line of words one side draws and the other does not
    linebreak   the same words, cut into lines differently
    advance     identical words on a line, ending at a different x
    indent      identical words on a line, starting at a different x
    pitch       identical lines, drifting down the page
    rule        a stroke, fill or image one side draws and the other does not

`advance` and `linebreak` are the two the corpus keeps confusing, and they are different
findings: `advance` is a metric fault that has not yet moved a break, `linebreak` is one that
has. Reporting them apart is the point.
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ElementTree
from pathlib import Path

HERE = Path(__file__).resolve().parent
OPS = HERE / "pdf-ops.py"

# How far a matched line's edge may move before it counts. Two rasterisers agree to well
# under a tenth of a point on a shared advance; a real metric difference on a full line is
# whole points. 1.0 pt sits in the gap and was chosen by measuring matching documents.
EDGE_TOLERANCE = 1.0

# A non-text record pairs with the nearest one inside this box, in points. Same reasoning as
# `pdf-ops.py`'s window; a rule that is genuinely absent is nowhere near anything.
RULE_TOLERANCE = 3.0


def run(cmd):
    return subprocess.run([str(c) for c in cmd], capture_output=True, text=True,
                          env=dict(os.environ, SOURCE_DATE_EPOCH="1700000000"))


def page_count(pdf: Path) -> int:
    m = re.search(r"^Pages:\s+(\d+)", run(["pdfinfo", pdf]).stdout, re.M)
    return int(m.group(1)) if m else 0


def lines_of(pdf: Path, page: int):
    """[(x0, y0, x1, y1, text)] for one page, in reading order as poppler assembles it."""
    out = run(["pdftotext", "-bbox-layout", "-f", page, "-l", page, pdf, "-"]).stdout
    try:
        root = ElementTree.fromstring(out)
    except ElementTree.ParseError:
        return []
    ns = "{http://www.w3.org/1999/xhtml}"
    rows = []
    for line in root.iter(f"{ns}line"):
        words = [(w.text or "") for w in line.iter(f"{ns}word")]
        text = " ".join(w for w in words if w)
        if not text.strip():
            continue
        rows.append((float(line.get("xMin", 0)), float(line.get("yMin", 0)),
                     float(line.get("xMax", 0)), float(line.get("yMax", 0)), text))
    return rows


_MARKS: dict[Path, dict[int, list]] = {}


def marks_of(pdf: Path, page: int):
    """Non-text records on one page, as (kind, x0, y0, x1, y1, colour).

    The whole document is dumped once and indexed. Dumping per page re-parses the file
    every time, which is quadratic on the long documents this track is full of — one is
    721 pages.
    """
    if pdf not in _MARKS:
        index: dict[int, list] = {}
        for line in run([OPS, "dump", pdf]).stdout.splitlines():
            m = re.match(r"^(fill|stroke|image)\s+p(\d+)\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-"
                         r"\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(\S+)", line)
            if m:
                index.setdefault(int(m.group(2)), []).append(
                    (m.group(1), float(m.group(3)), float(m.group(4)),
                     float(m.group(5)), float(m.group(6)), m.group(7)))
        _MARKS[pdf] = index
    return _MARKS[pdf].get(page, [])


def pair_marks(ours, ref):
    """(only_ours, only_ref) counts, pairing each mark with its nearest counterpart."""
    taken = set()
    only_ours = 0
    for a in ours:
        best, bd = None, RULE_TOLERANCE
        for i, b in enumerate(ref):
            if i in taken or b[0] != a[0]:
                continue
            d = max(abs(a[1] - b[1]), abs(a[2] - b[2]), abs(a[3] - b[3]), abs(a[4] - b[4]))
            if d <= bd:
                best, bd = i, d
        if best is None:
            only_ours += 1
        else:
            taken.add(best)
    return only_ours, len(ref) - len(taken)


def normalise(text: str) -> str:
    """A line's text with every space removed.

    Poppler re-infers word boundaries from geometry, so the *same* line comes back with
    different spacing from two renderers whenever their advances differ slightly — which is
    the thing being measured, not a difference in content. Measured over 59 matching
    documents: comparing space-preserving text reported one-sided lines on 34 of them, almost
    all of them a single line differing by one space.
    """
    return "".join(text.split())


def compare_page(ours_pdf: Path, ref_pdf: Path, page: int):
    # Sorted by position rather than left in poppler's reading order. Poppler orders by block,
    # and a page with text frames on it is blocked differently from the two files — measured on
    # `FAA-High-Level-Org-Chart.docx`, which matches the gate and reported 25 of its 38 lines
    # one-sided purely because the two orderings interleave.
    a = sorted(lines_of(ours_pdf, page), key=lambda r: (round(r[1], 0), round(r[0], 0)))
    b = sorted(lines_of(ref_pdf, page), key=lambda r: (round(r[1], 0), round(r[0], 0)))
    ka = [normalise(r[4]) for r in a]
    kb = [normalise(r[4]) for r in b]

    # Matched by text and nearest y, not by sequence position. A page holding text frames is
    # blocked differently by poppler from the two files, so the two line lists interleave rather
    # than align — measured on `FAA-High-Level-Org-Chart.docx`, which matches the gate and
    # reported 25 of its 38 lines one-sided under a positional alignment.
    pool: dict[str, list[int]] = {}
    for j, key in enumerate(kb):
        pool.setdefault(key, []).append(j)

    matched = []
    unmatched_ours, taken = [], set()
    for i, key in enumerate(ka):
        candidates = pool.get(key)
        if not candidates:
            unmatched_ours.append(i)
            continue
        j = min(candidates, key=lambda j: abs(a[i][1] - b[j][1]))
        candidates.remove(j)
        taken.add(j)
        matched.append((a[i], b[j]))
    unmatched_ref = [j for j in range(len(kb)) if j not in taken]

    # What is left over is a rewrap when the two sides hold the same characters in a different
    # cut, and content otherwise. The two are different findings and the corpus is full of both.
    left_ours = "".join(ka[i] for i in unmatched_ours)
    left_ref = "".join(kb[j] for j in unmatched_ref)
    if left_ours and left_ours == left_ref:
        rewrapped = max(len(unmatched_ours), len(unmatched_ref))
        only_ours = only_ref = 0
    else:
        rewrapped = 0
        only_ours, only_ref = len(unmatched_ours), len(unmatched_ref)

    # Matched lines: how far their edges moved. dy is taken against the page's *median* dy so
    # that a whole-page offset — which is a margin, not a per-line fault — does not read as one.
    dy_all = [x[1] - y[1] for x, y in matched]
    median_dy = sorted(dy_all)[len(dy_all) // 2] if dy_all else 0.0
    worst_x0 = worst_x1 = worst_dy = 0.0
    n_x0 = n_x1 = n_dy = 0
    for x, y in matched:
        d0, d1 = x[0] - y[0], x[2] - y[2]
        dy = (x[1] - y[1]) - median_dy
        if abs(d0) > abs(worst_x0):
            worst_x0 = d0
        if abs(d1) > abs(worst_x1):
            worst_x1 = d1
        if abs(dy) > abs(worst_dy):
            worst_dy = dy
        n_x0 += abs(d0) > EDGE_TOLERANCE
        n_x1 += abs(d1) > EDGE_TOLERANCE
        n_dy += abs(dy) > EDGE_TOLERANCE

    result = {
        "lines_ours": len(a), "lines_ref": len(b), "matched": len(matched),
        "text_only_ours": only_ours, "text_only_ref": only_ref, "rewrapped": rewrapped,
        "n_x0": n_x0, "n_x1": n_x1, "n_dy": n_dy,
        "worst_x0": worst_x0, "worst_x1": worst_x1, "worst_dy": worst_dy,
        "median_dy": median_dy, "rule_only_ours": 0, "rule_only_ref": 0,
    }
    # The marks are only consulted when nothing about the text differs, because `rule` is the
    # last verdict in the order anyway and reading them costs a content-stream parse.
    if not verdict(result):
        mo, mr = pair_marks(marks_of(ours_pdf, page), marks_of(ref_pdf, page))
        result["rule_only_ours"], result["rule_only_ref"] = mo, mr
    return result


def verdict(r):
    """The dominant difference, as one word. Order is by how much each explains the rest."""
    if r["text_only_ours"] or r["text_only_ref"]:
        return "text"
    if r["rewrapped"]:
        return "linebreak"
    if r["n_x1"]:
        return "advance"
    if r["n_x0"]:
        return "indent"
    if r["n_dy"]:
        return "pitch"
    if r["rule_only_ours"] or r["rule_only_ref"]:
        return "rule"
    return ""


def first_divergent(ours: Path, ref: Path, limit: int):
    for p in range(1, limit + 1):
        r = compare_page(ours, ref, p)
        v = verdict(r)
        if v:
            r["page"] = p
            r["verdict"] = v
            return r
    return None


def describe(r):
    print(f"page          {r['page']}")
    print(f"verdict       {r['verdict']}")
    print(f"lines         {r['lines_ours']} ours / {r['lines_ref']} reference, "
          f"{r['matched']} aligned, {r['rewrapped']} rewrapped")
    print(f"one-sided     {r['text_only_ours']} lines ours, {r['text_only_ref']} reference")
    print(f"line start    {r['n_x0']} past {EDGE_TOLERANCE}pt, worst {r['worst_x0']:+.2f}")
    print(f"line end      {r['n_x1']} past {EDGE_TOLERANCE}pt, worst {r['worst_x1']:+.2f}")
    print(f"line top      {r['n_dy']} past {EDGE_TOLERANCE}pt, worst {r['worst_dy']:+.2f} "
          f"(page offset {r['median_dy']:+.2f})")
    print(f"marks         {r['rule_only_ours']} ours only, {r['rule_only_ref']} reference only")


FIELDS = ["path", "pages", "page", "verdict", "lines_ours", "lines_ref", "matched",
          "text_only_ours", "text_only_ref", "rewrapped", "n_x0", "n_x1", "n_dy",
          "worst_x0", "worst_x1", "worst_dy", "median_dy", "rule_only_ours", "rule_only_ref"]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("ours", nargs="?", type=Path)
    ap.add_argument("ref", nargs="?", type=Path)
    ap.add_argument("--page", type=int)
    ap.add_argument("--corpus", type=Path)
    ap.add_argument("--root", type=Path, default=Path("/workspace/sample-files"))
    ap.add_argument("--cli", type=Path,
                    default=Path("dotnet/tools/Paperless.Cli/bin/Debug/net10.0/linux-x64/Paperless.Cli"))
    ap.add_argument("--out", type=Path)
    ap.add_argument("--only", choices=("match", "fail"),
                    help="restrict a corpus sweep to rows with that verdict")
    args = ap.parse_args()

    if args.corpus:
        rows = [l.split("\t") for l in args.corpus.read_text().splitlines() if l.strip()]
        rows = [r for r in rows if r[0] != "path"]
        if args.only:
            rows = [r for r in rows if (r[6] == "match") == (args.only == "match")]
        lines = ["\t".join(FIELDS)]
        for i, row in enumerate(rows, 1):
            tmp = Path(tempfile.mkdtemp())
            try:
                (tmp / "o").mkdir(); (tmp / "r").mkdir()
                run([args.cli, "render", "--outdir", tmp / "o", args.root / row[0]])
                run(["soffice", "--headless", f"-env:UserInstallation=file://{tmp}/prof",
                     "--convert-to", "pdf", "--outdir", tmp / "r", args.root / row[0]])
                o = next(iter(sorted((tmp / "o").glob("*.pdf"))), None)
                rf = next(iter(sorted((tmp / "r").glob("*.pdf"))), None)
                if not o or not rf:
                    print(f"  [{i}/{len(rows)}] {row[0]} — render failed", flush=True)
                    continue
                no, nr = page_count(o), page_count(rf)
                r = first_divergent(o, rf, min(no, nr))
                if r is None:
                    r = {k: 0 for k in FIELDS}
                    r["page"] = ""
                    r["verdict"] = "extra page" if no != nr else ""
                r["path"] = row[0]
                r["pages"] = f"{no}/{nr}"
                lines.append("\t".join(
                    f"{r[k]:.2f}" if isinstance(r.get(k), float) else str(r.get(k, ""))
                    for k in FIELDS))
                print(f"  [{i}/{len(rows)}] {row[0]} page={r['page']} {r['verdict']}", flush=True)
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
    no, nr = page_count(args.ours), page_count(args.ref)
    print(f"pages         {no} ours / {nr} reference")
    if args.page:
        r = compare_page(args.ours, args.ref, args.page)
        r["page"] = args.page
        r["verdict"] = verdict(r) or "-"
        describe(r)
        return 0
    r = first_divergent(args.ours, args.ref, min(no, nr))
    if r is None:
        print("first diverge none — every common page agrees line for line")
        return 0
    describe(r)
    return 0


if __name__ == "__main__":
    sys.exit(main())
