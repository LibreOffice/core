#!/usr/bin/env python3
"""Freeze a workbook's volatile date functions at the serial its own cache was built with.

The reference recalculates `TODAY()` on load and we print the value cached in the
file, so a workbook holding one drifts further from the reference every day and its
gate row says nothing about rendering.  Replacing `TODAY()` with the serial the
cached values were computed at makes both sides agree on the data, so whatever is
left of the divergence is the renderer's.

The serial is read out of the file: a cell whose formula is exactly `TODAY()` (or
`TODAY()+N` / `TODAY()-N`) carries the answer in its own `<v>`.
"""
import re, shutil, sys, zipfile
from datetime import date
from pathlib import Path

CELL = re.compile(rb'<c [^>]*r="([A-Z]+\d+)"[^>]*>(.*?)</c>', re.S)
FV = re.compile(rb'<f[^>]*>([^<]*)</f>\s*<v>([^<]*)</v>', re.S)
OFFSET = re.compile(rb'^TODAY\(\)\s*([+-]\s*(\d+))?$')
# `DATE(YEAR(TODAY()),M,D)` caches a serial that is itself a date in the year the cache
# was built, so the cached value substitutes for TODAY() directly.
YEARDATE = re.compile(rb'^DATE\(YEAR\(TODAY\(\)\),\s*\d+\s*,\s*\d+\s*\)$')
# `"Year " & YEAR(TODAY())` caches the year as text; 1 July of it is as good as any day.
YEARTEXT = re.compile(rb'YEAR\(TODAY\(\)\)$')


def serial_of(zf):
    """The date serial the workbook's cached values were computed at, or None.

    Three shapes answer it, in decreasing directness, and they are tried in that
    order rather than in file order — a workbook holding several states the same
    day in all of them, but only the first is exact.
    """
    best = {}
    for name in zf.namelist():
        if not name.startswith('xl/worksheets/') or not name.endswith('.xml'):
            continue
        data = zf.read(name)
        for m in CELL.finditer(data):
            fv = FV.search(m.group(2))
            if not fv:
                continue
            f = fv.group(1).strip()
            raw = fv.group(2)
            o = OFFSET.match(f)
            if o:
                try:
                    v = float(raw)
                except ValueError:
                    continue
                off = int(o.group(2) or 0) if not o.group(1) or b'-' not in o.group(1) \
                    else -int(o.group(2))
                best.setdefault(0, int(round(v - off)))
            elif YEARDATE.match(f):
                try:
                    best.setdefault(1, int(round(float(raw))))
                except ValueError:
                    pass
            elif YEARTEXT.search(f):
                m2 = re.search(rb'(\d{4})', raw)
                if m2:
                    best.setdefault(2, (date(int(m2.group(1)), 7, 1)
                                        - date(1899, 12, 30)).days)
    for k in (0, 1, 2):
        if k in best:
            return best[k]
    return None


def patch(src, dst, serial=None):
    with zipfile.ZipFile(src) as zf:
        if serial is None:
            serial = serial_of(zf)
        if serial is None:
            return None
        rep = str(serial).encode()
        with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as out:
            for i in zf.infolist():
                d = zf.read(i.filename)
                if i.filename.startswith('xl/') and i.filename.endswith('.xml'):
                    d = d.replace(b'TODAY()', rep).replace(b'NOW()', rep)
                out.writestr(i, d)
    return serial


if __name__ == '__main__':
    src, dst = Path(sys.argv[1]), Path(sys.argv[2])
    s = patch(src, dst, int(sys.argv[3]) if len(sys.argv) > 3 else None)
    if s is None:
        shutil.copy(src, dst)
        print(f'{src.name}\tno-serial')
    else:
        print(f'{src.name}\t{s}')
