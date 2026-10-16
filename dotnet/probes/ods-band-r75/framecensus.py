#!/usr/bin/env python3
"""How many .ods place a drawing with `draw:transform` rather than with `svg:x`/`svg:y`.

    framecensus.py <corpus-root>

A `draw:frame` carrying a transform and no coordinate pair is how LibreOffice writes a turned
or skewed picture, and `OdsDrawings` reads neither the transform nor, therefore, the frame.
That costs three separate things at once, which is why it looks like a text defect: the
picture's ink, the print area the drawing layer widens
(`ScDocument::GetPrintArea`, sc/source/core/data/documen2.cxx:644-666), and the
overlap test that keeps a picture-only page from being dropped as empty.
"""
import os, re, sys, zipfile

root = sys.argv[1]
FRAME = re.compile(r"<draw:frame[^>]*>")
docs = both = 0
frames = transformed = 0
worst = []
for dirpath, _, names in os.walk(root):
    for n in sorted(names):
        if not n.lower().endswith(".ods"):
            continue
        path = os.path.join(dirpath, n)
        try:
            with zipfile.ZipFile(path) as z:
                content = z.read("content.xml").decode("utf-8", "replace")
        except Exception:
            continue
        found = FRAME.findall(content)
        if not found:
            continue
        docs += 1
        t = [f for f in found if "draw:transform=" in f and "svg:x=" not in f]
        frames += len(found)
        transformed += len(t)
        if t:
            both += 1
            worst.append((len(t), len(found), os.path.relpath(path, root)))

worst.sort(reverse=True)
print(f"documents holding a draw:frame: {docs}")
print(f"of which some frame states a transform and no svg:x: {both}")
print(f"frames {frames}, of them transform-only {transformed}")
for t, f, rel in worst[:15]:
    print(f"  {t:4d}/{f:4d}  {rel}")
