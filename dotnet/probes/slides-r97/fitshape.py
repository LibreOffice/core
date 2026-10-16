#!/usr/bin/env python3
"""How many live `.ppt` slide shapes state `fFitShapeToText`, and how many of those hold text.

The driver behind `fitshape-census.txt`, which the round that produced it did not keep. Escher
states the bit as bit 1 of `DFF_Prop_FitTextToShape` (191) -- `include/svx/msdffdef.hxx`:119 --
and `filter/source/msfilter/svdfppt.cxx`:1051 reads it as `(GetPropertyValue(191, 0) & 2) != 0`.

    fitshape.py <corpus-slides-dir>
"""
import pathlib, sys
sys.path.insert(0, str(pathlib.Path(__file__).parent))
from shapes import read, shapes_of, SLIDE

FIT_TEXT_TO_SHAPE = 191
FIT_SHAPE_TO_TEXT = 2

def main(root):
    files = sorted(q for q in pathlib.Path(root).rglob('*') if q.suffix.lower() == '.ppt')
    total = fitted = withtext = 0
    per = {}
    for p in files:
        try:
            buf, roots = read(str(p))
        except Exception as e:
            print(f'SKIP {p.name}: {e}', file=sys.stderr); continue
        n = f = t = 0
        for kind, off, end in roots:
            if kind != SLIDE: continue
            for s in shapes_of(buf, off + 8, end):
                n += 1
                v = s['props'].get(FIT_TEXT_TO_SHAPE)
                if v and v[0] == 'V' and (v[1] & FIT_SHAPE_TO_TEXT):
                    f += 1
                    t += bool(s['text'])
        total += n; fitted += f; withtext += t
        if f: per[p.name] = (f, t)
    print(f'{len(files)} .ppt, {total} live slide shapes')
    print(f'{fitted} state fFitShapeToText, in {len(per)} documents; '
          f'{withtext} of those hold a ClientTextbox')
    for k, (f, t) in sorted(per.items(), key=lambda kv: -kv[1][1])[:10]:
        print(f'  {k[:56]:56s} {f:4d} / {t:4d} with text')

if __name__ == '__main__':
    main(sys.argv[1])
