#!/usr/bin/env python3
"""Rotated text, separated from *sheared* text.

`probes/slides-r54/rotated-text-census.py` fixed one artefact (the two stacks rotate through
different operators) and left a second in place: it calls a matrix rotated whenever `b` or `c`
is non-zero.  A synthetic-oblique text matrix is `[1 0 tan(t) 1]` -- `b` is zero, `c` is not --
so **every fake-italic run counts as rotated**.  On `section_1_our_rights_presentation` all 81
of the reference's "rotated" blocks are `c = 0.34625`, which is tan(19.1 deg), LibreOffice's
own italic skew, and nothing on that document is turned at all.

So classify each text block by what its matrix actually does:

    turn    b and c both non-zero, or a/d not both positive -- a real rotation
    shear   b == 0, a == d == 1, c != 0 -- synthetic oblique
    other   anything else (a pure quarter turn has a == d == 0 and lands in `turn`)

    turn-census.py <ours-dir> <ref-dir> [ext]
"""
import glob, math, os, re, sys
sys.path.insert(0, "/c/sandbox/workdir/wt-slides-r50/dotnet/research/probes/slides-r15")
import pdfops  # noqa: E402
from pdfops import objects, pages  # noqa: E402

TOKEN = re.compile(
    rb"(?:(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(cm|Tm))"
    rb"|\b(q|Q|BT)\b")

EPS = 1e-6


def kind(a, b, c, d):
    """'turn', 'shear' or None for a matrix that leaves text upright."""
    if abs(b) < EPS and abs(c) < EPS:
        return None
    if abs(b) < EPS and abs(a - d) < EPS and a > 0:
        return "shear"                      # [1 0 k 1] scaled -- synthetic oblique
    return "turn"


def blocks(stream):
    """(turned, sheared, total) text blocks in one content stream."""
    stack = [None]
    turned = sheared = tot = 0
    seen = None
    for m in TOKEN.finditer(stream):
        op = m.group(7) or m.group(8)
        if op == b"q":
            stack.append(stack[-1])
        elif op == b"Q":
            if len(stack) > 1:
                stack.pop()
        elif op == b"BT":
            tot += 1
            seen = stack[-1]
            if seen == "turn":
                turned += 1
            elif seen == "shear":
                sheared += 1
        elif op == b"cm":
            k = kind(*(float(m.group(i)) for i in range(1, 5)))
            if k:
                stack[-1] = k
        elif op == b"Tm":
            k = kind(*(float(m.group(i)) for i in range(1, 5)))
            if k and k != seen:
                if k == "turn" and seen != "turn":
                    turned += 1
                    if seen == "shear":
                        sheared -= 1
                elif k == "shear" and seen is None:
                    sheared += 1
                seen = k
    return turned, sheared, tot


def streams(path):
    data = open(path, "rb").read()
    objs = objects(data)
    for page in pages(data, objs):
        yield pdfops.content(data, objs, page)


if __name__ == "__main__":
    ours_dir, ref_dir = sys.argv[1], sys.argv[2]
    want = sys.argv[3] if len(sys.argv) > 3 else None

    rows = []
    to = tr = so = sr = 0
    pt = ps = 0
    for pdf in sorted(glob.glob(os.path.join(ref_dir, "*.pdf"))):
        ident = os.path.basename(pdf)[:-4]
        if want and not ident.endswith("__" + want):
            continue
        ours = os.path.join(ours_dir, ident + ".pdf")
        if not os.path.exists(ours):
            continue
        try:
            a = [blocks(s) for s in streams(ours)]
            b = [blocks(s) for s in streams(pdf)]
        except Exception as exc:
            print(f"  !! {ident}: {exc}")
            continue
        ato, ash = sum(x[0] for x in a), sum(x[1] for x in a)
        bto, bsh = sum(x[0] for x in b), sum(x[1] for x in b)
        mt = sum(1 for i in range(min(len(a), len(b))) if b[i][0] and not a[i][0])
        ms = sum(1 for i in range(min(len(a), len(b))) if b[i][1] and not a[i][1])
        to += ato; tr += bto; so += ash; sr += bsh; pt += mt; ps += ms
        if ato or bto or ash or bsh:
            rows.append((bto - ato, bsh - ash, ident, ato, bto, ash, bsh, mt, ms))

    rows.sort(key=lambda t: -abs(t[0]))
    print(f"{'dTURN':>6} {'ours':>6} {'ref':>6} | {'dSHEAR':>6} {'ours':>6} {'ref':>6} | "
          f"{'p-t':>4} {'p-s':>4}  document")
    for d, ds, ident, ato, bto, ash, bsh, mt, ms in rows:
        print(f"{d:6d} {ato:6d} {bto:6d} | {ds:6d} {ash:6d} {bsh:6d} | {mt:4d} {ms:4d}  {ident}")
    print(f"\ndocuments with a turned or sheared text block: {len(rows)}")
    print(f"TURNED  blocks: ours {to}, reference {tr}")
    print(f"SHEARED blocks: ours {so}, reference {sr}")
    print(f"pages the reference turns  and we do not: {pt}")
    print(f"pages the reference shears and we do not: {ps}")
