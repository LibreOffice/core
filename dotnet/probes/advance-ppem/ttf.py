"""Minimal TrueType/OpenType reader: unitsPerEm, cmap and hmtx advances.

fontTools is not installed in this container and the three tables needed here are
small, so they are read directly. Enough for `advance` of a Latin character.
"""

import struct


class Face:
    def __init__(self, path: str):
        self.path = path
        with open(path, 'rb') as handle:
            self.data = handle.read()
        tag, num = struct.unpack('>IH', self.data[:6])
        if tag == 0x74746366:  # 'ttcf'
            offset = struct.unpack('>I', self.data[12:16])[0]
            num = struct.unpack('>H', self.data[offset + 4:offset + 6])[0]
            base = offset + 12
        else:
            base = 12
        self.tables = {}
        for i in range(num):
            entry = self.data[base + 16 * i:base + 16 * i + 16]
            name = entry[:4].decode('latin1')
            off, length = struct.unpack('>II', entry[8:16])
            self.tables[name] = (off, length)

        head = self.tables['head'][0]
        self.upem = struct.unpack('>H', self.data[head + 18:head + 20])[0]
        self.index_to_loc = struct.unpack('>h', self.data[head + 50:head + 52])[0]
        hhea = self.tables['hhea'][0]
        self.num_h_metrics = struct.unpack('>H', self.data[hhea + 34:hhea + 36])[0]
        self._cmap = self._read_cmap()

    def _read_cmap(self) -> dict:
        off = self.tables['cmap'][0]
        count = struct.unpack('>H', self.data[off + 2:off + 4])[0]
        best = None
        for i in range(count):
            pid, eid, sub = struct.unpack('>HHI', self.data[off + 4 + 8 * i:off + 12 + 8 * i])
            if (pid, eid) in ((3, 10), (3, 1), (0, 3), (0, 4), (0, 6)):
                fmt = struct.unpack('>H', self.data[off + sub:off + sub + 2])[0]
                if fmt in (4, 12):
                    best = (off + sub, fmt)
                    if (pid, eid) == (3, 1) and fmt == 4:
                        break
        if best is None:
            raise SystemExit(f'no usable cmap in {self.path}')
        base, fmt = best
        mapping: dict[int, int] = {}
        if fmt == 4:
            seg2 = struct.unpack('>H', self.data[base + 6:base + 8])[0]
            seg = seg2 // 2
            ends = base + 14
            starts = ends + seg2 + 2
            deltas = starts + seg2
            ranges = deltas + seg2
            for s in range(seg):
                end = struct.unpack('>H', self.data[ends + 2 * s:ends + 2 * s + 2])[0]
                start = struct.unpack('>H', self.data[starts + 2 * s:starts + 2 * s + 2])[0]
                delta = struct.unpack('>h', self.data[deltas + 2 * s:deltas + 2 * s + 2])[0]
                offset = struct.unpack('>H', self.data[ranges + 2 * s:ranges + 2 * s + 2])[0]
                if start == 0xFFFF:
                    continue
                for c in range(start, end + 1):
                    if offset == 0:
                        gid = (c + delta) & 0xFFFF
                    else:
                        at = ranges + 2 * s + offset + 2 * (c - start)
                        gid = struct.unpack('>H', self.data[at:at + 2])[0]
                        if gid:
                            gid = (gid + delta) & 0xFFFF
                    if gid:
                        mapping[c] = gid
        else:
            groups = struct.unpack('>I', self.data[base + 12:base + 16])[0]
            for g in range(groups):
                at = base + 16 + 12 * g
                lo, hi, gid = struct.unpack('>III', self.data[at:at + 12])
                for c in range(lo, min(hi, lo + 0x10000) + 1):
                    mapping[c] = gid + (c - lo)
        return mapping

    def glyph(self, ch: str) -> int:
        gid = self._cmap.get(ord(ch))
        if gid is None:
            raise SystemExit(f'{ch!r} is not in {self.path}')
        return gid

    def advance(self, ch: str) -> int:
        """The character's `hmtx` advance, in font design units."""
        gid = self.glyph(ch)
        off = self.tables['hmtx'][0]
        index = min(gid, self.num_h_metrics - 1)
        return struct.unpack('>H', self.data[off + 4 * index:off + 4 * index + 2])[0]
