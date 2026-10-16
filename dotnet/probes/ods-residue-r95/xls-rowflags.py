#!/usr/bin/env python3
"""Rewrite a BIFF8 workbook's row-height flags, so the reference has to recompute.

A `.xls` LibreOffice wrote already carries Calc's own row heights, so asking it to
convert that file back tells you nothing about what it would compute. This clears the
two flags that say "these heights are a user's choice" and puts a uniform height on
every row, which is the state a foreign writer leaves:

  * `DEFAULTROWHEIGHT` (0x0225) — clear `EXC_DEFROW_UNSYNCED`, bit 0 of its `grbit`
    (`sc/source/filter/inc/xltable.hxx`:114). `XclImpColRowSettings::Convert` marks
    *every row of the sheet* `CRFlags::ManualSize` when it is set
    (`sc/source/filter/excel/colrowst.cxx`:212-215).
  * `ROW` (0x0208) — clear `fUnsynced`, bit 6 of the `grbit` at offset 12, and set the
    height at offset 6 to `--height` twips.

Usage: xls-rowflags.py <in.xls> <out.xls> [--height 255]
"""
import struct
import sys

ROW = 0x0208
DEFAULTROWHEIGHT = 0x0225
CONTINUE = 0x003C


def rewrite(data, height):
    out = bytearray(data)
    off = 0
    n = len(data)
    rows = 0
    defaults = 0
    while off + 4 <= n:
        rid, size = struct.unpack_from('<HH', data, off)
        body = off + 4
        if body + size > n:
            break
        if rid == ROW and size >= 16:
            struct.pack_into('<H', out, body + 6, height)
            grbit = struct.unpack_from('<H', data, body + 12)[0]
            struct.pack_into('<H', out, body + 12, grbit & ~0x0040)
            rows += 1
        elif rid == DEFAULTROWHEIGHT and size >= 4:
            grbit = struct.unpack_from('<H', data, body)[0]
            struct.pack_into('<H', out, body, grbit & ~0x0001)
            struct.pack_into('<H', out, body + 2, height)
            defaults += 1
        off = body + size
    return bytes(out), rows, defaults


def main():
    src, dst = sys.argv[1], sys.argv[2]
    height = 255
    if '--height' in sys.argv:
        height = int(sys.argv[sys.argv.index('--height') + 1])

    import olefile
    with open(src, 'rb') as fh:
        raw = fh.read()
    ole = olefile.OleFileIO(src)
    name = 'Workbook' if ole.exists('Workbook') else 'Book'
    stream = ole.openstream(name).read()
    ole.close()

    patched, rows, defaults = rewrite(stream, height)
    assert len(patched) == len(stream)

    # The stream sits verbatim inside the container at some offset; find and replace it.
    at = raw.find(stream)
    if at < 0:
        raise SystemExit('the workbook stream is not contiguous in the container')
    with open(dst, 'wb') as fh:
        fh.write(raw[:at] + patched + raw[at + len(stream):])
    print(f'{rows} ROW records, {defaults} DEFAULTROWHEIGHT, height {height} twips',
          file=sys.stderr)


if __name__ == '__main__':
    main()
