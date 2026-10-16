import sys, pymupdf
doc = pymupdf.open(sys.argv[1])
p = doc[int(sys.argv[2])]
y0, y1 = float(sys.argv[3]), float(sys.argv[4])
n = 0
kinds = {}
for d in p.get_drawings():
    for it in d['items']:
        if it[0] == 're':
            r = it[1]
            if y0 <= r.y0 <= y1:
                n += 1
                if n <= 14:
                    print('re  type=%-3s w=%.3f  rect=(%.3f,%.3f,%.3f,%.3f)  h=%.4f wd=%.3f fill=%s stroke=%s' % (
                        d.get('type'), d.get('width') or 0, r.x0, r.y0, r.x1, r.y1, r.height, r.width, d.get('fill'), d.get('color')))
            kinds[('re', d.get('type'))] = kinds.get(('re', d.get('type')), 0) + 1
        else:
            kinds[(it[0], d.get('type'))] = kinds.get((it[0], d.get('type')), 0) + 1
print('...total re in band:', n)
print('item kinds on page:', kinds)
print('page rect', p.rect)
print('--- text spans near that y ---')
for b in p.get_text('dict')['blocks']:
    for ln in b.get('lines', []):
        for s in ln['spans']:
            if y0 - 6 <= s['bbox'][1] <= y1 + 6 and s['text'].strip():
                print('  span y=%.2f x=%.2f-%.2f size=%.1f %r' % (s['bbox'][1], s['bbox'][0], s['bbox'][2], s['size'], s['text'][:60]))
