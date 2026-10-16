#!/usr/bin/env python3
"""The corpus DOCX that state a positioned table, and whether it leaves room beside it.

`Paginator.PlaceFloatedTable` floats such a table only when it fills its column; with room
beside it the table stays in the flow and nothing wraps. So the documents that a wrapping
implementation would move are those whose positioned table is narrower than its column.
"""
import pathlib, zipfile
import xml.etree.ElementTree as ET

W = "{http://schemas.openxmlformats.org/wordprocessingml/2006/main}"
ROOT = pathlib.Path("/home/user/sample-files")


def twips(element, name):
    if element is None:
        return None
    value = element.get(W + name)
    try:
        return int(value)
    except (TypeError, ValueError):
        return None


def columns(root):
    """The widest text column the document states, in twips."""
    widest = 0
    for sect in root.iter(W + "sectPr"):
        size, margin = sect.find(W + "pgSz"), sect.find(W + "pgMar")
        width = twips(size, "w") or 12240
        left = twips(margin, "left") or 1440
        right = twips(margin, "right") or 1440
        widest = max(widest, width - left - right)
    return widest or 9360


def main():
    rows = []
    for path in sorted(ROOT.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in (".docx", ".docm"):
            continue
        try:
            package = zipfile.ZipFile(path)
        except Exception:
            continue
        found = []
        column = 9360
        for name in package.namelist():
            if not name.startswith("word/") or not name.endswith(".xml"):
                continue
            data = package.read(name)
            if b"tblpPr" not in data:
                continue
            try:
                root = ET.fromstring(data)
            except ET.ParseError:
                continue
            if name == "word/document.xml":
                column = columns(root)
            for tbl in root.iter(W + "tbl"):
                pr = tbl.find(W + "tblPr")
                if pr is None or pr.find(W + "tblpPr") is None:
                    continue
                width = twips(pr.find(W + "tblW"), "w")
                if not width:
                    grid = tbl.find(W + "tblGrid")
                    width = sum(twips(c, "w") or 0
                                for c in (grid.iter(W + "gridCol") if grid is not None else []))
                found.append((name.split("/")[-1], width))
        if found:
            rows.append((path, column, found))

    narrow = [r for r in rows if any(w and w < r[1] * 0.9 for _, w in r[2])]
    print(f"{len(rows)} documents state a positioned table; "
          f"{len(narrow)} have one narrower than 90 % of the text column")
    for path, column, found in rows:
        mark = "*" if any(w and w < column * 0.9 for _, w in found) else " "
        widths = ", ".join(f"{part}:{w}" for part, w in found)
        print(f" {mark} {path.name[:52]:<53} column {column:>5}  {widths}")
    with open("movers.txt", "w") as fh:
        for path, _, _ in narrow:
            fh.write(str(path) + "\n")
    print(f"\nwrote movers.txt: {len(narrow)}")


if __name__ == "__main__":
    main()
