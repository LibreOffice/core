#!/usr/bin/env python3
"""Fold the post-fix outcome onto each case, so the catalogue says where each
document stands rather than only what was wrong with it.

`classification.json` is the input and it is measured, not asserted: `fixed`
means this build's rendering moved toward the reference against the same banked
LibreOffice PDFs, by SSIM or by the paint-order census, between the sweep's
binary and this one. A document a patch was written for that did not move is
not counted as fixed.
"""
import json, pathlib

BENCH = pathlib.Path("/data/bench")
CASES = BENCH / "pl-cases.json"
cases = json.loads(CASES.read_text())
klass = {k["rank"]: k for k in json.loads((BENCH / "classification.json").read_text())}

n = {}
for c in cases:
    k = klass.get(c["rank"])
    if not k:
        continue
    c["outcome"] = k["bucket"]
    c["outcome_why"] = k["why"]
    if k["bucket"] == "fixed" and "fixed" not in c["tags"]:
        c["tags"] = ["fixed"] + c["tags"]
    n[k["bucket"]] = n.get(k["bucket"], 0) + 1

CASES.write_text(json.dumps(cases))
print(n)
