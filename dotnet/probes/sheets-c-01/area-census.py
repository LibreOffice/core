#!/usr/bin/env python3
"""Every area chart in the corpus, by format, counted where it is actually stated.

OOXML: `c:areaChart` in any chart part of the package.
BIFF:  `CHAREA` (0x101A) anywhere in the Workbook stream.
ODF:   `chart:class="chart:area"` in any nested `content.xml`.

Written after the first version of this census reported one document and the corpus sweep
found two. An under-reaching census conceals itself: a low prediction that comes true reads
as well-calibrated, so the census is the thing to re-run, not the prediction.
"""
import olefile, os, re, struct, zipfile

for track in ('sheets', 'words', 'slides'):
    root = os.path.join('/c/sandbox/workdir/sample-files', track)
    hits = []
    total = 0
    for dirpath, _, names in os.walk(root):
        for name in sorted(names):
            path = os.path.join(dirpath, name)
            total += 1
            found = 0
            # OLE2 first, and the order is load-bearing: `zipfile.is_zipfile` answers *true* for
            # an OLE2 workbook that happens to carry an end-of-central-directory signature —
            # `EHEST-Pre-departure-checklist…xls` is one — so testing zip first routes it down the
            # OOXML branch and its nine CHAREA records are never counted. That is exactly how the
            # first version of this census reported one document where the corpus sweep found two.
            if olefile.isOleFile(path):
                try:
                    handle = olefile.OleFileIO(path)
                    streams = [e for e in handle.listdir() if e[-1] in ('Workbook', 'Book')]
                    if streams:
                        data = handle.openstream(streams[0]).read()
                        at = 0
                        while at + 4 <= len(data):
                            record, length = struct.unpack_from('<HH', data, at)
                            at += 4 + length
                            if record == 0x101A:
                                found += 1
                    handle.close()
                except Exception:
                    pass
            elif zipfile.is_zipfile(path):
                try:
                    with zipfile.ZipFile(path) as package:
                        for entry in package.namelist():
                            if not entry.endswith('.xml'):
                                continue
                            if '/charts/chart' not in entry and not entry.endswith('content.xml'):
                                continue
                            text = package.read(entry).decode('utf-8', 'replace')
                            found += text.count('<c:areaChart')
                            found += text.count('chart:class="chart:area"')
                except Exception:
                    pass
            if found:
                hits.append((name, found))
    print(f"{track}: {len(hits)} of {total} documents hold an area chart")
    for name, found in hits:
        print(f"    {found:3d}  {name}")
