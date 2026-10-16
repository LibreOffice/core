#!/usr/bin/env python3
r"""Offsets at which a prefix of the witness can be cut cleanly: group depth 1
and immediately after a `\row` or `\par`, so the cut does not land inside a
half-built table row whose content would then never be flushed."""
import sys, bisect, re


def cuts(d: bytes) -> list[int]:
    depth = i = 0
    n = len(d)
    out = []
    while i < n:
        c = d[i]
        if c == 0x5C:
            if i + 1 < n and d[i + 1] in b"\\{}'":
                i += 2
                continue
            j = i + 1
            while j < n and (65 <= d[j] <= 90 or 97 <= d[j] <= 122):
                j += 1
            if depth == 1 and d[i:j] in (rb"\row", rb"\par"):
                out.append(j)
            i = j
            continue
        if c == 0x7B:
            depth += 1
        elif c == 0x7D:
            depth -= 1
        i += 1
    return out


if __name__ == "__main__":
    d = open(sys.argv[1], "rb").read()
    c = cuts(d)
    print(f"{len(c)} clean cuts, first {c[:3]} last {c[-3:]}", file=sys.stderr)
    print(" ".join(str(c[min(bisect.bisect_left(c, int(t)), len(c) - 1)]) for t in sys.argv[2:]))
