#!/usr/bin/env python3
"""How many reachable VML text boxes sit inside a `v:group`, which is what decides the engine.

A shape at the top level of a `w:pict` becomes a Writer text frame and its `w:txbxContent` goes
through the whole paragraph machinery.  The same shape inside a `v:group` cannot be a text frame,
so it becomes a drawing object whose text is EditEngine text -- and almost none of the paragraph
formatting survives that.  The two cases need separate counts.
"""
import collections, pathlib, zipfile
import xml.etree.ElementTree as ET

FALLBACK = "{http://schemas.openxmlformats.org/markup-compatibility/2006}Fallback"


def local(tag):
    return tag.rsplit("}", 1)[-1]


def main():
    grouped = loose = 0
    rows = collections.Counter()
    for line in open("../vmlinset-r171/reachable.txt"):
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
                if local(element.tag) != "textbox":
                    continue
                node, buried, ingroup = element, False, False
                while node in parents:
                    node = parents[node]
                    if node.tag == FALLBACK:
                        buried = True
                        break
                    if local(node.tag) == "group":
                        ingroup = True
                if buried:
                    continue
                if ingroup:
                    grouped += 1
                    rows[path.name] += 1
                else:
                    loose += 1
    print(f"reachable v:textbox: {grouped + loose}")
    print(f"  {grouped} inside a v:group -- EditEngine text, most paragraph formatting dropped")
    print(f"  {loose} at the top level -- a Writer text frame, everything applies")
    for name, n in rows.most_common():
        print(f"    {n:>4}  {name[:72]}")


if __name__ == "__main__":
    main()
