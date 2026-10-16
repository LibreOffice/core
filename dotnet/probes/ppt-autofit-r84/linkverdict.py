#!/usr/bin/env python3
"""Does 26.2.4.2 draw each text-range hyperlink as a field, or as ordinary text?

The colour scheme cannot be read straight off the slide -- a slide following its master's
scheme carries an instance-1 `ColorSchemeAtom` of its own that is not the one in force --
so this uses *our own* rendering as the oracle for what the scheme's hyperlink slot is:
this tree now draws every such range in that slot with an underline, so for each range

  agree     the reference draws the same text on the same page in the same colour
  differs   it draws it in another colour, so it made no field there

and the underline is checked beside it, because it is scheme-independent: a field forces
`PPT_CharAttr_Underline` on (`svdfppt.cxx`:7054-7056), so a link the reference underlines
is one it made a field of whatever the colours say.
"""
import sys, pathlib, csv, struct
import olefile, census
from persist import directory, current_user_edit

try:
    import pymupdf as fitz
except ImportError:
    import fitz

TEXTCHARS, TEXTBYTES, TX, II, IIA = 4000, 4008, 4063, 4082, 4083

def link_texts(path):
    ole = olefile.OleFileIO(str(path))
    buf = ole.openstream('PowerPoint Document').read()
    cur = current_user_edit(ole); ole.close()
    offs, doc = directory(buf, cur)
    roots = census.live_roots(buf, offs, doc)
    ids = set(census.link_ids(buf, roots))
    out = []
    def scan(s, e):
        seq = list(census.records(buf, s, e))
        text = None
        for ver, rt, b, st in seq:
            if rt == TEXTCHARS: text = buf[b:st].decode('utf-16-le', 'replace')
            elif rt == TEXTBYTES: text = buf[b:st].decode('cp1252', 'replace')
        for i, (ver, rt, b, st) in enumerate(seq):
            if rt == II:
                hid = None
                for v2, t2, b2, s2 in census.records(buf, b, st):
                    if t2 == IIA and s2 - b2 >= 8:
                        hid = struct.unpack_from('<I', buf, b2 + 4)[0]
                nxt = seq[i + 1] if i + 1 < len(seq) else None
                if hid in ids and nxt and nxt[1] == TX and nxt[3] - nxt[2] >= 8:
                    a, z = struct.unpack_from('<II', buf, nxt[2])
                    if z and text:
                        body = text[a:z].strip().replace('\r', ' ').replace('\x0b', ' ')
                        if len(body) >= 6: out.append((hid, a, z, body))
            elif ver == 0x0F:
                scan(b, st)
    for rt, off, end in roots: scan(off + 8, end)
    return out

def spans_of(pdf):
    doc = fitz.open(pdf)
    out = []
    for pno, page in enumerate(doc):
        for b in page.get_text('dict')['blocks']:
            if b['type'] != 0: continue
            for line in b['lines']:
                for s in line['spans']:
                    out.append((pno, s['text'], s['color'], s['origin'][1], s['origin'][0],
                                s['bbox'][2]))
    strokes = {}
    for pno, page in enumerate(doc):
        for dr in page.get_drawings():
            # An underline reaches the PDF as a thin stroke from one writer and as a thin
            # filled rectangle from the other, so both count.
            r = dr['rect']
            if abs(r.y1 - r.y0) < 2.0 and r.x1 - r.x0 > 4:
                strokes.setdefault(pno, []).append((r.y0, r.x0, r.x1))
    doc.close()
    return out, strokes

def underlined(strokes, pno, y, x0, x1):
    for sy, sx0, sx1 in strokes.get(pno, []):
        if 0 < sy - y < 6 and sx0 < x1 and sx1 > x0: return True
    return False

def main(names, out):
    w = csv.writer(out, delimiter='\t', lineterminator='\n')
    w.writerow(['document', 'id', 'text', 'ours_colour', 'ref_colour',
                'ours_underlined', 'ref_underlined', 'verdict'])
    tally = {}
    for name in names:
        p = next(q for q in pathlib.Path('/home/user/sample-files').rglob('*') if q.name == name)
        head = pathlib.Path('head') / (p.stem + '.pdf')
        ref = pathlib.Path('ref') / (p.stem + '.pdf')
        if not head.exists() or not ref.exists(): continue
        hs, hst = spans_of(head)
        rs, rst = spans_of(ref)
        for hid, a, z, body in link_texts(p):
            probe = body[:20]
            ours = [s for s in hs if probe and probe in s[1]]
            if not ours: 
                tally['unseen'] = tally.get('unseen', 0) + 1
                w.writerow([name, hid, body[:50], '', '', '', '', 'unseen']); continue
            pno, text, col, y, x0, x1 = ours[0]
            # The same page AND close to the same baseline: a URL can appear on several
            # shapes of one slide -- `Inducement-to-Insurance-Business.ppt` draws "Bulletin
            # 158" once as a link and once as ordinary text -- and matching on the text alone
            # picks whichever comes first, which read as a difference that is not there.
            theirs = sorted((s for s in rs if s[0] == pno and probe in s[1]
                             and abs(s[3] - y) <= 8.0), key=lambda s: abs(s[3] - y))
            if not theirs:
                tally['unseen'] = tally.get('unseen', 0) + 1
                w.writerow([name, hid, body[:50], f'{col:06x}', '', '', '', 'unseen']); continue
            rp, rt, rc, ry, rx0, rx1 = theirs[0]
            v = 'agree' if rc == col else 'differs'
            tally[v] = tally.get(v, 0) + 1
            w.writerow([name, hid, body[:50], f'{col:06x}', f'{rc:06x}',
                        underlined(hst, pno, y, x0, x1), underlined(rst, rp, ry, rx0, rx1), v])
    print(' '.join(f'{k} {v}' for k, v in sorted(tally.items())), file=sys.stderr)

if __name__ == '__main__':
    main([l.strip() for l in open(sys.argv[1]) if l.strip()], sys.stdout)
