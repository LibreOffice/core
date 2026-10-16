import sys, glob, os, math, pymupdf
print(f"{'frame':>6s} {'diagram':>8s} {'0.95*d/5':>9s}  {'catlines':>8s} {'pitch':>7s}  verdict")
for path in sorted(glob.glob(sys.argv[1] + "/*.pdf")):
    F = float(os.path.basename(path)[1:-4])
    d = pymupdf.open(path); p = d[0]
    lines, rot = [], False
    for b in p.get_text('dict')['blocks']:
        if b['type'] != 0: continue
        for l in b['lines']:
            t = ''.join(s['text'] for s in l['spans'])
            if 'nn' not in t: continue
            lines.append(l)
            if abs(l['dir'][1]) > 1e-6: rot = True
    centres = []
    for b in p.get_text('dict')['blocks']:
        if b['type'] != 0: continue
        for l in b['lines']:
            t = ''.join(s['text'] for s in l['spans']).strip()
            if t.isdigit() and len(t) == 2 and l['spans'][0]['font'].startswith('Carlito-Bold'):
                centres.append((l['bbox'][0]+l['bbox'][2])/2)
    centres.sort()
    gaps = [centres[i+1]-centres[i] for i in range(len(centres)-1)]
    big = [g for g in gaps if g > 15]
    pitch = sum(big)/len(big) if big else float('nan')
    verdict = 'ROTATED-outlined' if not lines else ('ROTATED' if rot else 'upright')
    dia = 0.96*F
    print(f"{F:6.0f} {dia:8.2f} {0.95*dia/5:9.2f}  {len(lines):8d} {pitch:7.2f}  {verdict}")
