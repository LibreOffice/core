#!/usr/bin/env python3
"""Every Escher property of every WordArt shape in a .ppt, decoded.

Round 103's `wordart.py` censused the two text-path shapes in the 51-document column and
printed four of their fill properties. This prints *all* of them, plus the decoded gtext
booleans that `filter/source/msfilter/msdffimp.cxx`:4423-4530 and :2516-2570 read, plus the
shape's client anchor -- everything the .ppt Fontwork reader has to consume.

    wordart-props.py <file.ppt>...
"""
import struct, sys
import olefile

SP = 0xF00A
OPT = 0xF00B
SPC = 0xF004
ANCHOR = 0xF010          # msofbtClientAnchor
CHILD = 0xF00F           # msofbtChildAnchor
TEXT_PATH = range(136, 176)

NAMES = {
    127: 'LockAgainstGrouping', 128: 'wzName?', 191: 'fFitTextToShape group',
    192: 'gtextUNICODE', 193: 'gtextRTF', 194: 'gtextAlign', 195: 'gtextSize',
    196: 'gtextSpacing', 197: 'gtextFont', 255: 'gtext boolean group',
    324: 'geoRight?', 325: 'pVertices', 326: 'pSegmentInfo', 327: 'adjustValue',
    328: 'adjust2Value', 329: 'adjust3Value', 330: 'adjust4Value',
    331: 'adjust5Value', 332: 'adjust6Value', 333: 'adjust7Value',
    334: 'adjust8Value', 335: 'adjust9Value', 336: 'adjust10Value',
    384: 'fillType', 385: 'fillColor', 386: 'fillOpacity', 387: 'fillBackColor',
    388: 'fillBackOpacity', 390: 'fillBlip', 393: 'fillWidth', 394: 'fillHeight',
    395: 'fillAngle', 396: 'fillFocus', 397: 'fillToLeft', 398: 'fillToTop',
    399: 'fillToRight', 400: 'fillToBottom', 407: 'fillShadeColors',
    447: 'fill boolean group', 448: 'lineColor', 511: 'line boolean group',
}

# masks msdffimp.cxx reads off property 255
GTEXT_BITS = [
    (0x4000, 'fGtext (text path on)'),
    (0x2000, 'vertical writing'),
    (0x1000, 'kerning'),
    (0x0400, 'gtextFStretch (fit shape)'),
    (0x0100, 'fit path'),
    (0x0080, 'SameLetterHeights'),
    (0x0040, 'ScaleX'),
    (0x0020, 'bold'),
    (0x0010, 'italic'),
]


def walk(s, a, b, path, out):
    p = a
    while p + 8 <= b:
        vi, rt, rl = struct.unpack_from('<HHI', s, p)
        out.append((rt, vi >> 4, vi & 0xF, p, rl, tuple(path)))
        if (vi & 0x0F) == 0x0F and p + 8 + rl <= b:
            walk(s, p + 8, p + 8 + rl, path + [(rt, p)], out)
        p += 8 + rl


def properties(s, p, count):
    """The table: id -> (value, is_complex, data). Complex block starts at 6*count."""
    base = p + 8
    entries = []
    for i in range(count):
        q = base + i * 6
        if q + 6 > len(s):
            break
        pid, val = struct.unpack_from('<HI', s, q)
        entries.append((pid & 0x3FFF, val, (pid >> 15) & 1))
    complex_at = base + 6 * len(entries)
    props = {}
    for pid, val, cx in entries:
        data = b''
        if cx and val:
            data = s[complex_at:complex_at + val]
            complex_at += val
        props[pid] = (val, cx, data)
    return props


def main(paths):
    for path in paths:
        ole = olefile.OleFileIO(path)
        s = ole.openstream('PowerPoint Document').read()
        ole.close()
        out = []
        walk(s, 0, len(s), [], out)

        for rt, inst, ver, p, rl, parents in out:
            if rt != SP or inst not in TEXT_PATH:
                continue
            container = parents[-1] if parents else None
            props = {}
            anchors = []
            if container and container[0] == SPC:
                start = container[1] + 8
                end = start + struct.unpack_from('<I', s, container[1] + 4)[0]
                for r2, i2, v2, p2, l2, _ in out:
                    if not (start <= p2 < end):
                        continue
                    if r2 == OPT:
                        props.update(properties(s, p2, i2))
                    if r2 in (ANCHOR, CHILD):
                        anchors.append((r2, s[p2 + 8:p2 + 8 + l2]))
            spflags = struct.unpack_from('<I', s, p + 12)[0]
            print(f'=== {path.rsplit("/", 1)[-1]}  sp@{p}  type={inst}  '
                  f'spid={struct.unpack_from("<I", s, p + 8)[0]}  flags=0x{spflags:08x}')
            for r2, blob in anchors:
                if r2 == ANCHOR and len(blob) >= 8:
                    t, l, r, b = struct.unpack_from('<4h', blob)
                    print(f'    clientAnchor(8) top={t} left={l} right={r} bottom={b} '
                          f'-> w={r - l} h={b - t}')
                elif r2 == ANCHOR and len(blob) >= 16:
                    print(f'    clientAnchor(16) {struct.unpack_from("<4i", blob)}')
                else:
                    print(f'    anchor 0x{r2:04x} {blob.hex()}')
            for pid in sorted(props):
                val, cx, data = props[pid]
                name = NAMES.get(pid, '')
                extra = ''
                if cx:
                    try:
                        txt = data.decode('utf-16-le').rstrip('\x00')
                        if txt.isprintable():
                            extra = f'  text={txt!r}'
                    except Exception:
                        pass
                    if not extra:
                        extra = f'  data={data[:48].hex()}'
                print(f'    {pid:5d} {name:24s} = {val:11d} (0x{val:08x}){" cx" if cx else ""}{extra}')
            if 255 in props:
                v = props[255][0]
                on = [n for m, n in GTEXT_BITS if v & m]
                stated = [n for m, n in GTEXT_BITS if (v >> 16) & m]
                print(f'    prop255 value bits: {on}')
                print(f'    prop255 stated bits: {stated}')


if __name__ == '__main__':
    main(sys.argv[1:])
