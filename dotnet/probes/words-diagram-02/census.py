#!/usr/bin/env python3
"""What a SmartArt diagram's data model holds, per corpus document, for the whole corpus.

Walks by `git ls-files` and reads each package's zip directory, so it cannot see a case-variant
alias and cannot double-count. Counts the *authored* points -- the ones extraction is entitled to,
which is every `dgm:pt` that is not `doc`, `pres`, `parTrans` or `sibTrans` and whose `dgm:t`
carries text.
"""
import re, subprocess, sys, zipfile
from xml.etree import ElementTree as ET

DGM = "http://schemas.openxmlformats.org/drawingml/2006/diagram"
A = "http://schemas.openxmlformats.org/drawingml/2006/main"
ROOT = sys.argv[1] if len(sys.argv) > 1 else "/home/user/sample-files"
SKIP = {"doc", "pres", "parTrans", "sibTrans"}

paths = subprocess.run(["git", "-c", "core.quotePath=false", "ls-files"],
                       capture_output=True, text=True, cwd=ROOT).stdout.split("\n")
total = {}
for rel in paths:
    if not rel or not re.search(r"\.(doc|dot)[xm]?$|\.(ppt|pot|pps)[xm]?$|\.(xls|xlt)[xmb]?$", rel, re.I):
        continue
    try:
        z = zipfile.ZipFile(f"{ROOT}/{rel}")
    except Exception:                                            # noqa: BLE001
        continue
    parts = [n for n in z.namelist() if re.search(r"diagrams/data\d*\.xml$", n)]
    if not parts:
        continue
    pts = paras = words = 0
    for p in sorted(parts):
        try:
            root = ET.fromstring(z.read(p))
        except Exception:                                        # noqa: BLE001
            continue
        lst = root.find(f"{{{DGM}}}ptLst")
        if lst is None:
            continue
        for pt in lst.findall(f"{{{DGM}}}pt"):
            if pt.get("type") in SKIP:
                continue
            body = pt.find(f"{{{DGM}}}t")
            if body is None:
                continue
            got = []
            for para in body.findall(f"{{{A}}}p"):
                txt = "".join(e.text or "" for e in para.iter(f"{{{A}}}t"))
                got.append(txt)
            if any(t.strip() for t in got):
                pts += 1
                paras += len(got)
                words += sum(len(t.split()) for t in got)
    total[rel] = (len(parts), pts, paras, words)

fam = {}
for rel, (parts, pts, paras, words) in sorted(total.items()):
    print(f"{rel}\tparts={parts}\ttextpts={pts}\tparas={paras}\twords={words}")
    k = rel.split("/")[0]
    a = fam.setdefault(k, [0, 0, 0, 0, 0])
    a[0] += 1
    for i, v in enumerate((parts, pts, paras, words)):
        a[i + 1] += v
print()
for k, a in sorted(fam.items()):
    print(f"{k}: {a[0]} documents, {a[1]} data parts, {a[2]} authored points, "
          f"{a[3]} paragraphs, {a[4]} words")
