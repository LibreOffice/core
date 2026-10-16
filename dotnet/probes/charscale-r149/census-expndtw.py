#!/usr/bin/env python3
r"""Census `\expndtw` over the converted `.rtf` column — the adjacent gap §C.3 records.

Non-zero occurrences and documents, with the same base rate as `census-rtf.py`. `\expndtw` is
character tracking in twips and is dispatched to `NS_ooxml::LN_EG_RPrBase_spacing`
(`sw/source/writerfilter/rtftok/rtfdispatchvalue.cxx`:187-189), the sprm `w:spacing` uses; the RTF
reader reads none of it.
"""
import collections
import glob
import os
import re

PAT = re.compile(rb'\\expndtw(-?[0-9]+)')
FONT = re.compile(rb'\\f[0-9]+')


def main():
    os.chdir('/home/user/corpus-odf/rtf')
    files = sorted(glob.glob('*.rtf'))
    gross = net = fonts = 0
    docs, dnet, vals = set(), set(), collections.Counter()
    for f in files:
        data = open(f, 'rb').read()
        fonts += len(FONT.findall(data))
        for m in PAT.finditer(data):
            v = int(m.group(1))
            gross += 1
            vals[v] += 1
            docs.add(f)
            if v:
                net += 1
                dnet.add(f)
    print('files %d   base rate \\fN %d' % (len(files), fonts))
    print('gross %d in %d documents' % (gross, len(docs)))
    print('NON-ZERO %d in %d documents' % (net, len(dnet)))
    print('commonest values %s' % vals.most_common(8))


if __name__ == '__main__':
    main()
