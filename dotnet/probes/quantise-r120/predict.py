#!/usr/bin/env python3
"""The reference's own text-line thickness, predicted from a face and a size.

Chain, all of it read out of the 27.2 tree:

  1. `PDFExport::ExportSelection` records every page into a GDIMetaFile whose map mode is
     MapUnit::Map100thMM, on the PDF writer's own reference device -- RefDevMode::PDF1, 720 dpi
     (filter/source/pdf/pdfexport.cxx:168-179; vcl/source/pdf/pdfwriter_impl.cxx:424-428;
     vcl/source/gdi/virdev.cxx:409-411).
  2. The font is instantiated at a whole number of those pixels.
  3. FontMetricData::ImplInitTextLineSize answers a whole number of them for the thickness
     (vcl/source/font/fontmetric.cxx:200-330), on one of two branches.
  4. PDFWriterImpl::drawStraightTextLine converts it with HCONV = DevicePixelToLogicHeight =
     CoordinateMapper::ViewToLogicDistanceY, an llround to a whole 1/100 mm
     (pdfwriter_impl.cxx:6751; vcl/source/outdev/CoordinateMapper.cxx:278-283).
  5. PDFPage::appendMappedLength writes it in thousandths of a point
     (vcl/source/pdf/pdfwriter_utils.hxx:40).
"""
import sys
sys.path.insert(0, '.')
from fontmetrics import metrics

DPI = 720
UNITS_PER_INCH = 2540          # 1/100 mm
NO_UNDERLINE_METRICS = ('Liberation Serif', 'Liberation Sans', 'Liberation Mono')


def rnd(x):                    # C's round / llround: half away from zero
    return int(x + 0.5) if x >= 0 else -int(-x + 0.5)


def px_em(size_pt):
    """The em in whole 720 dpi pixels, through the 1/100 mm map unit the metafile is in."""
    mm100 = rnd(size_pt * UNITS_PER_INCH / 72.0)
    return rnd(mm100 * DPI / UNITS_PER_INCH)


def to_mm100(px):
    return rnd(px * UNITS_PER_INCH / DPI)


def line_metrics(m):
    """ImplCalcLineSpacing's precedence: hhea, then OS/2 win only if listed, then typo if asked."""
    asc, desc = m['hheaAsc'], -m['hheaDesc']
    if asc == 0 and desc == 0:
        asc, desc = m['winAsc'], m['winDesc']
    if m['useTypo'] and m['typoAsc'] >= 0 and m['typoDesc'] <= 0:
        asc, desc = m['typoAsc'], -m['typoDesc']
    return asc, desc


def widths_px(m, family, size_pt):
    em = px_em(size_pt)
    upem = m['upem']
    if family not in NO_UNDERLINE_METRICS and m['ulThick'] > 0 and m['strikeSize'] > 0:
        import math
        scale = em / upem
        single = m['ulThick'] * scale
        return (math.ceil(single), math.ceil(single * 2 / 3),
                math.ceil(m['strikeSize'] * scale))
    asc_u, desc_u = line_metrics(m)
    asc = rnd(asc_u * em / upem)
    desc = rnd(desc_u * em / upem)
    if desc <= 0:
        desc = max(1, asc // 10)
    if 3 * desc > asc:
        desc = asc // 3
    single = max(1, ((desc * 25) + 50) // 100)
    dbl = max(1, ((desc * 16) + 50) // 100)
    return single, dbl, single


def predict(path, family, size_pt):
    m = metrics(path)
    s, d, k = widths_px(m, family, size_pt)
    return {kind: (px, to_mm100(px), rnd(to_mm100(px) * 72000 / UNITS_PER_INCH) / 1000.0)
            for kind, px in (('single', s), ('double', d), ('strike', k))}


if __name__ == '__main__':
    F = '/usr/share/fonts/truetype'
    cases = [
        ('O64 row 1  invoice p1, Arial->Liberation Sans 10 pt, single',
         f'{F}/liberation/LiberationSans-Regular.ttf', 'Liberation Sans', 10, 'single', 18),
        ('O64 row 2  invoice p3, 12 pt Liberation Sans, single',
         f'{F}/liberation/LiberationSans-Regular.ttf', 'Liberation Sans', 12, 'single', 21),
        ('O64 row 3  invoice p3, 10 pt Liberation Sans, single',
         f'{F}/liberation/LiberationSans-Regular.ttf', 'Liberation Sans', 10, 'single', 18),
        ('O64 row 4  fixture, Liberation Sans 12 pt, single',
         f'{F}/liberation/LiberationSans-Regular.ttf', 'Liberation Sans', 12, 'single', 21),
        ('O64 row 5  002_Contextures, Calibri->Carlito 14 pt, single',
         f'{F}/crosextra/Carlito-Regular.ttf', 'Carlito', 14, 'single', 49),
        ('O64 row 6  TK-Syllabus, Carlito 11 pt, strikethrough',
         f'{F}/crosextra/Carlito-Regular.ttf', 'Carlito', 11, 'strike', 28),
        ('O64 row 7  fixture, Liberation Sans 12 pt, double',
         f'{F}/liberation/LiberationSans-Regular.ttf', 'Liberation Sans', 12, 'double', 14),
    ]
    ok = 0
    for label, path, family, size, kind, expected in cases:
        got = predict(path, family, size)[kind]
        hit = got[1] == expected
        ok += hit
        print('%-62s px %-3d -> %3d /100mm (%.3f pt)  expected %3d  %s'
              % (label, got[0], got[1], got[2], expected, 'OK' if hit else 'MISS'))
    print('%d of %d' % (ok, len(cases)))
