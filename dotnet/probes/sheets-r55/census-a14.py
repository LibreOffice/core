#!/usr/bin/env python3
"""Census: every `mc:Choice` in the 946-document corpus, keyed on the *resolved*
namespace URI of each prefix its `Requires` names, not on the prefix text.

Why it exists: `oox`'s `ContextHandler2Helper::prepareMceContext` keeps a list of
MCE namespaces it supports and `a14` is explicitly **not** on it, while XRay.Office's
`OoxmlNamespaces.UnderstoodExtensions` does contain `DrawingML2010` (= a14). This
counts what changes if the two are reconciled.

For each hit it records: the document, the part, the Requires string, the resolved
URIs, whether the `mc:Fallback` beside it is absent / empty / has content, and the
element names + graphicData URIs the choice wraps -- so reach is estimated from what
the choice *resolves to*, not from the attribute.
"""
import csv, io, os, re, sys, zipfile
import xml.etree.ElementTree as ET

CORPUS = "/c/sandbox/workdir/sample-files"
MC = "http://schemas.openxmlformats.org/markup-compatibility/2006"
GD = "http://schemas.openxmlformats.org/drawingml/2006/main"

def docs():
    man = os.path.join(CORPUS, "MANIFEST.tsv")
    with open(man, newline="") as fh:
        r = csv.DictReader(fh, delimiter="\t")
        for row in r:
            yield row

def qn(t):
    return t.split("}")[-1]

def walk(elem, nsstack, out, part):
    # ElementTree drops namespace declarations, so prefixes cannot be resolved from it.
    pass

PREFIX_RE = re.compile(rb'xmlns:([A-Za-z0-9_.-]+)\s*=\s*"([^"]*)"')

def scan_part(data, doc, part, rows):
    if b"AlternateContent" not in data:
        return
    try:
        root = ET.fromstring(data)
    except Exception:
        return
    # Build prefix -> uri from the raw bytes. Declarations are scoped, but a prefix is
    # almost never redeclared to a different URI inside one part; where it is, the last
    # declaration wins and the row is flagged.
    decls = {}
    conflict = set()
    for m in PREFIX_RE.finditer(data):
        p = m.group(1).decode("utf8"); u = m.group(2).decode("utf8")
        if p in decls and decls[p] != u:
            conflict.add(p)
        decls[p] = u

    for ac in root.iter("{%s}AlternateContent" % MC):
        fb = ac.find("{%s}Fallback" % MC)
        if fb is None:
            fbkind = "absent"
        elif len(fb) == 0 and not (fb.text or "").strip():
            fbkind = "empty"
        else:
            fbkind = "content"
        for ch in ac.findall("{%s}Choice" % MC):
            req = ch.get("Requires", "")
            prefixes = [p for p in req.split() if p]
            uris = [decls.get(p, "?" + p) for p in prefixes]
            kids = sorted({qn(k.tag) for k in ch.iter()} & {
                "twoCellAnchor", "oneCellAnchor", "absoluteAnchor", "graphicFrame",
                "pic", "sp", "drawing", "wsp", "AlternateContent"})
            gds = sorted({k.get("uri", "") for k in ch.iter("{%s}graphicData" % GD)})
            exts = sorted({k.get("uri", "") for k in ch.iter("{%s}ext" % GD)})
            cam = "a14:cameraTool" if b"cameraTool" in data else ""
            rows.append(dict(
                doc=doc, part=part, requires=req, uris=" ".join(uris),
                fallback=fbkind, wraps=",".join(kids), graphicdata=",".join(gds),
                conflict=",".join(sorted(conflict & set(prefixes))), camera=cam))

def main():
    rows = []
    n = 0
    for row in docs():
        p = os.path.join(CORPUS, row["path"])
        if not os.path.exists(p):
            continue
        n += 1
        try:
            z = zipfile.ZipFile(p)
        except Exception:
            continue
        with z:
            for info in z.infolist():
                if not info.filename.lower().endswith((".xml", ".rels", ".vml")):
                    continue
                try:
                    data = z.read(info)
                except Exception:
                    continue
                scan_part(data, row["path"], info.filename, rows)
    w = csv.DictWriter(sys.stdout, delimiter="\t",
                       fieldnames=["doc", "part", "requires", "uris", "fallback",
                                   "wraps", "graphicdata", "conflict", "camera"])
    w.writeheader()
    for r in rows:
        w.writerow(r)
    print("# documents opened: %d, choices: %d" % (n, len(rows)), file=sys.stderr)

main()
