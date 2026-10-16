#!/usr/bin/env python3
"""How closely our header and footer text lands on the reference's.

    band-agreement.py <ours-dir> <ref-dir> [--per-doc]

The gate cannot see the band's face at all: its three checks are page count, extractable words
and *our own* unembedded fonts, and a band drawn at the wrong size in the wrong family has the
same tokens on the same pages. So the quantity the change controls has to be measured directly.

Method. Take only the xlsx/xlsm sheets documents that actually state header or footer content
(81 of them). Read each worksheet's `pageMargins` and take the widest `top` and `bottom` any
sheet of that workbook states -- so the strip is the workbook's own furniture area and never a
body row. `pdftotext -bbox` both PDFs, keep the words inside those strips, pair them by page and
by exact text in reading order, and report mean |dx| and |dy| plus the words that pair with
nothing.

A first cut of this took a flat 10% of the page and paired 419577 words -- almost all of them
body rows, which cannot move between two runs and simply diluted the figure to nothing. The
margin-driven strip is what makes the number mean what its name says.
"""
import csv, os, re, subprocess, sys, zipfile
from collections import defaultdict
import xml.etree.ElementTree as ET

CORPUS = "/c/sandbox/workdir/sample-files"
NS = "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}"
WORD = re.compile(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>')


def band_documents():
    """{pdf stem: (top strip pt, bottom strip pt)} for the workbooks that state band content."""
    out = {}
    for r in csv.DictReader(open(os.path.join(CORPUS, "MANIFEST.tsv"), newline=""), delimiter="\t"):
        if r["family"] != "sheets" or not r["path"].lower().endswith((".xlsx", ".xlsm")):
            continue
        p = os.path.join(CORPUS, r["path"])
        if not os.path.exists(p):
            continue
        stem = os.path.basename(r["path"])
        stem = stem[:stem.rindex(".")] + "__" + r["ext"]
        try:
            z = zipfile.ZipFile(p)
        except Exception:
            continue
        top = bottom = None
        content = False
        with z:
            for name in z.namelist():
                if not re.match(r"xl/worksheets/sheet\d+\.xml$", name):
                    continue
                try:
                    root = ET.fromstring(z.read(name))
                except Exception:
                    continue
                hf, m = root.find(NS + "headerFooter"), root.find(NS + "pageMargins")
                if hf is None or m is None:
                    continue
                if not "".join((e.text or "") for e in hf).strip():
                    continue
                content = True
                # The *narrowest* strip any of the workbook's sheets states, not the widest.
                # A first cut took the widest and a workbook whose sheets disagree then had one
                # sheet's body rows inside another sheet's strip: `FY2023-AIP-grants` alone
                # contributed 4616 "band" pairs over 33 pages, which is not a header. The
                # narrowest strip under-covers a header on a sheet with roomier margins and
                # never catches a body row, and only the first of those two errors is one this
                # figure can survive.
                top = float(m.get("top", 0.75)) if top is None else min(top, float(m.get("top", 0.75)))
                bottom = (float(m.get("bottom", 0.75)) if bottom is None
                          else min(bottom, float(m.get("bottom", 0.75))))
        if content:
            out[stem] = (top * 72, bottom * 72)
    return out


def bands(pdf, top_pt, bottom_pt):
    try:
        out = subprocess.run(["pdftotext", "-q", "-bbox", pdf, "-"],
                             capture_output=True, text=True, timeout=300).stdout
    except Exception:
        return None
    rows = []
    for n, chunk in enumerate(out.split("<page ")[1:], 1):
        m = re.match(r'width="([\d.]+)" height="([\d.]+)"', chunk)
        if not m:
            continue
        h = float(m.group(2))
        for w in WORD.finditer(chunk):
            x0, y0, _, y1 = (float(w.group(i)) for i in (1, 2, 3, 4))
            if y0 < top_pt or y1 > h - bottom_pt:
                rows.append((n, w.group(5), x0, y0))
    return rows


def compare(ours, ref):
    a_by, b_by = defaultdict(list), defaultdict(list)
    for p, t, x, y in ours:
        a_by[(p, t)].append((x, y))
    for p, t, x, y in ref:
        b_by[(p, t)].append((x, y))
    dx = dy = 0.0
    n = uo = ur = 0
    for key in set(a_by) | set(b_by):
        a, b = a_by.get(key, []), b_by.get(key, [])
        for i in range(min(len(a), len(b))):
            dx += abs(a[i][0] - b[i][0])
            dy += abs(a[i][1] - b[i][1])
            n += 1
        uo += max(0, len(a) - len(b))
        ur += max(0, len(b) - len(a))
    return n, dx, dy, uo, ur


def main():
    ours_dir, ref_dir = sys.argv[1], sys.argv[2]
    per_doc = "--per-doc" in sys.argv
    strips = band_documents()
    tot_n = tot_dx = tot_dy = 0.0
    tot_uo = tot_ur = docs = 0
    rows = []
    for stem, (top, bottom) in sorted(strips.items()):
        o, r = os.path.join(ours_dir, stem + ".pdf"), os.path.join(ref_dir, stem + ".pdf")
        if not (os.path.exists(o) and os.path.exists(r)):
            continue
        a, b = bands(o, top, bottom), bands(r, top, bottom)
        if a is None or b is None:
            continue
        docs += 1
        n, dx, dy, uo, ur = compare(a, b)
        tot_n += n; tot_dx += dx; tot_dy += dy; tot_uo += uo; tot_ur += ur
        if n:
            rows.append((dx / n + dy / n, stem, n, dx / n, dy / n, uo, ur))
        elif uo or ur:
            rows.append((0.0, stem, 0, 0.0, 0.0, uo, ur))
    print("documents %d   paired band words %d" % (docs, int(tot_n)))
    print("mean |dx| %.4f pt   mean |dy| %.4f pt"
          % (tot_dx / max(1, tot_n), tot_dy / max(1, tot_n)))
    print("unpaired: ours-only %d   reference-only %d" % (tot_uo, tot_ur))
    if per_doc:
        print()
        print("%-62s %7s %9s %9s %6s %6s" % ("document", "pairs", "|dx|", "|dy|", "ours+", "ref+"))
        for s, name, n, dx, dy, uo, ur in sorted(rows, reverse=True):
            print("%-62s %7d %9.3f %9.3f %6d %6d" % (name[:62], n, dx, dy, uo, ur))


main()
