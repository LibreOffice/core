#!/usr/bin/env python3
"""Which documents' face sets moved between two `ourfaces.sh` sweeps, and how.

    facediff.py <before.tsv> <after.tsv> [--faces]

A font change barely moves a word count and routinely moves thirty face sets, so the honest measure
of one is which documents draw a different set of faces afterwards. Rows are `path\\tfaces`, faces
comma-separated with the PDF subset prefix already stripped.
"""
import sys


def read(path):
    out = {}
    for line in open(path, encoding="utf-8"):
        if "\t" not in line:
            continue
        doc, faces = line.rstrip("\n").split("\t", 1)
        out[doc] = set(f for f in faces.split(",") if f)
    return out


def main():
    before, after = read(sys.argv[1]), read(sys.argv[2])
    shared = sorted(set(before) & set(after))
    moved = [d for d in shared if before[d] != after[d]]

    print(f"{len(shared)} documents in both sweeps, {len(moved)} moved")
    tracks = {}
    for d in moved:
        tracks[d.split("/")[0]] = tracks.get(d.split("/")[0], 0) + 1
    print("by track:", ", ".join(f"{k} {v}" for k, v in sorted(tracks.items())))

    if "--faces" in sys.argv:
        for d in moved:
            gone = sorted(before[d] - after[d])
            got = sorted(after[d] - before[d])
            print(f"  {d}\n      -{','.join(gone) or '-'}  +{','.join(got) or '-'}")


if __name__ == "__main__":
    main()
