#!/usr/bin/env python3
"""Score a directory of our renderings against the banked reference PDFs.

    score-against-banked.py <ours-dir> [<ours-dir-2> ...]

The gate's own three checks, in `batch-check.sh`'s order and with its rules:

  1. page count exact;
  2. extractable words within max(2%, 3) of the reference's, where a word is a token
     carrying at least one Unicode letter or digit;
  3. no unembedded font in ours.

The reference half comes from /c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides rather than
from a fresh `soffice` run, which is the point: the references are banked and re-rendering
them would be measuring a different thing as well as costing 163 conversions.
"""
import os, subprocess, sys, collections

REF = "/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides"


def words(pdf):
    out = subprocess.run(["pdftotext", pdf, "-"], capture_output=True).stdout
    tokens = out.decode("utf-8", "replace").split()
    return sum(1 for t in tokens if any(c.isalnum() for c in t))


def pages(pdf):
    out = subprocess.run(["pdfinfo", pdf], capture_output=True, text=True).stdout
    for line in out.splitlines():
        if line.startswith("Pages:"):
            return int(line.split()[1])
    return -1


def fonts(pdf):
    out = subprocess.run(["pdffonts", pdf], capture_output=True, text=True).stdout
    rows = [r for r in out.splitlines()[2:] if r.strip()]
    unembedded = sum(1 for r in rows if len(r.split()) >= 8 and r.split()[-5] == "no")
    return len(rows), unembedded


def score(directory):
    verdicts = collections.Counter()
    rows = []
    for name in sorted(os.listdir(directory)):
        if not name.endswith(".pdf"):
            continue
        ours = os.path.join(directory, name)
        ref = os.path.join(REF, name)
        if not os.path.exists(ref):
            verdicts["ref-missing"] += 1
            continue
        op, rp = pages(ours), pages(ref)
        ow, rw = words(ours), words(ref)
        of, un = fonts(ours)
        rf, _ = fonts(ref)
        v = []
        if op != rp:
            v.append("pages")
        if rw > 0:
            d = abs(ow - rw)
            if d > rw * 0.02 and d > 3:
                v.append("words")
        elif ow > 3:
            v.append("words")
        if un:
            v.append("unembedded")
        verdict = ",".join(v) or "match"
        verdicts[verdict] += 1
        rows.append((name, op, rp, ow, rw, of, rf, un, verdict))
    return verdicts, rows


for directory in sys.argv[1:]:
    verdicts, rows = score(directory)
    total = sum(verdicts.values())
    print(f"== {directory}: {verdicts['match']} / {total} match")
    for v, n in sorted(verdicts.items()):
        if v != "match":
            print(f"     {n:3d}  {v}")
    with open(os.path.join(directory, "gate.tsv"), "w") as handle:
        handle.write("name\tpages\twords\tfonts\tunemb\tverdict\n")
        for name, op, rp, ow, rw, of, rf, un, verdict in rows:
            handle.write(f"{name}\t{op}/{rp}\t{ow}/{rw}\t{of}/{rf}\t{un}\t{verdict}\n")
