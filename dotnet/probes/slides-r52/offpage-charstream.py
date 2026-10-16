#!/usr/bin/env python3
"""Two instruments the word gate cannot see, run over a whole track's banked renderings.

1. OFF-PAGE TEXT.  A body autofitted to the wrong scale overflows its shape, and text that
   leaves the media box leaves the text layer with it.  r50 measured 30 of our renderings
   drawing text outside the page against the reference's 9; this is the same count, per leg.

2. CHARSTREAM IDENTITY.  All whitespace stripped from both pdftotext extractions and the
   remaining character multisets compared.  Same characters with a failing word count is a
   tokenisation ceiling; different characters is a real content or layout defect.
   r50's control stands: 138 of the 198 documents that PASS also differ, so character
   difference alone is not a classifier.

    offpage-charstream.py <ours-dir> <ref-dir> [label]
"""
import collections, glob, os, subprocess, sys, re

ours_dir, ref_dir = sys.argv[1], sys.argv[2]
label = sys.argv[3] if len(sys.argv) > 3 else os.path.basename(ours_dir.rstrip("/"))

BBOX = re.compile(r'<word xMin="([-\d.]+)" yMin="([-\d.]+)" xMax="([-\d.]+)" yMax="([-\d.]+)"')
PAGE = re.compile(r'<page width="([\d.]+)" height="([\d.]+)"')


def text(pdf):
    try:
        return subprocess.run(["pdftotext", pdf, "-"], capture_output=True, timeout=120).stdout
    except Exception:
        return b""


def offpage(pdf):
    """Words whose box falls outside the media box, and the pages holding them."""
    try:
        out = subprocess.run(["pdftotext", "-bbox", pdf, "-"],
                             capture_output=True, timeout=120).stdout.decode("utf-8", "replace")
    except Exception:
        return 0, 0
    words = pages = 0
    cur_w = cur_h = 0.0
    seen = set()
    page_no = 0
    for line in out.splitlines():
        m = PAGE.search(line)
        if m:
            cur_w, cur_h = float(m.group(1)), float(m.group(2))
            page_no += 1
            continue
        m = BBOX.search(line)
        if not m or not cur_w:
            continue
        x0, y0, x1, y1 = (float(g) for g in m.groups())
        if x1 < -1 or y1 < -1 or x0 > cur_w + 1 or y0 > cur_h + 1:
            words += 1
            seen.add(page_no)
    return words, len(seen)


rows = []
for o in sorted(glob.glob(os.path.join(ours_dir, "*.pdf"))):
    ident = os.path.basename(o)[:-4]
    r = os.path.join(ref_dir, ident + ".pdf")
    if not os.path.exists(r):
        continue
    ow, op = offpage(o)
    rw, rp = offpage(r)
    oc = collections.Counter(b"".join(text(o).split()).decode("utf-8", "replace"))
    rc = collections.Counter(b"".join(text(r).split()).decode("utf-8", "replace"))
    same = oc == rc
    inter = sum((oc & rc).values())
    union = sum((oc | rc).values())
    rows.append((ident, ow, op, rw, rp, same, inter / union if union else 1.0))

print(f"# {label}: {len(rows)} documents")
print("ident\tours_offpage_words\tours_offpage_pages\tref_offpage_words\tref_offpage_pages\tchars_identical\tjaccard")
for r in rows:
    print(f"{r[0]}\t{r[1]}\t{r[2]}\t{r[3]}\t{r[4]}\t{int(r[5])}\t{r[6]:.4f}")

odocs = sum(1 for r in rows if r[1])
rdocs = sum(1 for r in rows if r[3])
print(f"\n# documents with ANY off-page word: ours {odocs}, reference {rdocs}")
print(f"# total off-page words: ours {sum(r[1] for r in rows)}, reference {sum(r[3] for r in rows)}")
print(f"# character-identical documents: {sum(1 for r in rows if r[5])} of {len(rows)}")
