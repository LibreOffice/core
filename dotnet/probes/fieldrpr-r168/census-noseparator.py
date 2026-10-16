#!/usr/bin/env python3
"""How many corpus fields are a page field written with no `w:fldChar w:fldCharType="separate"`?

Such a field has no cached result at all. 26.2.4.2 computes and draws its value; this tree
draws nothing, because the value is written at paint time over a span and there is no span.
"""
import collections, pathlib, re, zipfile
import xml.etree.ElementTree as ET

W = "{http://schemas.openxmlformats.org/wordprocessingml/2006/main}"
ROOT = pathlib.Path("/home/user/sample-files")
PAGE = re.compile(r'\b(PAGE|NUMPAGES|SECTIONPAGES)\b', re.IGNORECASE)


def fields(part):
    """(instruction, has a separator) for each complex field in one part."""
    try:
        root = ET.fromstring(part)
    except ET.ParseError:
        return
    for para in root.iter(W + "p"):
        instr, state, separated = [], 0, False
        for run in para.iter(W + "r"):
            char = run.find(W + "fldChar")
            kind = char.get(W + "fldCharType") if char is not None else None
            if kind == "begin":
                instr, state, separated = [], 1, False
                continue
            if kind == "separate":
                separated, state = True, 2
                continue
            if kind == "end":
                if state:
                    yield "".join(instr), separated
                state = 0
                continue
            if state == 1:
                text = run.find(W + "instrText")
                if text is not None and text.text:
                    instr.append(text.text)


def main():
    bare = collections.Counter()
    docs = collections.Counter()
    where = collections.defaultdict(set)
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
            for instr, separated in fields(package.read(name)):
                if separated or not PAGE.search(instr):
                    continue
                bare[instr.strip()[:40]] += 1
                docs[path.name] += 1
                where[path.name].add(name.split("/")[-1].rstrip("0123456789.xml") or name)
    print(f"{sum(bare.values())} separator-less page fields in {len(docs)} documents")
    for instr, n in bare.most_common(10):
        print(f"   {n:5d}  {instr}")
    for name, n in docs.most_common(20):
        print(f"   {n:4d}  {name[:60]}   parts: {sorted(where[name])}")


if __name__ == "__main__":
    main()
