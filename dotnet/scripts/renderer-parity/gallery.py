#!/usr/bin/env python3
"""Build the side-by-side images the report shows, as base64 JPEG.

Two panels, not three: the question a worst case answers is "what did THIS engine
do to this page", and putting the reference immediately beside it is the only
arrangement where the same region lands next to itself.
"""
from __future__ import annotations

import base64, io, json, pathlib, sys
import numpy as np
import pymupdf
from PIL import Image, ImageDraw

sys.path.insert(0, "/data/bench/scripts")
import metrics as M

BENCH = pathlib.Path("/data/bench")
PANEL_H = 620
QUALITY = 80


def pdf_rgb(path: pathlib.Path, index: int, dpi: int = 150):
    if not path.exists():
        return None
    d = pymupdf.open(path)
    try:
        if index >= d.page_count:
            return None
        pm = d.load_page(index).get_pixmap(dpi=dpi, alpha=False)
        return Image.frombytes("RGB", (pm.width, pm.height), pm.samples)
    finally:
        d.close()


def viewer_png(rid: str, page: int):
    p = BENCH / "wv" / rid / f"page-{page}.png"
    return Image.open(p).convert("RGB") if p.exists() else None


def fit(im, h):
    if im is None:
        return None
    w = max(1, round(im.width * h / im.height))
    return im.resize((w, h), Image.LANCZOS)


def pair(left, right, height=PANEL_H):
    """Reference on the left, candidate on the right, on one canvas."""
    a, b = fit(left, height), fit(right, height)
    if b is None:
        b = Image.new("RGB", (round(height * 0.72), height), (240, 240, 242))
        d = ImageDraw.Draw(b)
        d.text((16, 16), "no output", fill=(160, 40, 40))
    gap = 10
    W, H = a.width + gap + b.width, height
    out = Image.new("RGB", (W, H), (255, 255, 255))
    out.paste(a, (0, 0))
    out.paste(b, (a.width + gap, 0))
    d = ImageDraw.Draw(out)
    d.rectangle([0, 0, a.width - 1, H - 1], outline=(210, 212, 218))
    d.rectangle([a.width + gap, 0, W - 1, H - 1], outline=(210, 212, 218))
    return out


def to_data_uri(im, quality=QUALITY):
    buf = io.BytesIO()
    im.save(buf, "JPEG", quality=quality, optimize=True, subsampling=1)
    return "data:image/jpeg;base64," + base64.b64encode(buf.getvalue()).decode()


def build_case(rec: dict, eng: str, height=PANEL_H):
    w = rec["engines"][eng]["worst_defect"]
    page = w["page"]
    ref = pdf_rgb(BENCH / "lo" / rec["id"] / "out.pdf", page - 1)
    if eng == "xray-office":
        cand = pdf_rgb(BENCH / "pl" / rec["id"] / "out.pdf", page - 1)
    else:
        cand = viewer_png(rec["id"], page)
    if ref is None:
        return None
    return {
        "path": rec["path"],
        "name": rec["path"].rsplit("/", 1)[-1],
        "ext": rec["ext"],
        "family": rec["family"],
        "page": page,
        "ref_pages": rec["ref_pages"],
        "engine_pages": rec["engines"][eng]["pages"],
        "ssim": w["ssim"],
        "mae": w["mae"],
        "ink_ratio": w["ink_ratio"],
        "shifted": w["shifted"],
        "diagnosis": w["diagnosis"],
        "image": to_data_uri(pair(ref, cand, height)),
    }


def main() -> int:
    docs = json.loads((BENCH / "documents.json").read_text())
    per = int(sys.argv[1]) if len(sys.argv) > 1 else 3
    out = {}
    for fam in ("docs", "sheets", "slides"):
        for eng in ("xray-office", "wasm-viewer"):
            rows = [d for d in docs
                    if d["family"] == fam and d["engines"][eng]["worst_defect"]]
            rows.sort(key=lambda r: r["engines"][eng]["worst_defect"]["ssim"])
            cases = []
            for r in rows:
                c = build_case(r, eng)
                if c:
                    cases.append(c)
                if len(cases) >= per:
                    break
            out[f"{fam}|{eng}"] = cases
            print(f"{fam}/{eng}: {len(cases)} cases", flush=True)

            # One median document per family and engine, so the worst-case wall is
            # read against what a typical page in this corpus actually looks like.
            done = [d for d in docs
                    if d["family"] == fam and d["engines"][eng]["rendered"]]
            done.sort(key=lambda r: r["engines"][eng]["fidelity_rendered"])
            mid = done[len(done) // 2]
            e = mid["engines"][eng]
            ref = pdf_rgb(BENCH / "lo" / mid["id"] / "out.pdf", 0)
            cand = (pdf_rgb(BENCH / "pl" / mid["id"] / "out.pdf", 0) if eng == "xray-office"
                    else viewer_png(mid["id"], 1))
            out[f"{fam}|{eng}|median"] = [{
                "path": mid["path"], "name": mid["path"].rsplit("/", 1)[-1],
                "ext": mid["ext"], "family": fam, "page": 1,
                "ref_pages": mid["ref_pages"], "engine_pages": e["pages"],
                "ssim": e["fidelity_rendered"], "mae": e["mean_mae"],
                "ink_ratio": e["mean_ink_ratio"], "shifted": e["shifted_tiles"],
                "diagnosis": "median document",
                "image": to_data_uri(pair(ref, cand, 520), 78),
            }] if ref is not None else []
    (BENCH / "gallery.json").write_text(json.dumps(out))
    size = (BENCH / "gallery.json").stat().st_size
    print(f"gallery.json {size / 1e6:.1f} MB")
    return 0


if __name__ == "__main__":
    sys.exit(main())
