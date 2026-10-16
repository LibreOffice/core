#!/usr/bin/env python3
"""Which corpus chart parts draw a legend whose face the per-object rule changes.

`FamilyOf`'s second term -- the first literal a:latin anywhere in the part -- is what a legend
that states no c:txPr currently takes.  The reference takes the chart space's c:txPr, and the
theme's minor face when there is none.  This lists every part where a c:legend exists, the
chart space states no c:txPr of its own, and some other element in the part states a literal
face that differs from the theme's minor.
"""
import os, re, sys, zipfile
import xml.etree.ElementTree as ET

C = "{http://schemas.openxmlformats.org/drawingml/2006/chart}"
A = "{http://schemas.openxmlformats.org/drawingml/2006/main}"


def face(el):
    if el is None:
        return None
    for rpr in list(el.iter(A + "defRPr")) + list(el.iter(A + "rPr")) + list(el.iter(A + "endParaRPr")):
        lat = rpr.find(A + "latin")
        if lat is not None:
            t = (lat.get("typeface") or "").strip()
            if t and t[0] != "+":
                return t
    for lat in el.iter(A + "latin"):
        t = (lat.get("typeface") or "").strip()
        if t and t[0] != "+":
            return t
    return None


def child(el, name):
    return None if el is None else el.find(C + name)


def minor(z):
    """The theme's minor Latin face.

    **Not `theme1.xml`.** A workbook names its theme part whatever it likes -- 
    `064_Small_business_cash_flow_Use_this_template_0a640756.xlsx` calls it `theme11.xml` -- and
    a census that hardcodes `theme1` reads *no theme at all* for it and then reports that the
    legend's face does not change.  That miss cost this round one of its two cross-track
    verdicts in the write-up before it was caught.  Every theme part is read; where they
    disagree the first is taken and the caller is none the wiser, which is a known limit.
    """
    for n in sorted(z.namelist()):
        if re.match(r"(ppt|word|xl)/theme/theme\d*\.xml$", n):
            try:
                r = ET.fromstring(z.read(n))
            except Exception:
                continue
            mn = r.find(f".//{A}fontScheme/{A}minorFont/{A}latin")
            if mn is not None and (mn.get("typeface") or "").strip():
                return mn.get("typeface").strip()
    return None


if __name__ == "__main__":
    seen = set()
    print("doc\tpart\tnow\tafter")
    hits = {}
    for dirpath, _, names in os.walk(sys.argv[1]):
        for n in sorted(names):
            if not n.lower().endswith((".pptx", ".pptm", ".potx", ".ppsx", ".ppsm",
                                       ".xlsx", ".xlsm", ".xltx", ".xltm",
                                       ".docx", ".docm", ".dotx")):
                continue
            p = os.path.join(dirpath, n)
            key = n.lower()
            if key in seen:
                continue
            seen.add(key)
            try:
                z = zipfile.ZipFile(p)
            except Exception:
                continue
            for m in sorted(z.namelist()):
                if not re.match(r"(ppt|word|xl)/charts/chart\d+\.xml$", m):
                    continue
                try:
                    root = ET.fromstring(z.read(m))
                except Exception:
                    continue
                chart = child(root, "chart")
                legend = child(chart, "legend")
                if legend is None:
                    continue
                space = face(child(root, "txPr"))
                anywhere = face(root)
                th = minor(z)
                now = space or anywhere or th
                after = face(child(legend, "txPr")) or space or th
                if now != after:
                    print(f"{n}\t{m}\t{now}\t{after}")
                    hits[n] = hits.get(n, 0) + 1
    sys.stderr.write(f"{len(hits)} documents, {sum(hits.values())} chart parts\n")
