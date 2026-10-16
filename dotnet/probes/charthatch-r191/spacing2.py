"""A 45-degree hatch family's perpendicular spacing, from each stroke's own midpoint.

A straight segment's bounding box corners ARE its endpoints, so its midpoint is exact and its
line is fixed by `c = y - x`; two neighbours differ in `c` by `spacing * sqrt(2)`. Reading the
box's MIN corner instead collapses every stroke the clip cut at the same corner onto one `c`,
which is how a first cut of this read 295 strokes as 42 lines.
"""
import collections, math, re, subprocess, sys
S='/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py'
pat=re.compile(r'stroke p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(#\w+)')
for i in range(0, len(sys.argv)-1, 2):
    label, pdf = sys.argv[i+1], sys.argv[i+2]
    out=subprocess.run(['python3',S,'dump',pdf,'--page','1','--only','stroke'],
                       capture_output=True,text=True).stdout
    cs=[]
    for m in pat.finditer(out):
        if m.group(5)!='#F2F2F2': continue
        x0,y0,x1,y1=(float(m.group(k)) for k in (1,2,3,4))
        cs.append(round(((y0+y1)/2)-((x0+x1)/2), 3))
    if not cs: print(f'{label}: no hatch'); continue
    u=sorted(set(cs))
    gaps=collections.Counter(round(b-a,3) for a,b in zip(u,u[1:]))
    step=gaps.most_common(1)[0][0]
    perp=step/math.sqrt(2)
    print(f'{label}: {len(cs)} strokes, {len(u)} lines, modal c-gap {step:.4f} '
          f'-> spacing {perp:.4f} pt = {perp/72*25.4*100:.1f} of 1/100 mm')

# KEPT AS A WARNING. This reads each stroke's bounding-box midpoint and calls it the line's own
# invariant, which is right only where the box's corners are the segment's endpoints. `pdf-ops.py`
# reports a box, and a 45-degree segment of the other diagonal sense has its endpoints at the
# box's OTHER two corners — so this reports 295 strokes as 42 lines and 522 as 3. Read the raw
# content stream's `m`/`l` operators when the question is where a line is; `spacing.py` counts
# strokes instead, which the box cannot corrupt.
