#!/usr/bin/env python3
"""Every OOXML chart part in the corpus by its c:style, and whether it states its own fills.

`ObjectFormatter`'s automatic chart-space and plot-area fill tables split at style 33, and
`ObjectTypeFormatter`'s constructor forces noFill for a pptx below that
(`objectformatter.cxx:956-959`, `oox/source/ppt/pptimport.cxx:309`), so only 33 and up can put
ink on a page that has none today.
"""
import os, re, sys, zipfile, collections

def parts(path):
    try: z = zipfile.ZipFile(path)
    except Exception: return
    for n in z.namelist():
        if re.match(r'(ppt|xl|word)/charts/chart\d*\.xml$', n):
            try: yield n, z.read(n).decode("utf-8", "replace")
            except Exception: pass

def style(s):
    for m in re.finditer(r'<c:style val="(\d+)"/>', s):
        v = int(m.group(1))
        if 1 <= v <= 48: return v
    return 2

if __name__ == "__main__":
    root = sys.argv[1]
    tal = collections.Counter()
    for fam in ("slides", "sheets", "words"):
        for dirpath, _, names in os.walk(os.path.join(root, fam)):
            for nm in sorted(names):
                if not nm.lower().endswith((".pptx", ".xlsx", ".docx", ".xlsm", ".pptm", ".docm")):
                    continue
                for part, s in parts(os.path.join(dirpath, nm)):
                    st = style(s)
                    plot = re.search(r'<c:plotArea>(.*?)</c:plotArea>', s, re.S)
                    body = plot.group(1) if plot else ""
                    # a plotArea's own spPr is its last direct child, not a series'
                    own_plot = bool(re.search(r'</c:\w+Ax><c:spPr>|</c:dTable><c:spPr>', s))
                    # chartSpace spPr is a direct child of c:chartSpace
                    own_space = bool(re.search(r'</c:chart><c:spPr>', s))
                    tal[(fam, st)] += 1
                    if st >= 33:
                        print(f"{fam}\t{nm}\t{part}\tstyle={st}\tspacespPr={own_space}\tplotspPr={own_plot}")
    print("# tally", file=sys.stderr)
    for k in sorted(tal): print(f"#\t{k[0]}\tstyle {k[1]}\t{tal[k]}", file=sys.stderr)
