#!/usr/bin/env python3
"""Which corpus documents name any of the given families (normalised)."""
import glob, os, re, sys, zipfile

def normalise(s):
    return ''.join(c.lower() for c in s if c.isalnum())

WANT = {normalise(a) for a in sys.argv[1:]}
PATTERNS = [
    re.compile(r'<name val="([^"]*)"'),
    re.compile(r'w:ascii="([^"]*)"'),
    re.compile(r'<w:rFonts[^>]*w:cs="([^"]*)"'),
    re.compile(r'typeface="([^"]*)"'),
]
root = '/workspace/sample-files'
for track in ('sheets', 'words', 'slides'):
    for path in sorted(glob.glob(f'{root}/{track}/batch-*/*/*')):
        low = path.lower()
        hits = set()
        if low.endswith(('.xlsx', '.xlsm', '.docx', '.docm', '.pptx', '.pptm')):
            try:
                with zipfile.ZipFile(path) as z:
                    for name in z.namelist():
                        if not name.endswith('.xml'):
                            continue
                        d = z.read(name).decode('utf8', 'replace')
                        for pat in PATTERNS:
                            for m in pat.finditer(d):
                                if normalise(m.group(1)) in WANT:
                                    hits.add(m.group(1))
            except Exception:                                        # noqa: BLE001
                continue
        else:
            # Binary formats keep font names as plain (or UTF-16) runs in the stream.
            try:
                b = open(path, 'rb').read()
            except OSError:
                continue
            for want in sys.argv[1:]:
                a = want.encode('latin1')
                u = want.encode('utf-16-le')
                if a in b or u in b:
                    hits.add(want)
        if hits:
            print(f"{path.split('/', 3)[-1]}\t{sorted(hits)}")
