#!/usr/bin/env python3
"""What `\\charscalex` and `sprmCCharScale` actually state, and where.

NON-IDENTITY occurrences, because a run stating 100 costs nothing: counting it overstated the ODF
reach by a factor of three in round 148 and overstates this one by the same shape. The base rate is
beside each count, because a count without one is not a reach figure.

The `.rtf` column is 26.2.4.2's own `--convert-to rtf` of the words track, built by
`probes/charscale-r149/convert-rtf.sh`. `dotnet/CLAUDE.md` describes four converted columns and this
container had two, so a glob over the absent directory used to return 0 occurrences AND 0 base-rate
tokens -- which is the tell that a census has measured nothing rather than found nothing.
"""
import collections
import pathlib
import re

RTF = pathlib.Path('/home/user/corpus-odf/rtf')
SCALE = re.compile(rb'\\charscalex(-?\d+)')
FONT = re.compile(rb'\\f\d+')


def main():
    docs = gross = net = 0
    base = 0
    values = collections.Counter()
    net_docs = 0

    for path in sorted(RTF.glob('*.rtf')):
        raw = path.read_bytes()
        base += len(FONT.findall(raw))
        hits = [int(v) for v in SCALE.findall(raw)]
        if hits:
            docs += 1
        gross += len(hits)
        off = [v for v in hits if v != 100]
        if off:
            net_docs += 1
        net += len(off)
        values.update(off)

    print('rtf column           %d documents' % len(list(RTF.glob('*.rtf'))))
    print('  base rate \\fN      %d' % base)
    print('  gross              %d occurrences in %d documents' % (gross, docs))
    print('  non-identity       %d occurrences in %d documents' % (net, net_docs))
    print('  commonest values   %s' % values.most_common(6))


if __name__ == '__main__':
    main()
