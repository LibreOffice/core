#!/usr/bin/env python3
"""`style:text-scale` over the 337 converted `.odt`, and the cross-tab against the sweep.

Counts NON-IDENTITY occurrences, because a run stating 100 % costs nothing and a census that
includes them overstates the reach by a factor of three here. Reads `content.xml` and
`styles.xml` both -- LibreOffice's exporter puts a list level's character style in the second.
"""
import collections
import pathlib
import re
import zipfile

CORPUS = pathlib.Path('/home/user/corpus-odf/odt')
PARTS = ('content.xml', 'styles.xml')
SCALE = re.compile(r'(style|loext):text-scale="([^"]*)"')


def read(path):
    with zipfile.ZipFile(path) as package:
        names = [n for n in PARTS if n in package.namelist()]
        return b''.join(package.read(n) for n in names).decode('utf8', 'replace')


def main():
    movers = set(pathlib.Path(__file__).with_name('movers.txt').read_text().splitlines()) - {''}
    total = collections.Counter()
    spans = 0
    stated = {}

    for path in sorted(CORPUS.glob('*.odt')):
        text = read(path)
        spans += text.count('<text:span')
        values = SCALE.findall(text)
        for namespace, value in values:
            total[namespace] += 1
        off = collections.Counter(v.strip().rstrip('%') for _, v in values
                                  if v.strip().rstrip('%') not in ('100', ''))
        if off:
            stated[path.stem] = off

    print('occurrences by namespace : %s' % dict(total))
    print('base rate, text:span     : %d' % spans)
    print('documents off the identity: %d' % len(stated))
    print()
    for stem, counts in sorted(stated.items()):
        print('%s %-34s %s' % ('MOVED ' if stem in movers else '  --  ',
                               str(dict(counts))[:34], stem[:76]))
    print()
    print('movers the census did not predict: %s' % sorted(set(m for m in movers if m) - set(stated)))


if __name__ == '__main__':
    main()
