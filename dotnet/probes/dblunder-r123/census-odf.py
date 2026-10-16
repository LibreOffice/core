#!/usr/bin/env python3
r"""Census the ODF and RTF double-underline spellings over /home/user/corpus-odf.

    census-odf.py <root> > rows.tsv

The converted column is 26.2.4.2's own `--convert-to` of the whole 947-document corpus, so
it is the only instrument that can see what the reference understood from a `.doc`, an
`.xls` or a `.ppt` -- those three are binary and no markup grep reaches them.

  ODF   `style:text-underline-type="double"` (and its `loext:` spelling, checked for by
        C-rule: an attribute LibreOffice exports is often not in the namespace the
        specification puts it in).
  RTF   `\uldb` and `\ululdbwave`, each requiring a non-alphabetic delimiter so that
        `\uldash`, `\uldashd` and `\uldashdd` are not counted as `\uldb`.
"""
import re, sys, os, zipfile

O_DOUBLE = re.compile(rb'(?:style|loext):text-underline-type="double"')
O_TYPE   = re.compile(rb'(?:style|loext):text-underline-type="([a-z-]+)"')
O_STYLE  = re.compile(rb'style:text-underline-style="([a-z-]+)"')
R_DOUBLE = re.compile(rb'\\uldb(?![a-zA-Z])|\\ululdbwave(?![a-zA-Z])')
R_ANY    = re.compile(rb'\\ul(?:d|db|th|w|wave|hwave|dash|dashd|dashdd|thd|thdash|thdashd|thdashdd|thldash|ldash)?(?![a-zA-Z0-9])')


def scan(path):
    ext = os.path.splitext(path)[1].lower()
    if ext == '.rtf':
        blob = open(path, 'rb').read()
        return len(R_DOUBLE.findall(blob)), len(R_ANY.findall(blob))
    dbl = anyu = 0
    with zipfile.ZipFile(path) as z:
        for info in z.infolist():
            if not info.filename.endswith('.xml'):
                continue
            blob = z.read(info)
            dbl += len(O_DOUBLE.findall(blob))
            anyu += len([m for m in O_STYLE.findall(blob) if m != b'none'])
    return dbl, anyu


def main(root):
    print('path\text\tdouble\tany_underline')
    for dirpath, _dirs, files in os.walk(root):
        for name in sorted(files):
            full = os.path.join(dirpath, name)
            ext = os.path.splitext(name)[1].lower()
            try:
                d, a = scan(full)
            except Exception as exc:
                print('%s\t%s\tERR\t%s' % (os.path.relpath(full, root), ext, type(exc).__name__))
                continue
            print('%s\t%s\t%d\t%d' % (os.path.relpath(full, root), ext, d, a))


if __name__ == '__main__':
    main(sys.argv[1])
