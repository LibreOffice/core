#!/usr/bin/env python3
"""Where does line 4 — the last line of the paragraph above the frame — begin?

    readline4.py <pdf> [<pdf> ...]

Prints the frame's drawn rectangle and every pen on the baseline the paragraph above the frame
ends on, which in these documents is 734.34 pt from the page bottom. One pen at 56.80 means the
line was left at the margin; anything else means the frame reached it.
"""

import re
import subprocess
import sys
from pathlib import Path

OPS = Path(__file__).resolve().parents[3] / ".claude/skills/render-comparison/scripts/pdf-ops.py"
BASELINE = 734.34


def read(pdf):
    out = subprocess.run(
        [sys.executable, str(OPS), "dump", pdf, "--page", "1"],
        capture_output=True, text=True, check=True).stdout
    if not out.strip():
        raise SystemExit(f"{pdf}: pdf-ops produced nothing")
    frame, pens = None, []
    for line in out.splitlines():
        m = re.match(r"\s*stroke\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)", line)
        if m:
            frame = tuple(float(g) for g in m.groups())
            continue
        m = re.match(r"\s*text\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)", line)
        if m and abs(float(m.group(2)) - BASELINE) < 0.02:
            pens.append(float(m.group(1)))
    return frame, sorted(pens)


if __name__ == "__main__":
    for path in sys.argv[1:]:
        frame, pens = read(path)
        left = f"{frame[0]:.2f}" if frame else "-"
        top = f"{841.89 - frame[3]:.3f}" if frame else "-"
        print(f"{path:44s} frame_x={left:>8s} frame_top={top:>8s} line4={['%.2f' % v for v in pens]}")
