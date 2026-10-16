#!/usr/bin/env python3
"""O17's decomposition: which half of the pivot table's generated formatting costs the ink.

`alle einzeln.ods` is 26.2.4.2's own conversion of `alle einzeln.xlsx`, so it carries the
`Pivot Table *` cell styles the reference GENERATES on import as ordinary markup.  This tree
renders that spelling at 0.02 summed unsigned ink over 186 pages and the `.xlsx` at 225.44,
which is the whole seat in one pair of numbers.

Stripping one half of those styles out of the `.ods` and rendering that says which half:

    ours(.ods)                       0.02   0 MAJOR   -- the control
    ours(.ods less the borders)    229.12  36 MAJOR
    ours(.ods less align+indent)    19.85   4 MAJOR
    ours(.xlsx)                    225.44  36 MAJOR   -- neither

Written to be run as `o17-variants.py <outdir>`; scoring is `pdf-image-diff.py` against
/home/user/gate-odf-r80/ref/alle einzeln__ods.pdf.
"""
import os, re, sys, zipfile

SRC = '/home/user/corpus-odf/sheets/done-013/ods/alle einzeln.ods'


def rewrite(out, parts, fn):
    zin = zipfile.ZipFile(SRC)
    zo = zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED)
    for item in zin.infolist():
        data = zin.read(item.filename)
        if item.filename in parts:
            data = fn(data.decode('utf-8')).encode('utf-8')
        zo.writestr(item, data)
    zo.close()
    zin.close()


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '.'
    os.makedirs(out, exist_ok=True)

    # Every `fo:border*` on every automatic cell style: the pivot's grid and outer frame.
    rewrite(os.path.join(out, 'noborders.ods'), {'content.xml'},
            lambda s: re.sub(r'\sfo:border(-bottom|-left|-right|-top)?="[^"]*"', '', s))

    # The `Pivot Table Category` indent and the left justification `Category` and `Title` take
    # from their own named style.
    def noalign(s):
        s = re.sub(r'<style:paragraph-properties fo:margin-left="[^"]*"/>', '', s)
        return s.replace(
            '<style:table-cell-properties style:text-align-source="fix" '
            'style:repeat-content="false"/><style:paragraph-properties fo:text-align="start"/>',
            '')

    rewrite(os.path.join(out, 'noalign.ods'), {'content.xml', 'styles.xml'}, noalign)
    print('wrote noborders.ods and noalign.ods into', out)


if __name__ == '__main__':
    main()
