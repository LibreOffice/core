#!/usr/bin/env python3
"""Score a sweep rendering against the predicted thickness.

    score.py <manifest.tsv> <rows.tsv> <module> [--tol pt]

`module` is `fodt`, `fods` or `fodp` and decides the LOGICAL UNIT the thickness is rounded
onto -- which is a property of the application whose page was recorded, not of the PDF
writer.  `PDFExport::ExportSelection` records every page in `MapUnit::Map100thMM`, but the
recording device's map mode is whatever the application sets while it paints, and Writer
paints in twips.  So the same 720 dpi pixel count comes out as a whole hundredth of a
millimetre from Calc and Impress and as a whole twip from Writer.
"""
import sys
sys.path.insert(0, __file__.rsplit('/', 1)[0])
from predict import widths_px, rnd  # noqa: E402
from fontmetrics import metrics     # noqa: E402

FILES = {
    'Liberation Sans': '/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf',
    'Liberation Serif': '/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf',
    'Liberation Mono': '/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf',
    'Carlito': '/usr/share/fonts/truetype/crosextra/Carlito-Regular.ttf',
    'Caladea': '/usr/share/fonts/truetype/crosextra/Caladea-Regular.ttf',
    'DejaVu Sans': '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',
}
UNITS_PER_INCH = {'fodt': 1440, 'fods': 2540, 'fodp': 2540}
KIND_INDEX = {'single': 0, 'double': 1, 'strike': 2}


def main(manifest, rows, module, tol=0.004):
    upi = UNITS_PER_INCH[module]
    man = {}
    for line in open(manifest).read().splitlines()[1:]:
        label, face, size, kind = line.split('\t')
        man[label] = (face, int(size), kind)
    faces = {f: metrics(p) for f, p in FILES.items()}

    hit = miss = 0
    for line in open(rows).read().splitlines()[1:]:
        label, count, drawn = line.split('\t')
        face, size, kind = man[label]
        px = widths_px(faces[face], face, size)[KIND_INDEX[kind]]
        logical = rnd(px * upi / 720.0)
        want = rnd(logical * 72000.0 / upi) / 1000.0
        got = float(drawn)
        ok = abs(got - want) <= tol
        hit += ok
        miss += not ok
        if not ok:
            print('MISS %-6s %-17s %2d %-6s  px %-3d -> %-4d units  want %.4f  drew %.4f  n=%s'
                  % (label, face, size, kind, px, logical, want, got, count))
    print('%s: %d of %d exact within %.3f pt' % (module, hit, hit + miss, tol))
    return miss


if __name__ == '__main__':
    sys.exit(1 if main(sys.argv[1], sys.argv[2], sys.argv[3]) else 0)
