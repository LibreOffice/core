#!/usr/bin/env python3
"""Every OOXML chart part that states a text colour, by where it states it.

`ChartLayout.AxisColour` is a hardcoded black, so any chart naming a colour in a c:txPr, a
c:title's a:rPr or a c:dLbls' text properties is drawn in the wrong colour today.  This counts
the *statements*, not the resolved colours: a schemeClr needs the theme and the part's own
c:clrMapOvr to resolve and this census deliberately stops short of that.
"""
import os, re, sys, zipfile, collections

WHERE = (("catAx", r'<c:catAx>.*?</c:catAx>'), ("valAx", r'<c:valAx>.*?</c:valAx>'),
         ("dateAx", r'<c:dateAx>.*?</c:dateAx>'), ("title", r'<c:title>.*?</c:title>'),
         ("dLbls", r'<c:dLbls>.*?</c:dLbls>'), ("legend", r'<c:legend>.*?</c:legend>'))


def parts(path):
    try: z = zipfile.ZipFile(path)
    except Exception: return
    for n in z.namelist():
        if re.match(r'(ppt|xl|word)/charts/chart\d*\.xml$', n):
            try: yield n, z.read(n).decode("utf-8", "replace")
            except Exception: pass


def colours(block):
    out = []
    for m in re.finditer(r'<a:(?:def)?rPr\b[^>]*>(.*?)</a:(?:def)?rPr>', block, re.S | re.I):
        f = re.search(r'<a:solidFill>(.*?)</a:solidFill>', m.group(1), re.S)
        if not f: continue
        s = re.search(r'<a:srgbClr val="([0-9A-Fa-f]{6})"', f.group(1))
        c = re.search(r'<a:schemeClr val="(\w+)"', f.group(1))
        out.append("#" + s.group(1).upper() if s else ("scheme:" + c.group(1) if c else "?"))
    return out


if __name__ == "__main__":
    root = sys.argv[1]
    tal = collections.Counter()
    docs = collections.defaultdict(set)
    for fam in ("slides", "sheets", "words"):
        for dirpath, _, names in os.walk(os.path.join(root, fam)):
            for nm in sorted(names):
                if not nm.lower().endswith((".pptx", ".xlsx", ".docx", ".xlsm", ".pptm", ".docm")):
                    continue
                for part, s in parts(os.path.join(dirpath, nm)):
                    for what, pat in WHERE:
                        for block in re.findall(pat, s, re.S):
                            for c in colours(block):
                                if c == "#000000": continue
                                tal[(fam, what, c)] += 1
                                docs[(fam, c)].add(nm)
    for k, v in sorted(tal.items(), key=lambda t: -t[1]):
        print(f"{k[0]}\t{k[1]}\t{k[2]}\t{v}")
    print("# documents per family/colour", file=sys.stderr)
    for k, v in sorted(docs.items()):
        print(f"#\t{k[0]}\t{k[1]}\t{len(v)}\t{sorted(v)[:4]}", file=sys.stderr)
