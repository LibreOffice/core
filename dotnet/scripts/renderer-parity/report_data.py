#!/usr/bin/env python3
"""Assemble everything the report page needs into one JSON bundle."""
from __future__ import annotations

import json, pathlib, statistics
from collections import Counter, defaultdict

BENCH = pathlib.Path("/data/bench")
ENGINES = ("xray-office", "wasm-viewer")
FAM_LABEL = {"docs": "Documents", "sheets": "Spreadsheets", "slides": "Presentations"}


def main():
    docs = json.loads((BENCH / "documents.json").read_text())
    summary = json.loads((BENCH / "summary.json").read_text())
    lo_status = json.loads((BENCH / "lo-status.json").read_text())
    pl_status = json.loads((BENCH / "pl-status.json").read_text())

    out = {"corpus": {"documents": len(docs),
                      "reference_pages": summary["reference_pages"],
                      "pages_compared": summary["overall"]["xray-office"]["pages_compared"]},
           "families": summary["families"], "overall": summary["overall"],
           "by_ext": {}}

    # ---- format coverage --------------------------------------------------
    by_ext = defaultdict(list)
    for d in docs:
        by_ext[d["ext"]].append(d)
    for ext, rows in sorted(by_ext.items(), key=lambda kv: -len(kv[1])):
        entry = {"documents": len(rows), "family": rows[0]["family"]}
        for eng in ENGINES:
            es = [r["engines"][eng] for r in rows]
            rendered = [e for e in es if e["rendered"]]
            entry[eng] = {
                "rendered": len(rendered),
                "render_rate": round(len(rendered) / len(rows), 4),
                "fidelity": (round(statistics.fmean([e["fidelity_rendered"] for e in rendered]), 4)
                             if rendered else None),
            }
        out["by_ext"][ext] = entry

    # ---- the like-for-like headline set -----------------------------------
    # Spreadsheets are left out of the headline for the viewer: it has no print
    # pagination, so its spreadsheet output is a screen grid and not a page. Both
    # engines are scored on the same set so the two numbers can be read together.
    paged = [d for d in docs if d["family"] in ("docs", "slides")]
    head = {}
    for eng in ENGINES:
        es = [d["engines"][eng] for d in paged]
        r = [e for e in es if e["rendered"]]
        head[eng] = {
            "documents": len(paged),
            "rendered": len(r),
            "render_rate": round(len(r) / len(paged), 4),
            "fidelity_rendered": round(statistics.fmean([e["fidelity_rendered"] for e in r]), 4),
            "fidelity_corpus": round(statistics.fmean([e["fidelity"] for e in es]), 4),
            "agreement_rendered": round(statistics.fmean([e["agreement"] for e in r]), 4),
            "page_exact_rate": round(sum(1 for e in es if e["page_exact"]) / len(es), 4),
            "page_exact_rate_rendered": round(sum(1 for e in r if e["page_exact"]) / len(r), 4),
        }
    out["headline"] = head

    # ---- fidelity distribution (for the chart) -----------------------------
    bins = [0.0, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 1.001]
    dist = {}
    for fam in ("docs", "sheets", "slides"):
        for eng in ENGINES:
            vals = [d["engines"][eng]["fidelity_rendered"] for d in docs
                    if d["family"] == fam and d["engines"][eng]["rendered"]]
            counts = [0] * (len(bins) - 1)
            for v in vals:
                for i in range(len(bins) - 1):
                    if bins[i] <= v < bins[i + 1]:
                        counts[i] += 1
                        break
            dist[f"{fam}|{eng}"] = {"bins": bins, "counts": counts, "n": len(vals)}
    out["distribution"] = dist

    # ---- why the viewer declines a document --------------------------------
    reasons = defaultdict(Counter)
    for d in docs:
        e = d["engines"]["wasm-viewer"]
        if e["rendered"]:
            continue
        msg = e.get("error", "") or e.get("status", "")
        if "legacy binary" in msg:
            key = "legacy binary format (.doc/.xls/.ppt) not supported"
        elif "Timeout" in msg or e["status"] == "timeout":
            key = "layout did not finish inside the time budget"
        elif "resource limit" in msg or "OoxmlResourceLimit" in msg:
            key = "built-in resource limit exceeded"
        elif "404" in msg:
            key = "file could not be fetched"
        else:
            key = "parser or layout error"
        reasons[d["family"]][key] += 1
    out["viewer_declines"] = {k: dict(v) for k, v in reasons.items()}

    # ---- conversion cost ---------------------------------------------------
    lo_secs = [v["seconds"] for v in lo_status.values() if v.get("status") == "ok"]
    pl_secs = [v["seconds"] for v in pl_status.values() if v.get("status") == "ok"]
    out["cost"] = {
        "libreoffice": {"documents": len(lo_secs), "total_s": round(sum(lo_secs), 1),
                        "median_s": round(statistics.median(lo_secs), 2),
                        "mean_s": round(statistics.fmean(lo_secs), 2)},
        "xray-office": {"documents": len(pl_secs), "total_s": round(sum(pl_secs), 1),
                      "median_s": round(statistics.median(pl_secs), 2),
                      "mean_s": round(statistics.fmean(pl_secs), 2)},
    }

    # ---- page-count agreement ---------------------------------------------
    pc = {}
    for eng in ENGINES:
        for fam in ("docs", "sheets", "slides"):
            rows = [d for d in docs if d["family"] == fam
                    and d["engines"][eng]["rendered"] and d["engines"][eng]["paginates"]]
            if not rows:
                continue
            deltas = Counter()
            for d in rows:
                x = d["engines"][eng]["page_delta"]
                deltas["exact" if x == 0 else ("over" if x > 0 else "under")] += 1
            pc[f"{fam}|{eng}"] = {"n": len(rows), **deltas}
    out["page_counts"] = pc

    (BENCH / "report.json").write_text(json.dumps(out, indent=1))
    print(json.dumps(out["headline"], indent=1))
    print(json.dumps(out["cost"], indent=1))
    print(json.dumps(out["viewer_declines"], indent=1))
    print(json.dumps(out["page_counts"], indent=1))


if __name__ == "__main__":
    main()
