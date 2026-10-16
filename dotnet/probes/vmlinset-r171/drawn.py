#!/usr/bin/env python3
"""How many of the corpus's VML text boxes a reader can actually reach.

The census counts `v:textbox` elements in the markup.  Most are the `mc:Fallback` half of an
`mc:AlternateContent` whose `mc:Choice` holds the same shape as DrawingML, so the DOCX reader takes
the `w:drawing` and never looks at the VML -- which is why 1638 stated boxes move 14 renderings.
Count what a rule paints, not how many times it is stated.
"""
import collections, pathlib, zipfile
import xml.etree.ElementTree as ET

FALLBACK = "{http://schemas.openxmlformats.org/markup-compatibility/2006}Fallback"


def main():
    fallback = live = 0
    documents = collections.Counter()
    reachable = set()
    for line in open("holders.txt"):
        path = pathlib.Path(line.strip())
        package = zipfile.ZipFile(path)
        for name in package.namelist():
            if not name.startswith("word/") or not name.endswith(".xml"):
                continue
            data = package.read(name)
            if b"textbox" not in data:
                continue
            try:
                root = ET.fromstring(data)
            except ET.ParseError:
                continue
            parents = {child: parent for parent in root.iter() for child in parent}
            for element in root.iter():
                if not element.tag.rsplit("}", 1)[-1] == "textbox":
                    continue
                node, buried = element, False
                while node in parents:
                    node = parents[node]
                    if node.tag == FALLBACK:
                        buried = True
                        break
                if buried:
                    fallback += 1
                else:
                    live += 1
                    documents[path.name] += 1
                    reachable.add(path)
    print(f"{fallback + live} v:textbox in the 131 documents")
    print(f"  {fallback} are the mc:Fallback half of an mc:AlternateContent -- never read")
    print(f"  {live} are reachable, in {len(reachable)} documents")
    for name, n in documents.most_common(20):
        print(f"    {n:>4}  {name[:72]}")
    with open("reachable.txt", "w") as fh:
        for p in sorted(reachable):
            fh.write(str(p) + "\n")


if __name__ == "__main__":
    main()
