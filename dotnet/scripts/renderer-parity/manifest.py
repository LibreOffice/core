#!/usr/bin/env python3
"""Build the corpus manifest: one stable id per document."""
import csv, hashlib, pathlib, re, sys

ROOT = pathlib.Path("/home/user/sample-files")
OUT = pathlib.Path("/data/bench/manifest.tsv")

FAMILY = {"words": "docs", "sheets": "sheets", "slides": "slides"}
EXTS = {".docx", ".doc", ".xlsx", ".xls", ".xlsm", ".pptx", ".ppt"}

rows = []
seen = set()
for p in sorted(ROOT.rglob("*")):
    if not p.is_file():
        continue
    if p.suffix.lower() not in EXTS:
        continue
    rel = p.relative_to(ROOT)
    top = rel.parts[0]
    if top not in FAMILY:
        continue
    slug = re.sub(r"[^A-Za-z0-9._-]", "_", str(rel))
    if len(slug) > 90:
        slug = slug[:70] + "-" + hashlib.sha1(str(rel).encode()).hexdigest()[:10] + p.suffix.lower()
    assert slug not in seen, slug
    seen.add(slug)
    rows.append({
        "id": slug,
        "family": FAMILY[top],
        "ext": p.suffix.lower().lstrip("."),
        "batch": rel.parts[1] if len(rel.parts) > 2 else "",
        "path": str(rel),
        "bytes": p.stat().st_size,
    })

with OUT.open("w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=list(rows[0]), delimiter="\t")
    w.writeheader()
    w.writerows(rows)
print(f"{len(rows)} documents -> {OUT}")
from collections import Counter
print(Counter((r['family'], r['ext']) for r in rows))
