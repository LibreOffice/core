#!/usr/bin/env python3
"""Render the X-Ray.Office-vs-LibreOffice difference catalogue to one HTML file."""
import json, pathlib

BENCH = pathlib.Path("/data/bench")
OUT = pathlib.Path("/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad/xray-office-differences.html")

cases = json.loads((BENCH / "pl-cases.json").read_text())
# The high-resolution reading copies are local files; the page carries only the
# embedded WebP.
for c in cases:
    c.pop("view", None)
    c.pop("pages", None)
    c.pop("id", None)

html = (BENCH / "scripts" / "pl_template.html").read_text()
html = html.replace("/*__CASES__*/", json.dumps(cases))
OUT.parent.mkdir(parents=True, exist_ok=True)
OUT.write_text(html)
print(f"{OUT}  {OUT.stat().st_size / 1e6:.2f} MB")
