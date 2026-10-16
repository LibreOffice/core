#!/usr/bin/env python3
"""How much of a growing frame's text never reaches the page.

`xray-office extract` walks the content tree and never lays anything out, so it reports what a
document holds; the render reports what was drawn.  A frame taller than the room left on its
page draws its tail outside the sheet, where `pdftotext` cannot see it — so
render << extract is the signature of a frame that the reference splits and we do not.

  overflow.py <cli> <bank>
"""
import os, re, subprocess, sys

CLI = sys.argv[1]
BANK = sys.argv[2]
CORPUS = "/home/user/corpus-odf"
GROW = os.path.join(os.path.dirname(os.path.abspath(__file__)), "growing-frames.tsv")


def glyphs_of_text(t):
    return sum(1 for c in t if c.isalnum())


def pdf_glyphs(path):
    if not os.path.exists(path):
        return None
    t = subprocess.run(["pdftotext", path, "-"], capture_output=True).stdout
    return glyphs_of_text(t.decode("utf-8", "replace"))


rows = []
for line in open(GROW):
    if line.startswith("document"):
        continue
    rel = line.split("\t")[0]
    doc = os.path.join(CORPUS, rel)
    stem, ext = os.path.splitext(os.path.basename(doc))
    ident = f"{stem}__{ext[1:].lower()}"
    try:
        out = subprocess.run([CLI, "extract", doc], capture_output=True, timeout=300)
        extracted = glyphs_of_text(out.stdout.decode("utf-8", "replace"))
    except subprocess.TimeoutExpired:
        extracted = None
    ours = pdf_glyphs(os.path.join(BANK, "after", ident + ".pdf"))
    ref = pdf_glyphs(os.path.join(BANK, "ref", ident + ".pdf"))
    rows.append((rel, ours, ref, extracted))

print("document\tdrawn\treference\textracted\tdrawn_minus_extracted")
for rel, ours, ref, ex in rows:
    d = "" if (ours is None or ex is None) else str(ours - ex)
    print(f"{rel}\t{ours}\t{ref}\t{ex}\t{d}")

short = [r for r in rows if r[1] is not None and r[3] is not None and r[3] - r[1] > max(15, 0.02 * r[3])]
print(f"\n# documents drawing materially less than they hold: {len(short)} of {len(rows)}")
for r in short:
    print(f"#   {r[0]}\tdrawn {r[1]}\theld {r[3]}\treference {r[2]}")
