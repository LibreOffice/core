#!/usr/bin/env python3
"""Score one document's two renderings against 26.2.4.2's banked one.

    score-movers.py <movers.txt> <out.tsv>

For each document it renders our side twice from ONE binary -- XRAY_OFFICE_HYPHEN_DICTS=0 for
the hyphenator-off leg and unset for the shipped default -- and compares each against the
reference PDF banked in /home/user/gate-orig-r83/ref, which was produced by
/opt/libreoffice26.2/program/soffice (LibreOffice 26.2.4.2).

The column that decides is |ink|% -- the UNSIGNED per-page ink distance summed over pages,
from the render-comparison skill's pdf-image-diff.py. Ranking on a signed sum lets a page
where we draw too much cancel one where we draw too little; ranking on path or operator
counts is what once called 44 of 48 documents defective.

`glyphs` is the gate's own column 9: alphanumeric CHARACTERS out of pdftotext, ours against
the reference, with the gate's max(2%, 15) band.
"""
import os, re, shutil, subprocess, sys, tempfile

CLI = "/home/user/wt-hyphen/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
DIFF = "/home/user/wt-hyphen/.claude/skills/render-comparison/scripts/pdf-image-diff.py"
REF = "/home/user/gate-orig-r83/ref"
ROOT = "/home/user/sample-files"


def ident(rel):
    # batch-check.sh's per-format identity: the BASENAME and the extension, not the path.
    # `report.doc` and `report.docx` both convert to report.pdf and one would overwrite the
    # other, which is the whole reason the extension is in the name.
    stem, ext = os.path.splitext(os.path.basename(rel))
    return stem + "__" + ext[1:].lower()


def render(rel, dicts, into):
    env = dict(os.environ)
    env["SOURCE_DATE_EPOCH"] = "1700000000"
    env["XRAY_OFFICE_HYPHEN_DICTS"] = dicts
    subprocess.run([CLI, "render", "--quiet", "--outdir", into, os.path.join(ROOT, rel)],
                   env=env, capture_output=True, timeout=600)
    pdfs = [f for f in os.listdir(into) if f.endswith(".pdf")]
    return os.path.join(into, pdfs[0]) if pdfs else None


def glyphs(pdf):
    out = subprocess.run(["pdftotext", pdf, "-"], capture_output=True).stdout
    return sum(1 for c in out.decode("utf-8", "replace") if c.isalnum())


def turned(pdf):
    """Lines drawn at chart2's automatic 45 degrees, by the line's own direction vector.

    PyMuPDF reports a rotated span's AXIS-ALIGNED box, so a 45 degree label comes back as a
    square and reads exactly like an unrotated label far too wide for its slot. The `dir`
    vector is the discriminator and it is the only one -- and a turned label the reference
    OUTLINES is absent from get_text entirely, so a zero on the reference side is not
    evidence that it drew the label upright.
    """
    import pymupdf
    count = 0
    with pymupdf.open(pdf) as doc:
        for page in doc:
            for block in page.get_text("dict")["blocks"]:
                for line in block.get("lines", []):
                    dx, dy = line["dir"]
                    if abs(dx - 0.70711) < 0.004 and abs(dy + 0.70711) < 0.004:
                        count += 1
    return count


def pages(pdf):
    out = subprocess.run(["pdfinfo", pdf], capture_output=True).stdout.decode()
    m = re.search(r"^Pages:\s+(\d+)", out, re.M)
    return int(m.group(1)) if m else -1


def ink(ours, ref, work):
    """Summed unsigned ink over the pages, or None when the pages do not line up."""
    out = subprocess.run(
        ["python3", DIFF, ours, ref, "--outdir", os.path.join(work, "d")],
        capture_output=True, text=True).stdout
    total, major = 0.0, 0
    for line in out.splitlines():
        cells = line.split("\t")
        if len(cells) >= 5 and cells[0].isdigit():
            try:
                total += float(cells[3])
            except ValueError:
                return None, None
            if len(cells) > 5 and "major" in cells[5].lower():
                major += 1
    return (total, major) if total or "pages," in out else (None, None)


HEAD = ("document\tpages_ref\tpages_off\tpages_on\tglyph_ref\tglyph_off\tglyph_on"
        "\tink_off\tink_on\tmajor_off\tmajor_on\tturn_ref\tturn_off\tturn_on")
print(HEAD)

with open(sys.argv[2], "w") as fh:
    fh.write(HEAD + "\n")

    for rel in [l.strip() for l in open(sys.argv[1]) if l.strip()]:
        ref = os.path.join(REF, ident(rel) + ".pdf")
        work = tempfile.mkdtemp(prefix="score-", dir="/home/user/r106-work/tmp")
        try:
            a, b = os.path.join(work, "off"), os.path.join(work, "on")
            os.makedirs(a); os.makedirs(b)
            off, on = render(rel, "0", a), render(rel, "", b)
            if off is None or on is None or not os.path.exists(ref):
                row = [rel] + ["-"] * 13
            else:
                io, mo = ink(off, ref, work)
                ii, mi = ink(on, ref, work)
                row = [rel, pages(ref), pages(off), pages(on),
                       glyphs(ref), glyphs(off), glyphs(on),
                       f"{io:.2f}" if io is not None else "-",
                       f"{ii:.2f}" if ii is not None else "-",
                       mo if mo is not None else "-", mi if mi is not None else "-",
                       turned(ref), turned(off), turned(on)]
            line = "\t".join(str(c) for c in row)
            print(line, flush=True)
            fh.write(line + "\n"); fh.flush()
        finally:
            shutil.rmtree(work, ignore_errors=True)
