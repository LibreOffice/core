#!/usr/bin/env python3
"""Read a rendered probe PDF back as a (row, column) grid of drawn text.

`pdftotext -bbox` gives every word a rectangle; the probe's geometry is known (the column
widths the fixture was authored with), so each word is assigned to the column its centre
falls in and to the row its baseline band falls in.  This is what makes a variant table
exact where a joined token stream is ambiguous: `############` is four cells, not one.
"""
import re
import subprocess
import sys


def words(pdf):
    xml = subprocess.run(["pdftotext", "-bbox", pdf, "-"],
                         capture_output=True, text=True, check=True).stdout
    pages = []
    for pg in re.findall(r"<page[^>]*>(.*?)</page>", xml, re.S):
        ws = []
        for m in re.finditer(
                r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">(.*?)</word>',
                pg, re.S):
            x0, y0, x1, y1, t = m.groups()
            ws.append((float(x0), float(y0), float(x1), float(y1),
                       t.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">")))
        pages.append(ws)
    return pages


def grid(ws, ytol=3.0):
    """Group words into rows by y, then sort each row by x."""
    rows = []
    for w in sorted(ws, key=lambda w: (w[1], w[0])):
        for r in rows:
            if abs(r[0] - w[1]) <= ytol:
                r[1].append(w)
                break
        else:
            rows.append((w[1], [w]))
    return [(y, sorted(r, key=lambda w: w[0])) for y, r in rows]


if __name__ == "__main__":
    for i, ws in enumerate(words(sys.argv[1]), 1):
        print("== page", i)
        for y, row in grid(ws):
            print("  y=%7.2f  " % y +
                  " | ".join("%.1f-%.1f:%s" % (w[0], w[2], w[4]) for w in row))
