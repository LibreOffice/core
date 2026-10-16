#!/usr/bin/env python3
"""Per-page geometry of a rendered pair: page box, first/last text y, line count, first text."""
import sys, pymupdf

def rows(path):
    d = pymupdf.open(path)
    out = []
    for p in d:
        words = p.get_text("words")
        if words:
            top = min(w[1] for w in words); bot = max(w[3] for w in words)
            ys = sorted({round(w[1], 1) for w in words})
            # collapse near-equal baselines
            lines = 0; prev = None
            for y in ys:
                if prev is None or y - prev > 1.5:
                    lines += 1; prev = y
            txt = " ".join(w[4] for w in words[:8])
            nch = sum(len(w[4]) for w in words)
        else:
            top = bot = float('nan'); lines = 0; txt = ""; nch = 0
        out.append(dict(page=p.number+1, w=round(p.rect.width,1), h=round(p.rect.height,1),
                        top=round(top,2) if top==top else None,
                        bot=round(bot,2) if bot==bot else None,
                        lines=lines, nch=nch, txt=txt[:60]))
    return out

if __name__ == "__main__":
    a, b = sys.argv[1], sys.argv[2]
    ra, rb = rows(a), rows(b)
    print(f"{'pg':>3} | {'OURS top':>8} {'bot':>7} {'ln':>3} {'ch':>5}  {'text':<42} | {'REF top':>8} {'bot':>7} {'ln':>3} {'ch':>5}  text")
    for i in range(max(len(ra), len(rb))):
        x = ra[i] if i < len(ra) else None
        y = rb[i] if i < len(rb) else None
        f = lambda r: f"{str(r['top']):>8} {str(r['bot']):>7} {r['lines']:>3} {r['nch']:>5}  {r['txt']:<42}" if r else " "*70
        print(f"{i+1:>3} | {f(x)} | {f(y)}")
