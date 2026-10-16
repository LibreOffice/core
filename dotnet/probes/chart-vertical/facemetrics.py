#!/usr/bin/env python3
"""A face's line metrics, resolved the way `LineSpacing.Resolve` does.

`hhea` first when its signs are plausible; `OS/2`'s win metrics only when `hhea` gave
nothing; the typographic metrics over either when `fsSelection` bit 7 asks for them. The
precedence is the one the tree implements, restated here so a probe can predict a pitch
without running our renderer at all.
"""

import struct


class Metrics:
    def __init__(self, path: str):
        self.path = path
        data = open(path, 'rb').read()
        self.data = data
        offset = 0
        if data[:4] == b'ttcf':
            offset = struct.unpack('>I', data[12:16])[0]
        count = struct.unpack('>H', data[offset + 4:offset + 6])[0]
        self.tables = {}
        for i in range(count):
            at = offset + 12 + 16 * i
            tag = data[at:at + 4].decode('latin1')
            start, length = struct.unpack('>II', data[at + 8:at + 16])
            self.tables[tag] = (start, length)

        head = self.tables['head'][0]
        self.upem = struct.unpack('>H', data[head + 18:head + 20])[0]

        hhea = self.tables['hhea'][0]
        h_asc, h_desc, h_gap = struct.unpack('>hhh', data[hhea + 4:hhea + 10])

        typo_asc = typo_desc = typo_gap = 0
        win_asc = win_desc = 0
        fs_selection = 0
        if 'OS/2' in self.tables:
            os2 = self.tables['OS/2'][0]
            typo_asc, typo_desc, typo_gap = struct.unpack('>hhh', data[os2 + 68:os2 + 74])
            win_asc, win_desc = struct.unpack('>HH', data[os2 + 74:os2 + 78])
            fs_selection = struct.unpack('>H', data[os2 + 62:os2 + 64])[0]

        # hhea first, when its signs make sense.
        plausible = h_asc >= 0 and h_desc <= 0 and (h_asc != 0 or h_desc != 0)
        if plausible:
            self.ascent, self.descent, self.gap = h_asc, -h_desc, max(0, h_gap)
            self.source = 'hhea'
        elif win_asc or win_desc:
            self.ascent, self.descent, self.gap = win_asc, win_desc, 0
            self.source = 'os2win'
        else:
            self.ascent = int(self.upem * 0.8)
            self.descent = self.upem - self.ascent
            self.gap = 0
            self.source = 'fallback'

        if (fs_selection & 0x80) and (typo_asc or typo_desc):
            self.ascent, self.descent, self.gap = typo_asc, -typo_desc, max(0, typo_gap)
            self.source = 'os2typo'


MM100_PER_INCH = 2540.0


def chart_pixels(size_pt: float, dpi: int = 96) -> int:
    """The whole number of device pixels the chart device sets the em at.

    Through the device's own map unit, hundredths of a millimetre, which is what
    `MetricGrid.ToPixels` does: `round(mm100 / (2540/dpi))`.
    """
    mm100 = round(size_pt * MM100_PER_INCH / 72.0)
    return int(_away(mm100 / (MM100_PER_INCH / dpi)))


def _away(value: float) -> int:
    """Round half away from zero, as C++ `round` and the tree's `MidpointRounding` do."""
    return int(value + 0.5) if value >= 0 else -int(-value + 0.5)


def to_length_pt(pixels: int, dpi: int = 96) -> float:
    """Whole device pixels back in points, through whole hundredths of a millimetre."""
    return _away(pixels * MM100_PER_INCH / dpi) * 72.0 / MM100_PER_INCH


def predicted(face: Metrics, size_pt: float, dpi: int = 96) -> dict:
    """The chart device's ascent, descent and line height at a size, in points.

    `EditHeightOn`: the taller of converting each metric on its own and converting their
    sum in one step, and no external leading — a chart's text is an EditEngine text.
    """
    hpx = chart_pixels(size_pt, dpi)
    asc_px = _away(face.ascent * hpx / face.upem)
    desc_px = _away(face.descent * hpx / face.upem)
    gap_px = _away(face.gap * hpx / face.upem)
    separately = to_length_pt(asc_px, dpi) + to_length_pt(desc_px, dpi)
    together = to_length_pt(asc_px + desc_px, dpi)
    return {
        'hpx': hpx,
        'ascent': to_length_pt(asc_px, dpi),
        'descent': to_length_pt(desc_px, dpi),
        'leading': to_length_pt(gap_px, dpi),
        'height': max(separately, together),
    }


def exact(face: Metrics, size_pt: float) -> dict:
    """The same three, scaled exactly — what a path with no device answers."""
    return {
        'hpx': 0,
        'ascent': face.ascent * size_pt / face.upem,
        'descent': face.descent * size_pt / face.upem,
        'leading': face.gap * size_pt / face.upem,
        'height': (face.ascent + face.descent + face.gap) * size_pt / face.upem,
    }


if __name__ == '__main__':
    import sys
    for path in sys.argv[1:]:
        face = Metrics(path)
        print(f'{path}: upem={face.upem} asc={face.ascent} desc={face.descent} '
              f'gap={face.gap} source={face.source}')
        for size in (8, 10, 11, 13, 18):
            p = predicted(face, size)
            e = exact(face, size)
            print(f'   {size:5.1f} pt  hpx={p["hpx"]:3d}  height {p["height"]:7.3f} '
                  f'(exact {e["height"]:7.3f})  ascent {p["ascent"]:7.3f} '
                  f'(exact {e["ascent"]:7.3f})')
