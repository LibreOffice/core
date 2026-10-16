#!/usr/bin/env python3
"""CORRECTED. What the pivot parts' own `<format>` records ask for, and whether the reference honours it.

    format-census.py <workbook.xlsx> ...

A pivot table may carry `<format>` children naming a `dxfId` and a `pivotArea`, which Calc reads
into `PivotTableFormat` and applies through `maFormatOutput` after `ScDPOutput` has written
(`sc/source/filter/oox/pivottablebuffer.cxx`, `PivotTable::finalizeImport`'s formats loop). This
tree reads none of them.

**This corrects `probes/pivot-res-r108/format-census.py`, whose count of nil was an artefact.**
That copy asked for `root.findall(m('format'))`, which is a *direct child* search, and a
`<format>` is never a direct child of `<pivotTableDefinition>` — it is a child of `<formats>`.
The one line changed here is that path.

The census is per pivot part: how many `<format>` elements it states,
how many name a `dxfId`, how many of those `dxf` entries state anything at all, and — because
`applyBorderFormats`, `applyFontFormats`, `applyPatternFormats`, `applyAlignmentFormats` and
`applyNumberFormats` on the definition itself say whether Excel meant the formats to be used —
what those five flags say.
"""
import sys, os, zipfile
import xml.etree.ElementTree as ET

MAIN = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
REL = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'


def m(t):
    return '{%s}%s' % (MAIN, t)


def flag(el, name):
    v = el.get(name)
    return v not in ('0', 'false') if v is not None else None


def dxfs(z):
    """Index -> which of the five kinds the dxf entry states."""
    if 'xl/styles.xml' not in z.namelist():
        return []
    root = ET.fromstring(z.read('xl/styles.xml'))
    out = []
    for dxf in (root.find(m('dxfs')) if root.find(m('dxfs')) is not None else []):
        kinds = []
        for tag, name in (('font', 'font'), ('fill', 'fill'), ('border', 'border'),
                          ('alignment', 'align'), ('numFmt', 'numfmt'), ('protection', 'prot')):
            if dxf.find(m(tag)) is not None:
                kinds.append(name)
        out.append(kinds)
    return out


def main():
    for path in sys.argv[1:]:
        z = zipfile.ZipFile(path)
        name = os.path.basename(path)
        table = dxfs(z)
        total = 0
        for part in sorted(n for n in z.namelist()
                           if n.startswith('xl/pivotTables/') and n.endswith('.xml')):
            root = ET.fromstring(z.read(part))
            formats = list(root.findall('%s/%s' % (m('formats'), m('format'))))
            if not formats:
                continue
            total += len(formats)
            withid = [f for f in formats if f.get('dxfId') is not None]
            stated = []
            for f in withid:
                at = int(f.get('dxfId'))
                if 0 <= at < len(table) and table[at]:
                    stated.append(','.join(table[at]))
            flags = ' '.join(
                '%s=%s' % (k, root.get(k))
                for k in ('applyNumberFormats', 'applyBorderFormats', 'applyFontFormats',
                          'applyPatternFormats', 'applyAlignmentFormats')
                if root.get(k) is not None)
            print('%s\t%s\tformats=%d dxfId=%d stating=%d [%s]\t%s'
                  % (name, os.path.basename(part), len(formats), len(withid), len(stated),
                     ';'.join(sorted(set(stated))), flags))
        if total == 0:
            print('%s\t-\tno <format> in any pivot part' % name)


main()
