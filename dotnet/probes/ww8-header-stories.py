#!/usr/bin/env python3
"""Dump, per section of a Word 97 (.doc) file, the six header-story lengths, their text,
and the grpfIhdt LibreOffice would synthesise from them.

Reads only the FIB, PlcfSed (section count), PlcfHdd (story CPs) and the piece table out
of the OLE2 streams, so it is independent of Paperless entirely.

Story order within a section, which is also bit order in grpfIhdt (1 << k):
    0 header even, 1 header odd, 2 footer even, 3 footer odd, 4 header first, 5 footer first
"""
import sys, struct, olefile

NAMES = ['hdrEven', 'hdrOdd', 'ftrEven', 'ftrOdd', 'hdrFirst', 'ftrFirst']


class Doc:
    def __init__(self, path):
        ole = olefile.OleFileIO(path)
        self.doc = ole.openstream('WordDocument').read()
        flags = struct.unpack_from('<H', self.doc, 0x0A)[0]
        tbl = '1Table' if (flags & 0x0200) else '0Table'
        if not ole.exists(tbl):
            tbl = '0Table' if ole.exists('0Table') else '1Table'
        self.table = ole.openstream(tbl).read()

        off = 32
        csw = struct.unpack_from('<H', self.doc, off)[0]; off += 2 + csw * 2
        cslw = struct.unpack_from('<H', self.doc, off)[0]; off += 2
        self.rgLw = struct.unpack_from('<%di' % cslw, self.doc, off); off += cslw * 4
        cb = struct.unpack_from('<H', self.doc, off)[0]; off += 2
        self.rgFcLcb = struct.unpack_from('<%dI' % (cb * 2), self.doc, off)

        self.ccpText, self.ccpFtn, self.ccpHdd = self.rgLw[3], self.rgLw[4], self.rgLw[5]
        self._pieces()

    def pair(self, i):
        return self.rgFcLcb[i * 2], self.rgFcLcb[i * 2 + 1]

    def _pieces(self):
        fc, lcb = self.pair(33)                      # fcClx
        clx = self.table[fc:fc + lcb]
        i = 0
        while i < len(clx):
            if clx[i] == 1:                          # Prc
                cb = struct.unpack_from('<h', clx, i + 1)[0]
                i += 3 + cb
            elif clx[i] == 2:                        # Pcdt
                n = struct.unpack_from('<I', clx, i + 1)[0]
                plc = clx[i + 5:i + 5 + n]
                npc = (n - 4) // 12
                cps = struct.unpack_from('<%dI' % (npc + 1), plc, 0)
                self.pieces = []
                for k in range(npc):
                    off = 4 * (npc + 1) + 8 * k
                    fcv = struct.unpack_from('<I', plc, off + 2)[0]
                    comp = bool(fcv & 0x40000000)
                    self.pieces.append((cps[k], cps[k + 1], (fcv & 0x3FFFFFFF) // (2 if comp else 1), comp))
                return
            else:
                break
        self.pieces = []

    def text(self, cp0, cp1):
        out = []
        for (a, b, fc, comp) in self.pieces:
            lo, hi = max(a, cp0), min(b, cp1)
            if lo >= hi:
                continue
            if comp:
                out.append((lo - a, self.doc[fc + (lo - a):fc + (hi - a)].decode('cp1252', 'replace')))
            else:
                out.append((lo - a, self.doc[fc + 2 * (lo - a):fc + 2 * (hi - a)].decode('utf-16le', 'replace')))
        return ''.join(t for _, t in out)

    def hdd(self):
        fc, lcb = self.pair(11)
        n = lcb // 4
        cps = struct.unpack_from('<%di' % n, self.table, fc) if n else ()
        return cps

    def sections(self):
        _, lcb = self.pair(6)
        return (lcb - 4) // 16 if lcb else 0

    def facing(self):
        fc, lcb = self.pair(31)
        return bool(self.table[fc] & 0x01) if lcb else False


def show(c):
    return (c.replace('\r', '¶').replace('\x07', '␉').replace('\t', '→')
             .replace('\x13', '▶').replace('\x14', '◆').replace('\x15', '◀')
             .replace('\x01', '□'))


def main(paths, quiet=False):
    for p in paths:
        d = Doc(p)
        cps, nsect, facing = d.hdd(), d.sections(), d.facing()
        base = d.ccpText + d.ccpFtn
        lens = [cps[i + 1] - cps[i] for i in range(len(cps) - 1)]
        print(f'== {p}  sections={nsect}  fFacingPages={facing}  stories={len(lens)}')
        prev = 0
        for s in range(nsect):
            g = 0b111010 | (0b000101 if facing else 0)
            six = []
            for k in range(6):
                idx = 6 + s * 6 + k
                six.append(lens[idx] if idx < len(lens) else 0)
            for k in range(6):
                if (g & (1 << k)) and not six[k] and (s == 0 or not (prev & (1 << k))):
                    g &= ~(1 << k)
            on = ','.join(NAMES[k] for k in range(6) if g & (1 << k))
            print(f'   sect {s:2}: len={six} grpfIhdt={g:06b} [{on}]')
            if not quiet:
                for k in range(6):
                    idx = 6 + s * 6 + k
                    if idx + 1 < len(cps) and six[k]:
                        t = d.text(base + cps[idx], base + cps[idx + 1])
                        print(f'        {NAMES[k]:9} {show(t)!r}')
            prev = g


if __name__ == '__main__':
    args = [a for a in sys.argv[1:] if not a.startswith('-')]
    main(args, quiet='--quiet' in sys.argv)


# ---------------------------------------------------------------------------
# Section descriptors, for the one question the story lengths cannot answer:
# whether the section before this one got a page descriptor at all.
SPRA = {0: 1, 1: 1, 2: 2, 3: 4, 4: 2, 5: 2, 7: 3}


def sprms(grpprl):
    i = 0
    while i + 2 <= len(grpprl):
        op = int.from_bytes(grpprl[i:i + 2], 'little'); i += 2
        spra = op >> 13
        if spra == 6:
            if i >= len(grpprl):
                return
            n = grpprl[i]; i += 1
            yield op, grpprl[i:i + n]; i += n
        else:
            n = SPRA.get(spra, 1)
            yield op, grpprl[i:i + n]; i += n


def seds(d):
    """(bkc, xaPage, yaPage, orient) per section."""
    fc, lcb = d.pair(6)
    if not lcb:
        return []
    n = (lcb - 4) // 16
    plc = d.table[fc:fc + lcb]
    out = []
    for k in range(n):
        off = 4 * (n + 1) + 12 * k
        fcSepx = int.from_bytes(plc[off + 2:off + 6], 'little', signed=True)
        bkc, xa, ya, orient = 2, 12240, 15840, 0
        if 0 <= fcSepx < len(d.doc) - 2 and fcSepx != 0xFFFFFFFF:
            cb = int.from_bytes(d.doc[fcSepx:fcSepx + 2], 'little')
            for op, val in sprms(d.doc[fcSepx + 2:fcSepx + 2 + cb]):
                if op == 0x3009 and val:
                    bkc = val[0]
                elif op == 0xB01F and len(val) >= 2:
                    xa = int.from_bytes(val[:2], 'little')
                elif op == 0xB020 and len(val) >= 2:
                    ya = int.from_bytes(val[:2], 'little')
                elif op == 0x301D and val:
                    orient = val[0]
        out.append((bkc, xa, ya, orient))
    return out


def census(paths):
    """Per document: sections whose header/footer slot LibreOffice leaves blank because the
    section before it is continuous, and we instead inherit from further back."""
    print('path\tsects\tcontinuous\tblanked-slots\tsects-affected')
    for p in paths:
        try:
            d = Doc(p)
        except Exception as e:                                   # noqa: BLE001
            print(f'{p}\tERROR {e}')
            continue
        cps = d.hdd()
        lens = [cps[i + 1] - cps[i] for i in range(len(cps) - 1)]
        sd = seds(d)
        facing = d.facing()
        # A section is continuous — hence has no page descriptor — when bkc == 0 and it agrees
        # with the section before it about the sheet.
        cont = []
        for i, (bkc, xa, ya, o) in enumerate(sd):
            c = (i > 0 and bkc == 0 and (xa, ya, o) == sd[i - 1][1:])
            cont.append(c)
        prev = 0
        blanked = 0
        affected = set()
        for s in range(len(sd)):
            g = 0b111010 | (0b000101 if facing else 0)
            six = [lens[6 + s * 6 + k] if 6 + s * 6 + k < len(lens) else 0 for k in range(6)]
            for k in range(6):
                if (g & (1 << k)) and not six[k] and (s == 0 or not (prev & (1 << k))):
                    g &= ~(1 << k)
            if not cont[s] and s > 0 and cont[s - 1]:
                for k in range(6):
                    if (g & (1 << k)) and six[k] < 2:
                        blanked += 1
                        affected.add(s)
            prev = g
        print(f'{p}\t{len(sd)}\t{sum(cont)}\t{blanked}\t{len(affected)}')
