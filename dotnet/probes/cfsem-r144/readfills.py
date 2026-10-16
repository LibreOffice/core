"""Map painted fills in a rendered PDF back to cells.

Works by locating each cell's own address text (probe sheets write the address
into the cell) and asking which filled rect contains that text's midpoint."""
import sys, fitz

def fills(page):
    out = []
    for d in page.get_drawings():
        if d.get('fill') is None:
            continue
        r = d['rect']
        if r.width < 2 or r.height < 2:
            continue
        out.append((r, tuple(round(c, 3) for c in d['fill'])))
    return out

def main(path, want_hex=True):
    doc = fitz.open(path)
    for pno, page in enumerate(doc):
        fl = fills(page)
        words = page.get_text('words')
        print('--- page %d : %d fills, %d words' % (pno, len(fl), len(words)))
        for w in words:
            x0, y0, x1, y1, txt = w[0], w[1], w[2], w[3], w[4]
            cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
            hit = None
            for r, col in fl:
                if r.x0 <= cx <= r.x1 and r.y0 <= cy <= r.y1:
                    # prefer the smallest containing rect
                    if hit is None or (r.width * r.height) < (hit[0].width * hit[0].height):
                        hit = (r, col)
            if hit is None:
                print('%-6s  -' % txt)
            else:
                c = hit[1]
                print('%-6s  #%02X%02X%02X' % (txt, round(c[0]*255), round(c[1]*255), round(c[2]*255)))

if __name__ == '__main__':
    main(sys.argv[1])
