#!/usr/bin/env python3
r"""Census `sprmCCharScale` (0x4852) over the corpus's legacy `.doc`.

    census-ww8.py <manifest.tsv> <corpus-root>

The FIB/piece-table/FKP machinery is `probes/words-pages-01/chpx.py`'s, lifted rather than
rewritten; what is added is (a) a walk of EVERY CHPX in EVERY FKP page of the PlcfBteChpx
rather than the one covering a given CP, (b) a walk of the style sheet's character UPX, because
round 143's witness stated the width in a STYLE and not in a run, and (c) a raw two-byte scan
of the whole file as an independent upper bound, in the shape `probes/dblunder-r123/census-ww8.py`
used.

WHY THE FKP WALK COVERS EVERY STORY.  The PlcfBteChpx is indexed by FC, not by CP, and every
story's text -- body, footnotes, headers, macros, annotations, endnotes, text boxes and header
text boxes -- lives in the same WordDocument stream and is covered by the same FKP pages.  So
enumerating the pages enumerates every character run of every story.  Each run is additionally
mapped FC -> CP through the piece table and CP -> story through the FIB's ccp* fields, so the
census can say WHICH story a hit is in.

sprmCCharScale is `sprmChr<0x52, 0, SPRA::operand_2b_2>` (`sw/source/filter/ww8/sprmids.hxx`:335)
-- a two-byte operand.  `SwWW8ImplReader::Read_ScaleWidth` (`ww8par6.cxx`:4985-4997) replaces
anything outside 1..600 with 100, so this counts those separately.
"""
import collections, struct, sys

import olefile

SPRM = 0x4852
STORIES = ('body', 'footnotes', 'headers', 'macros', 'annotations', 'endnotes',
           'textboxes', 'headertextboxes')


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
    if at >= len(data):
        return 1
    return 1 + data[at]


def sprms_of(grpprl):
    out, at = [], 0
    while at + 2 <= len(grpprl):
        sprm = struct.unpack_from('<H', grpprl, at)[0]
        at += 2
        size = operand_size(sprm, grpprl, at)
        out.append((sprm, grpprl[at:at + size]))
        at += size
    return out


class Doc:
    def __init__(self, path):
        ole = olefile.OleFileIO(path)
        self.wd = ole.openstream('WordDocument').read()
        base = struct.unpack_from('<H', self.wd, 10)[0]
        table = '1Table' if (base >> 9) & 1 else '0Table'
        self.tbl = ole.openstream(table).read()
        ole.close()

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
        self.ccps = self.rglw[3:11]          # ccpText .. ccpHdrTxbx
        self.pieces = self._pieces()

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
                out = []
                for i in range(n):
                    off = 4 * (n + 1) + 8 * i
                    pfc = struct.unpack_from('<I', plc, off + 2)[0]
                    comp = bool(pfc & 0x40000000)
                    real = (pfc & 0x3FFFFFFF) // 2 if comp else (pfc & 0x3FFFFFFF)
                    out.append((cps[i], cps[i + 1], real, comp))
                return out
            raise ValueError('unexpected CLX byte %#x' % clx[at])
        raise ValueError('no Pcdt in CLX')

    def cp_of(self, fc):
        for start, end, pfc, comp in self.pieces:
            width = 1 if comp else 2
            span = (end - start) * width
            if pfc <= fc < pfc + span:
                return start + (fc - pfc) // width
        return None

    def story_of(self, cp):
        if cp is None:
            return 'unmapped'
        at = 0
        for name, n in zip(STORIES, self.ccps):
            if at <= cp < at + n:
                return name
            at += n
        return 'beyond'

    def chpxs(self):
        """Every CHPX in every FKP page of the PlcfBteChpx, as (fc_start, fc_end, grpprl)."""
        fc, lcb = self.fclcb[12]
        plc = self.tbl[fc:fc + lcb]
        n = (len(plc) - 4) // 8
        pns = [struct.unpack_from('<I', plc, 4 * (n + 1) + 4 * i)[0] & 0x3FFFFF for i in range(n)]
        out = []
        for page in pns:
            fkp = self.wd[page * 512:(page + 1) * 512]
            if len(fkp) < 512:
                continue
            crun = fkp[511]
            if crun == 0 or 4 * (crun + 1) + crun > 511:
                continue
            rgfc = [struct.unpack_from('<I', fkp, 4 * i)[0] for i in range(crun + 1)]
            for i in range(crun):
                off = fkp[4 * (crun + 1) + i] * 2
                if off == 0 or off >= 511:
                    out.append((rgfc[i], rgfc[i + 1], b''))
                    continue
                cb = fkp[off]
                out.append((rgfc[i], rgfc[i + 1], fkp[off + 1:off + 1 + cb]))
        return out

    def style_chpx(self):
        """Every style's character UPX, as (istd, name, grpprl)."""
        fc, lcb = self.fclcb[1]   # fcStshf; pair 0 is fcStshfOrig
        stsh = self.tbl[fc:fc + lcb]
        if len(stsh) < 2:
            return []
        cbstshi = struct.unpack_from('<H', stsh, 0)[0]
        stshi = stsh[2:2 + cbstshi]
        cstd = struct.unpack_from('<H', stshi, 0)[0]
        cb_base = struct.unpack_from('<H', stshi, 2)[0]
        at = 2 + cbstshi
        out = []
        for istd in range(cstd):
            if at + 2 > len(stsh):
                break
            cbstd = struct.unpack_from('<H', stsh, at)[0]
            at += 2
            std = stsh[at:at + cbstd]
            at += cbstd + (cbstd & 1)
            if cbstd == 0 or len(std) < cb_base + 2:
                continue
            # stdfBase: word 0 is sti|flags, word 1 is sgc(4) | istdBase(12),
            # word 2 is cupx(4) | istdNext(12).  sgc 1 = paragraph style, 2 = character style.
            sgc = struct.unpack_from('<H', std, 2)[0] & 0x0F
            cupx = struct.unpack_from('<H', std, 4)[0] & 0x0F
            p = cb_base
            cch = struct.unpack_from('<H', std, p)[0]
            name = std[p + 2:p + 2 + 2 * cch].decode('utf-16-le', 'replace')
            p += 2 + 2 * cch + 2                      # xstzName: count, chars, terminating null
            upxs = []
            while p + 2 <= len(std) and len(upxs) < max(cupx, 2):
                cbupx = struct.unpack_from('<H', std, p)[0]
                p += 2
                upxs.append(std[p:p + cbupx])
                p += cbupx + (cbupx & 1)
            if sgc == 1 and len(upxs) >= 2:
                grpprl = upxs[1]                      # the character UPX of a paragraph style
            elif sgc == 2 and upxs:
                grpprl = upxs[0]                      # a character style's only UPX
            else:
                continue
            out.append((istd, name, grpprl))
        return out


