#!/usr/bin/env python3
"""Which documents draw a different face set between two sweeps, and how far each is from a third.

    facediff.py before.tsv after.tsv [ref.tsv]

Prints the documents whose set moved, and — where a reference sweep is given — the symmetric
difference between our set and the reference's, before and after. The symmetric difference is the
figure the fonts rounds report: 0 is an exact match and every face named on one side and not the
other counts 1.
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
    before, after = read(sys.argv[1]), read(sys.argv[2])
    ref = read(sys.argv[3]) if len(sys.argv) > 3 else {}

    moved = sorted(k for k in before if k in after and before[k] != after[k])
    total_b = total_a = 0
    for name in moved:
        if name in ref:
            b = len(before[name] ^ ref[name])
            a = len(after[name] ^ ref[name])
            total_b += b
            total_a += a
            mark = "closer" if a < b else "further" if a > b else "same"
            print(f"{b}\t{a}\t{mark}\t{name}")
        else:
            print(f"-\t-\t-\t{name}")
    print(f"# {len(moved)} of {len(before)} moved", file=sys.stderr)
    if ref:
        print(f"# symmetric difference over those: {total_b} -> {total_a}", file=sys.stderr)


if __name__ == "__main__":
    main()
