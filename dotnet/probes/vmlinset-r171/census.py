#!/usr/bin/env python3
"""What the corpus states on a VML text box: its inset, its inset mode, and its autofit.

`DocxVmlFrames` sets neither `PageFrame.Padding` (`Padding = box is null ? default : default`,
both branches the same) nor `PageFrame.HasFixedHeight`, so every VML text box is laid out with
its text against its own edge and grows to hold whatever it is given.  The reference does
neither: `TextBoxContext` (`oox/source/vml/vmltextboxcontext.cxx`:185-211) applies 0.1 in across
and 0.05 in down unless the box says `insetmode="auto"`, and `mso-fit-shape-to-text` in either
the shape's or the box's own style is what makes the height grow (`vmlshape.cxx`:776-777).
"""
import collections, pathlib, re, zipfile

ROOT = pathlib.Path("/home/user/sample-files")
BOX = re.compile(rb"<v:textbox\b[^>]*>", re.S)
SHAPE = re.compile(rb"<v:(?:shape|rect|roundrect|oval|line|polyline)\b[^>]*>", re.S)


def attribute(tag, name):
    m = re.search(rb'\b' + name.encode() + rb'="([^"]*)"', tag)
    return m.group(1).decode("utf-8", "replace") if m else None


def main():
    documents = collections.Counter()
    boxes = inset = automode = fit = shapefit = 0
    holders = set()
    insets = collections.Counter()
    for path in sorted(ROOT.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in (".docx", ".docm"):
            continue
        try:
            package = zipfile.ZipFile(path)
        except Exception:
            continue
        seen = False
        for name in package.namelist():
            if not name.startswith("word/") or not name.endswith(".xml"):
                continue
            data = package.read(name)
            if b"<v:textbox" not in data:
                continue
            for tag in BOX.findall(data):
                seen = True
                boxes += 1
                value = attribute(tag, "inset")
                mode = attribute(tag, "insetmode")
                style = attribute(tag, "style") or ""
                if value is not None:
                    inset += 1
                    insets[value] += 1
                if mode == "auto":
                    automode += 1
                if "mso-fit-shape-to-text" in style:
                    fit += 1
            for tag in SHAPE.findall(data):
                if "mso-fit-shape-to-text" in (attribute(tag, "style") or ""):
                    shapefit += 1
        if seen:
            holders.add(path)
            documents[path.parent.parent.name] += 1
    print(f"{len(holders)} DOCX hold a v:textbox, {boxes} boxes in all")
    print(f"  {inset} state an inset, {automode} state insetmode=auto")
    print(f"  {fit} boxes and {shapefit} shapes state mso-fit-shape-to-text")
    print("  the insets stated:")
    for value, n in insets.most_common(12):
        print(f"    {n:>5}  {value}")
    with open("holders.txt", "w") as fh:
        for p in sorted(holders):
            fh.write(str(p) + "\n")
    print(f"\nwrote holders.txt: {len(holders)}")


if __name__ == "__main__":
    main()
