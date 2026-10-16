#!/usr/bin/env python3
r"""Predict every rule's THICKNESS and its OFFSET, both lines of a double underline included.

    predict-pos.py <pos-manifest.tsv> <rows.tsv> <module>

`module` is `fodt` (twips) or `fods` (hundredths of a millimetre) and decides the logical
unit the device pixel count is rounded onto -- see `quantise-r120/results.md` for that half.

The chain this adds to round 120's is the OFFSET, which that round did not score at all:

  FontMetricData::ImplInitTextLineSize (vcl/source/font/fontmetric.cxx:200-352) answers a
  size and one or two offsets per kind, all in 720 dpi device pixels.  For a double
  underline the HarfBuzz branch gives

      mnDUnderlineSize    = ceil(2/3 x underlineThickness x scale)          :236
      mnDUnderlineOffset1 = ceil(-underlineOffset x scale - size/2)         :234, :237
      mnDUnderlineOffset2 = mnDUnderlineOffset1 + mnDUnderlineSize x 2      :238

  and the descent branch gives

      n2LineHeight        = ((descent x 16) + 50) / 100, min 1              :297
      n2LineDY            = max(n2LineHeight, 1 + DPIY/150)                 :299-306
      mnDUnderlineOffset1 = (descent/2 + 1) - n2LineDY/2 - n2LineHeight     :318
      mnDUnderlineOffset2 = mnDUnderlineOffset1 + n2LineDY + n2LineHeight   :319

  `1 + DPIY/150` is FIVE pixels on the PDF writer's 720 dpi device, so the gap between the
  two lines is floored at five pixels and the floor binds wherever the double underline is
  thinner than that -- which is every size below about 12 pt for the Liberation faces.
  Note the two different descents: the thicknesses use the #i55341-CLAMPED descent and
  `nUnderlineOffset` uses the raw `mnDescent` (:315).

  PDFWriterImpl::drawStraightTextLine (vcl/source/pdf/pdfwriter_impl.cxx:6740-6861) then
  turns each offset into a stroke centre:

      nOffset = nLineHeight / 2                    integer, tdf#154235      :6741
      line 1 centre = HCONV(offset1 + nOffset)                              :6845-6850
      line 2 centre = HCONV(offset2 + nOffset) + HCONV(nLineHeight)         :6853-6858

  **The `+ nLineHeight` on the second line is not symmetric with the first and is not a
  reading error.**  tdf#154235 added `nOffset` to both so that each states the middle of its
  own line; the second then has a further whole thickness added on top of that, so the two
  centres are three thicknesses apart where the metric put them two apart.  It is
  reproduced here because it is what the reference draws -- confirmed on 84 double rules
  over two modules, and independently on `RobertQ_Service.doc`'s own reference rendering.
"""
import math, sys
from fontmetrics import metrics

DPI = 720
NO_UNDERLINE_METRICS = ('Liberation Serif', 'Liberation Sans', 'Liberation Mono')
UNITS_PER_INCH = {'fodt': 1440, 'fods': 2540, 'fodp': 2540}

FILES = {
    'Liberation Sans': '/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf',
    'Liberation Serif': '/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf',
    'Liberation Mono': '/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf',
    'Carlito': '/usr/share/fonts/truetype/crosextra/Carlito-Regular.ttf',
    'Caladea': '/usr/share/fonts/truetype/crosextra/Caladea-Regular.ttf',
    'DejaVu Sans': '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',
}


def rnd(x):                       # C's round / llround: half away from zero
    return int(x + 0.5) if x >= 0 else -int(-x + 0.5)


def px_em(size_pt, upi):
    return rnd(rnd(size_pt * upi / 72.0) * DPI / upi)


def hconv(px, upi):
    return rnd(px * upi / DPI)


def line_metrics(m):
    asc, desc = m['hheaAsc'], -m['hheaDesc']
    if asc == 0 and desc == 0:
        asc, desc = m['winAsc'], m['winDesc']
    if m['useTypo'] and m['typoAsc'] >= 0 and m['typoDesc'] <= 0:
        asc, desc = m['typoAsc'], -m['typoDesc']
    return asc, desc


