#!/usr/bin/env python3
"""Re-score the catalogue's 192 documents after a code change.

Same reference, same dpi, same metrics as the sweep -- only our side is
re-rendered. That is what makes the before/after difference attributable to
the change rather than to the environment: the LibreOffice half is the bank
of PDFs the sweep already produced, untouched, so no reference-version
question enters here at all.

Reports per document: the worst page's SSIM before and after, the page count
before and after, and whether the two renderings are byte-identical -- a
document the change did not reach should be, and one that moved should not.
"""
from __future__ import annotations

import hashlib, json, pathlib, sys
import numpy as np
import pymupdf
from PIL import Image

sys.path.insert(0, "/data/bench/scripts")
import metrics as M

BENCH = pathlib.Path("/data/bench")
DPI = 150
MAX_PAGES = 5


def raster(path, i):
    """One page as the Rec. 601 luma the metrics expect -- not RGB, and not a
    channel average, which scores saturated yellow as ink."""
    d = pymupdf.open(path)
    try:
        if i >= d.page_count: return None
        pm = d.load_page(i).get_pixmap(dpi=DPI, alpha=False)
        return M.to_gray(Image.frombytes("RGB", (pm.width, pm.height), pm.samples))
    finally:
        d.close()


def pages(path):
    d = pymupdf.open(path)
    try: return d.page_count
    finally: d.close()


def score(ref_pdf, our_pdf):
    n = min(pages(ref_pdf), pages(our_pdf), MAX_PAGES)
    worst = None
    for i in range(n):
        a, b = raster(ref_pdf, i), raster(our_pdf, i)
        if a is None or b is None: continue
        if a.shape != b.shape:
            b = M.resize_to(b, a.shape)
        m = M.compare(b, a)          # (ours, reference) -- compare()'s own order
        s = m.get("ssim")
        if s is not None and (worst is None or s < worst[0]):
            worst = (s, i + 1, m.get("mean_abs_error"))
    return worst, pages(our_pdf)


def main():
    cases = json.loads((BENCH / "pl-cases.json").read_text())
    out = []
    for n, c in enumerate(cases, 1):
        ref = BENCH / "lo" / c["id"] / "out.pdf"
        before = BENCH / "pl" / c["id"] / "out.pdf"
        after = BENCH / "pl-final" / c["id"] / "out.pdf"
        if not (ref.exists() and before.exists() and after.exists()):
            continue
        same = (hashlib.md5(before.read_bytes()).hexdigest()
                == hashlib.md5(after.read_bytes()).hexdigest())
        rec = {"rank": c["rank"], "id": c["id"], "name": c["name"],
               "tags": c["tags"], "identical": same,
               "ref_pages": c["ref_pages"], "pages_before": c["engine_pages"]}
        if same:
            rec["pages_after"] = c["engine_pages"]
            rec["ssim_before"] = rec["ssim_after"] = (c["worst"] or {}).get("ssim")
        else:
            w, p = score(ref, after)
            rec["pages_after"] = p
            rec["ssim_before"] = (c["worst"] or {}).get("ssim")
            rec["ssim_after"] = None if w is None else round(w[0], 4)
            rec["mae_after"] = None if w is None else round(w[2] or 0, 4)
        out.append(rec)
        if n % 40 == 0: print(f"{n}/{len(cases)}", flush=True)
    (BENCH / "rescore-final.json").write_text(json.dumps(out, indent=1))
    moved = [r for r in out if not r["identical"]]
    print(f"\n{len(out)} scored; {len(moved)} documents changed, {len(out)-len(moved)} byte-identical")


if __name__ == "__main__":
    main()
