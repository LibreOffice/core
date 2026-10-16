#!/usr/bin/env python3
"""Export-side census: of the words-track .docx that state a PAGE/NUMPAGES field in a
header or footer, how many lose it in 26.2.4.2's own --convert-to odt of the same document?

Reads /home/user/sample-files/words/**/*.docx against /home/user/corpus-odf/words/**/odt/*.odt.
Prints the denominator and every loser.  Round 137.
"""
import zipfile, re, glob, os

FLD = re.compile(r'\b(page|numpages|sectionpages)\b', re.I)

def hf_pagefield(p):
    try:
        z = zipfile.ZipFile(p)
    except Exception:
        return False
    for n in z.namelist():
        if re.match(r'word/(header|footer)\d*\.xml$', n):
            x = z.read(n).decode('utf-8', 'replace')
            for m in re.finditer(r'<w:instrText[^>]*>(.*?)</w:instrText>', x, re.S):
                if FLD.search(m.group(1)):
                    return True
            for m in re.finditer(r'<w:fldSimple[^>]*\sw:instr="([^"]*)"', x):
                if FLD.search(m.group(1)):
                    return True
    return False

have = 0
lost = []
for p in sorted(glob.glob('/home/user/sample-files/words/**/*.docx', recursive=True)):
    if not hf_pagefield(p):
        continue
    have += 1
    rel = os.path.relpath(p, '/home/user/sample-files')
    d = os.path.dirname(rel).replace('/docx', '/odt')
    odt = os.path.join('/home/user/corpus-odf', d, os.path.basename(p)[:-5] + '.odt')
    if not os.path.exists(odt):
        lost.append((p, 'NO-ODT'))
        continue
    s = zipfile.ZipFile(odt).read('styles.xml').decode('utf-8', 'replace')
    if '<text:page-number' not in s:
        lost.append((p, 'NO-FIELD-IN-ODT'))

print("docx with a PAGE/NUMPAGES field in a header/footer:", have)
print("of those, whose 26.2.4.2 .odt export has no text:page-number:", len(lost))
for p, w in lost:
    print("  ", w, os.path.basename(p))
