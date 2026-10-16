#!/usr/bin/env python3
"""Author one-attribute variants of `CIS_Debian_Linux_8_Benchmark_v1.0.0.ods`.

    cis-variants.py <name>          writes <name>.ods beside the script and prints its path

Then render each through 26.2.4.2 with `refpages.sh` and through the tree's own CLI, and compare
the page counts. The variants and what each settles:

    copy          the control -- repackaging alone must not move the count (61)
    nocols        drop the 246-column co2 run carrying default-cell-style ce3
    nodefstyle    keep those columns but drop their default cell style
    norows        drop the trailing padding row blocks
    padN          replace the padding blocks with N rows
    only_lic / only_l1 / only_l2      keep one sheet
    nl2space      every raw newline inside a text:p becomes a space
    longest       every cell keeps only its longest paragraph

`nocols`, `nodefstyle`, `norows` and every `padN` leave 26.2.4.2 at 61 pages, which is what
refutes "a column-band question over 246 styled-but-empty columns". `nl2space` and `longest` are
what establish the rule: see `results.md` §1.
"""
import os
import re
import sys
import zipfile

SRC = "/home/user/corpus-odf/sheets/pagination-001/ods/CIS_Debian_Linux_8_Benchmark_v1.0.0.ods"

PAD_ROW = re.compile(
    r'<table:table-row [^>]*number-rows-repeated="10\d{5}"[^>]*>.*?</table:table-row>',
    re.S)

STYLED_COLS = ('<table:table-column table:style-name="co2" '
               'table:number-columns-repeated="246" '
               'table:default-cell-style-name="ce3"/>')

PARA = re.compile(r'<text:p>(.*?)</text:p>', re.S)


def load(src=SRC):
    z = zipfile.ZipFile(src)
    return {n: z.read(n) for n in z.namelist()}


def write(parts, path):
    if os.path.exists(path):
        os.remove(path)
    z = zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED)
    # `mimetype` first and stored, as the package format requires.
    z.writestr(zipfile.ZipInfo('mimetype'), parts['mimetype'], zipfile.ZIP_STORED)
    for n, b in parts.items():
        if n != 'mimetype':
            z.writestr(n, b)
    z.close()


def variant(name, fn, src=SRC):
    parts = load(src)
    parts['content.xml'] = fn(parts['content.xml'].decode('utf-8')).encode('utf-8')
    p = os.path.abspath(f'{name}.ods')
    write(parts, p)
    return p


def drop_padding_rows(c):
    return PAD_ROW.sub('', c)


def pad_rows(n):
    def go(c):
        return PAD_ROW.sub(
            f'<table:table-row table:style-name="ro3" table:number-rows-repeated="{n}">'
            '<table:table-cell table:number-columns-repeated="257"/></table:table-row>', c)
    return go


def drop_styled_cols(c):
    return c.replace(STYLED_COLS, '')


def strip_default_cell_style(c):
    return c.replace(
        STYLED_COLS,
        '<table:table-column table:style-name="co2" table:number-columns-repeated="246"/>')


def keep_sheet(name):
    def go(c):
        parts = re.split(r'(<table:table table:name="[^"]*"[^>]*>)', c)
        out = parts[0]
        tail = ''
        for i in range(1, len(parts), 2):
            header = parts[i]
            body, tail = parts[i + 1].split('</table:table>', 1)
            if re.search(r'table:name="([^"]*)"', header).group(1) == name:
                out += header + body + '</table:table>'
        return out + tail
    return go


def nl2space(c):
    return PARA.sub(
        lambda m: '<text:p>' + re.sub(r'\r\n|\n|\r', ' ', m.group(1)) + '</text:p>', c)


def longest(c):
    return PARA.sub(
        lambda m: '<text:p>' + max(re.split(r'\r\n|\n|\r', m.group(1)), key=len) + '</text:p>', c)


VARIANTS = {
    'copy': lambda c: c,
    'norows': drop_padding_rows,
    'nocols': drop_styled_cols,
    'nodefstyle': strip_default_cell_style,
    'nl2space': nl2space,
    'longest': longest,
    'only_lic': keep_sheet('License'),
    'only_l1': keep_sheet('Level 1'),
    'only_l2': keep_sheet('Level 2'),
}

if __name__ == '__main__':
    which = sys.argv[1]
    src = sys.argv[2] if len(sys.argv) > 2 else SRC
    fn = VARIANTS[which] if which in VARIANTS else pad_rows(int(which.removeprefix('pad')))
    print(variant(which, fn, src))
