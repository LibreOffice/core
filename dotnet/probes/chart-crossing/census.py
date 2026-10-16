#!/usr/bin/env python3
"""Which corpus charts put their category axis inside the plot rather than at its edge.

`c:catAx/c:crosses val="autoZero"` — the default — asks for the value zero on the axis it
crosses, and `ChartAxisPosition_ZERO` is `m_pfMainLinePositionAtOtherAxis = 0.0`
(chart2/source/view/axes/VAxisProperties.cxx:224-225), clamped into the value range by
`VCartesianAxis::get2DAxisMainLine` (:1253-1256). So the axis line — and, under the default
`c:tickLblPos val="nextTo"`, its labels, through `getLabelLineIntersectionValue` (:1103-1113) —
stands inside the plot exactly when the plotted values span zero.

    census.py [corpus] > census.tsv
"""
import re, sys, zipfile
from pathlib import Path

CORPUS = Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files')


def axes(part):
    for m in re.finditer(r'<c:(catAx|dateAx)>(.*?)</c:\1>', part, re.S):
        body = m.group(2)
        def val(tag):
            got = re.search(r'<c:%s val="([^"]*)"/>' % tag, body)
            return got.group(1) if got else ''
        yield m.group(1), val('crosses'), val('crossesAt'), val('tickLblPos'), val('delete')


def values(part):
    """Every plotted number, from the caches, which is what the axis' range is derived from."""
    out = []
    for m in re.finditer(r'<c:val>(.*?)</c:val>', part, re.S):
        for v in re.findall(r'<c:v>(-?[0-9.eE+-]+)</c:v>', m.group(1)):
            try:
                out.append(float(v))
            except ValueError:
                pass
    return out


print('path\tpart\taxis\tcrosses\tcrossesAt\ttickLblPos\tdeleted\tmin\tmax\tspans_zero')
for path in sorted(CORPUS.rglob('*')):
    if not path.is_file() or path.suffix.lower() not in ('.docx', '.xlsx', '.xlsm', '.pptx'):
        continue
    try:
        with zipfile.ZipFile(path) as z:
            for info in z.infolist():
                if 'chart' not in info.filename.lower() or not info.filename.endswith('.xml'):
                    continue
                try:
                    part = z.read(info.filename).decode('utf-8', 'replace')
                except Exception:
                    continue
                if 'chartSpace' not in part[:4096]:
                    continue
                nums = values(part)
                low = min(nums) if nums else 0.0
                high = max(nums) if nums else 0.0
                spans = low < 0.0 < high
                for kind, crosses, at, lbl, deleted in axes(part):
                    print(f'{path.relative_to(CORPUS)}\t{info.filename}\t{kind}'
                          f'\t{crosses}\t{at}\t{lbl}\t{deleted}'
                          f'\t{low:.4g}\t{high:.4g}\t{int(spans)}')
    except Exception:
        continue
