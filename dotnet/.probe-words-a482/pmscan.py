#!/usr/bin/env python3
"""Which corpus documents ask to be laid out against printer metrics."""
import sys, zipfile, os, struct

root = sys.argv[1]
docx = []
doc = []
for dirpath, _, names in os.walk(root):
    for n in sorted(names):
        p = os.path.join(dirpath, n)
        rel = os.path.relpath(p, root)
        low = n.lower()
        if low.endswith(('.docx', '.docm', '.dotx')):
            try:
                z = zipfile.ZipFile(p)
                s = z.read('word/settings.xml').decode('utf8', 'replace')
            except Exception:
                continue
            if 'usePrinterMetrics' in s and 'w:usePrinterMetrics w:val="0"' not in s \
               and 'w:usePrinterMetrics w:val="false"' not in s:
                docx.append(rel)
        elif low.endswith(('.doc', '.dot')):
            doc.append(rel)

print(f'DOCX stating w:usePrinterMetrics: {len(docx)}')
for d in docx:
    print('   ', d)
print(f'(DOC files in track: {len(doc)} — fUsePrinterMetrics is a Dop bit, not scanned here)')
