#!/usr/bin/env python3
"""Baselines of the shape's `Hxy` (and, where present, the page-body `Pgy`) and of the `RULER`.

Positions are reported relative to the RULER's own baseline, which every fixture shares, so no
absolute page origin is ever needed and a shape that moved cannot be mistaken for a line that
grew.
"""
import glob
import os
import sys

import pymupdf


def spans(path):
    out = {}
    page = pymupdf.open(path)[0]
    for block in page.get_text('dict')['blocks']:
        for line in block.get('lines', ()):
            for span in line['spans']:
                key = span['text'].strip()
                if key in ('RULER', 'Hxy', 'Pgy', 'AHxy'):
                    out.setdefault(key, []).append((span['origin'][1], span['size'], span['font']))
                elif key:
                    out.setdefault('?' + key[:8], []).append((span['origin'][1], span['size'],
                                                              span['font']))
    return out


def main():
    d = sys.argv[1] if len(sys.argv) > 1 else 'out3'
    print('%-12s %9s %9s %9s  %s' % ('arm', 'ruler', 'Hxy-ruler', 'Pgy-ruler', 'font/size/notes'))
    for path in sorted(glob.glob(os.path.join(d, '*.pdf'))):
        arm = os.path.basename(path)[:-4]
        s = spans(path)
        if 'RULER' not in s:
            print('%-12s RULER MISSING' % arm)
            continue
        r = s['RULER'][0][0]
        h = s.get('Hxy') or s.get('AHxy')
        note = ' '.join('%s@%.3f' % (k, v[0][0] - r) for k, v in sorted(s.items())
                        if k.startswith('?'))
        print('%-12s %9.3f %9s %9s  %s %s' % (
            arm, r,
            '%.3f' % (h[0][0] - r) if h else 'NONE',
            '%.3f' % (s['Pgy'][0][0] - r) if 'Pgy' in s else '-',
            ('%s/%.2f' % (h[0][2], h[0][1])) if h else '',
            note))


if __name__ == '__main__':
    main()
