#!/usr/bin/env python3
"""Whole-document glyph / show-operator granularity for a pair of PDFs."""
import re, subprocess, sys, pathlib

S = pathlib.Path("/c/sandbox/workdir/ver-out/sweep")
OPS = "/c/sandbox/workdir/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
RE_TEXT = re.compile(r"(\d+) glyphs in (\d+) show")

print("document\tside\tglyphs\tshows\tglyphs/show")
for pdf in sorted((S / "ours").glob("*.pdf")):
    ident = pdf.stem
    for side in ("ours", "ref"):
        out = subprocess.run(["python3", OPS, "dump", str(S / side / f"{ident}.pdf")],
                             capture_output=True, text=True).stdout
        g = sh = 0
        for m in RE_TEXT.finditer(out):
            g += int(m.group(1)); sh += int(m.group(2))
        print(f"{ident}\t{side}\t{g}\t{sh}\t{g/sh if sh else 0:.2f}")
        sys.stdout.flush()
