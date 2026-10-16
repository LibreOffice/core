#!/usr/bin/env python3
"""What the "lay the frame out nominally and stretch it" transform actually costs.

Round 94 §8 left `NAS-…-Weather.pptx`'s "table transform" with its seat: *we lay a
`p:graphicFrame` table out nominally and stretch it by 1.3151 x 1.3134; the reference lays
it out at its final size … could cost more where the factor is further from unity.*

Two things to measure and one to correct.  The frame is **not** a DrawingML table: its
`graphicData` uri is `.../presentationml/2006/ole` and it holds a
`p:oleObj progId="Excel.Sheet.12"`, so this is an embedded worksheet, not an `a:tbl`.  And
the cost is not the factor, it is what the factor leaves on the page: this prints the `cm`
our renderer emits, the drawn sizes on both sides, and the origin divergence of every text
record the two sides share.

    ole-transform.py <ours.pdf> <ref.pdf> <page> [<size-filter>]
"""
import importlib.util, math, pathlib, re, sys

HERE = pathlib.Path(__file__).resolve().parent
# <repo>/dotnet/probes/<this dir>/<this file> -> <repo>/.claude/skills/…
SKILL = (pathlib.Path(__file__).resolve().parents[3]
         / '.claude/skills/render-comparison/scripts/pdf-ops.py')
spec = importlib.util.spec_from_file_location('pdfops', SKILL)
po = importlib.util.module_from_spec(spec)
spec.loader.exec_module(po)

CM = re.compile(r'(-?[\d.]+) (-?[\d.]+) (-?[\d.]+) (-?[\d.]+) (-?[\d.]+) (-?[\d.]+) cm')


def stream(pdf, page):
    blob = pathlib.Path(pdf).read_bytes()
    objects = po.Objects(blob)
    for number, body in enumerate(objects.pages(), start=1):
        if number != page: continue
        m = re.search(rb"/Contents\s*(\[[^\]]*\]|\d+\s+\d+\s+R)", body)
        out = b""
        for ref in re.finditer(rb"(\d+)\s+\d+\s+R", m.group(1)):
            part = objects.raw(int(ref.group(1)))
            if part: out += objects.stream_of(part) + b"\n"
        return out.decode('latin-1')
    return ''


def main(ours, ref, page, want=None):
    for side, pdf in (('ours', ours), ('ref', ref)):
        seen = {}
        for m in CM.finditer(stream(pdf, page)):
            if m.group(2) != '0' or m.group(3) != '0': continue
            sx, sy = float(m.group(1)), float(m.group(4))
            if sx == 1.0 and sy == 1.0: continue
            seen[(m.group(1), m.group(4))] = seen.get((m.group(1), m.group(4)), 0) + 1
        rows = ['%s x %s (x%d)' % (a, b, n) for (a, b), n in sorted(seen.items(), key=lambda kv: -kv[1])]
        print('%s\tnon-unit scale cm ops: %s' % (side, '; '.join(rows) if rows else 'none'))

    a = [r for r in po.read(ours) if r['page'] == page and r['kind'] == 'text']
    b = [r for r in po.read(ref) if r['page'] == page and r['kind'] == 'text']
    if want is not None:
        a = [r for r in a if abs(r['size'] - want) < 0.5]
        b = [r for r in b if abs(r['size'] - want) < 0.5]
    print('text records\tours %d\tref %d' % (len(a), len(b)))
    if len(a) != len(b):
        print('unequal record counts; not paired')
        return
    key = lambda r: (round(r['y'], 0), round(r['x'], 0))
    pairs = list(zip(sorted(a, key=key), sorted(b, key=key)))
    d = sorted(math.hypot(x['x'] - y['x'], x['y'] - y['y']) for x, y in pairs)
    print('drawn size\tours %s\tref %s' % (
        sorted({round(r['size'], 4) for r in a}), sorted({round(r['size'], 4) for r in b})))
    print('origin divergence pt\tmax %.3f\tmean %.3f\tmedian %.3f' % (
        d[-1], sum(d) / len(d), d[len(d) // 2]))


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], int(sys.argv[3]),
         float(sys.argv[4]) if len(sys.argv) > 4 else None)
