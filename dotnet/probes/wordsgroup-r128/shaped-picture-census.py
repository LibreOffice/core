#!/usr/bin/env python3
"""Every way a words document can put a picture inside a non-rectangular outline.

Three channels, because the words track has four readers and they reach different markup:

  * OOXML `pic:pic` / `wps:wsp` whose own `spPr` states a non-rect `a:prstGeom` or an
    `a:custGeom` and which carries an image (a `pic:blipFill` or an `a:blipFill`);
  * VML -- a `v:oval`, `v:roundrect`, `v:shape` or `v:shapetype` carrying `v:imagedata`
    (a `v:rect` is the rectangle and is not counted);
  * Escher in an OLE2 `.doc` -- an `msofbtSp` whose shape type is not the picture frame
    (75) or the rectangle (1/2) and whose `msofbtOPT` carries `pib` (property 260).

Usage: shaped-picture-census.py <corpus-root>
"""
import collections
import os
import re
import struct
import sys
import xml.etree.ElementTree as ET
import zipfile

A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
VML = 'urn:schemas-microsoft-com:vml'

PICTURE_FRAME = 75
RECTANGLES = (0, 1, 2, 20)          # notPrimitive, rectangle, roundRectangle, line
DFF_BLIP = 260                       # DFF_Prop_pib


def ooxml(path):
    """(shaped pictures, preset counter) over every part that can hold a drawing."""
    presets = collections.Counter()
    vml = 0
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return presets, vml
    with z:
        for name in z.namelist():
            if not (name.startswith('word/') and name.endswith('.xml')):
                continue
            try:
                blob = z.read(name)
                root = ET.fromstring(blob)
            except Exception:
                continue
            for shape in root.iter():
                local = shape.tag.split('}')[-1]
                if local not in ('pic', 'wsp', 'sp'):
                    continue
                sp = next((c for c in shape if c.tag.split('}')[-1] == 'spPr'), None)
                if sp is None:
                    continue
                has_image = any(c.tag.split('}')[-1] == 'blipFill' for c in shape) \
                    or sp.find(f'{{{A}}}blipFill') is not None
                if not has_image:
                    continue
                cust = sp.find(f'{{{A}}}custGeom')
                prst = sp.find(f'{{{A}}}prstGeom')
                if cust is not None:
                    presets['(custGeom)'] += 1
                elif prst is not None and (prst.get('prst') or 'rect') != 'rect':
                    presets[prst.get('prst')] += 1
            for shape in root.iter():
                local = shape.tag.split('}')[-1]
                if local not in ('oval', 'roundrect', 'shape', 'polyline'):
                    continue
                if not shape.tag.startswith(f'{{{VML}}}'):
                    continue
                if any(c.tag.split('}')[-1] == 'imagedata' for c in shape):
                    vml += 1
    return presets, vml


def escher(path):
    """Shaped Escher pictures in an OLE2 word document, counted over the raw bytes.

    The container is walked as a flat record stream rather than through the OLE2
    directory: an `msofbtSp` (0xF00A) is followed by its `msofbtOPT` (0xF00B) inside the
    same `msofbtSpContainer`, so a linear scan pairs them without a reader.
    """
    try:
        data = open(path, 'rb').read()
    except Exception:
        return 0
    shaped = 0
    i = 0
    pending = None
    n = len(data)
    while i + 8 <= n:
        ver_inst, kind, size = struct.unpack_from('<HHI', data, i)
        if kind == 0xF00A and size == 8 and i + 16 <= n:
            shape_type = ver_inst >> 4
            pending = shape_type
            i += 8 + size
            continue
        if kind == 0xF00B and pending is not None:
            count = ver_inst >> 4
            end = i + 8 + size
            if end <= n and count * 6 <= size:
                blip = False
                for k in range(count):
                    pid, = struct.unpack_from('<H', data, i + 8 + k * 6)
                    if (pid & 0x3FFF) == DFF_BLIP:
                        blip = True
                if blip and pending not in RECTANGLES and pending != PICTURE_FRAME:
                    shaped += 1
            pending = None
            i = end if end > i else i + 8
            continue
        if size <= 0 or (kind & 0xF000) != 0xF000:
            i += 1
            continue
        i += 8 + (size if (ver_inst & 0x0F) == 0x0F else size)
    return shaped


def main():
    root = sys.argv[1]
    print('shaped\tvml\tescher\text\tdocument\tpresets')
    total = collections.Counter()
    docs = 0
    for base, _dirs, files in os.walk(root):
        for f in sorted(files):
            low = f.lower()
            ext = low.rsplit('.', 1)[-1] if '.' in low else ''
            p = os.path.join(base, f)
            if ext in ('docx', 'docm', 'dotx', 'dotm'):
                presets, vml = ooxml(p)
                n = sum(presets.values())
                if n or vml:
                    docs += 1
                    total.update(presets)
                    print(f'{n}\t{vml}\t0\t{ext}\t{f}\t' +
                          ','.join(f'{k}:{v}' for k, v in presets.most_common()))
            elif ext in ('doc', 'dot'):
                n = escher(p)
                if n:
                    docs += 1
                    print(f'0\t0\t{n}\t{ext}\t{f}\t')
    print(f'# documents: {docs}')
    print('# ooxml presets: ' + '  '.join(f'{k}={v}' for k, v in total.most_common()))


main()
