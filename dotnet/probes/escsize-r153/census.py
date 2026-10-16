#!/usr/bin/env python3
"""Which corpus documents STATE an escapement, per format.

A census of what a rule paints would have to resolve the style chain to pair each escaped run with
its base size, which this does not attempt: the reach figure of record is the sweep's mover list.
What this answers is the denominator -- how many documents can be reached at all.
"""
import re, sys, zipfile, csv, pathlib

CORPUS = pathlib.Path('/home/user/sample-files')
ODF = pathlib.Path('/home/user/corpus-odf')

def zip_hits(p, pat):
    try:
        with zipfile.ZipFile(p) as z:
            for n in z.namelist():
                if n.endswith('.xml') and ('word/' in n or n.startswith('xl/') or 'ppt/' in n):
                    if pat.search(z.read(n).decode('utf-8', 'replace')):
                        return True
    except Exception:
        return False
    return False

VA = re.compile(r'<w:vertAlign[^>]*w:val="(superscript|subscript)"')
rows = []
with open(CORPUS / 'MANIFEST.tsv') as f:
    for r in csv.DictReader(f, delimiter='\t'):
        rel = r.get('path') or list(r.values())[2]
        p = CORPUS / rel
        ext = p.suffix.lower()
        if ext in ('.docx', '.docm'):
            rows.append((ext, rel, zip_hits(p, VA)))

print('docx/docm stating a w:vertAlign super/subscript: '
      f'{sum(1 for _, _, h in rows if h)} of {len(rows)}')

# `style:text-position` states the rise as a percentage OR as the keyword `super`/`sub`, and
# LibreOffice's own exporter writes the keyword: reading only the numeric spelling finds none of
# the corpus's 477 escaped runs.
TP = re.compile(r'style:text-position="(super|sub|[-0-9.]+%)\s+([0-9.]+)%"')
n = m = 0
for p in sorted(ODF.glob('odt/*.odt')):
    n += 1
    try:
        with zipfile.ZipFile(p) as z:
            s = z.read('content.xml').decode('utf-8', 'replace') + z.read('styles.xml').decode('utf-8', 'replace')
    except Exception:
        continue
    if any(float(g[1]) != 100 for g in TP.findall(s)):
        m += 1
print(f'converted .odt stating a style:text-position proportion other than 100%: {m} of {n}')

SUP = re.compile(rb'\\(super|sub)[^a-z]')
n = m = 0
for p in sorted(ODF.glob('rtf/*.rtf')):
    n += 1
    if SUP.search(p.read_bytes()):
        m += 1
print(f'converted .rtf stating \\super or \\sub: {m} of {n}')
