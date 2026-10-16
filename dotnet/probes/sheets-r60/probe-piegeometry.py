#!/usr/bin/env python3
"""Our pie's centre and radius against the reference's, read at the wedge corner.

Round 59 § 3 refuted "our pie is 18% larger": `pdf-ops.py`'s path bounding box includes a bezier's
**control points**, and we emit one cubic per arc where LibreOffice emits a polygonised arc, so a
union of wedge boxes measures our control polygon.  The first wedge of each of these four documents
runs from twelve o'clock through less than ninety degrees, so it lies wholly in the upper-right
quadrant: its box's **lower-left corner is the pie centre** and its top edge is centre + radius.
That reading is exact for both renderers, and it is the one used here.

Refuses to print unless both halves of every document produced a wedge.
"""
import os
import re
import subprocess
import sys

PDFOPS = "/c/sandbox/workdir/wt-sheets-r50/.claude/skills/render-comparison/scripts/pdf-ops.py"
WEDGE = re.compile(
    r"^fill\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+#4F81BD")

DOCS = ["003_advanced_excel_pie__xlsx.pdf", "011_advanced_excel_pie__xlsx.pdf",
        "019_advanced_excel_pie__xlsx.pdf", "027_advanced_excel_pie__xlsx.pdf"]


def geometry(pdf):
    txt = subprocess.run([sys.executable, PDFOPS, "dump", pdf, "--page", "1"],
                         capture_output=True, text=True).stdout
    best = None
    for line in txt.splitlines():
        m = WEDGE.match(line)
        if not m:
            continue
        x0, y0, x1, y1 = (float(g) for g in m.groups())
        if best is None or (x1 - x0) * (y1 - y0) > (best[2] - best[0]) * (best[3] - best[1]):
            best = (x0, y0, x1, y1)
    return None if best is None else (best[0], best[1], best[3] - best[1])


def main():
    root = sys.argv[1]
    missing, rows = [], []
    for name in DOCS:
        our = geometry(os.path.join(root, "ours", name))
        ref = geometry(os.path.join(root, "ref", name))
        if our is None or ref is None:
            missing.append((name, our is None, ref is None))
            continue
        rows.append((name, our, ref))
    if missing:
        print("REFUSING TO PRINT — no wedge found:", missing, file=sys.stderr)
        sys.exit(2)
    print("%-34s %-24s %-24s %8s %8s %8s" %
          ("document", "our centre", "ref centre", "our r", "ref r", "err %"))
    for name, our, ref in rows:
        print("%-34s (%7.2f,%7.2f)      (%7.2f,%7.2f)      %8.2f %8.2f %+7.2f%%"
              % (name[:34], our[0], our[1], ref[0], ref[1], our[2], ref[2],
                 100.0 * (our[2] - ref[2]) / ref[2]))
        print("%-34s   centre offset %6.2f, %6.2f" %
              ("", our[0] - ref[0], our[1] - ref[1]))


if __name__ == "__main__":
    main()
