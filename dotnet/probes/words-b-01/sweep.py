#!/usr/bin/env python3
"""Render the words track with two CLIs and score both against the stored 26.2.4.2 references.

    sweep.py <before-cli> <after-cli> <outdir>

The reference half is not re-rendered: `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/` holds
one PDF per document under its per-format identity, and `ref-baseline-all.tsv` holds the
page, word, font and unembedded counts taken from them.  The three checks and the 2% word
band are `batch-check.sh`'s, in its order.
"""
from __future__ import annotations

import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

CORPUS = Path("/c/sandbox/workdir/sample-files")
REFS = Path("/c/sandbox/workdir/refpdfs-26.2.4.2-fonts")
EXTS = {".doc", ".docx", ".rtf", ".odt", ".ott"}


def reference() -> dict[str, tuple[int, int, int, int]]:
    got: dict[str, tuple[int, int, int, int]] = {}
    for line in (REFS / "ref-baseline-all.tsv").read_text().splitlines():
        if line.startswith("#"):
            continue
        parts = line.split("\t")
        if len(parts) < 8 or parts[0] != "words":
            continue
        try:
            got[parts[1]] = (int(parts[3]), int(parts[4]), int(parts[5]), int(parts[6]))
        except ValueError:
            continue
    return got


def measure(pdf: Path) -> tuple[int, int, int] | None:
    if not pdf.exists():
        return None
    info = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True).stdout
    m = re.search(r"^Pages:\s+(\d+)", info, re.M)
    text = subprocess.run(["pdftotext", str(pdf), "-"], capture_output=True, text=True).stdout
    fonts = subprocess.run(["pdffonts", str(pdf)], capture_output=True, text=True).stdout
    rows = fonts.splitlines()[2:]
    unembedded = sum(1 for r in rows if len(r.split()) >= 8 and r.split()[-5] == "no")
    return (int(m.group(1)) if m else 0, len(text.split()), unembedded)


def verdict(ours, ref) -> str:
    if ours is None:
        return "ours-failed"
    pages, words, unembedded = ours
    refpages, refwords, _, _ = ref
    bad = []
    if pages != refpages:
        bad.append("pages")
    delta = abs(words - refwords)
    if refwords > 0:
        if delta > refwords * 0.02 and delta > 3:
            bad.append("words")
    elif words > 3:
        bad.append("words")
    if unembedded:
        bad.append("unembedded")
    return ",".join(bad) or "match"


def render(cli: Path, files: list[Path], outdir: Path, workers: int = 10) -> None:
    outdir.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")

    def one(job):
        index, src = job
        tmp = outdir / f"t{index % workers}-{index}"
        tmp.mkdir(parents=True, exist_ok=True)
        subprocess.run([str(cli), "render", str(src), "--format", "pdf", "--outdir", str(tmp)],
                       capture_output=True, timeout=600, env=env)
        made = tmp / (src.stem + ".pdf")
        if made.exists():
            made.rename(outdir / f"{src.stem}__{src.suffix.lower().lstrip('.')}.pdf")
        for leftover in tmp.iterdir():
            leftover.unlink()
        tmp.rmdir()

    with ThreadPoolExecutor(max_workers=workers) as pool:
        list(pool.map(one, list(enumerate(files))))


def main() -> int:
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    before, after, out = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
    out.mkdir(parents=True, exist_ok=True)
    ref = reference()

    files = sorted(p for p in (CORPUS / "words").rglob("*")
                   if p.suffix.lower() in EXTS and p.is_file())
    print(f"{len(files)} words documents, {len(ref)} reference rows", flush=True)

    rows = {}
    for tag, cli in (("before", before), ("after", after)):
        print(f"rendering {tag}: {cli}", flush=True)
        render(cli, files, out / tag)
        rows[tag] = {}
        for src in files:
            key = str(src.relative_to(CORPUS.parent)).replace("sample-files/", "")
            key = f"words/{src.relative_to(CORPUS / 'words')}"
            entry = ref.get(key)
            if entry is None:
                continue
            got = measure(out / tag / f"{src.stem}__{src.suffix.lower().lstrip('.')}.pdf")
            rows[tag][key] = (got, verdict(got, entry), entry)

    lines = ["path\tbefore\tafter\tbeforepages/ref\tafterpages/ref"]
    changed = moved = 0
    summary = {}
    for tag in ("before", "after"):
        got = rows[tag]
        summary[tag] = dict(
            match=sum(1 for v in got.values() if v[1] == "match"),
            pageerror=sum(abs((v[0][0] if v[0] else 0) - v[2][0]) for v in got.values()),
            exact=sum(1 for v in got.values() if v[0] and v[0][0] == v[2][0]),
            worderror=sum(abs((v[0][1] if v[0] else 0) - v[2][1]) for v in got.values()),
            failed=sum(1 for v in got.values() if v[0] is None))
    for key in sorted(rows["before"]):
        b, a = rows["before"][key], rows["after"].get(key)
        if a is None:
            continue
        if b[0] != a[0]:
            changed += 1
            if not b[0] or not a[0] or b[0][0] != a[0][0]:
                moved += 1
            lines.append(f"{key}\t{b[1]}\t{a[1]}\t{b[0][0] if b[0] else '-'}/{b[2][0]}\t"
                         f"{a[0][0] if a[0] else '-'}/{a[2][0]}")

    (out / "changed.tsv").write_text("\n".join(lines) + "\n")
    print()
    for tag in ("before", "after"):
        s = summary[tag]
        print(f"{tag:>7}: match {s['match']:>3}  |page error| {s['pageerror']:>4}  "
              f"exact pages {s['exact']:>3}  |word error| {s['worderror']:>6}  "
              f"failed {s['failed']}")
    print(f"\nrenderings changed: {changed} of {len(rows['before'])}; "
          f"of those, page count moved on {moved}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
