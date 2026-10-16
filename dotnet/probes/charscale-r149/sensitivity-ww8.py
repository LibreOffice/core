#!/usr/bin/env python3
"""The WW8 half of the same question: what the sprm PAINTS on the one corpus `.doc` that states it.

`AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc` is rendered through 26.2.4.2 twice — as it
stands, and with every `sprmCCharScale` operand rewritten from 99 to 100 and no other byte
touched — and the two renderings are compared span for span.
"""
import importlib.util
import pathlib
import shutil
import struct
import subprocess
import sys

import olefile
import pymupdf

spec = importlib.util.spec_from_file_location(
    'ww8census', __file__.rsplit('/', 1)[0] + '/census-ww8.py')
ww8 = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ww8)

pspec = importlib.util.spec_from_file_location(
    'patchchpx', __file__.rsplit('/', 1)[0] + '/patch-chpx.py')

SOFFICE = '/opt/libreoffice26.2/program/soffice'
SPRM = 0x4852
SRC = ('/home/user/sample-files/words/done-011/doc/'
       'AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc')


def offsets(doc):
    """Every (offset, value) of a two-byte `sprmCCharScale` operand in the WordDocument stream."""
    fc, lcb = doc.fclcb[12]
    plc = doc.tbl[fc:fc + lcb]
    n = (len(plc) - 4) // 8
    pns = [struct.unpack_from('<I', plc, 4 * (n + 1) + 4 * i)[0] & 0x3FFFFF for i in range(n)]
    out = []
    for page in pns:
        base = page * 512
        fkp = doc.wd[base:base + 512]
        if len(fkp) < 512:
            continue
        crun = fkp[511]
        for i in range(crun):
            off = fkp[4 * (crun + 1) + i] * 2
            if off == 0 or off >= 511:
                continue
            grpprl = fkp[off + 1:off + 1 + fkp[off]]
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


def render(path, outdir, tag):
    subprocess.run(['timeout', '-k', '30', '600', SOFFICE, '--headless', '--norestore',
                    '-env:UserInstallation=file://%s/profile-%s' % (outdir, tag),
                    '--convert-to', 'pdf', '--outdir', str(outdir), str(path)],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    pdf = pathlib.Path(outdir) / (pathlib.Path(path).stem + '.pdf')
    return pdf if pdf.exists() and pdf.stat().st_size else None


def spans(pdf):
    doc = pymupdf.open(pdf)
    out = []
    for page in doc:
        for b in page.get_text('dict')['blocks']:
            for l in b.get('lines', ()):
                for s in l['spans']:
                    out.append((page.number, round(s['bbox'][0], 2), round(s['bbox'][1], 2),
                                s['text']))
    return len(doc), out


def main():
    work = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/r149-ww8sens')
    work.mkdir(parents=True, exist_ok=True)
    a = work / 'a-witness.doc'
    b = work / 'b-witness.doc'
    shutil.copyfile(SRC, a)
    shutil.copyfile(SRC, b)
    doc = ww8.Doc(str(b))
    spots = offsets(doc)
    print('sprmCCharScale operands: %s' % spots)
    wd = bytearray(doc.wd)
    for at, _v in spots:
        struct.pack_into('<H', wd, at, 100)
    ole = olefile.OleFileIO(str(b), write_mode=True)
    ole.write_stream('WordDocument', bytes(wd))
    ole.close()
    print('after patch            : %s' % offsets(ww8.Doc(str(b))))

    pa, pb = render(a, work, 'a'), render(b, work, 'b')
    if pa is None or pb is None:
        raise SystemExit('a render produced nothing')
    na, sa = spans(pa)
    nb, sb = spans(pb)
    moved = sum(1 for x, y in zip(sa, sb) if x != y) + abs(len(sa) - len(sb))
    print('as stands : %d pages, %d spans' % (na, len(sa)))
    print('at 100    : %d pages, %d spans' % (nb, len(sb)))
    print('spans that move: %d' % moved)
    for x, y in zip(sa, sb):
        if x != y:
            print('  p%-3d %8.2f %8.2f %r   ->  %8.2f %8.2f %r'
                  % (x[0], x[1], x[2], x[3], y[1], y[2], y[3]))


if __name__ == '__main__':
    main()
