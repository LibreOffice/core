#!/usr/bin/env python3
"""Which documents state a non-zero escapement at all -- the only ones FixedCellBox can move.

.pptx / .potx : any a:rPr/@baseline that is neither absent nor "0".
.odp          : any style:text-position whose first number is not 0%.
Reports the base rate beside the hit list: how many documents were scanned.
"""
import re, sys, zipfile

BASE = re.compile(rb'baseline="(-?\d+)"')
POS = re.compile(rb'text-position="(-?[\d.]+)%')


def main(paths):
    hit = []
    bad = 0
    for path in paths:
        try:
            z = zipfile.ZipFile(path)
        except Exception:
            bad += 1
            continue
        n = 0
        with z:
            for name in z.namelist():
                if not name.endswith('.xml'):
                    continue
                try:
                    data = z.read(name)
                except Exception:
                    continue
                n += sum(1 for m in BASE.finditer(data) if int(m.group(1)) != 0)
                n += sum(1 for m in POS.finditer(data) if float(m.group(1)) != 0.0)
        if n:
            hit.append((path, n))
    for p, n in hit:
        print(f'{p.rsplit("/",1)[-1]}\t{n}')
    print(f'## scanned {len(paths)}   unreadable {bad}   with escapement {len(hit)}'
          f'   base rate {len(hit)/max(1,len(paths)):.1%}')


main([l.rstrip('\n') for l in sys.stdin if l.strip()])
