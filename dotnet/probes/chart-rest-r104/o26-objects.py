#!/usr/bin/env python3
"""For every corpus rendering whose block clip removed ink the reference keeps
(`clip-seats-r97/overreach.tsv`, `over_px > 0`), what does the source document hold?

The escape mechanism (results.md §1) is a `MaskPrimitive2D` reaching
`VclMetafileProcessor2D::processMaskPrimitive2D`, which calls `SetClipRegion` — a
*replace*.  Only two things in a Calc drawing produce one: a WMF/EMF picture whose
own metafile carries clip records, and a metafile whose content overflows its frame.
So the census asks each over-clipping document for its media and its clip records.

    o26-objects.py OVERREACH.tsv MANIFEST.tsv SAMPLEROOT
"""
import collections, os, re, struct, sys, zipfile

def emf_clip_records(blob):
    """EMR_EXTSELECTCLIPRGN (75), EMR_SETMETARGN (28), EMR_INTERSECTCLIPRECT (30),
    EMR_EXCLUDECLIPRECT (29), EMR_OFFSETCLIPRGN (26).  Walks the record list."""
    counts = collections.Counter()
    if len(blob) < 8 or struct.unpack_from('<I', blob, 0)[0] != 1:
        return counts, False
    off, n = 0, 0
    while off + 8 <= len(blob) and n < 500000:
        typ, size = struct.unpack_from('<II', blob, off)
        if size < 8 or off + size > len(blob):
            break
        if typ in (26, 28, 29, 30, 75):
            counts[typ] += 1
        off += size
        n += 1
        if typ == 14:      # EMR_EOF
            break
    return counts, True

def wmf_clip_records(blob):
    """WMF META_INTERSECTCLIPRECT 0x0416, META_EXCLUDECLIPRECT 0x0415,
    META_OFFSETCLIPRGN 0x0220, META_SELECTCLIPREGION 0x012C."""
    counts = collections.Counter()
    off = 22 if blob[:4] == b'\xd7\xcd\xc6\x9a' else 0
    if len(blob) < off + 18:
        return counts, False
    if struct.unpack_from('<H', blob, off)[0] not in (1, 2):
        return counts, False
    p = off + 18
    n = 0
    while p + 6 <= len(blob) and n < 500000:
        size, func = struct.unpack_from('<IH', blob, p)
        if size < 3:
            break
        if func in (0x0416, 0x0415, 0x0220, 0x012C):
            counts[func] += 1
        p += size * 2
        n += 1
        if func == 0:
            break
    return counts, True

def main():
    over, manifest, root = sys.argv[1], sys.argv[2], sys.argv[3]

    pages = collections.defaultdict(list)
    with open(over) as fh:
        next(fh)
        for line in fh:
            f = line.rstrip('\n').split('\t')
            if float(f[2]) > 0:
                pages[f[0]].append((int(f[1]), int(f[2])))

    paths = {}
    with open(manifest) as fh:
        next(fh)
        for line in fh:
            f = line.rstrip('\n').split('\t')
            base = os.path.splitext(os.path.basename(f[2]))[0]
            paths[f"{base}__{f[3]}"] = os.path.join(root, f[2])

    print('\t'.join(['document', 'pages', 'over_px', 'media', 'emf_clip_recs',
                     'wmf_clip_recs', 'has_vector_media']))
    for name in sorted(pages, key=lambda k: -sum(p for _, p in pages[k])):
        px = sum(p for _, p in pages[name])
        src = paths.get(name)
        media, emf, wmf, vec = [], 0, 0, False
        if src and zipfile.is_zipfile(src):
            with zipfile.ZipFile(src) as z:
                for m in z.namelist():
                    if '/media/' not in m:
                        continue
                    ext = os.path.splitext(m)[1].lower().lstrip('.')
                    media.append(ext)
                    if ext in ('emf', 'wmf'):
                        vec = True
                        blob = z.read(m)
                        c, ok = emf_clip_records(blob)
                        if ok:
                            emf += sum(c.values())
                        else:
                            c, ok = wmf_clip_records(blob)
                            wmf += sum(c.values())
        elif src:
            media.append('(not a zip)')
        print('\t'.join([name, str(len(pages[name])), str(px),
                         ','.join(sorted(set(media))) or '-', str(emf), str(wmf),
                         'yes' if vec else 'no']))

main()
