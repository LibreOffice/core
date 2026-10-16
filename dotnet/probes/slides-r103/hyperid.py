#!/usr/bin/env python3
"""Which `exHyperlinkId` makes 26.2.4.2 draw a .ppt text range as a hyperlink field.

One byte-patched copy of the document per candidate id, rendered through 26.2.4.2, and the
answer read off the drawn text: a field is drawn in the colour scheme's hyperlink slot and
underlined (`svdfppt.cxx`:7054-7056 sets the underline bit and
`PPT_COLSCHEME_A_UND_HYPERLINK`), an ordinary run in the body's own colour with no rule under
it.  Nothing here depends on reading the record tree correctly -- only on finding the four
bytes, which is checked for uniqueness in the file first.

    hyperid.py <file.ppt> <page> <outdir>

The atom patched is the one whose `TxInteractiveInfoAtom` range starts at the given page's
last paragraph; pass --atom to name a stream offset instead.
"""
import argparse, os, struct, subprocess, sys
import olefile
import pymupdf

SOFFICE = '/opt/libreoffice26.2/program/soffice'


def stream_records(s):
    out = []

    def walk(a, b):
        q = a
        while q + 8 <= b:
            vi, rt, rl = struct.unpack_from('<HHI', s, q)
            out.append((rt, q, rl))
            if (vi & 0x0F) == 0x0F and q + 8 + rl <= b:
                walk(q + 8, q + 8 + rl)
            q += 8 + rl

    walk(0, len(s))
    return out


def render(path, outdir):
    subprocess.run([SOFFICE, '--headless', '--norestore',
                    '-env:UserInstallation=file:///home/user/r103-slidefw/lo',
                    '--convert-to', 'pdf', '--outdir', outdir, path],
                   check=False, capture_output=True, timeout=900)
    return os.path.join(outdir, os.path.splitext(os.path.basename(path))[0] + '.pdf')


def spans(pdf, page, needle):
    doc = pymupdf.open(pdf)
    p = doc[page - 1]
    out = []
    for b in p.get_text('dict')['blocks']:
        for l in b.get('lines', []):
            for s in l['spans']:
                if needle in s['text']:
                    out.append((f'{s["color"]:06x}', round(s['size'], 2), s['text'][:28]))
    rules = [d for d in p.get_drawings() if d['rect'].height < 2 and d['rect'].width > 40]
    return out, len(rules)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ppt'); ap.add_argument('page', type=int); ap.add_argument('outdir')
    ap.add_argument('--atom', type=int, help='stream offset of the InteractiveInfoAtom to patch')
    ap.add_argument('--needle', default='consumerfinance')
    a = ap.parse_args()
    os.makedirs(a.outdir, exist_ok=True)

    ole = olefile.OleFileIO(a.ppt)
    s = ole.openstream('PowerPoint Document').read()
    ole.close()
    raw = open(a.ppt, 'rb').read()
    recs = stream_records(s)

    ids = [struct.unpack_from('<I', s, q + 8)[0] for rt, q, _ in recs if rt == 4051]
    atoms = [(q, struct.unpack_from('<I', s, q + 8 + 4)[0]) for rt, q, rl in recs if rt == 4083]
    print(f'ExHyperlinkAtom ids in stream order: {ids}')
    print(f'InteractiveInfoAtoms: {atoms}')

    target = a.atom if a.atom is not None else atoms[2][0]
    body = s[target:target + 8 + 16]
    if raw.count(body) != 1:
        sys.exit(f'the atom at {target} is not unique in the file ({raw.count(body)} copies)')
    at = raw.find(body) + 8 + 4

    for cand in ['none'] + ids + [999]:
        out = bytearray(raw)
        name = f'hyperid-{cand}'
        if cand != 'none':
            struct.pack_into('<I', out, at, cand)
        path = os.path.join(a.outdir, name + '.ppt')
        with open(path, 'wb') as fh:
            fh.write(out)
        pdf = render(path, a.outdir)
        if not os.path.exists(pdf):
            print(f'{name}\tNO RENDER')
            continue
        sp, rules = spans(pdf, a.page, a.needle)
        print(f'{name}\trules={rules}\t{sp}')
        os.remove(path); os.remove(pdf)


if __name__ == '__main__':
    main()