def device_metrics(m, family, size_pt, upi):
    """Every size and offset the reference computes, in 720 dpi device pixels."""
    em = px_em(size_pt, upi)
    upem = m['upem']
    out = {}
    if family not in NO_UNDERLINE_METRICS and m['ulThick'] > 0 and m['strikeSize'] > 0:
        scale = em / upem
        n_off = -m['ulPos'] * scale
        n_size = m['ulThick'] * scale
        d_size = math.ceil(n_size * 2.0 / 3.0)
        d_off1 = math.ceil(n_off - n_size / 2.0)
        out['single'] = (math.ceil(n_size), [math.ceil(n_off)])
        out['double'] = (d_size, [d_off1, d_off1 + d_size * 2])
        # mnBUnderlineSize = ceil(nSize * 2), sharing the double's own offset (:232-234)
        out['bold'] = (math.ceil(n_size * 2.0), [d_off1])
        s_size = math.ceil(m['strikeSize'] * scale)
        out['strike'] = (s_size, [math.ceil(-m['strikePos'] * scale)])
        out['branch'] = 'harfbuzz'
        return out

    asc_u, desc_u = line_metrics(m)
    asc = rnd(asc_u * em / upem)
    desc = rnd(desc_u * em / upem)
    clamped = desc
    if clamped <= 0:
        clamped = max(1, asc // 10)
    if 3 * clamped > asc:
        clamped = asc // 3

    line_h = max(1, ((clamped * 25) + 50) // 100)
    line_h2 = max(1, line_h // 2)
    two_h = max(1, ((clamped * 16) + 50) // 100)
    two_dy = max(two_h, 1 + DPI // 150)
    two_dy2 = max(1, two_dy // 2)

    # The internal leading is the part of the line box outside the em -- NOT clamped at zero,
    # `mnIntLeading = mnAscent + mnDescent - mnHeight` and nothing else (fontmetric.cxx:543) --
    # and the strikeout offset is a third of the ascent net of it (:313).
    int_leading = (asc + desc - em) if (asc or desc) else 0
    under_off = desc // 2 + 1
    strike_off = -((asc - int_leading) // 3)        # negative: above the baseline

    d_off1 = under_off - two_dy2 - two_h
    bold_h = ((clamped * 50) + 50) // 100
    if bold_h == line_h:
        bold_h += 1
    bold_h2 = max(1, bold_h // 2)

    out['single'] = (line_h, [under_off - line_h2])
    out['bold'] = (bold_h, [under_off - bold_h2])
    out['double'] = (two_h, [d_off1, d_off1 + two_dy + two_h])
    out['strike'] = (line_h, [strike_off - line_h2])
    out['branch'] = 'descent'
    return out


def drawn(kind, size_px, offsets, upi):
    """The stroke centres the PDF writer emits, in points below the baseline."""
    n_offset = size_px // 2
    height = hconv(size_px, upi)
    pts = [hconv(offsets[0] + n_offset, upi)]
    if len(offsets) > 1:
        pts.append(hconv(offsets[1] + n_offset, upi) + height)
    per_pt = 72.0 / upi
    return height * per_pt, [p * per_pt for p in pts]


def main(manifest, rows, module, tol=0.0015):
    upi = UNITS_PER_INCH[module]
    man = {}
    for line in open(manifest).read().splitlines()[1:]:
        label, face, size, kind = line.split('\t')
        man[label] = (face, int(size), kind)
    faces = {f: metrics(p) for f, p in FILES.items()}

    hit = miss = 0
    for line in open(rows).read().splitlines()[1:]:
        parts = line.split('\t')
        label, count, drawn_s = parts[0], int(parts[1]), (parts[2] if len(parts) > 2 else '')
        face, size, kind = man[label]
        dm = device_metrics(faces[face], face, size, upi)
        size_px, offsets = dm[kind]
        want_h, want_pos = drawn(kind, size_px, offsets, upi)
        got = [tuple(float(v) for v in p.split('@')) for p in drawn_s.split()] if drawn_s else []
        ok = (len(got) == len(want_pos)
              and all(abs(g[0] - w) <= tol and abs(g[1] - want_h) <= tol
                      for g, w in zip(got, want_pos)))
        hit += ok
        miss += not ok
        if not ok:
            print('MISS %-5s %-17s %2d %-6s %-8s want %s  drew %s'
                  % (label, face, size, kind, dm['branch'],
                     ' '.join('%.4f@%.4f' % (p, want_h) for p in want_pos), drawn_s))
    print('%s: %d of %d exact within %.4f pt' % (module, hit, hit + miss, tol))
    return miss


if __name__ == '__main__':
    sys.exit(1 if main(sys.argv[1], sys.argv[2], sys.argv[3]) else 0)
