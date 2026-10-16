#!/usr/bin/env python3
"""Find text our renderer draws and then paints over.

A defect the pixel metrics can only ever report as `content missing`: the
glyphs are in the content stream, correctly positioned, and a fill emitted
later in the same stream covers them. The reference draws the same text
visibly, so the divergence is a paint ORDER divergence, not a missing-content
one -- and that is a different fix in a different place.

Two independent conditions must both hold before a block is counted, which is
the corroboration rule the rest of this harness runs on:

  1. STREAM ORDER  -- an opaque `re ... f` whose rectangle contains the text
     block's anchor appears at a later byte offset than the block itself.
  2. RENDERED FACT -- rasterising that patch of the page yields a single
     uniform colour, so nothing of the text survives to the paper.

Either alone is a false positive generator: fills are painted after text all
the time without hiding it, and a uniform patch can simply be an empty margin.

Version-independent: it reads only our own output. The reference is consulted
solely to confirm the text is visible there, which is what makes it a defect
rather than a property of the document.
"""
from __future__ import annotations

import json, pathlib, re, sys
import pymupdf

BENCH = pathlib.Path("/data/bench")
NUM = r'(-?[\d.]+)'
FILL = re.compile(NUM + r'\s+' + NUM + r'\s+' + NUM + r'\s+' + NUM +
                  r'\s+re\s*[\r\n ]*(f\*?|B|b)\b')
TM = re.compile((NUM + r'\s+') * 5 + NUM + r'\s+Tm')
TD = re.compile(NUM + r'\s+' + NUM + r'\s+Td')
BLOCK = re.compile(r'BT\b(.*?)\bET\b', re.S)
FLAT = 28          # a patch whose extremes differ by less than this shows nothing


def anchors(raw):
    for m in BLOCK.finditer(raw):
        body = m.group(1)
        t = TM.search(body)
        if t:
            yield m.start(), (float(t.group(5)), float(t.group(6)))
            continue
        d = TD.search(body)
        if d:
            yield m.start(), (float(d.group(1)), float(d.group(2)))


def fills(raw):
    out = []
    for m in FILL.finditer(raw):
        x, y, w, h = map(float, m.groups()[:4])
        r = pymupdf.Rect(x, y, x + w, y + h)
        r.normalize()
        if r.get_area() > 4:
            out.append((m.start(), r))
    return out


def flat(page, pt, pad=6.0):
    clip = pymupdf.Rect(pt[0] - pad, pt[1] - pad, pt[0] + pad, pt[1] + pad) & page.rect
    if clip.is_empty:
        return None
    pm = page.get_pixmap(dpi=110, clip=clip, alpha=False)
    s = pm.samples
    lo = hi = sum(s[0:3])
    for k in range(0, len(s), 3):
        v = s[k] + s[k + 1] + s[k + 2]
        lo = min(lo, v); hi = max(hi, v)
    return (hi - lo) // 3


def scan(pdf, page_no):
    d = pymupdf.open(pdf)
    try:
        if page_no > d.page_count:
            return None
        pg = d.load_page(page_no - 1)
        raw = pg.read_contents().decode("latin-1", errors="replace")
        fl = fills(raw)
        H = pg.rect.height
        hidden = []
        for off, (x, y) in anchors(raw):
            pt = pymupdf.Point(x, y)
            over = [f for f in fl if f[0] > off and f[1].contains(pt)]
            if not over:
                continue
            # device space for rasterising: PDF y grows upward
            c = flat(pg, (x, H - y))
            if c is not None and c < FLAT:
                hidden.append({"offset": off, "at": [round(x), round(y)],
                               "cover_at": over[0][0], "contrast": c})
        return hidden
    finally:
        d.close()


def main():
    cases = json.loads((BENCH / "pl-cases.json").read_text())
    out = []
    for n, c in enumerate(cases, 1):
        ours = scan(BENCH / "pl" / c["id"] / "out.pdf", c["page"])
        if not ours:
            continue
        # the reference must show text there, or it is not a divergence
        ref = scan(BENCH / "lo" / c["id"] / "out.pdf", c["page"])
        out.append({"rank": c["rank"], "id": c["id"], "page": c["page"],
                    "name": c["name"], "tags": c["tags"],
                    "hidden_blocks": len(ours),
                    "ref_hidden_blocks": 0 if ref is None else len(ref),
                    "detail": ours[:6]})
        if n % 40 == 0:
            print(f"{n}/{len(cases)}", flush=True)
    (BENCH / "zorder.json").write_text(json.dumps(out))
    ours_only = [r for r in out if r["ref_hidden_blocks"] < r["hidden_blocks"]]
    print(f"\n{len(out)} of {len(cases)} cases hide text under a later fill")
    print(f"{len(ours_only)} of those hide MORE than the reference does -- our defect")
    for r in sorted(ours_only, key=lambda r: -r["hidden_blocks"]):
        print(f"  #{r['rank']:03d}  {r['hidden_blocks']:3d} blocks hidden "
              f"(ref {r['ref_hidden_blocks']})  {r['name'][:46]}")


if __name__ == "__main__":
    main()
