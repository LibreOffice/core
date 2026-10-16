#!/usr/bin/env python3
"""Which rendered page of each corpus deck carries an OOXML chart part.

Reads the package rather than the rendering: a slide's own .rels naming a
.../relationships/chart target.  Slide order is presentation.xml's <p:sldIdLst>, and hidden
slides (p:sld/@show="0") are dropped because neither stack renders them -- which is what makes
the page number, rather than the slide number, come out right.
"""
import os, re, sys, zipfile

REL = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"


def pages(path):
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return []
    try:
        pres = z.read("ppt/presentation.xml").decode("utf-8", "replace")
        rels = z.read("ppt/_rels/presentation.xml.rels").decode("utf-8", "replace")
    except KeyError:
        return []
    target = dict(re.findall(r'Id="([^"]+)"[^>]*Target="([^"]+)"', rels))
    order = [target.get(m) for m in re.findall(r'<p:sldId [^>]*r:id="([^"]+)"', pres)]
    out, page = [], 0
    for t in order:
        if not t: continue
        name = "ppt/" + t.lstrip("/").replace("../", "")
        try:
            body = z.read(name).decode("utf-8", "replace")
        except KeyError:
            continue
        if re.search(r'<p:sld\b[^>]*\bshow="(?:0|false)"', body):
            continue
        page += 1
        try:
            r = z.read(os.path.dirname(name) + "/_rels/" + os.path.basename(name) + ".rels")
        except KeyError:
            continue
        if "/relationships/chart" in r.decode("utf-8", "replace"):
            out.append(page)
    return out


if __name__ == "__main__":
    root = sys.argv[1]
    for dirpath, _, names in os.walk(root):
        for n in sorted(names):
            if not n.lower().endswith((".pptx", ".ppt")): continue
            p = os.path.join(dirpath, n)
            ps = pages(p)
            if ps:
                stem, ext = os.path.splitext(n)
                print(f"{stem}__{ext[1:].lower()}\t{','.join(map(str, ps))}")