def main(manifest, root):
    import os
    print('\t'.join(('path', 'chpx_runs', 'chpx_hits', 'chpx_nonunit', 'chpx_values',
                     'stories', 'styles', 'style_hits', 'style_values', 'raw_scan', 'note')))
    grand = collections.Counter()
    values = collections.Counter()
    style_values = collections.Counter()
    stories = collections.Counter()
    docs_any, docs_nonunit, ndocs = set(), set(), 0
    for line in open(manifest, encoding='utf-8').read().splitlines()[1:]:
        f = line.split('\t')
        if f[3].lower() != 'doc':
            continue
        ndocs += 1
        path = os.path.join(root, f[2])
        blob = open(path, 'rb').read()
        raw = 0
        i = blob.find(b'\x52\x48')
        while i >= 0:
            raw += 1
            i = blob.find(b'\x52\x48', i + 1)
        note = ''
        runs = hits = nonunit = 0
        nstyles = style_hits = 0
        vals, svals, sts = collections.Counter(), collections.Counter(), collections.Counter()
        try:
            doc = Doc(path)
            chpxs = doc.chpxs()
            runs = len(chpxs)
            for fcs, _fce, grpprl in chpxs:
                for sprm, operand in sprms_of(grpprl):
                    if sprm != SPRM or len(operand) < 2:
                        continue
                    v = struct.unpack('<H', operand[:2])[0]
                    hits += 1
                    vals[v] += 1
                    values[v] += 1
                    story = doc.story_of(doc.cp_of(fcs))
                    sts[story] += 1
                    stories[story] += 1
                    if v != 100:
                        nonunit += 1
            styles = doc.style_chpx()
            nstyles = len(styles)
            for _istd, _name, grpprl in styles:
                for sprm, operand in sprms_of(grpprl):
                    if sprm != SPRM or len(operand) < 2:
                        continue
                    v = struct.unpack('<H', operand[:2])[0]
                    style_hits += 1
                    svals[v] += 1
                    style_values[v] += 1
        except Exception as exc:
            note = 'UNPARSED %s: %s' % (type(exc).__name__, exc)
        grand['runs'] += runs
        grand['hits'] += hits
        grand['nonunit'] += nonunit
        grand['styles'] += nstyles
        grand['style_hits'] += style_hits
        grand['raw'] += raw
        if hits or style_hits:
            docs_any.add(f[2])
        if nonunit or any(v != 100 for v in svals):
            docs_nonunit.add(f[2])
        print('\t'.join((f[2], str(runs), str(hits), str(nonunit),
                         ' '.join('%d=%d' % kv for kv in sorted(vals.items())),
                         ' '.join('%s=%d' % kv for kv in sorted(sts.items())),
                         str(nstyles), str(style_hits),
                         ' '.join('%d=%d' % kv for kv in sorted(svals.items())),
                         str(raw), note)))
    print('\n# documents\t%d' % ndocs)
    print('# base CHPX runs\t%d' % grand['runs'])
    print('# base styles\t%d' % grand['styles'])
    print('# CHPX sprmCCharScale\t%d' % grand['hits'])
    print('# CHPX sprmCCharScale != 100\t%d' % grand['nonunit'])
    print('# style sprmCCharScale\t%d' % grand['style_hits'])
    print('# documents stating it\t%d' % len(docs_any))
    print('# documents stating != 100\t%d' % len(docs_nonunit))
    print('# CHPX values\t%s' % dict(sorted(values.items())))
    print('# style values\t%s' % dict(sorted(style_values.items())))
    print('# stories\t%s' % dict(stories))
    print('# raw 0x4852 byte pairs (upper bound)\t%d' % grand['raw'])


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
