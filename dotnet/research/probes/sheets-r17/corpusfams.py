#!/usr/bin/env python3
"""Every font family any corpus OOXML document names, one per line."""
import glob, os, re, sys, zipfile

PATTERNS = [
    re.compile(r'<name val="([^"]*)"'),
    re.compile(r'w:ascii="([^"]*)"'),
    re.compile(r'<w:rFonts[^>]*w:cs="([^"]*)"'),
    re.compile(r'typeface="([^"]*)"'),
]
root = '/workspace/sample-files'
fams = set()
for track in ('sheets', 'words', 'slides'):
    for path in sorted(glob.glob(f'{root}/{track}/batch-*/*/*')):
        if not path.lower().endswith(('.xlsx', '.xlsm', '.docx', '.docm', '.pptx', '.pptm')):
            continue
        try:
            with zipfile.ZipFile(path) as z:
                for name in z.namelist():
                    if not name.endswith('.xml'):
                        continue
                    if not any(k in name for k in
                               ('styles', 'theme', 'document', 'slide', 'fontTable')):
                        continue
                    d = z.read(name).decode('utf8', 'replace')
                    for pat in PATTERNS:
                        for m in pat.finditer(d):
                            v = m.group(1).strip()
                            if v and v[0] not in '+' and '&' not in v and '<' not in v:
                                fams.add(v)
        except Exception:                                            # noqa: BLE001
            continue
with open(sys.argv[1], 'w', encoding='utf8') as out:
    for f in sorted(fams):
        out.write(f + '\n')
print(len(fams), '->', sys.argv[1])
