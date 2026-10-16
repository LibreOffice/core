#!/usr/bin/env python3
"""For every text-range hyperlink the census says makes a field, what colour does
26.2.4.2 actually draw it in -- and is that the scheme's hyperlink slot?

The discriminator that a span comparison cannot give on its own: LibreOffice imposes
`PPT_COLSCHEME_A_UND_HYPERLINK` on every portion of a text-range hyperlink
(`svdfppt.cxx`:7060, :7094), so a link drawn in any other colour is one the reference
did not make a field of.
"""
import struct, sys, pathlib, csv
import olefile, census
from persist import directory, current_user_edit

try:
    import pymupdf as fitz
except ImportError:
    import fitz

TEXTCHARS, TEXTBYTES, COLORSCHEME = 4000, 4008, 2032
TX, II, IIA = 4063, 4082, 4083

def slide_scheme(buf, off, end):
    """The page's own colour scheme: the direct-child ColorSchemeAtom of header instance 1.

    Instance 1 is the scheme in force; a master also carries instance-6 alternatives, and
    taking the last one seen while descending picks one of those instead -- which is how the
    first cut of this probe reported every link as drawn in the wrong colour.
    """
    for ver, rt, b, st in census.records(buf, off + 8, end):
        if rt == COLORSCHEME and st - b >= 32:
            vi = struct.unpack_from('<H', buf, b - 8)[0]
            if vi >> 4 == 1:
                return [pack_rgb(struct.unpack_from('<I', buf, b + 4 * i)[0]) for i in range(8)]
    return None

def pack_rgb(word):
    """The record's blue-green-red word as PyMuPDF's red-green-blue integer."""
    return ((word & 0xFF) << 16) | (word & 0xFF00) | ((word >> 16) & 0xFF)

def ranges_with_text(buf, roots):
    """(exHyperlinkId, start, end, linked text, the page's own scheme) per pair."""
    out = []
    def scan(s, e, scheme):
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
                if hid is not None and nxt and nxt[1] == TX and nxt[3] - nxt[2] >= 8:
                    a, z = struct.unpack_from('<II', buf, nxt[2])
                    if z and text:
                        out.append((hid, a, z, text[a:z], scheme))
            elif ver == 0x0F:
                scan(b, st, scheme)
    for rt, off, end in roots:
        scan(off + 8, end, slide_scheme(buf, off, end))
    return out

def pdf_colours(path, needle):
    """Every colour the PDF draws a span whose text starts the needle in."""
    doc = fitz.open(path)
    found = []
    probe = needle[:24]
    for page in doc:
        for b in page.get_text('dict')['blocks']:
            if b['type'] != 0: continue
            for line in b['lines']:
                for s in line['spans']:
                    if probe and probe in s['text']:
                        found.append(s['color'])
    doc.close()
    return found

def main(names, refdir, out):
    w = csv.writer(out, delimiter='\t', lineterminator='\n')
    w.writerow(['document', 'id', 'start', 'end', 'text', 'scheme6', 'ref_colour', 'field'])
    tally = {'field': 0, 'plain': 0, 'unseen': 0}
    for name in names:
        p = next(q for q in pathlib.Path('/home/user/sample-files').rglob('*') if q.name == name)
        ole = olefile.OleFileIO(str(p))
        buf = ole.openstream('PowerPoint Document').read()
        cur = current_user_edit(ole); ole.close()
        offs, doc = directory(buf, cur)
        roots = census.live_roots(buf, offs, doc)
        ids = set(census.link_ids(buf, roots))
        pdf = pathlib.Path(refdir) / (p.stem + '.pdf')
        if not pdf.exists(): continue
        for hid, a, z, text, scheme in ranges_with_text(buf, roots):
            if hid not in ids: continue
            body = text.strip().replace('\r', ' ').replace('\x0b', ' ')
            if not body: continue
            s6 = scheme[6] if scheme else None
            cols = pdf_colours(pdf, body)
            if not cols:
                verdict = 'unseen'
            elif s6 is not None and all(c == s6 for c in cols):
                verdict = 'field'
            else:
                verdict = 'plain'
            tally[verdict] += 1
            w.writerow([name, hid, a, z, body[:60], f'{s6:06x}' if s6 is not None else '',
                        ','.join(f'{c:06x}' for c in sorted(set(cols))), verdict])
    print(f"drawn in the scheme's hyperlink slot: {tally['field']}; "
          f"in another colour: {tally['plain']}; not found in the text layer: {tally['unseen']}",
          file=sys.stderr)

if __name__ == '__main__':
    names = [line.strip() for line in open(sys.argv[1]) if line.strip()]
    main(names, sys.argv[2], sys.stdout)
