#!/usr/bin/env python3
"""How many corpus documents state a field whose cached result carries its own `w:rPr`?

Split by whether the instruction carries `\\* MERGEFORMAT`, because that is what decides
whether Word itself keeps that formatting across an update and 26.2.4.2 therefore diverges
from Word rather than from the file.
"""
import collections, pathlib, re, sys, zipfile
import xml.etree.ElementTree as ET

W = "{http://schemas.openxmlformats.org/wordprocessingml/2006/main}"
ROOT = pathlib.Path("/home/user/sample-files")

# The fields whose value this tree recomputes, so that the question can arise at all.
COMPUTED = re.compile(r'\b(PAGE|NUMPAGES|SECTIONPAGES|DATE|TIME|FILENAME|REF|PAGEREF|SEQ|STYLEREF)\b',
                      re.IGNORECASE)


# The properties that change the drawn glyphs. A cached result stating only `w:noProof`,
# `w:webHidden` or `w:lang` is formatted identically either way, so it is not reach.
VISIBLE = {"rFonts", "sz", "szCs", "color", "b", "bCs", "i", "iCs", "u", "caps", "smallCaps",
           "strike", "dstrike", "spacing", "position", "vertAlign", "highlight", "shd",
           "outline", "emboss", "imprint", "w", "kern"}


def visible(run):
    pr = run.find(W + "rPr")
    return pr is not None and any(c.tag[len(W):] in VISIBLE for c in pr)


def arms(part):
    """(instruction, whether the result runs state an rPr) for each field in one part."""
    try:
        root = ET.fromstring(part)
    except ET.ParseError:
        return
    for element in root.iter(W + "fldSimple"):
        instr = element.get(W + "instr") or ""
        yield instr, any(visible(r) for r in element.iter(W + "r"))

    # A complex field: the runs between `separate` and `end` are the cached result.
    for para in root.iter(W + "p"):
        instr, state, styled = [], 0, False
        for run in para.iter(W + "r"):
            char = run.find(W + "fldChar")
            kind = char.get(W + "fldCharType") if char is not None else None
            if kind == "begin":
                instr, state, styled = [], 1, False
                continue
            if kind == "separate":
                state = 2
                continue
            if kind == "end":
                if state == 2:
                    yield "".join(instr), styled
                state = 0
                continue
            if state == 1:
                text = run.find(W + "instrText")
                if text is not None and text.text:
                    instr.append(text.text)
            elif state == 2 and visible(run):
                styled = True


def main():
    merge = collections.Counter()
    plain = collections.Counter()
    docs = {"merge": set(), "plain": set()}
    for path in sorted(ROOT.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in (".docx", ".docm"):
            continue
        try:
            package = zipfile.ZipFile(path)
        except Exception:
            continue
        for name in package.namelist():
            if not name.startswith("word/") or not name.endswith(".xml"):
                continue
            for instr, styled in arms(package.read(name)):
                if not styled or not COMPUTED.search(instr):
                    continue
                bucket = "merge" if "MERGEFORMAT" in instr.upper() else "plain"
                (merge if bucket == "merge" else plain)[instr.strip()[:40]] += 1
                docs[bucket].add(str(path))
    print(f"with \\* MERGEFORMAT : {sum(merge.values()):5d} fields in {len(docs['merge']):3d} documents")
    print(f"without            : {sum(plain.values()):5d} fields in {len(docs['plain']):3d} documents")
    for label, counter in (("MERGEFORMAT", merge), ("plain", plain)):
        print(f"--- {label}, commonest instructions")
        for instr, n in counter.most_common(8):
            print(f"   {n:6d}  {instr}")
    with open("movers.txt", "w") as fh:
        for name in sorted(docs["plain"]):
            fh.write(name + "\n")
    print("wrote movers.txt:", len(docs["plain"]))


if __name__ == "__main__":
    main()
