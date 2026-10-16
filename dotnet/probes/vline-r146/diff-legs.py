#!/usr/bin/env python3
"""Diff two legs of the words sweep and tag each mover by what this round could reach in it.

The tags are deliberately narrow, because the round's whole claim is that nothing outside them
moved:
  vline-live    a `v:line` that is NOT inside an `mc:Fallback` -- the only ones either renderer
                reads, and 859 of the corpus's 865 are excluded by this test
  vline-group   such a line inside a `v:group`
  syscolour     a `strokecolor`/`fillcolor` naming a system colour rather than a preset or a hex
"""
import collections
import os
import re
import sys
import zipfile

CORPUS = '/home/user/sample-files'
LINE = re.compile(r'<v:line\b')
SYSTEM = re.compile(r'(?:stroke|fill)color="([A-Za-z][A-Za-z0-9]*)"')
PRESETS = {'black', 'silver', 'gray', 'grey', 'white', 'maroon', 'red', 'purple', 'fuchsia',
           'green', 'lime', 'olive', 'yellow', 'navy', 'blue', 'teal', 'aqua', 'cyan',
           'magenta', 'orange', 'none'}


def fingerprints(path):
    rows = {}
    for line in open(path):
        digest, _, name = line.rstrip('\n').partition('\t')
        rows[name] = digest
    return rows


def live_lines(text):
    """Every `<v:line` whose ancestry holds no `mc:Fallback`, with a flag for being in a group.

    A depth counter over the tag stream rather than an XML parse, because the parts are large
    and this only has to know whether two elements are open."""
    out = []
    fallback = 0
    group = 0
    for match in re.finditer(r'<(/?)(mc:Fallback|v:group|v:line)\b([^>]*)', text):
        closing, name, rest = match.group(1), match.group(2), match.group(3)
        empty = rest.rstrip().endswith('/')
        if name == 'v:line':
            if not fallback:
                out.append(bool(group))
            continue
        if closing:
            if name == 'mc:Fallback':
                fallback = max(0, fallback - 1)
            else:
                group = max(0, group - 1)
        elif not empty:
            if name == 'mc:Fallback':
                fallback += 1
            else:
                group += 1
    return out


def tags(path):
    full = os.path.join(CORPUS, path)
    try:
        archive = zipfile.ZipFile(full)
    except Exception:
        return ['not-a-zip']

    out = set()
    for name in archive.namelist():
        if not (name.startswith('word/') and name.endswith('.xml')):
            continue
        text = archive.read(name).decode('utf8', 'replace')
        if LINE.search(text):
            for grouped in live_lines(text):
                out.add('vline-group' if grouped else 'vline-live')
        for colour in SYSTEM.findall(text):
            if colour.lower() not in PRESETS:
                out.add('syscolour')
    return sorted(out) or ['NOTHING']


def main():
    base, after = fingerprints(sys.argv[1]), fingerprints(sys.argv[2])
    manifest = [line.split('\t')[2]
                for line in open(os.path.join(CORPUS, 'MANIFEST.tsv')).read().splitlines()[1:]
                if line.split('\t')[0] == 'words']

    missing = [p for p in manifest if p not in base or p not in after]
    if missing:
        print('REFUSING TO SCORE: %d manifest paths have no row' % len(missing))
        return

    movers = [p for p in manifest if base[p] != after[p]]
    failed = [p for p in manifest if 'FAILED' in (base[p], after[p])]
    kinds = collections.Counter('+'.join(tags(p)) for p in movers)

    print('documents scored: %d' % len(manifest))
    print('failed on either leg: %d' % len(failed))
    print('byte-identical: %d' % (len(manifest) - len(movers)))
    print('moved: %d' % len(movers))
    for kind, count in kinds.most_common():
        print('   %-34s %d' % (kind, count))
    print()
    for p in sorted(movers):
        print('%-34s %s' % ('+'.join(tags(p)), p))


if __name__ == '__main__':
    main()
