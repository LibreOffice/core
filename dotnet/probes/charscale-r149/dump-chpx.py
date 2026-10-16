#!/usr/bin/env python3
"""Dump every CHPX sprm of a `.doc`, so a probe's `sprmCCharScale` is VERIFIED to be in the file
before anything is measured from it.

`--convert-to doc` is the only route to an authored WW8 file here, and a conversion that silently
drops the sprm would make the measurement below read as agreement where it is an absence. This is
that guard; it reuses `probes/charscale-r145/census-ww8.py`'s FIB / piece-table / FKP walk.
"""
import struct
import sys

sys.path.insert(0, __file__.rsplit('/', 1)[0])
from importlib import import_module

census = import_module('census-ww8'.replace('-', '_')) if False else None

import importlib.util
spec = importlib.util.spec_from_file_location('ww8census', __file__.rsplit('/', 1)[0] + '/census-ww8.py')
ww8 = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ww8)

SPRM = 0x4852


def main():
    for path in sys.argv[1:]:
        doc = ww8.Doc(path)
        hits = []
        for fcs, fce, grpprl in doc.chpxs():
            for sprm, operand in ww8.sprms_of(grpprl):
                if sprm == SPRM:
                    v = struct.unpack('<H', operand[:2])[0] if len(operand) >= 2 else None
                    hits.append((fcs, fce, v, len(operand)))
        styles = 0
        for _istd, _name, grpprl in doc.style_chpx():
            for sprm, operand in ww8.sprms_of(grpprl):
                if sprm == SPRM:
                    styles += 1
        print('%-40s chpx %3d   sprmCCharScale %s   style hits %d'
              % (path.rsplit('/', 1)[-1], len(doc.chpxs()),
                 ' '.join('fc%d..%d=%s(%db)' % h for h in hits) or 'NONE', styles))


if __name__ == '__main__':
    main()
