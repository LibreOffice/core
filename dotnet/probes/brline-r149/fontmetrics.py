#!/usr/bin/env python3
"""hhea / OS-2 metrics of the four faces the `f-` arms use, against the drawn line heights."""
import struct
import subprocess
import sys

MEASURED = {'DejaVu Sans': 23.300, 'Liberation Sans': 23.000,
            'Liberation Mono': 22.650, 'FreeSerif': 22.000}
SIZE = 20.0


def path_of(family):
    out = subprocess.run(['fc-match', '-f', '%{file}', family], capture_output=True, text=True)
    return out.stdout.strip()


def metrics(path):
    d = open(path, 'rb').read()
    n = struct.unpack('>H', d[4:6])[0]
    t = {}
    for i in range(n):
        o = 12 + 16 * i
        t[d[o:o + 4].decode()] = struct.unpack('>II', d[o + 8:o + 16])
    upem = struct.unpack('>H', d[t['head'][0] + 18:t['head'][0] + 20])[0]
    a, de, g = struct.unpack('>hhh', d[t['hhea'][0] + 4:t['hhea'][0] + 10])
    o2 = t['OS/2'][0]
    ta, td, tg = struct.unpack('>hhh', d[o2 + 68:o2 + 74])
    wa, wd = struct.unpack('>HH', d[o2 + 74:o2 + 78])
    return upem, a, -de, g, ta, -td, tg, wa, wd


def main():
    print('%-16s %-40s %8s %8s %8s %8s' %
          ('family', 'file', 'hhea', 'win', 'typo+gap', 'drawn'))
    for family, drawn in sorted(MEASURED.items()):
        p = path_of(family)
        upem, a, de, g, ta, td, tg, wa, wd = metrics(p)
        f = lambda v: v / upem * SIZE
        print('%-16s %-40s %8.3f %8.3f %8.3f %8.3f' %
              (family, p.split('/')[-1], f(a + de + g), f(wa + wd), f(ta + td + tg), drawn))


if __name__ == '__main__':
    main()
