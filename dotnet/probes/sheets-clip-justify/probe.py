#!/usr/bin/env python3
"""Does a justified cell's wrapped line reach the right edge of its column?

Calc maps `horizontal="justify"` and `horizontal="distributed"` to `SvxAdjust::Block`, and
EditEngine then shares each line's spare width among its blanks
(`ImpEditEngine::ImpAdjustBlocks`, `editeng/source/editeng/impedit3.cxx:2306`). A paragraph's
last line is exempt unless the justify method is Distribute (`:1694-1701`).

Measured as the x of the right edge of the last word on each line, against both installed
references and ours.

    python3 probe.py <out-dir>
"""
import glob, html, os, re, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkxlsx import write

OUT = sys.argv[1]
CLI = os.environ.get("XRAY_OFFICE_CLI")
LO = {"24.2": os.environ.get("LO24", "soffice"),
      "26.2": os.environ.get("LO26", "/opt/libreoffice26.2/program/soffice")}
os.makedirs(OUT, exist_ok=True)

TEXT = ("Certificate of Airworthiness from the redelivering airline and if applicable "
        "the airworthiness review certificate. Second sentence to force a third line.")


def render(who, src):
    d = os.path.join(OUT, who.replace(".", ""))
    os.makedirs(d, exist_ok=True)
    pdf = os.path.join(d, os.path.basename(src)[:-5] + ".pdf")
    if os.path.exists(pdf):
        os.remove(pdf)
    if who == "ours":
        subprocess.run([CLI, "render", src, "--format", "pdf", "--outdir", d],
                       capture_output=True, timeout=600)
    else:
        subprocess.run([LO[who], f"-env:UserInstallation=file://{d}/prof", "--headless",
                        "--convert-to", "pdf", "--outdir", d, src],
                       capture_output=True, timeout=600)
    return pdf if os.path.exists(pdf) else None


def lines(pdf):
    """Right edge of the last word on each text line, and the leftmost x seen."""
    out = subprocess.run(["pdftotext", "-bbox", "-f", "1", "-l", "1", pdf, "-"],
                         capture_output=True, text=True).stdout
    words = [(float(m.group(1)), float(m.group(2)), float(m.group(3)), html.unescape(m.group(5)))
             for m in re.finditer(
                 r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">(.*?)</word>',
                 out)]
    words = [w for w in words if w[3] != "MARK"]
    rows = {}
    for x0, y, x1, _ in words:
        key = round(y, 1)
        left, right = rows.get(key, (x0, x1))
        rows[key] = (min(left, x0), max(right, x1))
    return [rows[k] for k in sorted(rows)]


who = ["24.2", "26.2"] + (["ours"] if CLI else [])
print(f"{'alignment':>11} {'who':>6}  right edge of each line")
for horizontal in ("justify", "distributed", "left"):
    for w in who:
        src = write(os.path.join(OUT, f"{horizontal}.xlsx"), TEXT, horizontal)
        pdf = render(w, src)
        if not pdf:
            print(f"{horizontal:>11} {w:>6}  (no render)")
            continue
        got = lines(pdf)
        print(f"{horizontal:>11} {w:>6}  "
              + "  ".join(f"{r:.2f}" for _, r in got))
