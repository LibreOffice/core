#!/usr/bin/env python3
"""Which VML shapes the corpus states, and which of them `DocxVmlFrames.PaintOf` can paint.

`PaintOf` answers `VmlPaint.None` for anything that is not a `v:rect`, a `v:roundrect`, a
`v:line` or a straight connector, so every other shape is drawn with no fill and no outline
whatever it states.  This counts what the corpus actually reaches -- a `v:textbox` or shape buried
in an `mc:Fallback` is never read by either renderer, because the `mc:Choice` beside it carries the
same shape as DrawingML.
"""
import collections, pathlib, zipfile
import xml.etree.ElementTree as ET

ROOT = pathlib.Path("/home/user/sample-files")
FALLBACK = "{http://schemas.openxmlformats.org/markup-compatibility/2006}Fallback"
PAINTED = {"rect", "roundrect", "line"}
SHAPES = ("shape", "rect", "roundrect", "oval", "line", "polyline", "curve", "arc", "image",
          "background")


def local(tag):
    return tag.rsplit("}", 1)[-1]


def main():
    kinds = collections.Counter()
    presets = collections.Counter()
    documents = collections.defaultdict(set)
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
            data = package.read(name)
            if b"urn:schemas-microsoft-com:vml" not in data and b"<v:" not in data:
                continue
            try:
                root = ET.fromstring(data)
            except ET.ParseError:
                continue
            parents = {child: parent for parent in root.iter() for child in parent}
            for element in root.iter():
                tag = local(element.tag)
                if tag not in SHAPES or "vml" not in element.tag:
                    continue
                node, buried = element, False
                while node in parents:
                    node = parents[node]
                    if node.tag == FALLBACK:
                        buried = True
                        break
                if buried:
                    continue
                kinds[tag] += 1
                if tag == "shape":
                    preset = (element.get("type") or "").lstrip("#") or "(none)"
                    presets[preset] += 1
                    documents[preset].add(path.name)

    print("reachable VML shape elements:")
    for tag, n in kinds.most_common():
        note = " painted" if tag in PAINTED else " NOT painted unless it is a straight connector"
        print(f"  {n:>6}  v:{tag}{note}")
    print()
    print("v:shape presets:")
    for preset, n in presets.most_common(20):
        print(f"  {n:>6}  {preset:<22} in {len(documents[preset])} documents")


if __name__ == "__main__":
    main()
