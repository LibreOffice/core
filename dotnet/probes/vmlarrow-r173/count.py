#!/usr/bin/env python3
"""Arrowhead-sized filled paths on each side, for the documents stating a VML stroke arrow.

`LineEnds.Apply` turns a stroked path into a stroke plus a filled marker, so the count of small
filled paths is the direct measure: the reference draws one per arrow and this tree drew none.
"""
import hashlib, pathlib, sys


def key(path):
    return hashlib.md5(str(path).encode()).hexdigest()[:16]


def heads(pdf):
    """Filled paths small enough to be a marker rather than a shape."""
    import fitz
    n = 0
    with fitz.open(pdf) as doc:
        for page in doc:
            for drawing in page.get_drawings():
                if drawing["type"] not in ("f", "fs"):
                    continue
                r = drawing["rect"]
                if 0.5 < r.width < 20 and 0.5 < r.height < 20:
                    n += 1
    return n


def main():
    root = pathlib.Path(sys.argv[1])
    print("stated\tbefore\tafter\tref\tdocument")
    import re, zipfile
    total_ours = total_ref = total_stated = total_before = 0
    for line in open(sys.argv[2]):
        path = pathlib.Path(line.strip())
        if not path.name:
            continue
        stated = 0
        z = zipfile.ZipFile(path)
        for name in z.namelist():
            if name.endswith(".xml"):
                for tag in re.findall(rb"<v:stroke\b[^>]*>", z.read(name)):
                    stated += len(re.findall(rb'\b(?:start|end)arrow="', tag))
        legs = {}
        for leg in ("before", "after", "ref"):
            pdf = next((root / leg / key(path)).glob("*.pdf"), None)
            legs[leg] = heads(pdf) if pdf else -1
        total_ours += legs["after"]
        total_before += legs["before"]
        total_ref += legs["ref"]
        total_stated += stated
        print(f"{stated}\t{legs['before']}\t{legs['after']}\t{legs['ref']}\t{path.name[:62]}")
    print(f"\n{total_stated} stated; markers drawn: {total_before} before, "
          f"{total_ours} after, {total_ref} by the reference")


if __name__ == "__main__":
    main()
