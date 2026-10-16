#!/usr/bin/env python3
"""Which of a best-fit pie's labels the reference keeps inside its slice, and which we do not.

Round 61 closed the pie's geometry to 0.03 pt of centre and left one thing behind: all four
`advanced_excel_pie` documents come out **two words over** the reference, and a blind reader on
`003` reported that the reference draws M3 *inside* the yellow-green slice where we draw it
outside, below the pie.  `pdf-ops.py` corroborated with a ghost-key count of 2 against 1.

This reads both renderings' pages directly and says, per label:

  * the label's own drawn text box, from `pdftotext -bbox` (poppler's decoder, so the words are
    real words and not glyph counts);
  * how many lines it wrapped onto;
  * where its centre sits relative to the pie's centre and radius, i.e. inside or outside.

Nothing here runs our layout code — both sides are read off rendered PDFs — so the two readings
are the same measurement applied twice, and a disagreement cannot be an artefact of the port.
"""
import re, subprocess, sys, xml.etree.ElementTree as ET

PDFOPS = "/c/sandbox/workdir/wt-sheets-r50/.claude/skills/render-comparison/scripts/pdf-ops.py"
WEDGE = re.compile(
    r"^fill\s+p(\d+)\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+#([0-9A-Fa-f]{6})")
LABEL = re.compile(r"^M\d;$")


def pie(pdf, page=1, colour="4F81BD"):
    """Centre and radius, read at the first wedge's corner (round 59 s 3)."""
    txt = subprocess.run([sys.executable, PDFOPS, "dump", pdf, "--page", str(page)],
                         capture_output=True, text=True).stdout
    best = None
    for line in txt.splitlines():
        m = WEDGE.match(line)
        if not m or m.group(6).upper() != colour.upper():
            continue
        x0, y0, x1, y1 = (float(g) for g in m.groups()[1:5])
        if best is None or (x1 - x0) * (y1 - y0) > (best[2] - best[0]) * (best[3] - best[1]):
            best = (x0, y0, x1, y1)
    if best is None:
        return None
    x0, y0, x1, y1 = best
    return (x0, y0, y1 - y0)          # cx, cy, r   (PDF user space, y upward)


def words(pdf, page=1):
    """Every word on the page as (text, x0, ytop, x1, ybottom) with y measured downward."""
    out = subprocess.run(["pdftotext", "-bbox", "-f", str(page), "-l", str(page), pdf, "-"],
                         capture_output=True, text=True).stdout
    root = ET.fromstring(out)
    ns = {"x": root.tag.split("}")[0].strip("{")}
    pg = root.find(".//x:page", ns)
    h = float(pg.get("height"))
    got = []
    for w in pg.findall(".//x:word", ns):
        got.append((w.text or "", float(w.get("xMin")), float(w.get("yMin")),
                    float(w.get("xMax")), float(w.get("yMax"))))
    return got, h


def labels(pdf, page=1):
    """Group the page's words into the five `M<n>; ...` data labels, in reading order."""
    got, h = words(pdf, page)
    got.sort(key=lambda w: (round(w[2], 1), w[1]))
    lines, cur = [], []
    for w in got:
        if cur and abs(w[2] - cur[0][2]) > 1.0:
            lines.append(cur)
            cur = []
        cur.append(w)
    if cur:
        lines.append(cur)

    out = []
    for i, ln in enumerate(lines):
        if not LABEL.match(ln[0][0]):
            continue
        block = list(ln)
        nlines = 1
        # A wrapped continuation is the next line, starting within a point of this one's left
        # edge and holding no new `M<n>;`.
        for nxt in lines[i + 1:]:
            if LABEL.match(nxt[0][0]):
                break
            if abs(nxt[0][2] - block[-1][4]) > 6.0:
                break
            block.extend(nxt)
            nlines += 1
            break
        x0 = min(w[1] for w in block); x1 = max(w[3] for w in block)
        y0 = min(w[2] for w in block); y1 = max(w[4] for w in block)
        out.append({
            "text": " ".join(w[0] for w in block),
            "lines": nlines,
            "x0": x0, "x1": x1,
            # to y-upward
            "y0": h - y1, "y1": h - y0,
        })
    return out, h


def report(name, pdf, page=1):
    g = pie(pdf, page)
    if g is None:
        print("%s: NO WEDGE — refusing to report" % name)
        return None
    cx, cy, r = g
    print("%s  centre (%.2f, %.2f)  radius %.2f" % (name, cx, cy, r))
    rows = []
    for L in labels(pdf, page)[0]:
        mx = (L["x0"] + L["x1"]) / 2.0
        my = (L["y0"] + L["y1"]) / 2.0
        d = ((mx - cx) ** 2 + (my - cy) ** 2) ** 0.5
        half = (((L["x1"] - L["x0"]) / 2.0) ** 2 + ((L["y1"] - L["y0"]) / 2.0) ** 2) ** 0.5
        rows.append((L["text"], L["lines"], L["x1"] - L["x0"], L["y1"] - L["y0"],
                     mx, my, d, d + half <= r))
        print("   %-26s lines %d  box %7.2f x %6.2f  centre (%8.2f,%8.2f)  d %7.2f  %s"
              % (rows[-1][0][:26], L["lines"], rows[-1][2], rows[-1][3], mx, my, d,
                 "INSIDE" if rows[-1][7] else "outside"))
    return cx, cy, r, rows


if __name__ == "__main__":
    page = int(sys.argv[3]) if len(sys.argv) > 3 else 1
    report("ours", sys.argv[1], page)
    print()
    report("ref ", sys.argv[2], page)
