#!/usr/bin/env python3
"""Rewrite a `.doc`'s `sprmCCharScale` operand in place, so the OUT-OF-RANGE arms can be measured.

`--convert-to doc` cannot author them: the RTF *import* has already replaced anything outside
1..600 with 100 (`DomainMapper.cxx`:2485), so the converted file states 100 and an arm built that
way measures the importer twice rather than the WW8 reader once. This patches the two operand
bytes of the one CHPX in the probe and leaves every other byte of the file alone.

    patch-chpx.py <src.doc> <dst.doc> <value>
"""
import importlib.util
import shutil
import struct
import sys

import olefile

spec = importlib.util.spec_from_file_location(
    'ww8census', __file__.rsplit('/', 1)[0] + '/census-ww8.py')
ww8 = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ww8)

SPRM = 0x4852


def operand_offsets(wd, tbl, fclcb):
    """Every (offset-into-WordDocument, current value) of a `sprmCCharScale` operand."""
    fc, lcb = fclcb[12]
    plc = tbl[fc:fc + lcb]
    n = (len(plc) - 4) // 8
    pns = [struct.unpack_from('<I', plc, 4 * (n + 1) + 4 * i)[0] & 0x3FFFFF for i in range(n)]
    out = []
    for page in pns:
        base = page * 512
        fkp = wd[base:base + 512]
        if len(fkp) < 512:
            continue
        crun = fkp[511]
        for i in range(crun):
            off = fkp[4 * (crun + 1) + i] * 2
            if off == 0 or off >= 511:
                continue
            cb = fkp[off]
            grpprl = fkp[off + 1:off + 1 + cb]
            at = 0
            while at + 2 <= len(grpprl):
                sprm = struct.unpack_from('<H', grpprl, at)[0]
                at += 2
                size = ww8.operand_size(sprm, grpprl, at)
                if sprm == SPRM and size == 2:
                    out.append((base + off + 1 + at,
                                struct.unpack_from('<H', grpprl, at)[0]))
                at += size
    return out


def main():
    src, dst, value = sys.argv[1], sys.argv[2], int(sys.argv[3])
    shutil.copyfile(src, dst)
    doc = ww8.Doc(dst)
    spots = operand_offsets(doc.wd, doc.tbl, doc.fclcb)
    if len(spots) != 1:
        raise SystemExit('%s: expected one sprmCCharScale, found %d' % (src, len(spots)))
    at, old = spots[0]
    wd = bytearray(doc.wd)
    struct.pack_into('<H', wd, at, value & 0xFFFF)
    ole = olefile.OleFileIO(dst, write_mode=True)
    ole.write_stream('WordDocument', bytes(wd))
    ole.close()
    print('%s -> %s  operand at %d  %d -> %d' % (src, dst, at, old, value))


if __name__ == '__main__':
    main()
