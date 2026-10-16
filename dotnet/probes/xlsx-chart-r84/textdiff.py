#!/usr/bin/env python3
"""Multiset difference of the text spans of two PDFs, per page.

Reads both renderings of one document out of a gate bank and prints, per page,
the spans present in one and absent from the other.  A chart's labels are short
tokens with unusual coordinates, so the shape of the difference is usually enough
to say whether the divergence sits in the chart or in the sheet's own cells.
"""
import sys, collections
import pymupdf


def spans(path):
    doc = pymupdf.open(path)
    out = []
    for pno, page in enumerate(doc):
        d = page.get_text("dict")
        for b in d["blocks"]:
            for l in b.get("lines", []):
                for s in l["spans"]:
                    t = s["text"].strip()
                    if t:
                        out.append((pno, t, tuple(round(v, 1) for v in s["bbox"]),
                                    round(s["size"], 2), s["font"]))
    return out


def main(ours, ref, limit=40):
    a, b = spans(ours), spans(ref)
    ca = collections.Counter((p, t) for p, t, *_ in a)
    cb = collections.Counter((p, t) for p, t, *_ in b)
    only_a, only_b = ca - cb, cb - ca
    print(f"ours spans {len(a)}  ref spans {len(b)}")
    for name, c in (("ONLY OURS", only_a), ("ONLY REF", only_b)):
        print(f"--- {name} ({sum(c.values())})")
        for (p, t), n in sorted(c.items())[:limit]:
            print(f"  p{p+1} x{n} {t!r}")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], int(sys.argv[3]) if len(sys.argv) > 3 else 40)
