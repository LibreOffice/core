#!/usr/bin/env python3
r"""Census the four double-underline spellings over a corpus of documents.

    census-ooxml.py <manifest.tsv> <corpus-root> > rows.tsv

For a ZIP-based OOXML document every part is scanned; for a flat ODF the file itself is.
Counts are of ATTRIBUTE OCCURRENCES, not of documents, and the per-document row lets a
base rate be taken from the manifest's own totals.

Spellings, and why each is the whole of its format's vocabulary:

  WordprocessingML  `w:u w:val="double"` and `w:val="wavyDouble"` -- ST_Underline's only two
                    two-line values (ECMA-376 17.18.99).  `w:u` also appears inside
                    `w:rPrChange`/`w:del` revision marks and in styles.xml/numbering.xml, so
                    the part is recorded.
  DrawingML         `a:u="dbl"` and `a:u="wavyDbl"` -- ST_TextUnderlineType (20.1.10.82).
  ODF               `style:text-underline-type="double"`.  ODF splits the type from the
                    style, so a double underline is `type="double"` with any non-`none`
                    `style:text-underline-style`.
  RTF               `\uldb` (double) and `\ululdbwave` (double wave).  `\uld`, `\uldash`,
                    `\uldashd`, `\uldashdd` are DIFFERENT control words with `\uldb` as a
                    prefix, so the scan requires a non-alphabetic delimiter after it.
"""
import re, sys, os, zipfile

W_DOUBLE = re.compile(rb'<w:u\b[^>]*w:val="(double|wavyDouble)"')
W_ANY    = re.compile(rb'<w:u\b[^>]*w:val="([A-Za-z]+)"')
W_BARE   = re.compile(rb'<w:u\b(?![^>]*w:val=)[^>]*/?>')
A_DOUBLE = re.compile(rb'\bu="(dbl|wavyDbl)"')
S_DOUBLE = re.compile(rb'<u val="(double|doubleAccounting)"\s*/?>')
S_ANY    = re.compile(rb'<u(?:\s+val="[a-zA-Z]+")?\s*/?>')
A_ANY    = re.compile(rb'\bu="(sng|dbl|wavyDbl|words|heavy|dotted|dottedHeavy|dash|dashHeavy|dashLong|dashLongHeavy|dotDash|dotDashHeavy|dotDotDash|dotDotDashHeavy|wavy|wavyHeavy)"')
O_DOUBLE = re.compile(rb'style:text-underline-type="double"')
O_ANY    = re.compile(rb'style:text-underline-style="(?!none)[a-z-]+"')
R_DOUBLE = re.compile(rb'\\uldb(?![a-zA-Z])|\\ululdbwave(?![a-zA-Z])')
R_ANY    = re.compile(rb'\\ul(?![a-zA-Z0-9])|\\uld(?![a-zA-Z])|\\uldb(?![a-zA-Z])|\\ulth(?![a-zA-Z])|\\ulw(?![a-zA-Z])|\\ulwave(?![a-zA-Z])|\\ulhwave(?![a-zA-Z])|\\ululdbwave(?![a-zA-Z])|\\uldash(?![a-zA-Z])|\\uldashd(?![a-zA-Z])|\\uldashdd(?![a-zA-Z])')

OOXML = ('.docx', '.docm', '.dotx', '.dotm', '.pptx', '.pptm', '.potx', '.potm',
         '.ppsx', '.xlsx', '.xlsm', '.xltx', '.xlsb')
FLAT  = ('.fodt', '.fods', '.fodp')
ODFZIP = ('.odt', '.ods', '.odp', '.ott', '.otp', '.ots', '.sxw', '.sxc', '.sxi')


def scan_bytes(blob, part):
    """Return (dbl, any) for whichever vocabularies this blob speaks."""
    d = a = 0
    d += len(W_DOUBLE.findall(blob))
    a += len(W_ANY.findall(blob)) + len(W_BARE.findall(blob))
    d += len(A_DOUBLE.findall(blob))
    a += len(A_ANY.findall(blob))
    d += len(O_DOUBLE.findall(blob))
    a += len(O_ANY.findall(blob))
    # SpreadsheetML: `<u/>` is a single underline and `<u val="double"/>` a double one
    # (CT_UnderlineProperty, ECMA-376 18.4.13).  Scanned only in the parts that speak it,
    # because `<u ...>` is also HTML and appears inside `w:altChunk` and chart alt text.
    if part.startswith('xl/'):
        d += len(S_DOUBLE.findall(blob))
        a += len(S_ANY.findall(blob))
    return d, a


def scan(path):
    ext = os.path.splitext(path)[1].lower()
    dbl = any_ = 0
    parts = []
    try:
        if ext == '.rtf':
            blob = open(path, 'rb').read()
            dbl = len(R_DOUBLE.findall(blob))
            any_ = len(R_ANY.findall(blob))
            if dbl:
                parts.append('rtf')
        elif ext in FLAT:
            blob = open(path, 'rb').read()
            dbl, any_ = scan_bytes(blob, ext)
            if dbl:
                parts.append('flat')
        elif ext in OOXML or ext in ODFZIP:
            with zipfile.ZipFile(path) as z:
                for info in z.infolist():
                    if not info.filename.lower().endswith(('.xml', '.rels')):
                        continue
                    try:
                        blob = z.read(info)
                    except Exception:
                        continue
                    d, a = scan_bytes(blob, info.filename)
                    dbl += d
                    any_ += a
                    if d:
                        parts.append('%s:%d' % (info.filename, d))
        else:
            # a legacy binary: not scannable by markup, reported as such
            return None, None, 'binary'
    except Exception as exc:
        return None, None, 'error:%s' % type(exc).__name__
    return dbl, any_, ','.join(parts)


def main(manifest, root):
    print('path\text\tdouble\tany_underline\twhere')
    for line in open(manifest).read().splitlines()[1:]:
        f = line.split('\t')
        rel, ext = f[2], f[3]
        full = os.path.join(root, rel)
        d, a, where = scan(full)
        print('%s\t%s\t%s\t%s\t%s' % (rel, ext,
                                      '' if d is None else d,
                                      '' if a is None else a, where))


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
