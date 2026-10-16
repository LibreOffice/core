#!/usr/bin/env python3
"""Read a chart's category-axis tick labels out of a PDF page.

The labels may be drawn horizontally, rotated 45 degrees or rotated 90 degrees,
and pdftotext reports each as several fragments.  A label always reads along the
direction (x - y) increasing, whatever the rotation, and labels are separated
along one of three perpendicular coordinates (y for horizontal text, x + y for
45 degrees, x for 90 degrees).  Cluster on each in turn and keep whichever
yields the most strings that parse as dd/mm/yy.
"""
import re
import subprocess
import sys

DATE = re.compile(r'^\d{1,2}/\d{1,2}/\d{4}$')


def words(pdf, page=1):
    xml = subprocess.run(['pdftotext', '-bbox', '-f', str(page), '-l', str(page), pdf, '-'],
                         capture_output=True, text=True).stdout
    out = []
    for a, b, c, d, e in re.findall(
            r'<word xMin="([\d.-]+)" yMin="([\d.-]+)" xMax="([\d.-]+)" yMax="([\d.-]+)">(.*?)</word>',
            xml):
        out.append(((float(a) + float(c)) / 2, (float(b) + float(d)) / 2, e))
    height = float(re.search(r'height="([\d.]+)"', xml).group(1))
    return out, height


def cluster(items, key, tol):
    items = sorted(items, key=key)
    groups = []
    for it in items:
        if groups and key(it) - key(groups[-1][-1]) <= tol:
            groups[-1].append(it)
        else:
            groups.append([it])
    return groups


def read(pdf, page=1, band=0.72):
    ws, h = words(pdf, page)
    ws = [w for w in ws if w[1] > h * band]
    best = []
    for key, tol in ((lambda w: w[1], 3.0),
                     (lambda w: w[0] + w[1], 4.0),
                     (lambda w: w[0], 4.0)):
        got = []
        for g in cluster(ws, key, tol):
            g = sorted(g, key=lambda w: w[0] - w[1])
            acc = ''
            for w in g:
                acc += w[2]
                if DATE.match(acc):
                    got.append(acc)
                    acc = ''
        if len(got) > len(best):
            best = got
    return best


def main():
    for pdf in sys.argv[1:]:
        got = read(pdf)
        print(f'{pdf}: {len(got)} labels')
        print('   ', ' '.join(got))


if __name__ == '__main__':
    main()
