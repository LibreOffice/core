#!/usr/bin/env python3
"""Symmetric face-set difference against a reference sweep, over a named list of documents.

    distance.py <ref.tsv> <ours.tsv> [docs.txt]

0 is an exact match; every face named on one side and not the other counts 1. Without a list, every
document the reference sweep names is scored.
"""
import sys


def read(path):
    rows = {}
    with open(path, encoding="utf-8") as handle:
        for line in handle:
            if "\t" not in line:
                continue
            name, faces = line.rstrip("\n").split("\t", 1)
            rows[name] = frozenset(f for f in faces.split(",") if f)
    return rows


def main():
    ref, ours = read(sys.argv[1]), read(sys.argv[2])
    if len(sys.argv) > 3:
        with open(sys.argv[3], encoding="utf-8") as handle:
            names = [line.strip() for line in handle if line.strip()]
    else:
        names = sorted(ref)

    total = 0
    for name in names:
        if name not in ref or name not in ours:
            print(f"-\t{name}")
            continue
        d = len(ref[name] ^ ours[name])
        total += d
        print(f"{d}\t{name}")
    print(f"# total {total} over {len(names)}", file=sys.stderr)


if __name__ == "__main__":
    main()
