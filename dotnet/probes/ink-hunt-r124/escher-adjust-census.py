#!/usr/bin/env python3
"""Escher shapes stating an adjustment this tree throws away.

    escher-adjust-census.py <rows.tsv> <corpus-root> <out.tsv>

`PptShapeGeometry.Adjustment` (`src/XRay.Office.Presentations/MsBinary/PptShapeGeometry.cs`:334-337)
translates `DFF_Prop_adjustValue` for `roundRect` and `triangle` and answers null for every other
preset, so any other shape is drawn at the DrawingML preset's own default `adj` however the file
states it. This counts the shapes that state one, by preset, over every OLE2 document in the
passing set -- `.ppt`, `.doc` and `.xls` alike, because Escher is the same in all three.

This is a census of what the files DECLARE and is therefore an upper bound on reach: a stated
adjustment only costs ink where it differs from the preset's default and the shape is drawn.
The default is in the last column so the two can be told apart.
"""
import collections
import pathlib
import struct
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / 'slides-ink-r94'))
from ole import read_ole                              # noqa: E402
from esch import walk                                 # noqa: E402

TRANSLATED = {2, 5}          # roundRect (2) and isoceles triangle (5): the two `Adjustment` maps
ADJUST = 327

rowsfile, root, out = sys.argv[1:4]
root = pathlib.Path(root)
bytype = collections.Counter()
bytype_docs = collections.defaultdict(set)

with open(out, 'w', encoding='utf-8') as fh:
    fh.write('path\text\tshapes_with_adjust\tuntranslated\ttypes\n')
    for line in pathlib.Path(rowsfile).read_text(encoding='utf-8').splitlines():
        p = line.split('\t')
        if len(p) < 7 or p[6] != 'match' or p[1] not in ('ppt', 'doc', 'xls'):
            continue
        try:
            entries, get = read_ole(root / p[0])
        except Exception:                              # noqa: BLE001
            continue
        total = untranslated = 0
        kinds = collections.Counter()
        for entry in entries:
            name = entry[0] if isinstance(entry, (list, tuple)) else entry
            if not isinstance(name, str) or not name or name.startswith('\x05'):
                continue
            try:
                buf = get(name)
            except Exception:                          # noqa: BLE001
                continue
            if not buf or b'\x0b\xf0' not in buf:
                continue
            recs = list(walk(buf))
            pending = None
            for depth, off, rt, inst, ver, ln, body in recs:
                if rt == 0xF00A:
                    pending = inst
                elif rt == 0xF00B and pending is not None:
                    for k in range(inst):
                        try:
                            pid, _ = struct.unpack_from('<HI', buf, body + 6 * k)
                        except struct.error:
                            break
                        if (pid & 0x3FFF) == ADJUST:
                            total += 1
                            if pending not in TRANSLATED:
                                untranslated += 1
                                kinds[pending] += 1
                                bytype[pending] += 1
                                bytype_docs[pending].add(p[0])
                            break
                    pending = None
        if total:
            fh.write(f'{p[0]}\t{p[1]}\t{total}\t{untranslated}\t'
                     f'{",".join(f"{k}x{v}" for k, v in kinds.most_common())}\n')

print('shape types whose stated adjustment is discarded, by MSO shape type:')
for k, v in bytype.most_common(20):
    print(f'  type {k:3d}: {v:5d} shapes in {len(bytype_docs[k])} documents')
print(f'TOTAL {sum(bytype.values())} shapes in '
      f'{len(set().union(*bytype_docs.values())) if bytype_docs else 0} documents')
