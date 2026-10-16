#!/usr/bin/env python3
"""How many `#D9D9D9` strokes the reference draws and we do not, over a whole sweep.

The census of *what a chart part states* over-reaches by construction: a part with no
`c:chartSpace/c:spPr/a:ln` gains the automatic border only if the chart is drawn at all and
lands on a printed page.  This counts the ink instead, on both sides of a sweep that already
exists, and is therefore the control the census cannot be.
"""
import os, re, subprocess, sys, collections

OPS = "/c/sandbox/workdir/wt-sheets-r50/.claude/skills/render-comparison/scripts/pdf-ops.py"
STROKE = re.compile(r"^stroke\s+p\d+\s.*#([0-9a-fA-F]{6})", re.M)


def strokes(pdf, colour="d9d9d9"):
    try:
        out = subprocess.run([sys.executable, OPS, "dump", pdf],
                             capture_output=True, text=True, timeout=300).stdout
    except Exception:
        return None
    return sum(1 for m in STROKE.finditer(out) if m.group(1).lower() == colour)


if __name__ == "__main__":
    ours_dir, ref_dir = sys.argv[1], sys.argv[2]
    only = sys.argv[3] if len(sys.argv) > 3 else None
    rows = []
    for name in sorted(os.listdir(ref_dir)):
        if not name.endswith(".pdf"):
            continue
        if only and only not in name:
            continue
        r = strokes(os.path.join(ref_dir, name))
        if not r:
            continue
        o = strokes(os.path.join(ours_dir, name))
        rows.append((name, o, r))
    tot_o = sum(o or 0 for _, o, _ in rows)
    tot_r = sum(r for _, _, r in rows)
    for name, o, r in rows:
        print("%-90s ours %3s ref %3d" % (name[:90], o, r))
    print("\n%d documents draw a #D9D9D9 stroke in the reference; ours %d, reference %d"
          % (len(rows), tot_o, tot_r))
