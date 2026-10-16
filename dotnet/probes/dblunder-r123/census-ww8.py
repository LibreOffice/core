#!/usr/bin/env python3
r"""Second, independent leg for the 66 `.doc`: scan for WW8's own underline sprm.

The converted corpus is 26.2.4.2's reading of a binary document; this is the bytes'.  A
CHPX grpprl states the underline as `sprmCKul`, opcode 0x2A3E little-endian, one operand
byte -- `KUL`, of which **3 is double** (`sw/source/filter/ww8/ww8par6.cxx`'s
`Read_Underline` maps `case 3: eUnderline = LINESTYLE_DOUBLE`).  The scan is over the whole
file rather than over a parsed FKP, so it OVER-counts: any two bytes 0x3E 0x2A anywhere in
the stream match.  That direction is the safe one -- a document with the sprm cannot be
missed, only a document without one wrongly flagged -- so a zero is evidence and a non-zero
is a candidate to confirm.
"""
import sys, os, collections

OP = b'\x3e\x2a'
KUL = {0: 'none', 1: 'single', 2: 'word', 3: 'DOUBLE', 4: 'dotted', 6: 'thick',
       7: 'dash', 9: 'dotdash', 10: 'dotdotdash', 11: 'wave'}


def scan(path):
    blob = open(path, 'rb').read()
    hits = collections.Counter()
    i = blob.find(OP)
    while i >= 0:
        if i + 2 < len(blob):
            hits[blob[i + 2]] += 1
        i = blob.find(OP, i + 1)
    return hits


def main(manifest, root):
    print('path\ttotal_sprm\tkul3_double\tbreakdown')
    for line in open(manifest).read().splitlines()[1:]:
        f = line.split('\t')
        if f[3] != 'doc':
            continue
        h = scan(os.path.join(root, f[2]))
        print('%s\t%d\t%d\t%s' % (f[2], sum(h.values()), h.get(3, 0),
                                  ' '.join('%s=%d' % (KUL.get(k, k), v)
                                           for k, v in sorted(h.items()))))


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
