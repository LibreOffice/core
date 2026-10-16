#!/usr/bin/env python3
"""How often an ODF header or footer band's declared height is smaller than text + gap.

Calc's band is `max(nManHeight, maxTextHeight + nDistance)` (ScPrintFunc::UpdateHFHeight,
sc/source/ui/view/printfun.cxx:838-850), where nManHeight is fo:min-height (or svg:height) and
nDistance is the header's fo:margin-bottom / the footer's fo:margin-top. Reading the declared
height alone is right only while text + gap stays under it.

A one-line band is counted as 11.5 pt of text, which is a 10 pt Liberation Sans line -- the shape
LibreOffice puts in the bands it writes. The point of the census is the *gap*, which is the term
that varies.
"""
import os, re, sys, zipfile
CORPUS = "/home/user/corpus-odf"
LINE = 11.5  # pt


def pt(v):
    if not v: return None
    for suffix, scale in (("in", 72.0), ("cm", 72 / 2.54), ("mm", 72 / 25.4), ("pt", 1.0)):
        if v.endswith(suffix):
            try: return float(v[:-len(suffix)]) * scale
            except ValueError: return None
    return None


def main():
    ext = sys.argv[1] if len(sys.argv) > 1 else "ods"
    docs, hit, worst = 0, 0, []
    for root, _d, files in os.walk(CORPUS):
        for n in files:
            if not n.lower().endswith("." + ext): continue
            try: s = zipfile.ZipFile(os.path.join(root, n)).read("styles.xml").decode("utf-8", "replace")
            except Exception: continue
            docs += 1
            over = 0.0
            for kind, gap in (("header", "margin-bottom"), ("footer", "margin-top")):
                for m in re.finditer(
                        r'<style:%s-style>\s*<style:header-footer-properties([^>]*)' % kind, s):
                    a = dict(re.findall(r'([\w:-]+)="([^"]*)"', m.group(1)))
                    declared = pt(a.get("svg:height")) or pt(a.get("fo:min-height")) or 0.0
                    distance = pt(a.get("fo:" + gap)) or 0.0
                    over = max(over, LINE + distance - declared)
            if over > 0.5:
                hit += 1
                worst.append((round(over, 2), n))
    worst.sort(reverse=True)
    print(f".{ext}: {hit} of {docs} documents declare a band smaller than one line plus its gap")
    for w in worst[:12]: print("   ", w[0], "pt short  ", w[1][:60])


main()
