#!/usr/bin/env python3
r"""Byte offsets in an RTF file at which the group depth is 1 (inside the
document group and inside nothing else), so a prefix cut there needs no
repair.  Prints the first such offset at or after each requested target."""
import sys, bisect


def boundaries(d: bytes) -> list[int]:
    depth = i = 0
    n = len(d)
    out = []
    while i < n:
        c = d[i]
        if c == 0x5C:
            i += 2 if i + 1 < n and d[i + 1] in b"\\{}'" else 1
            continue
        if c == 0x7B:
            depth += 1
        elif c == 0x7D:
            depth -= 1
            if depth == 1:
                out.append(i + 1)
        i += 1
    return out


if __name__ == "__main__":
    d = open(sys.argv[1], "rb").read()
    b = boundaries(d)
    print(" ".join(str(b[bisect.bisect_left(b, int(t))]) for t in sys.argv[2:]))
