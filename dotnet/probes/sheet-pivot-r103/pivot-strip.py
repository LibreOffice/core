#!/usr/bin/env python3
"""What a pivot table's generated borders are worth, document by document.

26.2.4.2 generates `Pivot Table *` cell styles when it imports a pivot table, and its own
`.ods` conversion of the same workbook writes those styles out as ordinary markup -- which is
why the `.ods` twin renders correctly and the `.xlsx` does not (r100 section 1.4).

This takes each twin and removes `fo:border*` from exactly the automatic cell styles whose
parent is a `Pivot_20_Table_20_*` style, leaving every other border in the document alone.
Rendering the result against the banked `.ods` reference says how much of that document the
pivot's generated grid is worth -- an upper bound on what implementing it would recover.

    pivot-strip.py <out.ods> <in.ods>
"""
import re, sys, zipfile


def strip(content: str) -> tuple[str, int]:
    out, count = [], 0
    for chunk in re.split(r'(?=<style:style )', content):
        if (chunk.startswith('<style:style ')
                and 'style:family="table-cell"' in chunk
                and 'style:parent-style-name="Pivot_20_Table_20_' in chunk):
            chunk, n = re.subn(r'\sfo:border(-bottom|-left|-right|-top)?="[^"]*"', '', chunk)
            count += n
        out.append(chunk)
    return ''.join(out), count


def main() -> None:
    dst, src = sys.argv[1], sys.argv[2]
    zin = zipfile.ZipFile(src)
    zo = zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED)
    removed = 0
    for item in zin.infolist():
        data = zin.read(item.filename)
        if item.filename in ('content.xml', 'styles.xml'):
            text, n = strip(data.decode('utf-8'))
            removed += n
            data = text.encode('utf-8')
        zo.writestr(item, data)
    zo.close()
    zin.close()
    print(f'{removed}\t{src}')


if __name__ == '__main__':
    main()
