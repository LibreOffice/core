#!/usr/bin/env python3
"""Does a row's declared `w:trHeight` floor stop LibreOffice breaking the row across a page?

Two documents in this tree answer that question differently, and the whole point of this probe is
to keep both answers reproducible in one command, because each one alone is convincing and wrong.

  * `words/batch-006/docx/f445896eb008d14c1746fc37d412dc22.docx` — the floor plainly *does* stop it.
    Row 8 has 140.00 pt of room left on page 3 and a declared floor of 4965 twips (248.25 pt).
    LibreOffice leaves the 140 pt blank and moves the row whole. Lower that one number and nothing
    else, and LibreOffice breaks the row again: page 3 goes from 337 tokens to 432 — which is
    exactly what Paperless produces with the floor ignored. The flip is sharp, between **4250 and
    4300 twips**, and that boundary is *not* the room (2800 twips).

    Those two figures are `pdftotext` split on whitespace. `batch-check.sh` reads the same pages as
    323 and 416, because `wc -w` in the POSIX locale does not count a token made entirely of
    non-ASCII characters and this page holds sixteen of them. Same pages, same finding, two
    counters — quoted here so the difference is not read as the document moving.

  * `dotnet/tests/corpus/features/table-row-min-height.fodt` — the floor plainly does *not* stop it.
    Its middle row has about 100 pt of room and breaks at every declared floor from 4.8 cm to
    8.0 cm, the last of which is the entire body height of that page. There is no threshold at all.

So the gate is real, it is caused by the floor, and its condition is **not** "the room left is less
than the floor". Anything built on that comparison passes the corpus document and fails the
fixture — measured: it fails five tests in `Paperless.WordProcessing.Tests`, four of them the
fixture's own, and it is the third time the floor has been proposed as a bar on breaking.

What is already ruled out, so that nobody spends a round on it again:

  * *The floor being lost across the split.* It is not: on the corpus document nine rows break and
    every one of them already sums to more than its floor across its parts. LibreOffice charges the
    floor to the sum too — `lcl_CalcMinRowHeight` skips it for a row `IsInSplit()`
    (`sw/source/core/layout/tabfrm.cxx`:5087) and `lcl_calcHeightOfRowBeforeThisFrame` subtracts the
    earlier parts' heights for a follow (:5696), so both engines agree here.
  * *"The remaining size is less than the minimum row height, don't even try to split."* That branch
    exists (`SwTabFrame::Split`, :1188-1196) and is reached only for a table inside a splittable fly.
  * *The row being on a follow table rather than a master one.* Row 5 of the corpus document is on a
    follow table and breaks; row 8 is on a follow table and does not.
  * *A ratio between floor and room.* The corpus document flips at floor ≈ 1.52 × room; the fixture
    does not flip at 4.5 × room.

Usage:

    dotnet/probes/trheight-split-gate.py --outdir /abs/scratch/trheight [--corpus /workspace/sample-files]

It measures a **count** — how many words land on a page, and how many of a row's lines stay behind —
so it is not exposed to the hand-built-DOCX trap that invalidates a probe measuring a length.
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
import zipfile

DOC = "words/batch-006/docx/f445896eb008d14c1746fc37d412dc22.docx"
FIXTURE = "dotnet/tests/corpus/features/table-row-min-height.fodt"

# Row 8 is the one with 140.00 pt of room on page 3; its declared floor is 4965 twips.
ROW = 8
FLOORS = [2000, 4000, 4200, 4250, 4300, 4400, 4965]
HEIGHTS_CM = ["4.8", "5.2", "5.6", "6.0", "8.0"]


def convert(src: str, outdir: str, profile: str) -> str | None:
    """Convert one file with its own soffice profile, and decide success by the output existing."""
    stem = os.path.splitext(os.path.basename(src))[0]
    subprocess.run(
        ["soffice", f"-env:UserInstallation=file://{profile}", "--headless",
         "--convert-to", "pdf", "--outdir", outdir, src],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=300, check=False)
    out = os.path.join(outdir, f"{stem}.pdf")
    return out if os.path.exists(out) else None


def words_on(pdf: str, page: int) -> int:
    text = subprocess.run(["pdftotext", "-f", str(page), "-l", str(page), pdf, "-"],
                          capture_output=True, text=True).stdout
    return len(text.split())


def pages(pdf: str) -> int:
    info = subprocess.run(["pdfinfo", pdf], capture_output=True, text=True).stdout
    match = re.search(r"^Pages:\s+(\d+)", info, re.M)
    return int(match.group(1)) if match else 0


def corpus_variants(corpus: str, outdir: str, profile: str) -> None:
    src = os.path.join(corpus, DOC)
    with zipfile.ZipFile(src) as zin:
        names = zin.namelist()
        data = {name: zin.read(name) for name in names}

    document = data["word/document.xml"].decode("utf8")
    found = list(re.finditer(r'<w:trHeight w:val="(\d+)"/>', document))
    if len(found) <= ROW:
        sys.exit(f"{src}: expected more than {ROW} w:trHeight elements, found {len(found)}")

    at = found[ROW]
    print(f"{os.path.basename(src)} — row {ROW}, declared floor {at.group(1)} twips, "
          f"140.00 pt of room left on page 3")
    print("  floor(tw)   pt   pages   words on page 3   verdict")

    for floor in FLOORS:
        body = document[:at.start()] + f'<w:trHeight w:val="{floor}"/>' + document[at.end():]
        path = os.path.join(outdir, f"row{ROW}-{floor}.docx")
        with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zout:
            for name in names:
                zout.writestr(
                    name,
                    body.encode("utf8") if name == "word/document.xml" else data[name])

        pdf = convert(path, outdir, profile)
        if pdf is None:
            print(f"  {floor:>9}   {floor / 20:5.1f}   soffice produced nothing")
            continue

        third = words_on(pdf, 3)
        print(f"  {floor:>9}   {floor / 20:5.1f}   {pages(pdf):>5}   {third:>15}   "
              f"{'row 8 breaks' if third > 350 else 'row 8 moves whole'}")


def fixture_variants(root: str, outdir: str, profile: str) -> None:
    src = os.path.join(root, FIXTURE)
    text = open(src, encoding="utf8").read()
    print(f"\n{os.path.basename(src)} — middle row, about 100 pt of room left on page 1")
    print("  min-row-height   pages   floored lines kept on page 1")

    for height in HEIGHTS_CM:
        body = text.replace('style:min-row-height="5.2cm"', f'style:min-row-height="{height}cm"')
        path = os.path.join(outdir, f"min-{height}.fodt")
        open(path, "w", encoding="utf8").write(body)

        pdf = convert(path, outdir, profile)
        if pdf is None:
            print(f"  {height + 'cm':>14}   soffice produced nothing")
            continue

        first = subprocess.run(["pdftotext", "-f", "1", "-l", "1", pdf, "-"],
                               capture_output=True, text=True).stdout
        kept = first.count("Floored row left")
        print(f"  {height + 'cm':>14}   {pages(pdf):>5}   {kept:>27}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--outdir", required=True, help="absolute scratch directory")
    parser.add_argument("--corpus", default="/workspace/sample-files")
    parser.add_argument(
        "--root",
        default=os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))),
        help="the checkout holding dotnet/tests/corpus")
    args = parser.parse_args()

    if not os.path.isabs(args.outdir):
        sys.exit("--outdir must be absolute: a relative path lands inside the repository")

    shutil.rmtree(args.outdir, ignore_errors=True)
    profile = os.path.join(args.outdir, "profile")
    os.makedirs(profile, exist_ok=True)

    corpus_variants(args.corpus, args.outdir, profile)
    fixture_variants(args.root, args.outdir, profile)


if __name__ == "__main__":
    main()
