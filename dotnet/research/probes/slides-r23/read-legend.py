#!/usr/bin/env python3
"""Read a legend's geometry straight out of a rendered chart page.

    read-legend.py <pdf> …

Reported per file:

    borderW/H   the legend's own rectangle, so its reserved size directly
    pad         the first key's left edge less the border's, the padding inside it
    keyW        the key's width
    gap         key right edge to the text pen — the symbol-to-text distance, plus any inset
    pitch       key 1's y minus key 2's y — the row height plus the y offset
    nameW+pad   border right minus the text pen: the name's own width plus the right padding

The legend is found by its keys rather than by a coordinate: a legend key is the only
*square* filled rectangle a bar chart draws, since every bar is far taller than it is wide.
The border is then the stroked rectangle that contains them, and the names are the text pens
to the right of a key.
"""
import re
import subprocess
import sys

OPS = ('/home/user/libreoffice-core/.claude/skills/render-comparison/'
       'scripts/pdf-ops.py')

RECORD = re.compile(
    r'^(fill|stroke|text)\s+p(\d+)\s+\(\s*([-0-9.]+),\s*([-0-9.]+)\)'
    r'(?:-\(\s*([-0-9.]+),\s*([-0-9.]+)\))?')


def records(path):
    out = subprocess.run([sys.executable, OPS, 'dump', path, '--page', '1'],
                         capture_output=True, text=True).stdout
    for line in out.splitlines():
        m = RECORD.match(line.strip())
        if m:
            kind, _, x0, y0, x1, y1 = m.groups()
            yield (kind, float(x0), float(y0),
                   float(x1) if x1 else None, float(y1) if y1 else None)


def main():
    print(f'{"file":28} {"borderW":>8} {"borderH":>8} {"pad":>6} {"keyW":>6} '
          f'{"gap":>6} {"pitch":>7} {"nameW+pad":>10}')
    for path in sys.argv[1:]:
        rows = list(records(path))
        keys = [r for r in rows if r[0] == 'fill' and r[3]
                and abs((r[3] - r[1]) - (r[4] - r[2])) < 0.2 and (r[3] - r[1]) < 20]
        keys.sort(key=lambda r: -r[2])
        if len(keys) < 2:
            print(f'{path.split("/")[-1]:28} no legend keys found')
            continue

        keyL, keyR = keys[0][1], keys[0][3]
        pitch = keys[0][2] - keys[1][2]

        texts = [r for r in rows if r[0] == 'text' and r[1] > keyR
                 and abs(r[2] - keys[0][2]) < pitch]
        if not texts:
            print(f'{path.split("/")[-1]:28} no legend name found')
            continue
        pen = min(t[1] for t in texts)

        borders = [r for r in rows if r[0] == 'stroke' and r[3]
                   and r[1] < keyL and r[3] > pen and (r[4] - r[2]) > pitch]
        if borders:
            b = min(borders, key=lambda r: (r[3] - r[1]) * (r[4] - r[2]))
            bw, bh, pad, nameplus = b[3] - b[1], b[4] - b[2], keyL - b[1], b[3] - pen
        else:
            bw = bh = pad = nameplus = float('nan')

        print(f'{path.split("/")[-1]:28} {bw:8.2f} {bh:8.2f} {pad:6.2f} '
              f'{keyR - keyL:6.2f} {pen - keyR:6.2f} {pitch:7.2f} {nameplus:10.2f}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
