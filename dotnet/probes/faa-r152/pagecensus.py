#!/usr/bin/env python3
"""Per-page size/orientation and alphanumeric-character census of two PDFs, paired by index."""
import sys, pymupdf
a, b = pymupdf.open(sys.argv[1]), pymupdf.open(sys.argv[2])
print(f'pages\t{a.page_count}\t{b.page_count}')
print('page\tref_w\tref_h\tref_o\tours_w\tours_h\tours_o\tref_an\tours_an\tsame_o')
n = max(a.page_count, b.page_count)
bad = []
for i in range(n):
    def info(d):
        if i >= d.page_count: return (0,0,'-',0)
        p = d[i]; r = p.rect
        an = sum(c.isalnum() for c in p.get_text())
        return (round(r.width,1), round(r.height,1), 'L' if r.width > r.height else 'P', an)
    rw,rh,ro,ran = info(a); ow,oh,oo,oan = info(b)
    same = 'yes' if ro == oo else 'NO'
    if same == 'NO': bad.append(i+1)
    print(f'{i+1}\t{rw}\t{rh}\t{ro}\t{ow}\t{oh}\t{oo}\t{ran}\t{oan}\t{same}')
print('# orientation mismatches:', len(bad), bad, file=sys.stderr)
