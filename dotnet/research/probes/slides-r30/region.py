#!/usr/bin/env python3
"""LibreOffice's own computed plot rectangle for every chart in a deck.

`chart:coordinate-region` is the diagram rectangle *excluding axes*, as
`XDiagramPositioning::calculateDiagramPositionExcludingAxes` computed it
(xmloff/source/chart/SchXMLExport.cxx:2274). Converting a deck to `.odp` therefore
hands us the number we would otherwise have to infer from gridlines — for every chart,
including the ones that draw no grid at all.

Emits, per chart object: the slide page it sits on, the frame it was drawn in (page
points), the region (page points), and the four insets frame->region.
"""
import re, sys, zipfile, os

CM = 28.34645669  # points per cm
UNIT = re.compile(r"^([-\d.]+)(cm|mm|in|pt)$")

def pts(v):
    m = UNIT.match(v)
    if not m: raise ValueError(v)
    x = float(m.group(1))
    return {"cm": x * CM, "mm": x * CM / 10, "in": x * 72.0, "pt": x}[m.group(2)]

def frames(content):
    """Object name -> (page index 1-based, x, y, w, h) in page points."""
    out = {}
    for pi, page in enumerate(re.findall(r"<draw:page\b.*?</draw:page>", content, re.S), 1):
        for fr in re.findall(r"<draw:frame\b[^>]*>.*?</draw:frame>", page, re.S):
            href = re.search(r'<draw:object[^>]*xlink:href="\./([^"]+)"', fr)
            if not href: continue
            g = lambda a: re.search(r'\b%s="([^"]+)"' % a, fr[:fr.index(">")+1])
            try:
                x, y = pts(g("svg:x").group(1)), pts(g("svg:y").group(1))
                w, h = pts(g("svg:width").group(1)), pts(g("svg:height").group(1))
            except Exception:
                continue
            out[href.group(1)] = (pi, x, y, w, h)
    return out

def regions(odp):
    rows = []
    with zipfile.ZipFile(odp) as z:
        fr = frames(z.read("content.xml").decode("utf-8"))
        for n in sorted(z.namelist()):
            if not (n.endswith("/content.xml") and n.startswith("Object")): continue
            obj = n.split("/")[0]
            s = z.read(n).decode("utf-8")
            m = re.search(r'<chart:coordinate-region ([^/>]*)/>', s)
            if not m: continue
            a = dict(re.findall(r'(\S+)="([^"]+)"', m.group(1)))
            rx, ry = pts(a["svg:x"]), pts(a["svg:y"])
            rw, rh = pts(a["svg:width"]), pts(a["svg:height"])
            kind = re.search(r'chart:class="([^"]+)"', s)
            if obj not in fr: continue
            pi, fx, fy, fw, fh = fr[obj]
            rows.append(dict(obj=obj, page=pi, kind=(kind.group(1) if kind else "?"),
                             frame=(fx, fy, fw, fh),
                             region=(fx + rx, fy + ry, rw, rh),
                             inset=(rx, ry, fw - rx - rw, fh - ry - rh)))
    rows.sort(key=lambda r: (r["page"], r["frame"][1], r["frame"][0]))
    return rows

if __name__ == "__main__":
    for r in regions(sys.argv[1]):
        fx, fy, fw, fh = r["frame"]
        x, y, w, h = r["region"]
        l, t, rr, b = r["inset"]
        print(f'{r["obj"]:>9} p{r["page"]:<3} {r["kind"]:<14} '
              f'frame({fx:7.2f},{fy:7.2f},{fw:7.2f},{fh:7.2f}) '
              f'region({x:7.2f},{y:7.2f},{w:7.2f},{h:7.2f}) '
              f'inset L{l:7.2f} T{t:7.2f} R{rr:7.2f} B{b:7.2f}')
