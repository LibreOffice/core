"""Diff two legs of the sweep and say, for each mover, what states a fill this round can reach.

The fingerprint files are `md5<TAB>corpus-relative path`. A row missing from either leg is
reported rather than silently dropped: a leg that lost a worker reports fewer rows, not an
error, so the count is checked against the manifest before anything is compared."""
import collections
import os
import re
import sys
import zipfile

CORPUS = '/home/user/sample-files'
TYPE = re.compile(r'patternType="([^"]+)"')
EXPRESSION = re.compile(r'type="expression"')


def fingerprints(path):
    rows = {}
    for line in open(path):
        digest, _, name = line.rstrip('\n').partition('\t')
        rows[name] = digest
    return rows


def states(path):
    """What the mover holds that this round could have reached."""
    full = os.path.join(CORPUS, path)
    try:
        archive = zipfile.ZipFile(full)
    except Exception:
        return 'not-an-opc-workbook'
    names = archive.namelist()
    held = []
    if 'xl/styles.xml' in names:
        styles = archive.read('xl/styles.xml').decode('utf8', 'replace')
        start, end = styles.find('<dxfs'), styles.find('</dxfs>')
        if start >= 0 and any(k not in ('none', 'solid')
                              for k in TYPE.findall(styles[start:end])):
            held.append('dxf-hatch')
        start, end = styles.find('<fills'), styles.find('</fills>')
        if start >= 0 and any(k not in ('none', 'solid')
                              for k in TYPE.findall(styles[start:end])):
            held.append('cell-hatch')
    for name in names:
        if name.startswith('xl/worksheets/') and name.endswith('.xml'):
            if EXPRESSION.search(archive.read(name).decode('utf8', 'replace')):
                held.append('expression-rule')
                break
    return '+'.join(held) or 'NOTHING'


def main():
    base, after = fingerprints(sys.argv[1]), fingerprints(sys.argv[2])
    manifest = [line.split('\t')[2]
                for line in open(os.path.join(CORPUS, 'MANIFEST.tsv')).read().splitlines()[1:]
                if line.split('\t')[0] == 'sheets']

    missing = [p for p in manifest if p not in base or p not in after]
    if missing:
        print('REFUSING TO SCORE: %d manifest paths have no row' % len(missing))
        for p in missing[:10]:
            print('   ', p)
        return

    movers = [p for p in manifest if base[p] != after[p]]
    failed = [p for p in manifest if 'FAILED' in (base[p], after[p])]
    kinds = collections.Counter(states(p) for p in movers)

    print('documents scored: %d' % len(manifest))
    print('failed on either leg: %d' % len(failed))
    print('byte-identical: %d' % (len(manifest) - len(movers)))
    print('moved: %d' % len(movers))
    for kind, count in kinds.most_common():
        print('   %-40s %d' % (kind, count))
    print()
    for p in sorted(movers):
        print('%-16s %s' % (states(p), p))


if __name__ == '__main__':
    main()
