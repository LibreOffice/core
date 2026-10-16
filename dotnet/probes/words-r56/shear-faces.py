#!/usr/bin/env python3
"""Which *face* is drawn under a sheared text matrix, on each side.

`shear-chars.py` says how many glyphs lean.  This says what they are set in, which is the
question a font-resolution divergence answers: a run whose request is italic leans only when
the resolved face has no italic of its own, so the two stacks disagreeing about the lean is
the two stacks disagreeing about the face.

Emits, per document, a table of /BaseFont -> sheared glyph count for ours and for the
reference, so a face that leans on one side and not the other is named rather than inferred.

    shear-faces.py <ours-dir> <ref-dir> [substring-of-ident]
"""
import glob, os, re, sys
from collections import Counter
sys.path.insert(0, "/c/sandbox/workdir/wt-words-r50/dotnet/research/probes/slides-r15")
import pdfops  # noqa: E402
from pdfops import objects, pages, content, stream_of  # noqa: E402

EPS = 1e-6

TOKEN = re.compile(
    rb"(?:(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(cm|Tm))"
    rb"|\b(q|Q|BT)\b"
    rb"|/([^\s/\[\]<>()]+)\s+(-?[\d.]+)\s+Tf"
    rb"|(\((?:\\.|[^\\()])*\))\s*Tj"
    rb"|(<[0-9A-Fa-f\s]*>)\s*Tj"
    rb"|(\[(?:\\.|[^\\\[\]])*\])\s*TJ")


def sheared(a, b, c, d):
    return abs(b) < EPS and abs(c) > EPS and abs(a - d) < EPS and a > 0


def glyphs_of(m):
    if m.group(11):
        return len(re.sub(rb"\\(\d{1,3}|.)", b"x", m.group(11)[1:-1]))
    if m.group(12):
        return len(re.sub(rb"\s", b"", m.group(12)[1:-1])) // 2
    n = 0
    for part in re.finditer(rb"\((?:\\.|[^\\()])*\)|<[0-9A-Fa-f\s]*>", m.group(13)):
        s = part.group(0)
        n += (len(re.sub(rb"\\(\d{1,3}|.)", b"x", s[1:-1])) if s[0:1] == b"("
              else len(re.sub(rb"\s", b"", s[1:-1])) // 2)
    return n


def basefonts(data, objs, pnum):
    """/Fn -> /BaseFont for one page, following an indirect /Resources and /Font."""
    body = objs[pnum]
    src = b""
    m = re.search(rb"/Font\s*<<(.*?)>>", body, re.S)
    if m:
        src = m.group(1)
    else:
        rm = re.search(rb"/Resources\s+(\d+)\s+\d+\s+R", body)
        if rm:
            rb_ = objs.get(int(rm.group(1)), b"")
            m2 = re.search(rb"/Font\s*<<(.*?)>>", rb_, re.S)
            if m2:
                src = m2.group(1)
            else:
                fm = re.search(rb"/Font\s+(\d+)\s+\d+\s+R", rb_)
                if fm:
                    src = objs.get(int(fm.group(1)), b"")
    out = {}
    for fm in re.finditer(rb"/([^\s/\[\]<>()]+)\s+(\d+)\s+\d+\s+R", src):
        fo = objs.get(int(fm.group(2)), b"")
        bf = re.search(rb"/BaseFont\s*/([^\s/\]>]+)", fo)
        out[fm.group(1).decode()] = (bf.group(1).decode() if bf else "?")
    return out


def census(path):
    """Counter of basefont -> sheared glyphs, and basefont -> upright glyphs."""
    data = open(path, "rb").read()
    objs = objects(data)
    lean, flat = Counter(), Counter()
    for pnum in pages(data, objs):
        res = basefonts(data, objs, pnum)
        stream = content(data, objs, pnum)
        stack = [False]
        cur = False
        font = "?"
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
            elif m.group(9):
                font = res.get(m.group(9).decode(), m.group(9).decode())
            else:
                (lean if cur else flat)[font] += glyphs_of(m)
    return lean, flat


def strip(name):
    return name.split("+", 1)[1] if "+" in name else name


if __name__ == "__main__":
    ours_dir, ref_dir = sys.argv[1], sys.argv[2]
    want = sys.argv[3] if len(sys.argv) > 3 else None
    for pdf in sorted(glob.glob(os.path.join(ref_dir, "*.pdf"))):
        ident = os.path.basename(pdf)[:-4]
        if want and want not in ident:
            continue
        ours = os.path.join(ours_dir, ident + ".pdf")
        if not os.path.exists(ours):
            continue
        try:
            ol, of = census(ours)
            rl, rf = census(pdf)
        except Exception as exc:
            print(f"  !! {ident}: {exc}")
            continue
        if not (sum(ol.values()) or sum(rl.values())):
            continue
        names = sorted({strip(k) for k in list(ol) + list(rl) + list(of) + list(rf)})
        rows = []
        for n in names:
            o_l = sum(v for k, v in ol.items() if strip(k) == n)
            r_l = sum(v for k, v in rl.items() if strip(k) == n)
            o_f = sum(v for k, v in of.items() if strip(k) == n)
            r_f = sum(v for k, v in rf.items() if strip(k) == n)
            if o_l or r_l:
                rows.append((n, o_l, r_l, o_f, r_f))
        print(f"\n=== {ident}   sheared ours {sum(ol.values())}  ref {sum(rl.values())}")
        print(f"    {'face':40s} {'lean-ours':>9} {'lean-ref':>9} {'flat-ours':>9} {'flat-ref':>9}")
        for n, a, b, c, d in sorted(rows, key=lambda t: -(t[1] + t[2])):
            print(f"    {n:40s} {a:9d} {b:9d} {c:9d} {d:9d}")
