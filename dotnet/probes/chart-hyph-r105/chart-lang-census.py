"""Census the languages and shapes of OOXML chart category-axis labels across the corpus.

For every chart part that states a category axis (c:catAx), record:
  * the cached category strings (c:cat//c:strCache//c:v, or c:multiLvlStrRef)
  * every xml:lang-ish @lang value stated anywhere in the chart part
  * the @lang values stated inside the c:catAx subtree specifically
  * whether any label is multi-word, and the longest single word

Legacy binary containers (.xls/.ppt/.doc) are counted but not opened: their charts are
BIFF/escher records, not XML.  They are reported as an unmeasured remainder.
"""
import os, re, sys, json, zipfile
import xml.etree.ElementTree as ET

ROOT = "/home/user/sample-files"
C = "{http://schemas.openxmlformats.org/drawingml/2006/chart}"
A = "{http://schemas.openxmlformats.org/drawingml/2006/main}"

def texts(el, tag):
    return [e.text or "" for e in el.iter(tag)]

def langs(el):
    out = set()
    for e in el.iter():
        v = e.get("lang")
        if v:
            out.add(v)
    return out

rows = []
nolabel = []
containers = 0
charted = 0
for dirpath, dirnames, filenames in os.walk(ROOT):
    if "/.git" in dirpath:
        continue
    for fn in sorted(filenames):
        ext = fn.rsplit(".", 1)[-1].lower() if "." in fn else ""
        if ext not in ("xlsx", "xlsm", "pptx", "docx"):
            continue
        p = os.path.join(dirpath, fn)
        rel = os.path.relpath(p, ROOT)
        containers += 1
        try:
            z = zipfile.ZipFile(p)
        except Exception as e:
            print("ZIPFAIL", rel, e, file=sys.stderr)
            continue
        parts = [n for n in z.namelist()
                 if re.search(r"charts/chart[^/]*\.xml$", n)]
        if parts:
            charted += 1
        for n in parts:
            try:
                root = ET.fromstring(z.read(n))
            except Exception as e:
                print("XMLFAIL", rel, n, e, file=sys.stderr)
                continue
            partlangs = langs(root)
            for ax in root.iter(C + "catAx"):
                axlangs = langs(ax)
                # categories live on the series, not the axis; grab all of them in the part
                cats = []
                for cat in root.iter(C + "cat"):
                    for v in cat.iter(C + "v"):
                        if v.text:
                            cats.append(v.text)
                cats = [c for c in cats if c.strip()]
                words = [w for c in cats for w in c.split()]
                rows.append(dict(
                    doc=rel, part=n,
                    axid=(ax.find(C + "axId").get("val") if ax.find(C + "axId") is not None else ""),
                    ncat=len(cats),
                    multiword=sum(1 for c in cats if len(c.split()) > 1),
                    longest_word=max((len(w) for w in words), default=0),
                    longest_label=max((len(c) for c in cats), default=0),
                    part_langs=sorted(partlangs),
                    ax_langs=sorted(axlangs),
                    sample=cats[:8],
                ))
        z.close()

json.dump(dict(containers=containers, charted=charted, rows=rows),
          open(sys.argv[1], "w"), indent=0)
print(f"containers={containers} charted={charted} catAx_rows={len(rows)}", file=sys.stderr)
