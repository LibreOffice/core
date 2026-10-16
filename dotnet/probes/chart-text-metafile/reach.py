#!/usr/bin/env python3
"""How much of the corpus draws chart text, and how far the 96 dpi round trip moves it.

A chart's text is measured on `DrawModelWrapper`'s reference `VirtualDevice`, which is 96 dpi
headless. An `OutputDevice` selects a font at a whole number of device pixels, so a run whose
height is H hundredths of a millimetre is laid out at `round(H x 96 / 2540)` pixels and every
advance in it is scaled by `round(px) / px`. The scale is a pure function of the declared font
size, so the reach can be computed from the documents rather than measured on all of them --
`chart-gap.py` and `chart-vs-textbox.py` are what establish that the function is right.

This walks `MANIFEST.tsv`, finds every document that carries a chart, reads the text sizes the
chart declares, and reports the worst predicted per-glyph error on each.

Usage:  python3 reach.py <corpus root>
"""

import os
import re
import subprocess
import sys
import zipfile

OOXML_CHART = re.compile(r'(?:^|/)charts?/chart\d*\.xml$', re.I)
ODF_CHART = re.compile(r'^Object ?\d*/content\.xml$|^ObjectReplacements/', re.I)
# The names an embedded chart's OLE2 storage carries, as UTF-16LE in the raw file.
OLE_CHART = [name.encode('utf-16-le') for name in
             ('MSGraph.Chart', 'Microsoft Graph Chart', 'StarChart', 'sch.SmartArt')]


def error_at(size_pt: float) -> float:
    """The per-glyph relative error the 96 dpi whole-pixel font height applies at this size."""
    height = round(size_pt * 2540.0 / 72.0)          # the chart model's 1/100 mm height
    px = height * 96.0 / 2540.0
    if px < 1:
        return 0.0
    return round(px) / px - 1.0


def ooxml_sizes(archive: zipfile.ZipFile, names: list[str]) -> set[float]:
    found: set[float] = set()
    for name in names:
        text = archive.read(name).decode('utf-8', 'replace')
        for m in re.finditer(r'\bsz="(\d+)"', text):
            found.add(int(m.group(1)) / 100.0)
    return found or {10.0}   # LibreOffice's own chart default when nothing is declared


def odf_sizes(archive: zipfile.ZipFile, names: list[str]) -> set[float]:
    found: set[float] = set()
    for name in names:
        text = archive.read(name).decode('utf-8', 'replace')
        for m in re.finditer(r'fo:font-size="([\d.]+)pt"', text):
            found.add(float(m.group(1)))
    return found or {10.0}


def charts_in(path: str) -> tuple[str, set[float]] | None:
    """('ooxml'|'odf'|'ole', the chart text sizes) if the document carries a chart."""
    try:
        with zipfile.ZipFile(path) as archive:
            names = archive.namelist()
            ooxml = [n for n in names if OOXML_CHART.search(n)]
            if ooxml:
                return 'ooxml', ooxml_sizes(archive, ooxml)
            odf = [n for n in names
                   if ODF_CHART.match(n) and n.endswith('content.xml')
                   and b'chart:chart' in archive.read(n)]
            if odf:
                return 'odf', odf_sizes(archive, odf)
            if any(n.endswith('content.xml') for n in names):
                flat = archive.read('content.xml') if 'content.xml' in names else b''
                if b'<chart:chart' in flat:
                    return 'odf', odf_sizes(archive, ['content.xml'])
            return None
    except (zipfile.BadZipFile, KeyError, OSError):
        pass
    try:
        with open(path, 'rb') as handle:
            head = handle.read(64 * 1024 * 1024)
    except OSError:
        return None
    if head[:8] == b'\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1' and any(n in head for n in OLE_CHART):
        return 'ole', set()
    if b'<chart:chart' in head:                      # a flat ODF document
        return 'odf', {float(m.group(1)) for m in
                       re.finditer(rb'fo:font-size="([\d.]+)pt"', head)} or {10.0}
    return None


def main() -> int:
    root = os.path.abspath(sys.argv[1])
    manifest = os.path.join(root, 'MANIFEST.tsv')
    print('# reach. Measured ' + subprocess.run(
        ['date', '-u', '+%Y-%m-%d'], capture_output=True, text=True).stdout.strip()
        + f'. corpus {root}, scored from MANIFEST.tsv')
    print('# a prediction from the documents, not a rendering: the reference binaries it is'
          ' calibrated against are 24.2.7.2 (/usr/bin/soffice) and 26.2.4.2'
          ' (/opt/libreoffice26.2/program/soffice, its Latin metric duplicates, its Latin'
          ' NotoSans/NotoSerif and opens___.ttf moved aside), system fonts /usr/share/fonts')
    print('# the rule and its residuals are measured in chart-gap.py and chart-vs-textbox.py')
    print('# predicted per-glyph error = round(px96)/px96 - 1 at the chart\'s declared text'
          ' sizes, px96 = round(size_pt x 2540/72) x 96/2540')
    print('family\tpath\tkind\tsizes\tworst_error_pct\tworst_size_pt')

    rows = 0
    hits: dict[str, int] = {}
    worst: list[tuple[float, str, str, float]] = []
    with open(manifest, encoding='utf-8') as handle:
        header = handle.readline().rstrip('\n').split('\t')
        family_at, path_at = header.index('family'), header.index('path')
        for line in handle:
            fields = line.rstrip('\n').split('\t')
            if len(fields) <= path_at:
                continue
            rows += 1
            document = os.path.join(root, fields[path_at])
            if not os.path.isfile(document):
                continue
            found = charts_in(document)
            if found is None:
                continue
            kind, sizes = found
            hits[fields[family_at]] = hits.get(fields[family_at], 0) + 1
            if sizes:
                size = max(sizes, key=lambda s: abs(error_at(s)))
                error = error_at(size) * 100.0
            else:
                size, error = 0.0, float('nan')
            worst.append((abs(error) if error == error else -1.0,
                          fields[family_at], fields[path_at], size))
            print(f'{fields[family_at]}\t{fields[path_at]}\t{kind}'
                  f'\t{",".join(f"{s:g}" for s in sorted(sizes))}\t{error:+.2f}\t{size:g}')

    print(f'\n# {rows} manifest rows, {sum(hits.values())} carry a chart: '
          + ', '.join(f'{k} {v}' for k, v in sorted(hits.items())))
    worst.sort(reverse=True)
    print('# the ten worst predicted, by |error| at the chart\'s own declared sizes')
    for error, family, path, size in worst[:10]:
        print(f'#   {error:5.2f}%  {size:5g} pt  {family}/{os.path.basename(path)}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
