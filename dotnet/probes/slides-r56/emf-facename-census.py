#!/usr/bin/env python3
"""How many metafiles name a face whose 64-byte field carries rubbish past the terminator?

`EmfReader.CreateFont` reads `lfFaceName` as 32 UTF-16 code units and **skips** the NULs
instead of stopping at the first one, on the reasoning that the field is NUL-*padded* when the
name fills it exactly.  It usually is.  When it is not — and GDI leaves whatever was on the
stack in the tail — the family becomes the real name with a dozen junk code points welded on,
which no substitution table recognises, so it falls through to the generic sans.

That was invisible until round 55 taught this reader to synthesise an oblique: a wrong face and
a right face both drew upright, and the wrong one only started to lean.

Counts, per document, the `EMR_EXTCREATEFONTINDIRECTW` (82) records whose face-name field has a
non-zero code unit *after* its first NUL, and names what the two readings give.

What it CANNOT see: WMF's `META_CREATEFONTINDIRECT`, whose face name is a 32-BYTE field with the
same hazard and the same reader habit; EMF+ `DrawString` fonts; and metafiles reached through an
OLE object rather than as a package entry.

    emf-facename-census.py <corpus-root> [family ...]
"""
import collections, os, struct, sys, zipfile

CREATEFONT = 82


def fonts(data):
    off, out = 0, []
    while off + 8 <= len(data):
        try:
            kind, size = struct.unpack_from("<II", data, off)
        except struct.error:
            return out
        if size < 8 or off + size > len(data):
            return out
        if kind == CREATEFONT and off + 12 + 92 <= len(data):
            raw = data[off + 12 + 28: off + 12 + 92]
            units = [raw[i] | (raw[i + 1] << 8) for i in range(0, 64, 2)]
            stop = "".join(chr(u) for u in units).split("\x00")[0]
            skip = "".join(chr(u) for u in units if u != 0)
            out.append((stop, skip))
        off += size
    return out


def entries(path):
    if path.lower().endswith((".emf", ".wmf")):
        yield path, open(path, "rb").read()
        return
    try:
        with zipfile.ZipFile(path) as z:
            for name in z.namelist():
                if name.lower().endswith(".emf"):
                    yield name, z.read(name)
    except Exception:
        return


if __name__ == "__main__":
    root = sys.argv[1]
    fams = sys.argv[2:] or ["slides", "sheets", "words"]
    rows = []
    with open(os.path.join(root, "MANIFEST.tsv"), encoding="utf-8") as fh:
        hdr = fh.readline().rstrip("\n").split("\t")
        for line in fh:
            r = dict(zip(hdr, line.rstrip("\n").split("\t")))
            if r["family"] in fams:
                rows.append(r)

    per = collections.Counter()
    docs = collections.Counter()
    names = collections.Counter()
    for r in rows:
        bad = 0
        for _name, data in entries(os.path.join(root, r["path"])):
            for stop, skip in fonts(data):
                if stop != skip:
                    bad += 1
                    names[stop] += 1
        if bad:
            docs[r["family"]] += 1
            per[r["family"]] += bad
            print(f"{r['family']:7} {bad:5} records  {r['path']}")
    print("---")
    for f in fams:
        print(f"{f}: {docs[f]} documents, {per[f]} font records")
    print("faces:", names.most_common(12))
