#!/usr/bin/env python3
"""Every document where X-Ray.Office does not match the LibreOffice reference.

Writes two things per document: a high-resolution JPEG for reading (the defect has
to survive to be diagnosed) and a compact WebP for embedding in the report. The
list is ordered worst first, by the SSIM of the worst page a second metric agrees
is wrong; documents whose pixels match but whose page count does not come last,
since nothing is visibly wrong on the pages that were compared.
"""
from __future__ import annotations

import base64, io, json, pathlib, sys
import pymupdf
from PIL import Image, ImageDraw

sys.path.insert(0, "/data/bench/scripts")

BENCH = pathlib.Path("/data/bench")
VIEW = BENCH / "pairs-view"
EMBED_H = 600
EMBED_Q = 70
VIEW_H = 1040


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


def fit(im, h):
    if im is None:
        return None
    return im.resize((max(1, round(im.width * h / im.height)), h), Image.LANCZOS)


def pair(left, right, height):
    a, b = fit(left, height), fit(right, height)
    if b is None:
        b = Image.new("RGB", (round(height * 0.72), height), (240, 240, 242))
        ImageDraw.Draw(b).text((16, 16), "no output", fill=(160, 40, 40))
    gap = max(6, height // 70)
    out = Image.new("RGB", (a.width + gap + b.width, height), (255, 255, 255))
    out.paste(a, (0, 0))
    out.paste(b, (a.width + gap, 0))
    d = ImageDraw.Draw(out)
    d.rectangle([0, 0, a.width - 1, height - 1], outline=(208, 211, 217))
    d.rectangle([a.width + gap, 0, out.width - 1, height - 1], outline=(208, 211, 217))
    return out


def severity(d):
    e = d["engines"]["xray-office"]
    wd = e["worst_defect"]
    # Pixel defects first, ordered by how badly the worst page diverges; then the
    # documents that only paginate differently, worst page-count drift first.
    if wd:
        return (0, wd["ssim"], -abs(e["page_delta"]))
    return (1, -abs(e["page_delta"]), 0)


def main():
    docs = json.loads((BENCH / "documents.json").read_text())
    bad = []
    for d in docs:
        e = d["engines"]["xray-office"]
        if not e["rendered"] or e["defect_pages"] > 0 or not e["page_exact"]:
            bad.append(d)
    bad.sort(key=severity)
    VIEW.mkdir(parents=True, exist_ok=True)

    scores_dir = BENCH / "scores"
    out = []
    for i, d in enumerate(bad, 1):
        e = d["engines"]["xray-office"]
        page = (e["worst_defect"] or {}).get("page") or e["worst_page"] or 1
        ref = pdf_rgb(BENCH / "lo" / d["id"] / "out.pdf", page - 1)
        cand = pdf_rgb(BENCH / "pl" / d["id"] / "out.pdf", page - 1)
        if ref is None:
            continue
        view = pair(ref, cand, VIEW_H)
        vp = VIEW / f"{i:03d}.jpg"
        view.save(vp, "JPEG", quality=86, optimize=True)

        buf = io.BytesIO()
        pair(ref, cand, EMBED_H).save(buf, "WEBP", quality=EMBED_Q, method=5)

        raw = json.loads((scores_dir / f"{d['id']}.json").read_text())
        pages = [{k: m.get(k) for k in ("page", "ssim", "mean_abs_error", "ink_ratio",
                                        "shifted_tiles", "tiles", "row_profile_shift",
                                        "max_tile_error", "diagnosis", "size_match")}
                 for m in raw["engines"]["xray-office"]["page_metrics"] if "ssim" in m]
        out.append({
            "rank": i,
            "id": d["id"],
            "name": d["path"].rsplit("/", 1)[-1],
            "dir": d["path"].rsplit("/", 1)[0],
            "ext": d["ext"],
            "family": d["family"],
            "page": page,
            "ref_pages": d["ref_pages"],
            "engine_pages": e["pages"],
            "page_delta": e["page_delta"],
            "fidelity": e["fidelity_rendered"],
            "agreement": e["agreement"],
            "defect_pages": e["defect_pages"],
            "compared": e["compared"],
            "worst": e["worst_defect"],
            "kind": "pixel" if e["worst_defect"] else "pagination",
            "pages": pages,
            "view": str(vp),
            "image": "data:image/webp;base64," + base64.b64encode(buf.getvalue()).decode(),
        })
        if i % 25 == 0:
            print(f"{i}/{len(bad)}", flush=True)

    (BENCH / "pl-cases.json").write_text(json.dumps(out))
    size = (BENCH / "pl-cases.json").stat().st_size
    print(f"{len(out)} cases, pl-cases.json {size / 1e6:.1f} MB")


if __name__ == "__main__":
    main()
