#!/usr/bin/env python3
"""Fit ref = ours * a + b per channel over a page, to test the washout formula.

The reference applies LibreOffice's ColorMode_WATERMARK to a blip whose <a:lum> states
bright="70000" contrast="-70000"; we draw the picture untouched. If that mode is
BColorModifier_RGBLuminanceContrast at luminance +0.5 and contrast -0.7, then every pixel
of the picture satisfies

    out = clamp(in * 0.30546875 + 0.848627451, 0, 1)

so a least-squares fit of one render against the other over the unclamped range recovers
those two constants. Fitting rather than asserting is the point: the source in this
checkout is a 27.2 alpha and the binary that made the reference is 24.2.7.2.
"""
import struct
import sys
import zlib


def read_png(path):
    data = open(path, 'rb').read()
    pos, width, height, idat = 8, 0, 0, b''
    while pos < len(data):
        length = struct.unpack('>I', data[pos:pos + 4])[0]
        kind = data[pos + 4:pos + 8]
        body = data[pos + 8:pos + 8 + length]
        if kind == b'IHDR':
            width, height, depth, colour = struct.unpack('>IIBB', body[:10])
            assert depth == 8 and colour in (2, 6), (depth, colour)
            channels = 3 if colour == 2 else 4
        elif kind == b'IDAT':
            idat += body
        elif kind == b'IEND':
            break
        pos += length + 12

    raw = zlib.decompress(idat)
    stride = width * channels
    rows, previous, at = [], bytearray(stride), 0
    for _ in range(height):
        filt = raw[at]; at += 1
        line = bytearray(raw[at:at + stride]); at += stride
        for i in range(stride):
            a = line[i - channels] if i >= channels else 0
            b = previous[i]
            c = previous[i - channels] if i >= channels else 0
            if filt == 1: line[i] = (line[i] + a) & 0xFF
            elif filt == 2: line[i] = (line[i] + b) & 0xFF
            elif filt == 3: line[i] = (line[i] + ((a + b) >> 1)) & 0xFF
            elif filt == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[i] = (line[i] + pr) & 0xFF
        rows.append(bytes(line))
        previous = line
    return width, height, channels, rows


def main():
    ours, ref = sys.argv[1], sys.argv[2]
    w1, h1, c1, r1 = read_png(ours)
    w2, h2, c2, r2 = read_png(ref)
    if (w1, h1) != (w2, h2):
        print(f'sizes differ: {w1}x{h1} vs {w2}x{h2}', file=sys.stderr)
        h1 = min(h1, h2); w1 = min(w1, w2)

    # Sample away from the text, which neither model covers: the top eighth of the page.
    pairs = []
    for y in range(2, h1 // 8):
        for x in range(2, w1 - 2):
            a = r1[y][x * c1:x * c1 + 3]
            b = r2[y][x * c2:x * c2 + 3]
            for k in range(3):
                pairs.append((a[k] / 255.0, b[k] / 255.0))

    unclamped = [(i, o) for i, o in pairs if 0.02 < o < 0.98]
    n = len(unclamped)
    if n < 100:
        print('not enough unclamped samples', file=sys.stderr)
        return 2
    sx = sum(i for i, _ in unclamped)
    sy = sum(o for _, o in unclamped)
    sxx = sum(i * i for i, _ in unclamped)
    sxy = sum(i * o for i, o in unclamped)
    slope = (n * sxy - sx * sy) / (n * sxx - sx * sx)
    offset = (sy - slope * sx) / n
    residual = max(abs(o - min(max(i * slope + offset, 0.0), 1.0)) for i, o in unclamped)

    print(f'{n} unclamped channel samples')
    print(f'fitted   out = in * {slope:.5f} + {offset:.5f}   worst residual {residual:.4f}')
    print('expected out = in * 0.30547 + 0.84863   (luminance +0.5, contrast -0.7)')
    return 0


if __name__ == '__main__':
    sys.exit(main())
