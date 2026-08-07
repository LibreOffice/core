#!/usr/bin/env python3
"""Split NAS-Infrastructure-Roadmaps-v16.0.pptx's per-page ink by what the slide carries.

The record says 152.12 of its ink is a named ceiling — `mc:Choice Requires="v"` around a
linked p:oleObj that LibreOffice takes and draws nothing for — and that the rest was
unexamined. This re-derives the split from this round's own sweep rather than trusting it,
and then subdivides the remainder by other features so the next thing to chase is named.
"""
import re, sys, zipfile, collections, os

DECK = "/workspace/sample-files/slides/batch-012/pptx/NAS-Infrastructure-Roadmaps-v16.0.pptx"
REPORT = ("/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/"
          "scratchpad/sl14-base/cmp/NAS-Infrastructure-Roadmaps-v16.0__pptx.txt")

ink, major = {}, {}
for line in open(REPORT, encoding="utf-8", errors="replace"):
    f = line.rstrip("\n").split("\t")
    if len(f) >= 3 and re.fullmatch(r"\d+", f[0]) and re.fullmatch(r"[\d.]+", f[2]):
        ink[int(f[0])] = float(f[2])
        major[int(f[0])] = ("MAJOR" in line)

z = zipfile.ZipFile(DECK)

# ppt/slides/slideN.xml -> presentation order. The sweep's page numbers are the PDF's, which
# follow p:sldIdLst, so read that rather than assuming slideN.xml is page N.
pres = z.read("ppt/presentation.xml").decode("utf-8", "replace")
rels = z.read("ppt/_rels/presentation.xml.rels").decode("utf-8", "replace")
target = dict(re.findall(r'Id="([^"]+)"[^>]*Target="([^"]+)"', rels))
order = []
for m in re.finditer(r'<p:sldId[^>]*r:id="([^"]+)"', pres):
    t = target.get(m.group(1), "")
    order.append("ppt/slides/" + os.path.basename(t))

print("slides in presentation order: %d   pages with ink: %d" % (len(order), len(ink)))

FEATURES = {
    "oleObj_requires_v": None,   # computed specially
    "graphicFrame":  re.compile(r"<p:graphicFrame\b"),
    "table":         re.compile(r"<a:tbl\b"),
    "chart":         re.compile(r"<c:chart\b|graphicframe.*chart"),
    "blipFill":      re.compile(r"<a:blipFill\b"),
    "pattFill":      re.compile(r"<a:pattFill\b"),
    "gradFill":      re.compile(r"<a:gradFill\b"),
    "diagram":       re.compile(r"<dgm:relIds\b"),
    "prstTxWarp":    re.compile(r"<a:prstTxWarp\b"),
    "outerShdw":     re.compile(r"<a:outerShdw\b"),
}

rows = []
for page, name in enumerate(order, start=1):
    if page not in ink:
        continue
    try:
        xml = z.read(name).decode("utf-8", "replace")
    except KeyError:
        continue
    # An mc:Choice requiring the VML namespace, wrapped round an OLE object.
    ole_v = False
    for m in re.finditer(r'<mc:Choice\b[^>]*Requires="([^"]*)"', xml):
        if "v" in m.group(1).split():
            end = xml.find("</mc:Choice>", m.start())
            if end > 0 and "<p:oleObj" in xml[m.start():end]:
                ole_v = True
    feats = {k: bool(v.search(xml)) for k, v in FEATURES.items() if v is not None}
    feats["oleObj_requires_v"] = ole_v
    rows.append((page, ink[page], major[page], feats))

def report(title, keep):
    sel = [r for r in rows if keep(r)]
    tot = sum(r[1] for r in sel)
    maj = sum(1 for r in sel if r[2])
    print("%-42s pages %4d   ink %8.2f   major %3d" % (title, len(sel), tot, maj))
    return sel

print()
print("=== the recorded split ===")
report("carries a Requires=\"v\" p:oleObj", lambda r: r[3]["oleObj_requires_v"])
rest = report("everything else", lambda r: not r[3]["oleObj_requires_v"])
print()
print("=== the remainder, subdivided (overlapping) ===")
for key in FEATURES:
    if key == "oleObj_requires_v":
        continue
    sel = [r for r in rest if r[3][key]]
    if not sel:
        continue
    print("  %-24s pages %4d   ink %8.2f   major %3d"
          % (key, len(sel), sum(r[1] for r in sel), sum(1 for r in sel if r[2])))

print()
print("=== the remainder's worst pages ===")
for page, v, maj, feats in sorted(rest, key=lambda r: -r[1])[:15]:
    on = ",".join(k for k, y in feats.items() if y and k != "oleObj_requires_v")
    print("  page %-4d ink %6.2f %-6s %s" % (page, v, "MAJOR" if maj else "", on))
