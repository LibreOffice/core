#!/usr/bin/env python3
"""Score both candidate renderers against the LibreOffice reference.

Rasterises the reference and the X-Ray.Office PDF at the same DPI, loads the WASM
viewer's captured PNGs, resizes anything that does not already match the
reference page's pixel box, and writes one JSON per document.

Deliberately per-document files: the pass is resumable, and 946 documents times
five pages times three engines is too much to hold or redo.
"""
from __future__ import annotations

import argparse, csv, json, pathlib, sys, traceback
import numpy as np
import fitz                                   # PyMuPDF
from PIL import Image

sys.path.insert(0, "/data/bench/scripts")
import metrics as M

DPI = 150
MAX_PAGES = 5
BENCH = pathlib.Path("/data/bench")
OUTDIR = BENCH / "scores"


def pdf_page_gray(doc: "fitz.Document", index: int) -> np.ndarray:
    pm = doc.load_page(index).get_pixmap(dpi=DPI, colorspace=fitz.csGRAY, alpha=False)
    return np.frombuffer(pm.samples, dtype=np.uint8).reshape(pm.height, pm.width).copy()


def png_gray(path: pathlib.Path) -> np.ndarray:
    with Image.open(path) as im:
        return M.to_gray(im)


def engine_pages_pl(rid: str):
    pdf = BENCH / "pl" / rid / "out.pdf"
    if not pdf.exists():
        return None, 0
    try:
        doc = fitz.open(pdf)
    except Exception:
        return None, 0
    return doc, doc.page_count


def score_document(row: dict) -> dict:
    rid = row["id"]
    out = {"id": rid, "family": row["family"], "ext": row["ext"], "path": row["path"],
           "engines": {}}

    ref_pdf = BENCH / "lo" / rid / "out.pdf"
    if not ref_pdf.exists():
        out["reference"] = {"status": "missing"}
        return out
    try:
        ref = fitz.open(ref_pdf)
    except Exception as e:
        out["reference"] = {"status": "unreadable", "error": str(e)[:200]}
        return out
    ref_pages = ref.page_count
    out["reference"] = {"status": "ok", "pages": ref_pages}
    compare_n = min(ref_pages, MAX_PAGES)

    # ---- X-Ray.Office: its own PDF, rasterised at the reference's DPI ------------
    pl_doc, pl_pages = engine_pages_pl(rid)
    pl = {"pages": pl_pages, "status": "ok" if pl_doc else "no-output", "page_metrics": []}
    if pl_doc:
        for i in range(min(compare_n, pl_pages)):
            try:
                r = pdf_page_gray(ref, i)
                a = pdf_page_gray(pl_doc, i)
                exact = a.shape == r.shape
                a = M.resize_to(a, r.shape)
                m = M.compare(a, r)
                m["page"] = i + 1
                m["size_match"] = exact
                m["diagnosis"] = M.diagnose(m)
                pl["page_metrics"].append(m)
            except Exception as e:
                pl["page_metrics"].append({"page": i + 1, "error": str(e)[:200]})
        pl_doc.close()
    out["engines"]["xray-office"] = pl

    # ---- WASM viewer: PNGs captured through Playwright -----------------------
    wv_dir = BENCH / "wv" / rid
    res_file = wv_dir / "result.json"
    wv = {"pages": 0, "status": "no-output", "page_metrics": []}
    if res_file.exists():
        res = json.loads(res_file.read_text())
        wv["pages"] = res.get("pages", 0)
        wv["status"] = res.get("status", "failed")
        wv["error"] = res.get("error", "")
        for i in range(compare_n):
            png = wv_dir / f"page-{i + 1}.png"
            if not png.exists():
                continue
            try:
                r = pdf_page_gray(ref, i)
                a = png_gray(png)
                exact = a.shape == r.shape
                if row["family"] == "sheets":
                    # The viewer has no print pagination: it drew an A1-anchored
                    # grid, not a page. Strip its row/column header strip -- the
                    # reference prints the grid alone -- then compare the region
                    # the two have in common rather than stretching one onto the
                    # other's page box.
                    hw, hh = int(res.get("headerW", 0)), int(res.get("headerH", 0))
                    a = a[hh:, hw:]
                    r2, a2 = crop_common(r, a)
                    m = M.compare(a2, r2)
                    m["mode"] = "grid-region"
                else:
                    a = M.resize_to(a, r.shape)
                    m = M.compare(a, r)
                    m["mode"] = "page"
                m["page"] = i + 1
                m["size_match"] = exact
                m["diagnosis"] = M.diagnose(m)
                wv["page_metrics"].append(m)
            except Exception as e:
                wv["page_metrics"].append({"page": i + 1, "error": str(e)[:200]})
    out["engines"]["wasm-viewer"] = wv
    ref.close()
    return out


def crop_common(ref: np.ndarray, act: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    """Align two images on their top-left ink and keep the overlap.

    For a spreadsheet the reference page carries print margins and the viewer's
    grid does not, so the two share an origin only after each is trimmed to where
    its own content starts."""
    def ink_origin(g: np.ndarray) -> tuple[int, int]:
        mask = g < M.INK_THRESHOLD
        rows = np.flatnonzero(mask.any(axis=1))
        cols = np.flatnonzero(mask.any(axis=0))
        if not len(rows) or not len(cols):
            return 0, 0
        return int(rows[0]), int(cols[0])
    ry, rx = ink_origin(ref)
    ay, ax = ink_origin(act)
    r = ref[ry:, rx:]
    a = act[ay:, ax:]
    h = min(r.shape[0], a.shape[0])
    w = min(r.shape[1], a.shape[1])
    return r[:h, :w], a[:h, :w]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--shard", type=int, default=0)
    ap.add_argument("--shards", type=int, default=1)
    ap.add_argument("--force", action="store_true")
    ap.add_argument("--family", default=None)
    args = ap.parse_args()

    rows = list(csv.DictReader((BENCH / "manifest.tsv").open(), delimiter="\t"))
    if args.family:
        rows = [r for r in rows if r["family"] in args.family.split(",")]
    rows = [r for i, r in enumerate(rows) if i % args.shards == args.shard]
    OUTDIR.mkdir(parents=True, exist_ok=True)
    for n, row in enumerate(rows, 1):
        dest = OUTDIR / f"{row['id']}.json"
        if dest.exists() and not args.force:
            continue
        try:
            dest.write_text(json.dumps(score_document(row)))
        except Exception:
            dest.write_text(json.dumps({"id": row["id"], "family": row["family"],
                                        "ext": row["ext"], "path": row["path"],
                                        "fatal": traceback.format_exc()[-500:]}))
        if n % 20 == 0:
            print(f"shard {args.shard}: {n}/{len(rows)}", flush=True)
    print(f"shard {args.shard}: done {len(rows)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
