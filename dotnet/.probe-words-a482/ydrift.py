#!/usr/bin/env python3
"""Compare per-line y positions between two PDFs, page by page.

Usage: ydrift.py ours.pdf ref.pdf [page]
"""
import subprocess, sys, re, html
from xml.etree import ElementTree as ET

NS = '{http://www.w3.org/1999/xhtml}'


def lines(pdf, page):
    out = subprocess.run(['pdftotext', '-bbox-layout', '-f', str(page), '-l', str(page), pdf, '-'],
                         capture_output=True, text=True).stdout
    root = ET.fromstring(out)
    res = []
    for pg in root.iter(NS + 'page'):
        for ln in pg.iter(NS + 'line'):
            words = [w.text or '' for w in ln.iter(NS + 'word')]
            txt = ' '.join(words).strip()
            if not txt:
                continue
            res.append((float(ln.get('yMin')), float(ln.get('yMax')),
                        float(ln.get('xMin')), float(ln.get('xMax')), txt))
    return res


def main():
    o, r = sys.argv[1], sys.argv[2]
    page = int(sys.argv[3]) if len(sys.argv) > 3 else 1
    lo, lr = lines(o, page), lines(r, page)
    # align by text
    i = j = 0
    print(f'{"ours_yMin":>10} {"ref_yMin":>10} {"dy":>7} {"ours_x":>8} {"ref_x":>8} {"dx":>7} {"ourXM":>8} {"refXM":>8} {"dxM":>7}  text')
    while i < len(lo) and j < len(lr):
        a, b = lo[i], lr[j]
        if a[4] == b[4]:
            print(f'{a[0]:10.2f} {b[0]:10.2f} {a[0]-b[0]:7.2f} {a[2]:8.2f} {b[2]:8.2f} {a[2]-b[2]:7.2f} {a[3]:8.2f} {b[3]:8.2f} {a[3]-b[3]:7.2f}  {a[4][:70]}')
            i += 1; j += 1
        else:
            # try to resync
            k = next((k for k in range(j, min(j + 8, len(lr))) if lr[k][4] == a[4]), None)
            if k is not None:
                for m in range(j, k):
                    print(f'{"":10} {lr[m][0]:10.2f} {"REF-ONLY":>7} {"":8} {lr[m][2]:8.2f} {"":7}  {lr[m][4][:60]}')
                j = k
                continue
            k = next((k for k in range(i, min(i + 8, len(lo))) if lo[k][4] == b[4]), None)
            if k is not None:
                for m in range(i, k):
                    print(f'{lo[m][0]:10.2f} {"":10} {"OUR-ONLY":>7} {lo[m][2]:8.2f} {"":8} {"":7}  {lo[m][4][:60]}')
                i = k
                continue
            print(f'{a[0]:10.2f} {b[0]:10.2f} {a[0]-b[0]:7.2f} {a[2]:8.2f} {b[2]:8.2f} {a[2]-b[2]:7.2f}  O:{a[4][:35]!r} R:{b[4][:35]!r}')
            i += 1; j += 1


main()
