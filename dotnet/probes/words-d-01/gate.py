#!/usr/bin/env python3
"""Render the words track with one CLI and score it against the stored 26.2.4.2 references.

    gate.py <cli> <outdir>

The reference half is not re-rendered: `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/` holds one
PDF per document under its per-format identity and `ref-baseline-all.tsv` holds the counts
taken from them. The three checks and the 2% word band are `batch-check.sh`'s, in its order.
Derived from `words-b-01/sweep.py`, which is derived from the same script.
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


def reference():
    got = {}
    for line in (REFS / "ref-baseline-all.tsv").read_text().splitlines():
        if line.startswith("#"):
            continue
        p = line.split("\t")
        if len(p) < 8 or p[0] != "words":
            continue
        try:
            got[p[1]] = (int(p[3]), int(p[4]), int(p[5]), int(p[6]))
        except ValueError:
            continue
    return got


def measure(pdf: Path):
    if not pdf.exists():
        return None
    info = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True).stdout
    m = re.search(r"^Pages:\s+(\d+)", info, re.M)
    text = subprocess.run(["pdftotext", str(pdf), "-"], capture_output=True, text=True).stdout
    fonts = subprocess.run(["pdffonts", str(pdf)], capture_output=True, text=True).stdout
    rows = fonts.splitlines()[2:]
    unemb = sum(1 for r in rows if len(r.split()) >= 8 and r.split()[-5] == "no")
    nfonts = sum(1 for r in rows if r.strip())
    return (int(m.group(1)) if m else 0, len(text.split()), unemb, nfonts)


def verdict(ours, ref) -> str:
    if ours is None:
        return "ours-failed"
    pages, words, unemb, _ = ours
    refpages, refwords, _, _ = ref
    bad = []
    if pages != refpages:
        bad.append("pages")
    d = abs(words - refwords)
    if refwords > 0:
        if d > refwords * 0.02 and d > 3:
            bad.append("words")
    elif words > 3:
        bad.append("words")
    if unemb:
        bad.append("unembedded")
    return ",".join(bad) or "match"


def render(cli: Path, files, outdir: Path, workers: int = 8) -> None:
    outdir.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")

    def one(job):
        i, src = job
        dest = outdir / f"{src.stem}__{src.suffix.lower().lstrip('.')}.pdf"
        if dest.exists():
            return
        tmp = outdir / f"t{i}"
        tmp.mkdir(parents=True, exist_ok=True)
        subprocess.run([str(cli), "render", str(src), "--format", "pdf", "--outdir", str(tmp)],
                       capture_output=True, timeout=900, env=env)
        made = tmp / (src.stem + ".pdf")
        if made.exists():
            made.rename(dest)
        for leftover in tmp.iterdir():
            leftover.unlink()
        tmp.rmdir()

    with ThreadPoolExecutor(max_workers=workers) as pool:
        list(pool.map(one, list(enumerate(files))))


def main() -> int:
    cli, out = Path(sys.argv[1]), Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)
    ref = reference()
    files = sorted(p for p in (CORPUS / "words").rglob("*")
                   if p.suffix.lower() in EXTS and p.is_file())
    print(f"{len(files)} documents, {len(ref)} reference rows, cli {cli}", flush=True)
    render(cli, files, out / "ours")

    lines = ["path\text\tpages\twords\tfonts\tunemb\tverdict"]
    match = pageerr = exact = worderr = failed = 0
    for src in files:
        key = f"words/{src.relative_to(CORPUS / 'words')}"
        entry = ref.get(key)
        if entry is None:
            print(f"NO REFERENCE ROW {key}", flush=True)
            continue
        ident = f"{src.stem}__{src.suffix.lower().lstrip('.')}.pdf"
        got = measure(out / "ours" / ident)
        v = verdict(got, entry)
        if v == "match":
            match += 1
        if got is None:
            failed += 1
        else:
            pageerr += abs(got[0] - entry[0])
            worderr += abs(got[1] - entry[1])
            if got[0] == entry[0]:
                exact += 1
        lines.append("\t".join([
            key, src.suffix.lower().lstrip('.'),
            f"{got[0] if got else '-'}/{entry[0]}",
            f"{got[1] if got else '-'}/{entry[1]}",
            f"{got[3] if got else '-'}/{entry[2]}",
            str(got[2] if got else '-'), v]))
    (out / "gate.tsv").write_text("\n".join(lines) + "\n")
    print(f"\nmatch {match}  |page error| {pageerr}  exact pages {exact}  "
          f"|word error| {worderr}  failed {failed}")
    print(f"TSV {out/'gate.tsv'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
