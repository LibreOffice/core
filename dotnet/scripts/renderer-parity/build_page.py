#!/usr/bin/env python3
"""Render the comparison report to a single self-contained HTML file."""
from __future__ import annotations

import json, pathlib

BENCH = pathlib.Path("/data/bench")
OUT = pathlib.Path("/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad/renderer-parity.html")

report = json.loads((BENCH / "report.json").read_text())
gallery = json.loads((BENCH / "gallery.json").read_text())
readings = json.loads(pathlib.Path("/data/bench/scripts/readings.json").read_text())

TEMPLATE = pathlib.Path("/data/bench/scripts/page_template.html").read_text()
html = TEMPLATE.replace("/*__REPORT__*/", json.dumps(report))
html = html.replace("/*__GALLERY__*/", json.dumps(gallery))
html = html.replace("/*__READINGS__*/", json.dumps(readings))
OUT.parent.mkdir(parents=True, exist_ok=True)
OUT.write_text(html)
print(f"{OUT}  {OUT.stat().st_size / 1e6:.1f} MB")
