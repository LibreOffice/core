#!/usr/bin/env python3
"""Every drawn span of a page with its face, size, colour and position.

The instrument for a bullet question: an image cannot tell a different bullet
*character* from the same character in a substituted face, and both from a
character drawn in the wrong colour.  All three are stated here.
"""
import sys, pymupdf

def spans(pdf, page):
    d = pymupdf.open(pdf)
    for b in d[page].get_text("dict")["blocks"]:
        if b["type"]:
            continue
        for line in b["lines"]:
            for s in line["spans"]:
                yield s

if __name__ == "__main__":
    only = None
    args = []
    for a in sys.argv[1:]:
        if a.startswith("--only="):
            only = a.split("=", 1)[1]
        else:
            args.append(a)
    for arg in args:
        pdf, _, page = arg.rpartition(":")
        print("==", pdf.split("/")[-1], "page", page)
        for s in spans(pdf, int(page) - 1):
            text = s["text"]
            if only is not None and only not in text:
                continue
            print("   %8.3f %8.3f  %-34s %6.2f  #%06x  %s"
                  % (s["origin"][0], s["origin"][1], s["font"], s["size"], s["color"],
                     " ".join("U+%04X" % ord(c) for c in text[:8])))
