#!/usr/bin/env python3
"""For every text run in a PDF page, report the drawn font, the total drawn
width (the font's own declared widths) and the total *implied* width (declared
plus the TJ adjustments), i.e. the advance the producer actually laid out at.
A run whose implied width differs from its drawn width by more than kerning was
measured with something other than the face it is drawn in.
"""
import sys, re, json
import pymupdf

path = sys.argv[1]
doc = pymupdf.open(path)
pages = range(len(doc)) if len(sys.argv) < 3 else [int(sys.argv[2])]

for pno in pages:
    page = doc[pno]
    res = {}
    for f in page.get_fonts(full=True):
        xref, ext, ftype, basefont, name, enc = f[:6]
        d = doc.xref_object(xref, compressed=True)
        fc = int(re.search(r"/FirstChar\s+(\d+)", d).group(1))
        w = re.search(r"/Widths\s+(\d+)\s+0\s+R", d)
        arr = doc.xref_object(int(w.group(1)), compressed=True) if w \
              else re.search(r"/Widths\s*(\[[^\]]*\])", d).group(1)
        widths = [float(x) for x in re.findall(r"-?[\d.]+", arr.strip()[1:-1])]
        tu = {}
        m = re.search(r"/ToUnicode\s+(\d+)\s+0\s+R", d)
        if m:
            s = doc.xref_stream(int(m.group(1))).decode("latin-1")
            for blk in re.findall(r"beginbfchar(.*?)endbfchar", s, re.S):
                for a, b in re.findall(r"<([0-9A-Fa-f]+)>\s*<([0-9A-Fa-f]+)>", blk):
                    tu[int(a,16)] = "".join(chr(int(b[i:i+4],16)) for i in range(0,len(b),4))
        res[name] = (basefont, fc, widths, tu)

    raw = b"".join(doc.xref_stream(x) for x in page.get_contents()).decode("latin-1")
    print(f"=== page {pno} of {path}")
    print(f"{'font':32s} {'size':>6} {'drawn':>9} {'implied':>9} {'ratio':>7}  text")
    cur = None
    for m in re.finditer(r"BT(.*?)ET", raw, re.S):
        blk = m.group(1)
        tf = re.search(r"/(F\d+)\s+([\d.]+)\s+Tf", blk)
        if not tf: continue
        fname, size = tf.group(1), float(tf.group(2))
        if fname not in res: continue
        basefont, fc, widths, tu = res[fname]
        for tjm in re.finditer(r"\[(.*?)\]\s*TJ|<([0-9A-Fa-f]+)>\s*Tj", blk, re.S):
            body = tjm.group(1) if tjm.group(1) is not None else "<%s>" % tjm.group(2)
            toks = re.findall(r"<([0-9A-Fa-f]+)>|(-?[\d.]+)", body)
            seq = []
            for hexs, num in toks:
                if hexs:
                    for k in range(0, len(hexs), 2):
                        seq.append([int(hexs[k:k+2],16), 0.0])
                elif seq:
                    seq[-1][1] += float(num)
            if not seq: continue
            drawn = sum(widths[c-fc] for c,_ in seq) * size/1000.0
            impl  = sum(widths[c-fc]-a for c,a in seq) * size/1000.0
            txt = "".join(tu.get(c,"?") for c,_ in seq)
            print(f"{basefont:32s} {size:6.2f} {drawn:9.3f} {impl:9.3f} {impl/drawn:7.4f}  {txt[:44]!r}")
