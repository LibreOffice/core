#!/usr/bin/env python3
"""How many *glyphs* are drawn under a sheared text matrix, per page.

`turn-census.py` counts text BLOCKS, and blocks are not comparable between the two stacks:
LibreOffice writes one `BT ... ET` per text object with a `Tm` per run inside it, and we write
one `BT ... ET` per glyph run.  A paragraph of three italic runs is therefore one sheared block
on their side and three on ours -- which reads as a 20% over-shear that is entirely an artefact
of granularity.  Counting glyphs removes it: a glyph is a glyph on both sides.

Emits, per document: sheared glyphs ours and reference, and the pages where the two disagree by
more than `SLACK` glyphs (a subsetted `Tj` string is bytes, one per glyph, so the count is exact
for the simple encodings both stacks use here).

    shear-chars.py <ours-dir> <ref-dir> [ext]
"""
import glob, os, re, sys
sys.path.insert(0, "/c/sandbox/workdir/wt-words-r50/dotnet/research/probes/slides-r15")
import pdfops  # noqa: E402
from pdfops import objects, pages  # noqa: E402

EPS = 1e-6
SLACK = 0

TOKEN = re.compile(
    rb"(?:(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(cm|Tm))"
    rb"|\b(q|Q|BT)\b"
    rb"|(\((?:\\.|[^\\()])*\))\s*Tj"
    rb"|(<[0-9A-Fa-f\s]*>)\s*Tj"
    rb"|(\[(?:\\.|[^\\\[\]])*\])\s*TJ")


def sheared(a, b, c, d):
    """True for [1 0 k 1] scaled -- a synthetic oblique, and not a rotation."""
    return abs(b) < EPS and abs(c) > EPS and abs(a - d) < EPS and a > 0


def glyphs_of(match):
    if match.group(9):
        return len(re.sub(rb"\\(\d{1,3}|.)", b"x", match.group(9)[1:-1]))
    if match.group(10):
        return len(re.sub(rb"\s", b"", match.group(10)[1:-1])) // 2
    body = match.group(11)
    n = 0
    for part in re.finditer(rb"\((?:\\.|[^\\()])*\)|<[0-9A-Fa-f\s]*>", body):
        s = part.group(0)
        n += (len(re.sub(rb"\\(\d{1,3}|.)", b"x", s[1:-1])) if s[0:1] == b"("
              else len(re.sub(rb"\s", b"", s[1:-1])) // 2)
    return n


def count(stream):
    stack = [False]
    cur = False
    total = 0
    for m in TOKEN.finditer(stream):
        op = m.group(7) or m.group(8)
        if op == b"q":
            stack.append(stack[-1])
        elif op == b"Q":
            if len(stack) > 1:
                stack.pop()
        elif op == b"BT":
            cur = stack[-1]
        elif op == b"cm":
            stack[-1] = sheared(*(float(m.group(i)) for i in range(1, 5)))
        elif op == b"Tm":
            cur = sheared(*(float(m.group(i)) for i in range(1, 5))) or stack[-1]
        else:
            if cur:
                total += glyphs_of(m)
    return total


def streams(path):
    data = open(path, "rb").read()
    objs = objects(data)
    for page in pages(data, objs):
        yield pdfops.content(data, objs, page)


if __name__ == "__main__":
    ours_dir, ref_dir = sys.argv[1], sys.argv[2]
    want = sys.argv[3] if len(sys.argv) > 3 else None

    rows = []
    to = tr = 0
    bad_pages = 0
    for pdf in sorted(glob.glob(os.path.join(ref_dir, "*.pdf"))):
        ident = os.path.basename(pdf)[:-4]
        if want and not ident.endswith("__" + want):
            continue
        ours = os.path.join(ours_dir, ident + ".pdf")
        if not os.path.exists(ours):
            continue
        try:
            a = [count(s) for s in streams(ours)]
            b = [count(s) for s in streams(pdf)]
        except Exception as exc:
            print(f"  !! {ident}: {exc}")
            continue
        pages_off = sum(1 for i in range(min(len(a), len(b)))
                        if abs(a[i] - b[i]) > SLACK)
        to += sum(a)
        tr += sum(b)
        bad_pages += pages_off
        if sum(a) or sum(b):
            rows.append((sum(b) - sum(a), ident, sum(a), sum(b), pages_off))

    rows.sort(key=lambda t: -abs(t[0]))
    print(f"{'ref-ours':>9} {'ours':>8} {'ref':>8} {'pages off':>10}  document")
    for d, ident, x, y, p in rows:
        print(f"{d:9d} {x:8d} {y:8d} {p:10d}  {ident}")
    print(f"\ndocuments drawing sheared glyphs on either side: {len(rows)}")
    print(f"sheared glyphs: ours {to}, reference {tr}")
    print(f"pages whose sheared-glyph counts disagree: {bad_pages}")
