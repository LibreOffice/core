#!/usr/bin/env python3
"""The size the value of each arm is drawn at, in points, per rendering."""
import os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
OPS = "/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"
ARMS = ["CTRL", "SIMPLE", "COMPLEX", "SIMPLERPR", "NOMERGE", "FIELDRUNRPR", "NOSEPARATOR"]


def sizes(pdf):
    out = subprocess.run(["python3", OPS, "dump", pdf, "--only", "text"],
                         capture_output=True, text=True).stdout
    runs = []
    for line in out.splitlines():
        m = re.match(r'\s*text\s+p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)pt\s+\S+\s+\d+'
                     r' glyphs?.*?"?([^"]*)"?$', line)
        if m:
            runs.append((float(m.group(2)), float(m.group(1)), float(m.group(3)), m.group(4).strip()))
    runs.sort(key=lambda t: (-t[0], t[1]))

    found, current = {}, None
    for _, _, size, text in runs:
        m = re.match(r'^(%s)\b' % "|".join(ARMS), text)
        if m:
            current = m.group(1)
            found.setdefault(current, None)
            if text.strip() != current:
                found[current] = size          # label and value drawn as one run
            continue
        if current and found.get(current) is None:
            found[current] = size
    return found


if __name__ == "__main__":
    tables = {a: sizes(os.path.join(HERE, a, "fldfooter.pdf")) for a in sys.argv[1:]}
    print(f"{'arm':<13}" + "".join(f"{a:>10}" for a in sys.argv[1:]))
    for arm in ARMS:
        cells = []
        for a in sys.argv[1:]:
            v = tables[a].get(arm)
            cells.append(f"{v:10.2f}" if v is not None else f"{'not drawn':>10}")
        print(f"{arm:<13}" + "".join(cells))
