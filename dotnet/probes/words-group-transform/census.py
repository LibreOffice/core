import sys, zipfile, re
from pathlib import Path
import xml.etree.ElementTree as ET

A = "{http://schemas.openxmlformats.org/drawingml/2006/main}"
WP = "{http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing}"
WPG = "{http://schemas.microsoft.com/office/word/2010/wordprocessingGroup}"
WPS = "{http://schemas.microsoft.com/office/word/2010/wordprocessingShape}"
PIC = "{http://schemas.openxmlformats.org/drawingml/2006/picture}"

def xfrm(sp_pr):
    if sp_pr is None: return None
    x = sp_pr.find(A + "xfrm")
    if x is None: return None
    off, ext = x.find(A + "off"), x.find(A + "ext")
    cho, che = x.find(A + "chOff"), x.find(A + "chExt")
    def pair(e, a, b):
        if e is None: return None
        try: return int(e.get(a)), int(e.get(b))
        except (TypeError, ValueError): return None
    return dict(off=pair(off, "x", "y"), ext=pair(ext, "cx", "cy"),
                chOff=pair(cho, "x", "y"), chExt=pair(che, "cx", "cy"))

def members(group):
    for child in group:
        t = child.tag
        if t == WPS + "wsp":
            yield "shape", child.find(WPS + "spPr")
        elif t == PIC + "pic":
            yield "pic", child.find(PIC + "spPr")
        elif t == WPG + "grpSp":
            yield "group", child

def walk(group, sx, sy, tx, ty, depth, out):
    """Place a group's members. sx/sy are cumulative scales, tx/ty the group's origin."""
    if depth > 64: return
    pr = group.find(WPG + "grpSpPr")
    t = xfrm(pr)
    fx = fy = 1.0
    chx = chy = 0
    if t:
        if t["chExt"] and t["ext"]:
            if t["chExt"][0]: fx = t["ext"][0] / t["chExt"][0]
            if t["chExt"][1]: fy = t["ext"][1] / t["chExt"][1]
        if t["chOff"]: chx, chy = t["chOff"]
    for kind, node in members(group):
        if kind == "group":
            gt = xfrm(node.find(WPG + "grpSpPr"))
            if not gt or not gt["off"] or not gt["ext"]: continue
            walk(node, sx * fx, sy * fy,
                 tx + (gt["off"][0] - chx) * sx * fx,
                 ty + (gt["off"][1] - chy) * sy * fy, depth + 1, out)
        else:
            st = xfrm(node)
            if not st or not st["off"] or not st["ext"]: continue
            x = tx + (st["off"][0] - chx) * sx * fx
            y = ty + (st["off"][1] - chy) * sy * fy
            out.append((x, y, x + st["ext"][0] * sx * fx, y + st["ext"][1] * sy * fy))

def anchors(root):
    for tag in ("anchor", "inline"):
        for a in root.iter(WP + tag):
            ext = a.find(WP + "extent")
            if ext is None: continue
            g = a.find(".//" + WPG + "wgp")
            if g is None: continue
            try: yield int(ext.get("cx")), int(ext.get("cy")), g
            except (TypeError, ValueError): pass

worst = []
for path in sorted(Path(sys.argv[1]).rglob("*")):
    if path.suffix.lower() not in (".docx", ".docm", ".dotx"): continue
    try: z = zipfile.ZipFile(path)
    except Exception: continue
    groups = off = 0
    ratios = []
    for name in z.namelist():
        if not re.match(r"word/(document|header\d*|footer\d*)\.xml$", name): continue
        try: root = ET.fromstring(z.read(name))
        except Exception: continue
        for cx, cy, g in anchors(root):
            groups += 1
            out = []
            walk(g, 1.0, 1.0, 0.0, 0.0, 0, out)
            if not out: continue
            w = max(r[2] for r in out) - min(r[0] for r in out)
            h = max(r[3] for r in out) - min(r[1] for r in out)
            if w <= 0 or h <= 0 or cx <= 0 or cy <= 0: continue
            rx, ry = cx / w, cy / h
            if abs(rx - 1) > 0.02 or abs(ry - 1) > 0.02:
                off += 1
                ratios.append((rx, ry))
    if off:
        m = max(max(abs(r[0] - 1), abs(r[1] - 1)) for r in ratios)
        worst.append((m, off, groups, path.name))
worst.sort(reverse=True)
print(f"{len(worst)} documents with a mis-fitting group anchor")
print(f"{sum(w[1] for w in worst)} anchors of {sum(w[2] for w in worst)} in those documents")
for m, off, groups, name in worst[:40]:
    print(f"  {m:8.3f}  {off:4d}/{groups:<4d}  {name}")
