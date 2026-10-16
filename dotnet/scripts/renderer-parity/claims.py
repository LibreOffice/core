#!/usr/bin/env python3
"""Check each reading's own quoted strings against the two PDFs.

A reading that says a phrase is missing names the phrase, in <em>. That is a
falsifiable claim and it does not depend on which LibreOffice produced the
reference: either our page's text layer holds those characters or it does not.

The third answer is the one that matters, and it is why this cannot stop at
`found`. Several of the templates in this corpus set white text over a
gradient the renderer failed to paint. The words are in the PDF, correctly
positioned, and invisible on white paper -- so `found` would refute a reading
that is in fact right. Every hit therefore also reports the fill colour the
text is drawn in and the colour actually behind it in the rasterised page.
"""
from __future__ import annotations

import html, json, pathlib, re, sys
import pymupdf

BENCH = pathlib.Path("/data/bench")
EM = re.compile(r"<em>(.*?)</em>", re.S)
# a reading only claims absence when it says so
ABSENT = re.compile(
    r"\b(missing|not drawn|are gone|is gone|dropped|absent|omitted|no marker|"
    r"leaves? (?:it|them|the band) empty|does not draw|not painted|loses?)\b", re.I)
WS = re.compile(r"\s+")


def norm(s: str) -> str:
    s = html.unescape(s)
    s = s.replace("’", "'").replace("‘", "'")
    s = s.replace("“", '"').replace("”", '"')
    s = s.replace("—", "-").replace("–", "-").replace("−", "-")
    s = s.replace(" ", " ").replace("­", "")
    return WS.sub(" ", s).strip().lower()


def page_text(doc, i):
    if doc is None or i >= doc.page_count:
        return ""
    return norm(doc.load_page(i).get_text("text"))


def find_span(doc, i, phrase):
    """Locate the phrase and report the colour it is drawn in and the colour behind it."""
    if doc is None or i >= doc.page_count:
        return None
    page = doc.load_page(i)
    rects = page.search_for(phrase[:80], quads=False)
    if not rects:
        return None
    r = rects[0]
    colour = None
    for blk in page.get_text("dict")["blocks"]:
        for ln in blk.get("lines", []):
            for sp in ln["spans"]:
                if pymupdf.Rect(sp["bbox"]).intersects(r):
                    colour = sp["color"]
                    break
            if colour is not None:
                break
        if colour is not None:
            break
    # what is actually behind it: rasterise a strip and take the modal pixel
    clip = pymupdf.Rect(r.x0 - 2, r.y0 - 2, r.x1 + 2, r.y1 + 2)
    pm = page.get_pixmap(dpi=72, clip=clip, alpha=False)
    px = pm.samples
    hist = {}
    for k in range(0, len(px), 3):
        t = (px[k], px[k + 1], px[k + 2])
        hist[t] = hist.get(t, 0) + 1
    if not hist:
        return None
    ground = max(hist, key=hist.get)
    g = sum(ground)
    # The pixel FARTHEST from the ground in either direction -- light text on a
    # dark ground is as visible as dark text on a light one, and an earlier cut
    # of this that only looked for the darkest pixel scored white-on-navy as
    # invisible, which is the opposite of the truth.
    far = max(hist, key=lambda t: abs(sum(t) - g))
    return {
        "rect": [round(v, 1) for v in (r.x0, r.y0, r.x1, r.y1)],
        "fill": None if colour is None else f"#{colour:06x}",
        "ground": ground, "farthest": far,
        "contrast": abs(sum(far) - g) // 3,
    }


def main():
    cases = json.loads((BENCH / "pl-cases.json").read_text())
    out = []
    for n, c in enumerate(cases, 1):
        text = c.get("analysis") or ""
        phrases = [p for p in (EM.findall(text)) if len(norm(p)) >= 4]
        if not phrases:
            continue
        claims_absence = bool(ABSENT.search(text))
        rp = BENCH / "lo" / c["id"] / "out.pdf"
        op = BENCH / "pl" / c["id"] / "out.pdf"
        rd = pymupdf.open(rp) if rp.exists() else None
        od = pymupdf.open(op) if op.exists() else None
        i = c["page"] - 1
        rt, ot = page_text(rd, i), page_text(od, i)
        # our whole document, for a phrase that merely moved
        owhole = " ".join(page_text(od, k) for k in range(min(od.page_count, 40))) if od else ""
        rows = []
        for p in phrases:
            q = norm(p)
            row = {"phrase": p, "in_ref_page": q in rt, "in_our_page": q in ot,
                   "in_our_doc": q in owhole}
            if row["in_our_page"]:
                row["where"] = find_span(od, i, html.unescape(p))
            rows.append(row)
        if rd: rd.close()
        if od: od.close()
        out.append({"rank": c["rank"], "id": c["id"], "page": c["page"],
                    "claims_absence": claims_absence, "quotes": rows})
        if n % 50 == 0:
            print(f"{n}/{len(cases)}", flush=True)
    (BENCH / "claims.json").write_text(json.dumps(out))
    print(f"wrote claims.json: {len(out)} cases carrying quoted strings, "
          f"{sum(len(r['quotes']) for r in out)} quotes")


if __name__ == "__main__":
    main()
