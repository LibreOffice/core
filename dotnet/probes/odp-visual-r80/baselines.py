#!/usr/bin/env python3
"""Print the drawn baselines of one page, and the pitch between them.

The instrument the round is measured with: a paragraph-spacing defect is a difference
between two baselines and nothing else on the page can show it.
"""
import sys, pymupdf

def baselines(pdf, page):
    out = []
    for b in pymupdf.open(pdf)[page].get_text("dict")["blocks"]:
        if b["type"]:
            continue
        for line in b["lines"]:
            text = "".join(s["text"] for s in line["spans"])
            if text.strip():
                out.append((line["spans"][0]["origin"][1], round(line["spans"][0]["size"], 2), text))
    return sorted(out)

if __name__ == "__main__":
    for arg in sys.argv[1:]:
        pdf, _, page = arg.rpartition(":")
        print("==", pdf, "page", page)
        prev = None
        for y, size, text in baselines(pdf, int(page) - 1):
            print("   %8.3f  %+7.3f  %5.2f  %s" % (y, (y - prev) if prev else 0, size, text[:52]))
            prev = y
