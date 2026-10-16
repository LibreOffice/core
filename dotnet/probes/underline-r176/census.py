#!/usr/bin/env python3
"""How many corpus DOCX state a `w:u` carrying no `w:val`, and where.

`CT_Underline`'s `val` is optional with no default, so such an element emits no sprm and the run
keeps whatever an outer layer stated.  This tree read the absent attribute as `single`.
"""
import collections, pathlib, re, zipfile

ROOT = pathlib.Path("/home/user/sample-files")
U = re.compile(rb"<w:u\b[^>]*/?>")


def main():
    documents = collections.Counter()
    valueless = withvalue = 0
    for path in sorted(ROOT.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in (".docx", ".docm"):
            continue
        try:
            package = zipfile.ZipFile(path)
        except Exception:
            continue
        n = 0
        for name in package.namelist():
            if not name.startswith("word/") or not name.endswith(".xml"):
                continue
            for tag in U.findall(package.read(name)):
                if re.search(rb'\sw:val="', tag):
                    withvalue += 1
                else:
                    valueless += 1
                    n += 1
        if n:
            documents[path.name] = n
    print(f"{withvalue} <w:u> state a w:val; {valueless} do not, in {len(documents)} documents")
    for name, n in documents.most_common(20):
        print(f"  {n:>5}  {name[:70]}")


if __name__ == "__main__":
    main()
