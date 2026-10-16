#!/usr/bin/env python3
"""Dump the WW8 CHPX that applies at each paragraph mark of a .doc.

WHY this exists. A paragraph with no text still has a height, and in WW8 that height comes from
the character properties of the paragraph *mark* — the U+000D that ends it. Nothing in a rendered
PDF shows it, and LibreOffice's flat-ODF export shows only the value it arrived at, not where it
came from. This reads the file itself: for every paragraph it prints the CP of the mark, whether an
FKP CHPX covers that CP, and the sprms in it, so "does the mark carry sprmCHps" is answered from
the bytes rather than inferred.

    chpx.py <file.doc> [first-paragraph] [count]
"""
import sys
import struct
import olefile

SPRM_CHPS = 0x4A43      # font size, half-points
SPRM_CISTD = 0x4A30     # character style istd
SPRM_CRGFTC0 = 0x4A4F   # font index, ascii


def operand_size(sprm, data, at):
    spra = (sprm >> 13) & 7
    if spra in (0, 1):
        return 1
    if spra in (2, 4, 5):
        return 2
    if spra == 3:
        return 4
    if spra == 7:
        return 3
    # spra 6: variable, length byte first (two sprms carry a two-byte length)
    if sprm in (0xD608, 0xC615):
        return 1 + data[at]
    return 1 + data[at]


def sprms_of(grpprl):
    out = []
    at = 0
    while at + 2 <= len(grpprl):
        sprm = struct.unpack_from('<H', grpprl, at)[0]
        at += 2
        size = operand_size(sprm, grpprl, at)
        operand = grpprl[at:at + size]
        at += size
        out.append((sprm, operand))
    return out


class Doc:
    def __init__(self, path):
        ole = olefile.OleFileIO(path)
        self.wd = ole.openstream('WordDocument').read()
        base = struct.unpack_from('<H', self.wd, 10)[0]
        table = '1Table' if (base >> 9) & 1 else '0Table'
        self.tbl = ole.openstream(table).read()

        at = 32
        csw = struct.unpack_from('<H', self.wd, at)[0]
        at += 2 + csw * 2
        cslw = struct.unpack_from('<H', self.wd, at)[0]
        at += 2
        self.rglw = [struct.unpack_from('<i', self.wd, at + 4 * i)[0] for i in range(cslw)]
        at += cslw * 4
        cbrgfclcb = struct.unpack_from('<H', self.wd, at)[0]
        at += 2
        self.fclcb = [struct.unpack_from('<II', self.wd, at + 8 * i) for i in range(cbrgfclcb)]

        self.ccp_text = self.rglw[3]
        self.pieces = self._pieces()
        self.chpx_pages = self._bte(12)

    def _pieces(self):
        fc, lcb = self.fclcb[33]
        clx = self.tbl[fc:fc + lcb]
        at = 0
        while at < len(clx):
            if clx[at] == 1:
                cb = struct.unpack_from('<H', clx, at + 1)[0]
                at += 3 + cb
                continue
            if clx[at] == 2:
                cb = struct.unpack_from('<I', clx, at + 1)[0]
                plc = clx[at + 5:at + 5 + cb]
                n = (len(plc) - 4) // 12
                cps = [struct.unpack_from('<I', plc, 4 * i)[0] for i in range(n + 1)]
                pieces = []
                for i in range(n):
                    off = 4 * (n + 1) + 8 * i
                    pfc = struct.unpack_from('<I', plc, off + 2)[0]
                    compressed = bool(pfc & 0x40000000)
                    real = (pfc & 0x3FFFFFFF) // 2 if compressed else (pfc & 0x3FFFFFFF)
                    pieces.append((cps[i], cps[i + 1], real, compressed))
                return pieces
            raise ValueError(f'unexpected CLX byte {clx[at]:#x}')
        raise ValueError('no Pcdt in CLX')

    def _bte(self, index):
        """The PlcfBte at fibRgFcLcb index: (fc boundaries, fkp page numbers)."""
        fc, lcb = self.fclcb[index]
        plc = self.tbl[fc:fc + lcb]
        n = (len(plc) - 4) // 8
        fcs = [struct.unpack_from('<I', plc, 4 * i)[0] for i in range(n + 1)]
        pns = [struct.unpack_from('<I', plc, 4 * (n + 1) + 4 * i)[0] & 0x3FFFFF for i in range(n)]
        return fcs, pns

    def fc_of(self, cp):
        for start, end, fc, compressed in self.pieces:
            if start <= cp < end:
                return fc + (cp - start) * (1 if compressed else 2), compressed
        return None, None

    def text(self):
        out = []
        for start, end, fc, compressed in self.pieces:
            n = end - start
            if compressed:
                raw = self.wd[fc:fc + n]
                out.append(raw.decode('cp1252', 'replace'))
            else:
                raw = self.wd[fc:fc + 2 * n]
                out.append(raw.decode('utf-16-le', 'replace'))
        return ''.join(out)

    def chpx_at(self, cp):
        fc, _ = self.fc_of(cp)
        if fc is None:
            return None
        fcs, pns = self.chpx_pages
        page = None
        for i in range(len(pns)):
            if fcs[i] <= fc < fcs[i + 1]:
                page = pns[i]
                break
        if page is None:
            return None
        fkp = self.wd[page * 512:(page + 1) * 512]
        crun = fkp[511]
        rgfc = [struct.unpack_from('<I', fkp, 4 * i)[0] for i in range(crun + 1)]
        for i in range(crun):
            if rgfc[i] <= fc < rgfc[i + 1]:
                off = fkp[4 * (crun + 1) + i] * 2
                if off == 0:
                    return b''
                cb = fkp[off]
                return fkp[off + 1:off + 1 + cb]
        return None


def main():
    doc = Doc(sys.argv[1])
    first = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    count = int(sys.argv[3]) if len(sys.argv) > 3 else 40

    text = doc.text()[:doc.ccp_text]
    marks = [i for i, c in enumerate(text) if c == '\r']
    start = 0
    for index, mark in enumerate(marks):
        body = text[start:mark]
        start = mark + 1
        if index < first or index >= first + count:
            continue
        grpprl = doc.chpx_at(mark)
        if grpprl is None:
            shown = 'no FKP'
        elif grpprl == b'':
            shown = '(empty CHPX)'
        else:
            parts = []
            for sprm, operand in sprms_of(grpprl):
                name = {SPRM_CHPS: 'CHps', SPRM_CISTD: 'CIstd',
                        SPRM_CRGFTC0: 'CRgFtc0'}.get(sprm, f'{sprm:#06x}')
                parts.append(f'{name}={operand.hex()}')
            shown = ' '.join(parts)
        preview = body[:34].replace('\x07', '<cell>').replace('\x0c', '<brk>')
        print(f'  p{index:4} cp={mark:6} len={len(body):3} | {preview!r:38} {shown}')


main()
