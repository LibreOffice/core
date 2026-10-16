#!/usr/bin/env python3
"""The base rate C9 asks for behind r94's `srcand-census.tsv`.

That census lists the eight corpus documents holding a PAIRED SRCAND blit and the three
holding a LONE one.  Eight out of what?  This counts, over the whole 947-document
manifest: documents scanned, documents holding at least one metafile, metafiles found,
blits found, and only then the SRCAND arms.  Reuses r94's own reader so the numerator is
literally the same code.
"""
import sys, pathlib, csv
HERE = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / 'slides-ink-r94'))
import importlib.util
spec = importlib.util.spec_from_file_location('c94', HERE.parent / 'slides-ink-r94' / 'census-srcand.py')
c94 = importlib.util.module_from_spec(spec)
spec.loader.exec_module(c94)

CORPUS = pathlib.Path('/home/user/sample-files')
rows = list(csv.DictReader(open(CORPUS / 'MANIFEST.tsv'), delimiter='\t'))

scanned = with_mf = mf_total = blit_total = 0
paired_docs = lone_docs = paired_blits = lone_blits = 0
by_ext = {}
for r in rows:
    p = CORPUS / r['path']
    if not p.is_file(): continue
    scanned += 1
    ext = r['ext'].lower()
    try:
        blobs = c94_blobs(p) if False else None
    except Exception:
        pass
    # inline the blob collection from census-srcand.scan so the metafile count is visible
    import io, zipfile
    d = p.read_bytes()
    blobs = []
    if d[:2] == b'PK':
        try:
            with zipfile.ZipFile(io.BytesIO(d)) as z:
                for nm in z.namelist():
                    if nm.lower().endswith(('.wmf', '.emf')):
                        try: blobs.append(('raw', z.read(nm)))
                        except Exception: pass
        except Exception: pass
    elif d[:8] == b'\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1':
        try:
            entries, read = c94.read_ole(str(p))
            for nm, t, st, sz in entries:
                if t != 2 or sz == 0: continue
                try: s = read(nm)
                except Exception: continue
                if not s: continue
                for rt, mf in c94.metafiles(s):
                    blobs.append(('emf' if rt == 0xF01A else 'wmf', mf))
        except Exception: pass
    if not blobs: continue
    with_mf += 1
    mf_total += len(blobs)
    lone = paired = 0
    for kind, b in blobs:
        if kind == 'raw':
            if b[:4] == b'\x01\x00\x09\x00' or b[:4] == b'\xd7\xcd\xc6\x9a':
                off = 22 if b[:4] == b'\xd7\xcd\xc6\x9a' else 0
                bl = list(c94.wmf_blits(b[off:]))
            else:
                bl = list(c94.emf_blits(b))
        elif kind == 'wmf':
            bl = list(c94.wmf_blits(b))
        else:
            bl = list(c94.emf_blits(b))
        blit_total += len(bl)
        l, q = c94.classify(bl)
        lone += l; paired += q
    if lone: lone_docs += 1
    if paired: paired_docs += 1
    lone_blits += lone; paired_blits += paired
    by_ext[ext] = by_ext.get(ext, 0) + 1

print('manifest rows           : %d' % len(rows))
print('documents scanned       : %d' % scanned)
print('documents with metafiles: %d' % with_mf)
print('metafiles found         : %d' % mf_total)
print('blit records found      : %d' % blit_total)
print('documents, paired SRCAND: %d  (blits %d)' % (paired_docs, paired_blits))
print('documents, lone   SRCAND: %d  (blits %d)' % (lone_docs, lone_blits))
print('metafile-holding docs by extension: %s' % sorted(by_ext.items()))
