#!/usr/bin/env python3
"""The two one-attribute variants of architecture6.ppt that decide the seat.

  var-class00.ppt  the two Helvetica FontEntityAtoms' lfPitchAndFamily 0x22 -> 0x02: the NAME
                   is untouched and only the declared family class (FF_SWISS) is removed.
  var-arial.ppt    those two atoms' name Helvetica -> Arial: the CLASS is untouched and only
                   the name changes, to one whose bare and class-ful fc-match agree.

Both are byte-length-preserving edits of the file's own records; `var-ctrl.ppt` rewrites the
name to itself and must compare equal to the original.
"""
import sys

SRC = '/home/user/sample-files/slides/ceiling-001/ppt/architecture6.ppt'


def name_of(body):
    out = []
    for k in range(0, 64, 2):
        if body[k] == 0 and body[k + 1] == 0:
            break
        out.append(body[k:k + 2])
    return b''.join(out).decode('utf-16-le', errors='replace')


def offsets(data, named):
    """Byte offset of the 64-byte name field of every FontEntityAtom naming `named`."""
    out, i = [], 0
    while True:
        j = data.find(b'\xb7\x0f', i)
        if j < 0:
            return out
        i = j + 1
        if j + 6 + 68 > len(data) or int.from_bytes(data[j + 2:j + 6], 'little') != 68:
            continue
        if name_of(data[j + 6:j + 6 + 68]) == named:
            out.append(j + 6)


def main(outdir):
    src = open(SRC, 'rb').read()
    names = offsets(src, 'Helvetica')
    assert len(names) == 2, names

    for label, name in (('var-arial', 'Arial'), ('var-ctrl', 'Helvetica')):
        d = bytearray(src)
        raw = name.encode('utf-16-le')
        for j in names:
            d[j:j + 64] = raw + b'\x00' * (64 - len(raw))
        open(f'{outdir}/{label}.ppt', 'wb').write(d)

    d = bytearray(src)
    for j in names:
        assert d[j + 67] == 0x22, hex(d[j + 67])
        d[j + 67] &= 0x0F
    open(f'{outdir}/var-class00.ppt', 'wb').write(d)
    print('name-field offsets', names)


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '.')
