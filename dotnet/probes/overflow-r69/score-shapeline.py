#!/usr/bin/env python3
"""Score three candidate shape line-height laws against a reference rendering.

    gen-shapeline.py shapeline.xlsx
    soffice --headless --convert-to pdf --outdir ref shapeline.xlsx
    score-shapeline.py ref/shapeline.pdf

Sixteen wrapping text boxes, four faces x four sizes, on one unscaled sheet, so a baseline
pitch is read straight off the reference's own text origins with nothing else in the way.
`pdf-ops.py dump` gives (page, origin, drawn size, face) per line; boxes are grouped by
origin x, and only those wrapping to three lines or more are scored.
"""
import collections, os, re, struct, subprocess, sys

OPS = os.path.join(os.path.dirname(__file__),
                   '../../../.claude/skills/render-comparison/scripts/pdf-ops.py')

FACES = {
    'LiberationSerif': '/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf',
    'LiberationSans': '/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf',
    'DejaVuSans': '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',
    'Carlito-Regular': '/usr/share/fonts/truetype/crosextra/Carlito-Regular.ttf',
}


def metrics(path):
    d = open(path, 'rb').read()
    tables = {}
    for i in range(struct.unpack('>H', d[4:6])[0]):
        o = 12 + 16 * i
        tables[d[o:o + 4].decode()] = struct.unpack('>II', d[o + 8:o + 16])
    upem = struct.unpack('>H', d[tables['head'][0] + 18:tables['head'][0] + 20])[0]
    asc, desc, gap = struct.unpack('>hhh', d[tables['hhea'][0] + 4:tables['hhea'][0] + 10])
    return upem, asc, -desc, gap


def pixels(design, upem, size_pt, dpi, per_inch):
    em = round(size_pt / 72 * per_inch / (per_inch / dpi) + 1e-9)
    return round(design * em / upem + 1e-9)


def to_pt(px, dpi, per_inch):
    return round(px * (per_inch / dpi) + 1e-9) / per_inch * 72


def main(pdf):
    out = subprocess.run(['python3', OPS, 'dump', pdf], capture_output=True, text=True).stdout
    rows = re.findall(
        r'^text\s+p(\d+)\s+\(\s*([-\d.]+),\s*([-\d.]+)\)\s+([\d.]+)pt\s+\S*\+(\S+)', out, re.M)
    if not rows:
        sys.exit('pdf-ops produced no text records for ' + pdf)

    boxes = collections.defaultdict(list)
    for page, x, y, size, face in rows:
        boxes[(page, round(float(x), 1), face, size)].append(float(y))

    faces = {k: metrics(v) for k, v in FACES.items() if os.path.exists(v)}
    error = collections.Counter()
    scored = 0
    print(f"{'face':18s}{'size':>7s}{'lines':>6s}{'pitch':>9s}"
          f"{'+leading':>10s}{'asc+desc':>10s}{'720 dpi':>9s}")
    for (page, x, face, size), ys in sorted(boxes.items()):
        if len(ys) < 3 or face not in faces:
            continue
        ys.sort(reverse=True)
        pitch = (ys[0] - ys[-1]) / (len(ys) - 1)
        upem, asc, desc, gap = faces[face]
        pt = float(size)
        leading = pt * (asc + desc + gap) / upem
        plain = pt * (asc + desc) / upem
        ap, dp = (pixels(m, upem, pt, 720, 2540) for m in (asc, desc))
        grid = max(to_pt(ap, 720, 2540) + to_pt(dp, 720, 2540), to_pt(ap + dp, 720, 2540))
        print(f'{face:18s}{size:>7s}{len(ys):6d}{pitch:9.3f}'
              f'{leading:10.3f}{plain:10.3f}{grid:9.3f}')
        for key, value in (('+leading', leading), ('asc+desc', plain), ('720 dpi', grid)):
            error[key] += abs(value - pitch)
        scored += 1

    print(f'\n{scored} boxes scored; mean |error| in points:')
    for key, total in error.most_common():
        print(f'  {key:10s} {total / scored:.4f}')


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else 'ref/shapeline.pdf')
