#!/usr/bin/env python3
"""Which corpus documents either arm of this round can reach, straight off the markup.

Two independent censuses, printed as one TSV so the base rates sit beside each other:

    chart      the document states at least one chart part (any of the three families)
    emptysec   ...and at least one of those charts gives a value axis a `c:numFmt formatCode`
               with an empty section, which is the only way a *non-date* axis' ticks can format
               to the same string twice without rounding doing it
    oversize   a word-processing document states a drawing larger than the page of the section
               it sits in — `w:sectPr/w:pgSz` is per section, and reading only the first one
               calls two landscape-page drawings oversize that are nothing of the kind
    squeezed   ...and that drawing is a *free* fly that the escape does not exempt, which needs
               two more things: it is not `wp:inline` (an as-character object is a
               `SwFlyInContentFrame`, and `CheckClip` is a `SwFlyFreeFrame` method —
               `sw/source/core/inc/flyfrms.hxx`:150 against :212), and it is not wrap-through
               (`SwAnchoredObject::IsDraggingOffPageAllowed`)

`oversize`/`squeezed` are read out of OOXML `wp:anchor`/`wp:inline` only. A `.doc`, `.rtf` or
`.odt` reaches the same `FrameLayout.Place`, and this census cannot see those — the sweep is what
scores them, and this only predicts.
"""
import re, sys, zipfile

EMU = 914400.0
PAGE = re.compile(r'<w:pgSz\b[^>]*/>')


def emptysection(code):
    """A `formatCode` with a section that formats a number to nothing at all."""
    if code is None:
        return False
    # Sections split on `;` outside quotes; an empty one draws nothing.
    parts, cur, q = [], '', False
    for ch in code:
        if ch == '"':
            q = not q
        if ch == ';' and not q:
            parts.append(cur)
            cur = ''
        else:
            cur += ch
    parts.append(cur)
    return len(parts) > 1 and any(p.strip() == '' for p in parts)


def charts(z):
    """(has chart, has an empty-section numFmt on a val axis)."""
    names = [n for n in z.namelist() if re.search(r'charts?/chart\d*\.xml$', n)]
    if not names:
        return False, False
    for n in names:
        try:
            x = z.read(n).decode('utf-8', 'replace')
        except Exception:
            continue
        for m in re.finditer(r'<c:valAx>.*?</c:valAx>', x, re.S):
            for f in re.findall(r'<c:numFmt[^>]*formatCode="([^"]*)"', m.group(0)):
                if emptysection(f.replace('&quot;', '"')):
                    return True, True
    return True, False


def pagesizes(x):
    """Every `w:pgSz` in document order, as (EMU width, EMU height) at its offset."""
    out = []
    for m in PAGE.finditer(x):
        w = re.search(r'w:w="(\d+)"', m.group(0))
        h = re.search(r'w:h="(\d+)"', m.group(0))
        if w and h:
            out.append((m.start(), int(w.group(1)) * 635, int(h.group(1)) * 635))
    return out


def frames(z):
    try:
        x = z.read('word/document.xml').decode('utf-8', 'replace')
    except Exception:
        return False, False
    sizes = pagesizes(x)
    if not sizes:
        return False, False
    over = squeezed = False
    for m in re.finditer(r'<wp:(anchor|inline)\b.*?</wp:\1>', x, re.S):
        a = m.group(0)
        e = re.search(r'<wp:extent cx="(\d+)" cy="(\d+)"/>', a)
        if not e:
            continue
        # The section a drawing belongs to is the first `w:sectPr` *after* it; a body-final
        # `sectPr` is the last of them and covers everything not otherwise claimed.
        pw, ph = next(((w, h) for off, w, h in sizes if off > m.start()), sizes[-1][1:])
        if int(e.group(1)) <= pw and int(e.group(2)) <= ph:
            continue
        over = True
        if m.group(1) == 'inline':
            continue
        # Word's wrapNone is Writer's WrapTextMode_THROUGH; wrapSquare/Tight/Through/TopAndBottom
        # are not.
        if '<wp:wrapNone/>' in a:
            continue
        squeezed = True
    return over, squeezed


def main():
    print('path\text\tchart\temptysec\toversize\tsqueezed')
    with open(sys.argv[1], encoding='utf-8') as fh:
        head = fh.readline().rstrip('\n').split('\t')
        ci, ei = head.index('path'), head.index('ext')
        for line in fh:
            p = line.rstrip('\n').split('\t')
            rel, ext = p[ci], p[ei]
            full = sys.argv[2].rstrip('/') + '/' + rel
            c = e = o = s = False
            if ext in ('docx', 'docm', 'dotx', 'dotm', 'xlsx', 'xlsm', 'xltx', 'xltm',
                       'pptx', 'pptm', 'potx', 'potm', 'ppsx', 'ppsm'):
                try:
                    with zipfile.ZipFile(full) as z:
                        c, e = charts(z)
                        if ext.startswith('do'):
                            o, s = frames(z)
                except Exception:
                    pass
            print(f'{rel}\t{ext}\t{int(c)}\t{int(e)}\t{int(o)}\t{int(s)}')


main()
