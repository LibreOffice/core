#!/usr/bin/env python3
"""Aggregate the per-document scores into the numbers the report is built from.

Three things are kept deliberately apart:

  * fidelity on what the engine rendered -- how close its pages are to the
    reference when it produced one at all;
  * corpus-wide score -- the same, with every document it could not open scored
    as zero;
  * page-count parity -- whether it paginates the way the reference does.

Reporting only the first flatters an engine that declines half the corpus;
reporting only the second hides how good the half it does render is.

Two similarity axes are carried through, because one of them alone misranks:
structural similarity (SSIM) is what separates a laid-out page from a wrong one,
but it collapses on high-frequency content -- a mottled slide background, a
graph-paper rule -- where a half-pixel resampling difference flips every pixel
while the page is visually identical. Photometric agreement (1 - mean absolute
error) does not, so a page is only called a defect when both axes agree.
"""
from __future__ import annotations

import json, pathlib, statistics, sys
from collections import defaultdict

BENCH = pathlib.Path("/data/bench")
SCORES = BENCH / "scores"
MAX_PAGES = 5
ENGINES = ("xray-office", "wasm-viewer")

# A page counts as a defect only when the structure has genuinely diverged AND at
# least one independent metric corroborates it.
DEFECT_SSIM = 0.80
DEFECT_MAE = 0.07
DEFECT_INK = 0.12
DEFECT_SHIFT = 0.10


def is_defect(m: dict) -> bool:
    if m.get("ssim", 1.0) >= DEFECT_SSIM:
        return False
    return (m["mean_abs_error"] > DEFECT_MAE
            or abs(1.0 - min(m["ink_ratio"], 3.0)) > DEFECT_INK
            or m["shifted_tiles"] / max(1, m["tiles"]) > DEFECT_SHIFT)


def _worst_defect(pms):
    """The page a reader should be shown: the lowest-SSIM page among those a
    second metric agrees are wrong. Ranking on SSIM alone puts a visually
    identical mottled slide at the top of the list."""
    cands = [m for m in pms if is_defect(m)]
    if not cands:
        return None
    m = min(cands, key=lambda x: x["ssim"])
    return {"page": m["page"], "ssim": m["ssim"], "mae": m["mean_abs_error"],
            "ink_ratio": m["ink_ratio"], "diagnosis": m.get("diagnosis"),
            "shifted": round(m["shifted_tiles"] / max(1, m["tiles"]), 4)}


def doc_record(js: dict) -> dict | None:
    if js.get("reference", {}).get("status") != "ok":
        return None
    ref_pages = js["reference"]["pages"]
    rec = {"id": js["id"], "family": js["family"], "ext": js["ext"], "path": js["path"],
           "ref_pages": ref_pages, "engines": {}}
    for eng in ENGINES:
        e = js["engines"].get(eng, {})
        pms = [m for m in e.get("page_metrics", []) if "ssim" in m]
        scores = [max(0.0, min(1.0, m["ssim"])) for m in pms]
        expected = min(ref_pages, MAX_PAGES)
        # Pages the engine never produced count against it: a renderer that emits
        # three of five pages has not matched the other two.
        filled = scores + [0.0] * max(0, expected - len(scores))
        # A spreadsheet viewer with no print pagination reports sheets, not pages;
        # comparing that count against printed pages would be a category error.
        paginates = not (eng == "wasm-viewer" and js["family"] == "sheets")
        rec["engines"][eng] = {
            "status": e.get("status", "no-output"),
            "error": e.get("error", ""),
            "pages": e.get("pages", 0),
            "page_delta": e.get("pages", 0) - ref_pages,
            "page_exact": (e.get("pages", 0) == ref_pages) if paginates else None,
            "paginates": paginates,
            "compared": len(scores),
            "fidelity": round(statistics.fmean(filled), 5) if filled else 0.0,
            "fidelity_rendered": round(statistics.fmean(scores), 5) if scores else 0.0,
            "agreement": (round(1.0 - statistics.fmean(
                [m["mean_abs_error"] for m in pms]), 5) if pms else 0.0),
            "rendered": len(scores) > 0,
            "defect_pages": sum(1 for m in pms if is_defect(m)),
            "worst_page": (min(pms, key=lambda m: m["ssim"])["page"] if pms else None),
            "worst_ssim": (round(min(m["ssim"] for m in pms), 5) if pms else None),
            "worst_defect": _worst_defect(pms),
            "diagnoses": [m.get("diagnosis") for m in pms],
            "mean_ink_ratio": (round(statistics.fmean([m["ink_ratio"] for m in pms]), 4)
                               if pms else None),
            "mean_mae": (round(statistics.fmean([m["mean_abs_error"] for m in pms]), 5)
                         if pms else None),
            "shifted_tiles": (round(statistics.fmean(
                [m["shifted_tiles"] / max(1, m["tiles"]) for m in pms]), 5) if pms else None),
            "size_match_all": all(m.get("size_match", False) for m in pms) if pms else False,
        }
    return rec


