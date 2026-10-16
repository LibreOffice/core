#!/usr/bin/env python3
"""Check every published reading against evidence that does not depend on the
reference binary's version.

Three questions, each answered from the two PDFs' own content streams rather
than from a second look at the same image:

  1. ALIGNMENT -- is the page pair under comparison the same page of the
     document at all? A page-count divergence slides every later page along,
     so `our page 4` may hold `their page 5`, and a reading of that pair
     describes the slide, not a defect.
  2. PRESENCE  -- does our output actually lack the words a `content missing`
     reading says it lacks? A word that is merely somewhere else is not
     missing.
  3. RESOURCES -- do the two sides draw the same number of images, and shape
     the same faces? `missing graphics` and `text styling` are both claims an
     image can suggest and only the resource lists can settle.

Everything here reads `/data/bench/{lo,pl}/<id>/out.pdf`. Nothing is rendered
and nothing is installed, so the answers are as true of 26.2 as of 24.2.
"""
from __future__ import annotations

import json, pathlib, re, sys
from collections import Counter
import pymupdf

BENCH = pathlib.Path("/data/bench")
MAXP = 40          # far past MAX_PAGES; alignment needs the neighbourhood
WORD = re.compile(r"[^\W\d_]{2,}", re.UNICODE)


def page_facts(doc, i):
    p = doc.load_page(i)
    text = p.get_text("text")
    fonts = {f[3].split("+")[-1] for f in p.get_fonts(full=False)}
    images = len(p.get_images(full=False))
    return {
        "words": Counter(w.lower() for w in WORD.findall(text)),
        "chars": len(text.strip()),
        "fonts": fonts,
        "images": images,
    }


def doc_facts(path):
    if not path.exists():
        return None
    d = pymupdf.open(path)
    try:
        return {
            "count": d.page_count,
            "pages": [page_facts(d, i) for i in range(min(d.page_count, MAXP))],
        }
    finally:
        d.close()


def containment(a: Counter, b: Counter) -> float:
    """Fraction of a's word mass that b also carries. 1.0 == b holds all of a."""
    tot = sum(a.values())
    if not tot:
        return 1.0
    return sum(min(n, b[w]) for w, n in a.items()) / tot


def main():
    cases = json.loads((BENCH / "pl-cases.json").read_text())
    out = []
    for n, c in enumerate(cases, 1):
        ref = doc_facts(BENCH / "lo" / c["id"] / "out.pdf")
        ours = doc_facts(BENCH / "pl" / c["id"] / "out.pdf")
        rec = {"rank": c["rank"], "id": c["id"], "page": c["page"]}
        if ref is None or ours is None:
            rec["error"] = "missing pdf"
            out.append(rec); continue

        i = c["page"] - 1
        rp = ref["pages"][i] if i < len(ref["pages"]) else None
        op = ours["pages"][i] if i < len(ours["pages"]) else None
        rec["ref_has_page"] = rp is not None
        rec["our_has_page"] = op is not None

        if rp is not None and op is not None:
            # 1. ALIGNMENT: which reference page does OUR compared page best match?
            scores = [containment(op["words"], q["words"]) for q in ref["pages"]]
            best = max(range(len(scores)), key=lambda k: scores[k]) if scores else i
            rec["align"] = {
                "same_index": round(scores[i], 4) if i < len(scores) else None,
                "best_index": best + 1,
                "best_score": round(scores[best], 4),
                "our_page_chars": op["chars"],
                "ref_page_chars": rp["chars"],
            }
            # 2. PRESENCE: words on the reference page, sought across our whole doc
            whole = Counter()
            for q in ours["pages"]:
                whole += q["words"]
            rec["presence"] = {
                "page_pair": round(containment(rp["words"], op["words"]), 4),
                "doc_wide": round(containment(rp["words"], whole), 4),
                "ref_page_words": sum(rp["words"].values()),
            }
            # 3. RESOURCES
            rec["fonts"] = {
                "ref": sorted(rp["fonts"]), "ours": sorted(op["fonts"]),
                "same": sorted(rp["fonts"]) == sorted(op["fonts"]),
            }
            rec["images"] = {"ref": rp["images"], "ours": op["images"]}
            # doc-wide image totals: a picture moved to another page is not missing
            rec["images_doc"] = {
                "ref": sum(q["images"] for q in ref["pages"]),
                "ours": sum(q["images"] for q in ours["pages"]),
            }
        rec["pages"] = {"ref": ref["count"], "ours": ours["count"]}
        out.append(rec)
        if n % 40 == 0:
            print(f"{n}/{len(cases)}", flush=True)

    (BENCH / "audit.json").write_text(json.dumps(out))
    print(f"wrote audit.json, {len(out)} records")


if __name__ == "__main__":
    main()
