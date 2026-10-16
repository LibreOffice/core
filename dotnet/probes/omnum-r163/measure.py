import subprocess,re,sys,glob,os,collections
def lines(pdf):
    out=subprocess.run(['pdftotext','-bbox',pdf,'-'],capture_output=True,text=True).stdout
    ls=collections.defaultdict(list)
    for m in re.finditer(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="[\d.]+" yMax="[\d.]+">(.*?)</word>',out):
        ls[round(float(m.group(2)),1)].append((float(m.group(1)),m.group(3)))
    res=[]
    for y in sorted(ls):
        ws=sorted(ls[y])
        res.append((y,[(round(x,2),w) for x,w in ws]))
    return res
MARGIN=1134/20.0   # 1134 twips = 56.7pt
for arm in sorted(glob.glob('fixtures/*.docx')):
    b=os.path.basename(arm)[:-5]
    key=re.sub(r'[^A-Za-z0-9._-]','_',os.path.basename(arm))
    ref=f'fixref/{key}/{b}.pdf'
    our=f'fixours/{b}/{b}.pdf'
    print('###',b)
    for name,pdf in (('REF ',ref),('OURS',our)):
        if not os.path.exists(pdf): print('  ',name,'MISSING',pdf); continue
        for y,ws in lines(pdf):
            if not ws: continue
            desc=' | '.join(f'{w}@{x-MARGIN:+.2f}' for x,w in ws)
            print(f'  {name} y={y:7.2f} {desc}')
    print()