def _count_diag(entries):
    c = defaultdict(int)
    for e in entries:
        for d in e["diagnoses"]:
            c[d] += 1
    return dict(sorted(c.items(), key=lambda kv: -kv[1]))


def block(rows, eng):
    es = [r["engines"][eng] for r in rows]
    rendered = [e for e in es if e["rendered"]]
    paged = [e for e in es if e["paginates"]]
    paged_rendered = [e for e in rendered if e["paginates"]]
    total_pages = sum(e["compared"] for e in rendered)
    return {
        "documents": len(rows),
        "rendered_documents": len(rendered),
        "render_rate": round(len(rendered) / len(rows), 4),
        "fidelity_corpus": round(statistics.fmean([e["fidelity"] for e in es]), 4),
        "fidelity_rendered": (round(statistics.fmean(
            [e["fidelity_rendered"] for e in rendered]), 4) if rendered else 0.0),
        "median_fidelity_rendered": (round(statistics.median(
            [e["fidelity_rendered"] for e in rendered]), 4) if rendered else 0.0),
        "agreement_rendered": (round(statistics.fmean(
            [e["agreement"] for e in rendered]), 4) if rendered else 0.0),
        "page_exact_rate": (round(sum(1 for e in paged if e["page_exact"]) / len(paged), 4)
                            if paged else None),
        "page_exact_rate_rendered": (round(
            sum(1 for e in paged_rendered if e["page_exact"]) / len(paged_rendered), 4)
            if paged_rendered else None),
        "pages_compared": total_pages,
        "defect_pages": sum(e["defect_pages"] for e in rendered),
        "defect_page_rate": (round(sum(e["defect_pages"] for e in rendered) / total_pages, 4)
                             if total_pages else 0.0),
        "clean_documents": sum(1 for e in rendered if e["defect_pages"] == 0),
        "size_match_rate": (round(sum(1 for e in rendered if e["size_match_all"])
                                  / len(rendered), 4) if rendered else 0.0),
        "diagnoses": _count_diag(rendered),
    }


def main() -> int:
    recs = []
    for f in sorted(SCORES.glob("*.json")):
        js = json.loads(f.read_text())
        if "fatal" in js:
            continue
        r = doc_record(js)
        if r:
            recs.append(r)

    families = defaultdict(list)
    for r in recs:
        families[r["family"]].append(r)

    summary = {"documents": len(recs),
               "reference_pages": sum(r["ref_pages"] for r in recs),
               "families": {}, "overall": {}, "by_ext": {}}
    for fam, rows in sorted(families.items()):
        summary["families"][fam] = {
            "documents": len(rows),
            "ref_pages": sum(r["ref_pages"] for r in rows),
            "engines": {eng: block(rows, eng) for eng in ENGINES},
        }
    for eng in ENGINES:
        summary["overall"][eng] = block(recs, eng)

    by_ext = defaultdict(list)
    for r in recs:
        by_ext[r["ext"]].append(r)
    for ext, rows in sorted(by_ext.items()):
        summary["by_ext"][ext] = {eng: block(rows, eng) for eng in ENGINES}

    (BENCH / "summary.json").write_text(json.dumps(summary, indent=1))
    (BENCH / "documents.json").write_text(json.dumps(recs))
    for eng in ENGINES:
        o = summary["overall"][eng]
        print(f"{eng:12s} render {o['render_rate']:.3f}  fid_rendered {o['fidelity_rendered']:.4f}"
              f"  fid_corpus {o['fidelity_corpus']:.4f}  page_exact {o['page_exact_rate']}"
              f"  defect {o['defect_page_rate']:.4f}")
    for fam, e in summary["families"].items():
        print(f"-- {fam} ({e['documents']} docs, {e['ref_pages']} ref pages)")
        for eng, v in e["engines"].items():
            print(f"   {eng:12s} render {v['render_rate']:.3f} fid {v['fidelity_rendered']:.4f}"
                  f" agree {v['agreement_rendered']:.4f} pageexact {v['page_exact_rate']}"
                  f" defect {v['defect_page_rate']:.4f} clean {v['clean_documents']}"
                  f"/{v['rendered_documents']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
