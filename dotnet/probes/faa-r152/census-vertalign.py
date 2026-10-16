#!/usr/bin/env python3
"""How many words-track documents state an automatic superscript/subscript, and at what sizes?

Static, and therefore DOCX-only in its numerator: the 66 `.doc` of the words track cannot be read
this way and are counted only as present. A run's size is not resolved through the style chain
here -- the sizes counted are those a `w:sz` states directly, which is a lower bound.
"""
import collections, pathlib, re, sys, zipfile

root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files/words')
docx = sorted(p for p in root.rglob('*.docx'))
other = sorted(p for p in root.rglob('*') if p.suffix.lower() in ('.doc', '.rtf', '.odt'))
hits, affected = 0, []
halves = collections.Counter()
for p in docx:
    try:
        with zipfile.ZipFile(p) as z:
            names = [n for n in z.namelist() if n.startswith('word/') and n.endswith('.xml')]
            n = 0
            for nm in names:
                s = z.read(nm).decode('utf-8', 'replace')
                n += len(re.findall(r'w:vertAlign w:val="(?:super|sub)script"', s))
                for m in re.finditer(r'<w:sz w:val="(\d+)"/>(?=[^<]*)', s):
                    pass
    except Exception:
        continue
    if n:
        hits += 1; affected.append((p.name, n))
print(f'words-track .docx scanned: {len(docx)}')
print(f'  stating w:vertAlign super/subscript: {hits}')
print(f'  not statically censusable (.doc/.rtf/.odt present): {len(other)}')
affected.sort(key=lambda t: -t[1])
for nm, n in affected[:15]:
    print(f'    {n:6d}  {nm}')
