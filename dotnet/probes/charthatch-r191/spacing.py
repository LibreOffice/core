"""The perpendicular spacing of a page's hatch, read from consecutive strokes at one corner."""
import collections, math, re, subprocess, sys
S='/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py'
pat=re.compile(r'stroke p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(#\w+)')
for i in range(0, len(sys.argv)-1, 2):
    label, pdf = sys.argv[i+1], sys.argv[i+2]
    out=subprocess.run(['python3',S,'dump',pdf,'--page','1','--only','stroke'],
                       capture_output=True,text=True).stdout
    segs=[(float(m.group(1)),float(m.group(2)),float(m.group(3)),float(m.group(4)))
          for m in pat.finditer(out) if m.group(5)=='#F2F2F2']
    if not segs: print(f'{label}: no hatch'); continue
    # Horizontal lines have a zero-height box; a diagonal family is read from the step of its
    # bounding boxes' shared corner.
    horiz=[s for s in segs if abs(s[3]-s[1])<0.05]
    if len(horiz)>len(segs)/2:
        ys=sorted({round(s[1],3) for s in horiz})
        gaps=collections.Counter(round(b-a,3) for a,b in zip(ys,ys[1:]))
        step=min(g for g in gaps if g>0)
        print(f'{label}: {len(segs)} strokes, HORIZONTAL, spacing {step:.4f} pt '
              f'({step/72*25.4*100:.1f} of 1/100 mm)')
        continue
    xs=sorted({round(s[0],3) for s in segs})
    gaps=collections.Counter(round(b-a,3) for a,b in zip(xs,xs[1:]))
    step=min(g for g in gaps if g>0.05)
    perp=step/math.sqrt(2)
    print(f'{label}: {len(segs)} strokes, 45deg, corner step {step:.4f} -> spacing {perp:.4f} pt '
          f'({perp/72*25.4*100:.1f} of 1/100 mm)')
