#!/usr/bin/env python3
"""Walk an RTF file's group structure and report every top-level (depth-1) group
with its opening control word and byte span.  Used to find what precedes a
control word in the document's preamble."""
import sys

def scan(path):
    d = open(path, 'rb').read()
    depth = 0
    i = 0
    n = len(d)
    stack = []
    out = []
    while i < n:
        c = d[i]
        if c == 0x5c:  # backslash
            if i + 1 < n and d[i+1] in b"\\{}'":
                i += 2
                continue
            j = i + 1
            while j < n and (65 <= d[j] <= 90 or 97 <= d[j] <= 122):
                j += 1
            i = j
            continue
        if c == 0x7b:
            depth += 1
            stack.append(i)
            if depth == 2:
                # opening word of this depth-2 group (a child of the document group)
                k = i + 1
                while k < n and d[k] in b' \r\n':
                    k += 1
                out.append((i, d[k:k+40]))
            i += 1
            continue
        if c == 0x7d:
            start = stack.pop() if stack else -1
            if depth == 2 and out and out[-1][0] == start:
                out[-1] = (start, out[-1][1], i)
            depth -= 1
            i += 1
            continue
        i += 1
    return out

if __name__ == '__main__':
    for e in scan(sys.argv[1]):
        s = e[0]; end = e[2] if len(e) > 2 else -1
        lim = int(sys.argv[2]) if len(sys.argv) > 2 else 10**9
        if s > lim: break
        print(f"{s:>9} {end:>9} {end-s:>9}  {e[1]!r}")
