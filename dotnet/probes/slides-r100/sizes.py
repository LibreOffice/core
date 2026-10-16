#!/usr/bin/env python3
"""Per-page dominant drawn text size: the size carrying the most alphanumeric
characters.  Round 84's statistic, which is the one OPEN-ISSUES' O15 is scored on.

Emitted as  <id>\t<page>\t<size>\t<alphanumerics at that size>  so it can be
compared row for row against probes/slides-r97/sizes-ref.tsv and
probes/slides-r99/sizes-head.tsv.
"""
import collections, sys
import pymupdf

def rows(path, ident):
    doc = pymupdf.open(path)
    for i, page in enumerate(doc, 1):
        per = collections.Counter()
        for block in page.get_text("dict")["blocks"]:
            for line in block.get("lines", []):
                for span in line.get("spans", []):
                    n = sum(1 for c in span["text"] if c.isalnum())
                    if n:
                        per[round(span["size"], 2)] += n
        if per:
            # ties go to the size met FIRST in the page, which is what slides-r97/sizes-ref.tsv does
            size, n = max(per.items(), key=lambda kv: kv[1])
        else:
            size, n = 0.0, 0
        print(f"{ident}\t{i}\t{size}\t{n}")

if __name__ == '__main__':
    rows(sys.argv[1], sys.argv[2])
