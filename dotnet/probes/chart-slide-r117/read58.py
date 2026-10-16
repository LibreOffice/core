import fitz, pathlib, sys
for pdf in sorted(pathlib.Path(sys.argv[1]).glob('*.pdf')):
    d=fitz.open(pdf); p=d[5]
    bar=0; left=None
    for dr in p.get_drawings():
        r=dr['rect']
        if dr['type']=='f' and r.x1>650 and r.y0>150 and r.y1<345 and r.width>bar:
            bar, left = r.width, r.x0
    labs=[]
    for b in p.get_text('rawdict')['blocks']:
        for l in b.get('lines',[]):
            for s in l['spans']:
                t=''.join(c['c'] for c in s['chars']).strip()
                bb=s['bbox']
                if t.startswith('$') and bb[0]>640 and bb[3]>325 and bb[1]<355 and s['size']<6:
                    labs.append((t,s['size'],bb,l['dir']))
    W=bar*14000.0/12000.0
    right=left+W
    if labs:
        last=max(labs,key=lambda x:x[2][2])
        print('%-22s left=%7.2f W=%8.3f right=%7.2f n=%2d last=%-15s ink=%6.2f inkright=%7.2f over=%6.2f'
          % (pdf.stem,left,W,right,len(labs),last[0],last[2][2]-last[2][0],last[2][2],last[2][2]-right))
    else:
        print('%-22s left=%7.2f W=%8.3f right=%7.2f n= 0 (no labels)' % (pdf.stem,left,W,right))
