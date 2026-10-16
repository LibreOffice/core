#!/usr/bin/env python3
"""The same census through 26.2.4.2's own eyes.

`/home/user/corpus-odf/sheets` is the reference's `--convert-to ods` of the whole sheets
track, so its `fo:border` values are the widths *and the dash styles* the reference itself
resolved — one uniform instrument over `.xlsx`, `.xlsm` and `.xls` alike, where the
statement census has to read three different containers.
"""
import os, re, sys, zipfile, collections

PATTERNED = ('dotted', 'dashed', 'fine-dashed', 'dash-dot', 'dash-dot-dot')
BORDER = re.compile(rb'fo:border[a-z-]*="([^"]*)"')


def styles_of(path):
    found = collections.Counter()
    with zipfile.ZipFile(path) as z:
        for part in ('content.xml', 'styles.xml'):
            if part not in z.namelist():
                continue
            for m in BORDER.finditer(z.read(part)):
                value = m.group(1).decode('utf-8', 'replace')
                for word in value.split():
                    if word in PATTERNED:
                        found[word] += 1
    return found


def main(root, out):
    paths = []
    for dirpath, _, files in os.walk(root):
        for f in sorted(files):
            if f.endswith('.ods'):
                paths.append(os.path.join(dirpath, f))
    paths.sort()
    hit = 0
    with open(out, 'w') as fh:
        fh.write('doc\tpatterned\tstyles\n')
        for p in paths:
            try:
                found = styles_of(p)
            except Exception as e:
                fh.write('%s\tERR\t%s\n' % (os.path.basename(p), type(e).__name__))
                continue
            total = sum(found.values())
            if total:
                hit += 1
            fh.write('%s\t%d\t%s\n'
                     % (os.path.basename(p), total,
                        ','.join('%s=%d' % kv for kv in sorted(found.items()))))
    print('documents %d  with a patterned border %d' % (len(paths), hit))


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
