#!/usr/bin/env python3
"""Where each arm's prose or index entries start, horizontally: beside the obstacle or below it."""
import glob, os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
OPS = "/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py"


def runs(pdf):
    out = subprocess.run(["python3", OPS, "dump", pdf, "--only", "text"],
                         capture_output=True, text=True).stdout
    got = []
    for line in out.splitlines():
        m = re.match(r'\s*text\s+p(\d+)\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+[\d.]+pt\s+\S+\s+\d+'
                     r' glyphs?.*?"?([^"]*)"?$', line)
        if m:
            got.append((int(m.group(1)), float(m.group(2)), float(m.group(3)), m.group(4).strip()))
    return got


def first(pdf, prefix):
    """The leftmost x at which a run starting with `prefix` is drawn, and its page."""
    for page, x, y, text in sorted(runs(pdf), key=lambda t: (t[0], -t[2], t[1])):
        if text.startswith(prefix):
            return page, x
    return None, None


if __name__ == "__main__":
    print(f"{'arm':<20} {'what':<14} {'26.2.4.2':>16} {'ours':>16}")
    for path in sorted(glob.glob(os.path.join(HERE, "doc", "*.doc"))):
        name = os.path.basename(path)[:-4]
        for label, prefix in (("obstacle", "QUICK LINK 1"),
                              ("heading", "INFORMATION"),
                              ("body", "PROSE line 1"),
                              ("index", "INDEX ENTRY 0")):
            r = first(os.path.join(HERE, "dref", name + ".pdf"), prefix)
            o = first(os.path.join(HERE, "dours", name + ".pdf"), prefix)
            if r[0] is None and o[0] is None:
                continue
            fmt = lambda t: "absent" if t[0] is None else f"p{t[0]} x{t[1]:7.2f}"
            flag = "" if (r[0] == o[0] and r[1] is not None and o[1] is not None
                          and abs(r[1] - o[1]) < 1.0) else "  <--"
            print(f"{name:<20} {label:<14} {fmt(r):>16} {fmt(o):>16}{flag}")
        print()
