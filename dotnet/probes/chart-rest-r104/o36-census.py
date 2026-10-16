#!/usr/bin/env python3
"""Which corpus workbooks state a chartex chart, and which state a chart `c:f` that names a
workbook-level defined name rather than a sheet-qualified range.

The second question is the confinement question for this round's `XlsxChartRanges` change:
a defined name resolved where it was previously declined replaces a chart's *cached* points
with the workbook's live ones.

    o36-census.py MANIFEST.tsv SAMPLEROOT
"""
import os, re, sys, zipfile

CX = 'schemas.microsoft.com/office/drawing/2014/chartex'
F = re.compile(rb'<(?:\w+:)?f>([^<]*)</(?:\w+:)?f>')

def main():
    manifest, root = sys.argv[1], sys.argv[2]
    print('\t'.join(['document', 'ext', 'chartex_parts', 'cx_f', 'c_f',
                     'c_f_defined_name', 'defined_names']))
    n_cx = n_dn = total = 0
    with open(manifest) as fh:
        next(fh)
        for line in fh:
            fields = line.rstrip('\n').split('\t')
            path = os.path.join(root, fields[2])
            if not zipfile.is_zipfile(path):
                continue
            total += 1
            with zipfile.ZipFile(path) as z:
                names = set()
                try:
                    wb = z.read('xl/workbook.xml')
                except KeyError:
                    wb = b''
                for m in re.finditer(rb'<definedName\b([^>]*)>', wb):
                    if b'localSheetId' in m.group(1):
                        continue
                    q = re.search(rb'name="([^"]*)"', m.group(1))
                    if q:
                        names.add(q.group(1).decode('utf8', 'replace'))

                cx_parts, cx_f, c_f, c_dn = 0, 0, 0, 0
                for member in z.namelist():
                    if 'chart' not in member.lower() or not member.endswith('.xml'):
                        continue
                    blob = z.read(member)
                    extended = CX.encode() in blob
                    if extended:
                        cx_parts += 1
                    for m in F.finditer(blob):
                        text = m.group(1).decode('utf8', 'replace').strip()
                        if not text:
                            continue
                        if extended:
                            cx_f += 1
                        else:
                            c_f += 1
                            if '!' not in text and text in names:
                                c_dn += 1

                if cx_parts == 0 and c_dn == 0:
                    continue
                n_cx += 1 if cx_parts else 0
                n_dn += 1 if c_dn else 0
                print('\t'.join([os.path.basename(fields[2]), fields[3], str(cx_parts),
                                 str(cx_f), str(c_f), str(c_dn), str(len(names))]))
    print(f'# {total} zip documents scanned; {n_cx} state a chartex part; '
          f'{n_dn} state a plain c:f naming a workbook-level defined name', file=sys.stderr)

main()
