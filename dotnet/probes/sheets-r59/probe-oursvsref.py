#!/usr/bin/env python3
"""Our pie radius against the reference's, over the label-size series.

A single document fits any one-parameter model.  `probe-pieradius.py` established that the
reference's shrink is driven by what the labels consume, and gave three points on the same
chart with only the label font size varied: 8 pt -> 101.28, 10 pt -> 99.78, 16 pt -> 72.08.
Those three are what tells a correct model of the label's own box from a fitted one.

Refuses to print unless every variant produced a rendering and a wedge on both sides.
"""
import os, re, subprocess, sys

OUT = "/c/sandbox/workdir/scratch-r59-sheets/pieradius"
CLI = ("/c/sandbox/workdir/wt-sheets-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/"
       "net10.0/linux-x64/XRay.Office.Cli")
PDFOPS = "/c/sandbox/workdir/wt-sheets-r50/.claude/skills/render-comparison/scripts/pdf-ops.py"

CASES = ["00-asis", "13-label-8pt", "14-label-16pt", "01-nolabels", "07-pos-ctr",
         "04-cat-only", "12-bare", "03-notitle"]

WEDGE = re.compile(
    r"^fill\s+p1\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+#4F81BD")


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


def ours(name):
    src = os.path.join(OUT, name + ".xlsx")
    dst = os.path.join(OUT, "o-" + name)
    subprocess.run(["rm", "-rf", dst])
    os.makedirs(dst, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")
    subprocess.run([CLI, "render", src, "--format", "pdf", "--outdir", dst],
                   capture_output=True, env=env, timeout=300)
    pdf = os.path.join(dst, name + ".pdf")
    return geometry(pdf) if os.path.exists(pdf) else None


def main():
    rows, missing = [], []
    for name in CASES:
        r = os.path.join(OUT, "r-" + name, name + ".pdf")
        ref = geometry(r) if os.path.exists(r) else None
        our = ours(name)
        if ref is None or our is None:
            missing.append((name, ref is None, our is None))
            continue
        rows.append((name, our, ref))

    if missing:
        print("REFUSING TO SUMMARISE — %d of %d variants incomplete:" % (len(missing), len(CASES)),
              file=sys.stderr)
        for n, a, b in missing:
            print("   %s ref=%s ours=%s" % (n, "MISSING" if a else "ok", "MISSING" if b else "ok"),
                  file=sys.stderr)
        sys.exit(2)

    print("%-16s %-24s %-24s %8s %8s" % ("variant", "ours cx/cy/r", "ref cx/cy/r", "dr", "dr%"))
    for name, (ox, oy, orr), (rx, ry, rr) in rows:
        print("%-16s %7.2f %7.2f %7.2f  %7.2f %7.2f %7.2f %8.2f %7.2f%%"
              % (name, ox, oy, orr, rx, ry, rr, orr - rr, 100 * (orr - rr) / rr))


if __name__ == "__main__":
    main()
