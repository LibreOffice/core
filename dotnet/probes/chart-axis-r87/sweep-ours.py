#!/usr/bin/env python3
"""Render our half of the gate and score it against a banked reference.

The round's diff is confined to `dotnet/src`, which cannot reach `soffice`, so the
reference half of the gate is the same bytes whatever this tree does — the rule
`dotnet/CLAUDE.md` records for `probes/overflow-r69/sweep-ours.sh`.  This renders
ours alone and applies `batch-check.sh`'s own verdict rule to the banked
reference's columns, so a row here is comparable with a row there.

Two things it copies from `batch-check.sh` deliberately rather than approximating:
the per-format identity (`report__xlsx`), because two documents differing only in
extension otherwise overwrite one another; and the `glyphs` metric with its
`max(2%, 15)` band, because a scorer written from the prose disagrees with the
scoreboard.

One thing it does NOT copy: the worker-slot output directory.  Each document gets
its own, keyed on its identity — `probes/chart-secaxis/sweep-parallel.py`'s form.

    sweep-ours.py --corpus ROOT --out DIR --cli PATH --ref-bank DIR \
                  --jobs 3 sheets/chartset-'*' slides/'*'
"""
import argparse
import concurrent.futures as futures
import shutil
import subprocess
import sys
from pathlib import Path

EXTS = {
    ".doc", ".docx", ".docm", ".dot", ".dotx", ".dotm", ".rtf", ".odt", ".ott",
    ".fodt", ".sxw", ".xls", ".xlsx", ".xlsm", ".xlsb", ".xlt", ".xltx", ".xltm",
    ".ods", ".ots", ".fods", ".csv", ".sxc", ".ppt", ".pptx", ".pptm", ".pot",
    ".potx", ".potm", ".ppsx", ".ppsm", ".pps", ".odp", ".otp", ".fodp", ".sxi",
}


def counts(pdf: Path):
    """pages, words, rawwords, glyphs, fonts, unembedded — batch-check.sh's columns."""
    if not pdf.is_file():
        return None
    info = subprocess.run(["pdfinfo", str(pdf)], capture_output=True, text=True).stdout
    pages = next((l.split()[1] for l in info.splitlines() if l.startswith("Pages")), "-")
    text = subprocess.run(["pdftotext", str(pdf), "-"],
                          capture_output=True).stdout.decode("utf-8", "replace")
    tokens = text.split()
    words = sum(1 for w in tokens if any(c.isalnum() for c in w))
    glyphs = sum(1 for c in text if c.isalnum())
    fonts_out = subprocess.run(["pdffonts", str(pdf)], capture_output=True, text=True).stdout
    rows = [r for r in fonts_out.splitlines()[2:] if r.strip()]
    unemb = sum(1 for r in rows if len(r.split()) >= 8 and r.split()[-5] == "no")
    return pages, words, len(tokens), glyphs, len(rows), unemb


def verdict(ours, ref):
    if ours is None and ref is None:
        return "both-failed"
    if ref is None:
        return "ref-failed"
    if ours is None:
        return "ours-failed"
    bad = []
    if ours[0] != ref[0]:
        bad.append("pages")
    og, rg = ours[3], ref[3]
    if rg > 0:
        if abs(og - rg) > rg * 0.02 and abs(og - rg) > 15:
            bad.append("words")
    elif og > 15:
        bad.append("words")
    if ours[5] != 0:
        bad.append("unembedded")
    return ",".join(bad) if bad else "match"


def enumerate_files(corpus: Path, globs):
    tracked = set()
    out = subprocess.run(
        ["git", "-C", str(corpus), "-c", "core.quotePath=false", "ls-files"],
        capture_output=True, text=True)
    for line in out.stdout.splitlines():
        tracked.add(str(corpus / line))

    best = {}
    for pattern in globs:
        for d in sorted(corpus.glob(pattern)):
            if not d.is_dir():
                continue
            for f in d.rglob("*"):
                if not f.is_file() or f.suffix.lower() not in EXTS:
                    continue
                st = f.stat()
                key = (st.st_dev, st.st_ino)
                path = str(f)
                if key not in best or (path in tracked and best[key] not in tracked):
                    best[key] = path
    return sorted(best.values())


def identity(path: str) -> str:
    name = Path(path).name
    stem, _, ext = name.rpartition(".")
    return f"{stem}__{ext.lower()}"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--corpus", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--cli", required=True)
    ap.add_argument("--ref-bank", required=True)
    ap.add_argument("--ref-cache", default=None)
    ap.add_argument("--jobs", type=int, default=3)
    ap.add_argument("globs", nargs="+")
    args = ap.parse_args()

    corpus = Path(args.corpus)
    out = Path(args.out)
    (out / "ours").mkdir(parents=True, exist_ok=True)
    bank = Path(args.ref_bank)

    cache = {}
    cache_path = Path(args.ref_cache) if args.ref_cache else out / "refcols.tsv"
    if cache_path.is_file():
        for line in cache_path.read_text().splitlines():
            parts = line.split("\t")
            cache[parts[0]] = None if parts[1] == "-" else tuple(
                [parts[1]] + [int(v) for v in parts[2:]])

    files = enumerate_files(corpus, args.globs)
    print(f"# {len(files)} documents", file=sys.stderr)

    rows = []
    new_ref = {}

    def work(path):
        ident = identity(path)
        work_dir = out / "work" / ident
        shutil.rmtree(work_dir, ignore_errors=True)
        work_dir.mkdir(parents=True, exist_ok=True)
        # SOURCE_DATE_EPOCH is deliberately NOT set: `batch-check.sh` sets it on neither
        # side, so a `&D` header prints today on both and cancels.  Setting it here alone
        # would print the epoch's date against a reference bank that printed today's.
        subprocess.run(
            ["timeout", "-k", "30", "240", args.cli, "render", path,
             "--format", "pdf", "--outdir", str(work_dir)],
            capture_output=True)
        produced = list(work_dir.glob("*.pdf"))
        ours_pdf = out / "ours" / f"{ident}.pdf"
        if produced:
            shutil.move(str(produced[0]), str(ours_pdf))
        shutil.rmtree(work_dir, ignore_errors=True)
        ours = counts(ours_pdf)
        if ident in cache:
            ref = cache[ident]
        else:
            ref = counts(bank / f"{ident}.pdf")
            new_ref[ident] = ref
        return path, ident, ours, ref

    with futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for path, ident, ours, ref in pool.map(work, files):
            v = verdict(ours, ref)
            rel = str(Path(path).relative_to(corpus))
            ext = Path(path).suffix.lower().lstrip(".")
            o = ours or ("-", "-", "-", "-", "-", "-")
            r = ref or ("-", "-", "-", "-", "-", "-")
            rows.append(f"{rel}\t{ext}\t{o[0]}/{r[0]}\t{o[1]}/{r[1]}\t{o[4]}/{r[4]}\t"
                        f"{o[5]}\t{v}\t{o[2]}/{r[2]}\t{o[3]}/{r[3]}")

    (out / "rows.tsv").write_text("\n".join(sorted(rows)) + "\n")
    if new_ref:
        with cache_path.open("a") as fh:
            for ident, ref in sorted(new_ref.items()):
                fh.write(ident + "\t" + ("-" if ref is None else
                                         "\t".join(str(v) for v in ref)) + "\n")

    tally = {}
    for row in rows:
        tally[row.split("\t")[6]] = tally.get(row.split("\t")[6], 0) + 1
    print("TOTAL " + str(len(rows)) + "  " +
          "  ".join(f"{k} {v}" for k, v in sorted(tally.items())))


if __name__ == "__main__":
    main()
