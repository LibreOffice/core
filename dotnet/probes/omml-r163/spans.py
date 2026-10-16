#!/usr/bin/env python3
"""Per-span baseline origin, font and size out of a PDF page.

PyMuPDF's span `origin` is the text-positioning baseline the writer emitted
(Td/Tm), NOT the font-descriptor ink box `pdftotext -bbox` reports.  The two
differ by a constant per face, which is page-vision's documented trap; the
question here is vertical position, so the baseline is the only usable channel.

Usage:  spans.py FILE.pdf PAGE[1-based] [ymin ymax]
"""
import sys
import pymupdf


def main():
    path, page_no = sys.argv[1], int(sys.argv[2])
    lo = float(sys.argv[3]) if len(sys.argv) > 3 else -1e9
    hi = float(sys.argv[4]) if len(sys.argv) > 4 else 1e9
    doc = pymupdf.open(path)
    page = doc[page_no - 1]
    print("# %s page %d  (%.2f x %.2f pt)" % (path, page_no, page.rect.width, page.rect.height))
    print("x\ty_base\tsize\tfont\ttext")
    rows = []
    for block in page.get_text("rawdict")["blocks"]:
        if block.get("type") != 0:
            continue
        for line in block["lines"]:
            for span in line["spans"]:
                text = "".join(c["c"] for c in span["chars"])
                ox, oy = span["origin"]
                if not (lo <= oy <= hi):
                    continue
                rows.append((oy, ox, span["size"], span["font"], text))
    for oy, ox, size, font, text in sorted(rows):
        print("%8.3f\t%8.3f\t%6.3f\t%s\t%r" % (ox, oy, size, font, text))


if __name__ == "__main__":
    main()

