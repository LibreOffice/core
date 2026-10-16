#!/usr/bin/env python3
r"""What the attribute PAINTS, at the reference, over the documents that state it.

A census counts statements; this counts consequences. For each `.rtf` of the converted column that
states a non-identity `\charscalex`, the document is rendered through 26.2.4.2 twice — as it stands
and with every non-identity `\charscalex` rewritten to `\charscalex100`, one byte-for-byte
substitution and nothing else — and the two renderings are compared. A document whose two
renderings are identical is one the attribute does not reach, whatever its census row says.

  sensitivity-rtf.py <workdir>
"""
import pathlib
import re
import subprocess
import sys

import pymupdf

SOFFICE = '/opt/libreoffice26.2/program/soffice'
SCALE = re.compile(rb'\\charscalex(-?[0-9]+)')


def flatten(data):
    def sub(m):
        return b'\\charscalex100' if int(m.group(1)) != 100 else m.group(0)
    return SCALE.sub(sub, data)


def render(path, outdir, tag):
    subprocess.run(['timeout', '-k', '30', '600', SOFFICE, '--headless', '--norestore',
                    '-env:UserInstallation=file://%s/profile-%s' % (outdir, tag),
                    '--convert-to', 'pdf', '--outdir', str(outdir), str(path)],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
    pdf = pathlib.Path(outdir) / (path.stem + '.pdf')
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
    work = pathlib.Path(sys.argv[1])
    work.mkdir(parents=True, exist_ok=True)
    src = pathlib.Path('/home/user/corpus-odf/rtf')
    print('document\tnet\tpages_as_is\tpages_flat\tspans_as_is\tspans_flat\tspans_moved\tverdict')
    for path in sorted(src.glob('*.rtf')):
        data = path.read_bytes()
        net = sum(1 for m in SCALE.finditer(data) if int(m.group(1)) != 100)
        if not net:
            continue
        d = work / path.stem[:12]
        d.mkdir(parents=True, exist_ok=True)
        a = d / ('a-' + path.stem[:12] + '.rtf')
        b = d / ('b-' + path.stem[:12] + '.rtf')
        a.write_bytes(data)
        b.write_bytes(flatten(data))
        pa, pb = render(a, d, 'a'), render(b, d, 'b')
        if pa is None or pb is None:
            print('%s\t%d\t-\t-\t-\t-\t-\tRENDER-FAILED' % (path.stem[13:], net))
            continue
        na, sa = spans(pa)
        nb, sb = spans(pb)
        moved = sum(1 for x, y in zip(sa, sb) if x != y) + abs(len(sa) - len(sb))
        verdict = ('pages' if na != nb else
                   'moves' if moved else 'IDENTICAL')
        print('%s\t%d\t%d\t%d\t%d\t%d\t%d\t%s'
              % (path.stem[13:], net, na, nb, len(sa), len(sb), moved, verdict))
        sys.stdout.flush()


if __name__ == '__main__':
    main()
