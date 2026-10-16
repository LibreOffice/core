#!/usr/bin/env python3
"""How far the two findings of `audit_pagedecoration.py` reach in the corpus.

(1) `header > top` or `footer > bottom` -- the stated band is *negative*, where 26.2.4.2
    starts the body at the page margin and we start it at the band margin, an 18 pt
    displacement on the probe.
(2) a band that is positive but smaller than the reference's text-fit threshold, which the
    probe brackets at 1.44-2.16 pt for 8 pt text and 4.32-5.76 pt for 20 pt text -- so
    roughly 0.27x the font size, and certainly not the `> 0` the site claims.

Counts only worksheets that actually state header or footer *content*, because a band with
no text in it draws nothing either way and cannot separate the two behaviours.
"""
import collections, csv, os, re, zipfile
import xml.etree.ElementTree as ET

CORPUS = "/c/sandbox/workdir/sample-files"
NS = "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}"


def main():
    negative = []
    tiny = []
    sheets = 0
    docs = 0
    for r in csv.DictReader(open(os.path.join(CORPUS, "MANIFEST.tsv"), newline=""), delimiter="\t"):
        if not r["path"].lower().endswith((".xlsx", ".xlsm", ".xltx")):
            continue
        p = os.path.join(CORPUS, r["path"])
        if not os.path.exists(p):
            continue
        try:
            z = zipfile.ZipFile(p)
        except Exception:
            continue
        docs += 1
        with z:
            for name in z.namelist():
                if not re.match(r"xl/worksheets/sheet\d+\.xml$", name):
                    continue
                try:
                    root = ET.fromstring(z.read(name))
                except Exception:
                    continue
                m = root.find(NS + "pageMargins")
                hf = root.find(NS + "headerFooter")
                if m is None or hf is None:
                    continue
                text = "".join((e.text or "") for e in hf)
                if not text.strip():
                    continue
                sheets += 1
                g = lambda k, d: float(m.get(k, d))
                top, bottom = g("top", 0.75), g("bottom", 0.75)
                head, foot = g("header", 0.3), g("footer", 0.3)
                for label, band in (("header", top - head), ("footer", bottom - foot)):
                    pts = band * 72
                    if pts < 0:
                        negative.append((r["family"], r["status"], r["path"], name, label, round(pts, 2)))
                    elif 0 < pts < 6:
                        tiny.append((r["family"], r["status"], r["path"], name, label, round(pts, 2)))
    print("worksheets with header/footer content: %d, in %d packages opened" % (sheets, docs))
    print("== negative bands (header > top, or footer > bottom) ==", len(negative))
    for row in sorted(negative):
        print("   ", row[0], row[1], row[2].split("/")[-1], row[3], row[4], row[5], "pt")
    print("== positive bands under 6 pt ==", len(tiny))
    for row in sorted(tiny):
        print("   ", row[0], row[1], row[2].split("/")[-1], row[3], row[4], row[5], "pt")


main()
