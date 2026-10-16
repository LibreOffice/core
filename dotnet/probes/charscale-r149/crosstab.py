#!/usr/bin/env python3
"""Cross-tabulate the `.rtf` census against round 148's `.odt` census, by hash stem.

Both columns are 26.2.4.2's own conversion of the SAME 337 words-track originals and both
stems are the first 12 hex of the md5 of the original's absolute path, so a document that
states a character width in one column and not in the other is a statement about the two
EXPORTERS rather than about the corpus.
"""
import pathlib
import re
import subprocess
import sys

SCALE_RTF = re.compile(rb'\\charscalex(-?[0-9]+)')
SCALE_ODF = re.compile(rb'style:text-scale="([^"]*)"')


def net_odt(path):
    import zipfile
    n = 0
    with zipfile.ZipFile(path) as z:
        for part in ('content.xml', 'styles.xml'):
            if part in z.namelist():
                for m in SCALE_ODF.finditer(z.read(part)):
                    v = m.group(1).decode().strip().rstrip('%')
                    try:
                        if abs(float(v) - 100.0) > 1e-9:
                            n += 1
                    except ValueError:
                        pass
    return n


def main():
    rtf = {p.name[:12]: p for p in pathlib.Path('/home/user/corpus-odf/rtf').glob('*.rtf')}
    odt = {p.name[:12]: p for p in pathlib.Path('/home/user/corpus-odf/odt').glob('*.odt')}
    print('stems rtf %d  odt %d  shared %d' % (len(rtf), len(odt), len(set(rtf) & set(odt))))
    rows = []
    for stem in sorted(set(rtf) | set(odt)):
        r = sum(1 for m in SCALE_RTF.finditer(rtf[stem].read_bytes())
                if int(m.group(1)) != 100) if stem in rtf else None
        o = net_odt(odt[stem]) if stem in odt else None
        if r or o:
            rows.append((stem, r, o, (rtf.get(stem) or odt[stem]).name[13:]))
    print('stem\trtf_net\todt_net\tdocument')
    for row in rows:
        print('%s\t%s\t%s\t%s' % row)
    print()
    print('documents stating one in rtf : %d' % sum(1 for r in rows if r[1]))
    print('documents stating one in odt : %d' % sum(1 for r in rows if r[2]))
    print('in both                      : %d' % sum(1 for r in rows if r[1] and r[2]))
    print('rtf only                     : %d' % sum(1 for r in rows if r[1] and not r[2]))
    print('odt only                     : %d' % sum(1 for r in rows if r[2] and not r[1]))


if __name__ == '__main__':
    main()
