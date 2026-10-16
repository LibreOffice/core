#!/usr/bin/env python3
"""Font, size, x and baseline y of every text show in a PDF, in PAGE space.

Why this exists rather than slides-r99's tfy.py, which it replaces:

  1. tfy.py finds pages by regex-scanning every `N 0 obj` and testing a 700-byte
     window for `/Type /Page`.  Our writer packs objects tightly, so that window
     runs past `endobj` into the *next* object and a shading dictionary is read as
     a page, taking the following object's `/Contents`.  On
     Inducement-to-Insurance-Business.ppt it reports **28 pages for a 22-page PDF**,
     so every per-page pairing past the first artefact is off by one or more.
     Page enumeration here is PyMuPDF's, i.e. the real page tree.
  2. tfy.py ignores `cm`.  26.2.4.2's own PDFs place an underline with
     `q 1 0 0 1 x y cm ... Q`, and ours wraps runs in `q ... Q` as well; a writer
     that translated with `cm` instead of `Td` would be reported at y = 0.
     The CTM is tracked here and composed with the text matrix.

Baselines are therefore comparable across the two writers, which is the only
property this seat needs.
"""
import re, sys
import pymupdf

NUM = r'[-+]?(?:\d+\.?\d*|\.\d+)'
TOK = re.compile(
    r'/([^\s/\[\]<>(){}]+)\s+(%s)\s+Tf'
    r'|(%s)\s+(%s)\s+(%s)\s+(%s)\s+(%s)\s+(%s)\s+(cm|Tm)'
    r'|(%s)\s+(%s)\s+(TD|Td)'
    r'|(%s)\s+TL'
    r'|(?<![\w/])(T\*)|(?<![\w/])(BT)(?![\w])|(?<![\w/])(ET)(?![\w])'
    r'|(?<![\w/])(q)(?![\w])|(?<![\w/])(Q)(?![\w])'
    r'|(\((?:\\.|[^\\()])*\)|<[0-9A-Fa-f\s]*>)\s*(Tj|TJ|\')'
    r'|(\[)(?=(?:[^\[\]]|\\.)*\]\s*TJ)'
    % ((NUM,) * 10))

def mul(m, n):
    a,b,c,d,e,f = m; A,B,C,D,E,F = n
    return (a*A+b*C, a*B+b*D, c*A+d*C, c*B+d*D, e*A+f*C+E, e*B+f*D+F)

def shows(stream, names):
    text = stream.decode('latin1')
    ctm = (1,0,0,1,0,0); stack = []
    tm = tlm = (1,0,0,1,0,0)
    size = 0.0; font = ''; leading = 0.0
    out = []
    for m in TOK.finditer(text):
        g = m.group
        if g(1) is not None:
            font = g(1); size = float(g(2))
        elif g(9) is not None:                       # cm | Tm
            v = tuple(float(g(i)) for i in range(3, 9))
            if g(9) == 'cm': ctm = mul(v, ctm)
            else: tm = tlm = v
        elif g(12) is not None:                      # Td | TD
            tx, ty = float(g(10)), float(g(11))
            if g(12) == 'TD': leading = -ty
            tlm = tm = mul((1,0,0,1,tx,ty), tlm)
        elif g(13) is not None:
            leading = float(g(13))
        elif g(14):                                  # T*
            tlm = tm = mul((1,0,0,1,0,-leading), tlm)
        elif g(15):                                  # BT
            tm = tlm = (1,0,0,1,0,0)
        elif g(17):                                  # q
            stack.append(ctm)
        elif g(18):                                  # Q
            if stack: ctm = stack.pop()
        elif g(19) is not None or g(21) is not None: # a show
            if g(21) is not None and g(19) is None:
                pass
            full = mul(tm, ctm)
            out.append((names.get(font, font), round(size * full[3], 4),
                        round(full[4], 3), round(full[5], 4)))
            if g(20) == "'":
                tlm = tm = mul((1,0,0,1,0,-leading), tlm)
    return out

def page_names(page):
    names = {}
    for f in page.get_fonts(full=False):
        # (xref, ext, type, basefont, name, encoding)
        names[f[4]] = f[3]
    return names

def read(path):
    doc = pymupdf.open(path)
    for i, page in enumerate(doc, 1):
        yield i, page.rect.height, shows(page.read_contents(), page_names(page))

if __name__ == '__main__':
    for path in sys.argv[1:]:
        for i, h, ss in read(path):
            for fn, sz, x, y in ss:
                print(f"{path}\t{i}\t{fn}\t{sz}\t{x}\t{y}")
