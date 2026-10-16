#!/usr/bin/env python3
"""Where each renderer draws a VML group's filled rectangles.

Pairing text spans on these templates resolves nothing -- they repeat `SUBTASK` thirty times -- so
the instrument is the ink itself: every shape of the group carries a `fillcolor`, so both renderers
draw a filled rectangle per shape and those can be matched on size, which is what the group's own
coordinate space decides.
"""
import hashlib, pathlib, sys


def key(path):
    return hashlib.md5(str(path).encode()).hexdigest()[:16]


def rectangles(pdf):
    import fitz
    out = []
    with fitz.open(pdf) as doc:
        for number, page in enumerate(doc):
            for drawing in page.get_drawings():
                if drawing["type"] not in ("f", "fs"):
                    continue
                for item in drawing["items"]:
                    if item[0] != "re":
                        continue
                    r = item[1]
                    if r.width < 2 or r.height < 2:
                        continue
                    out.append((number, round(r.x0, 2), round(r.y0, 2),
                                round(r.width, 2), round(r.height, 2)))
    return sorted(out)


def main():
    root = pathlib.Path(sys.argv[1])
    for line in open(sys.argv[2]):
        path = pathlib.Path(line.strip())
        if not path.name:
            continue
        k = key(path)
        legs = {}
        for leg in ("after", "ref"):
            pdf = next((root / leg / k).glob("*.pdf"), None)
            legs[leg] = rectangles(pdf) if pdf else []
        print(f"=== {path.name[:60]} ===")
        print(f"  ours {len(legs['after'])} filled rects, reference {len(legs['ref'])}")
        # Match on size: a shape's extent is what the group's coordinate space decides.
        theirs = {}
        for page, x, y, w, h in legs["ref"]:
            theirs.setdefault((page, w, h), []).append((x, y))
        matched = []
        for page, x, y, w, h in legs["after"]:
            slot = theirs.get((page, w, h))
            if slot:
                rx, ry = slot.pop(0)
                matched.append((x - rx, y - ry))
        if matched:
            dx = sum(abs(a) for a, _ in matched) / len(matched)
            dy = sum(abs(b) for _, b in matched) / len(matched)
            print(f"  {len(matched)} rects of the same size; mean |dx| {dx:.2f} |dy| {dy:.2f}"
                  f"  max |dx| {max(abs(a) for a, _ in matched):.2f}")
        else:
            print("  no rectangle of ours has the size of one of theirs")
        for label, leg in (("ours", "after"), ("ref", "ref")):
            for row in legs[leg][:6]:
                print(f"    {label}  page {row[0]}  x {row[1]:>8} y {row[2]:>8} "
                      f"w {row[3]:>8} h {row[4]:>8}")


if __name__ == "__main__":
    main()
