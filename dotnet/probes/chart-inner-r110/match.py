import sys, re
def parse_rect(t):
    return [float(v) for v in re.search(r'\(([^)]*)\)', t).group(1).split(',')]
def ours(p):
    d={}
    for line in open(p):
        f=line.rstrip('\n').split('\t')
        if 'Pie' not in f[2]: continue
        d.setdefault(f[0],[]).append((parse_rect(f[3]), parse_rect(f[4])))
    return d
ref={}
for line in open('pie-refrects.tsv'):
    f=line.rstrip('\n').split('\t')
    if f[2] not in ('chart:circle','chart:ring'): continue
    ref.setdefault(f[0],[]).append((parse_rect(f[4]), parse_rect(f[5])))
b=ours('ours-before.tsv'); a=ours('ours-after.tsv')
print(f'{"document":52} {"refCR":>16} {"base":>16} {"head":>16}  verdict')
tot=imp=reg=same=0
for doc in sorted(ref):
    for pa,cr in ref[doc]:
        cands=[(i,o,ar) for i,(o,ar) in enumerate(b.get(doc,[]))]
        if not cands: continue
        i,o,ar = min(cands, key=lambda c: abs(c[1][2]-pa[2])+abs(c[1][3]-pa[3]))
        ah = a[doc][i][1]
        rs=min(cr[2],cr[3]); bs=min(ar[2],ar[3]); hs=min(ah[2],ah[3])
        eb=abs(bs-rs); eh=abs(hs-rs)
        tot+=1
        v='same'
        if abs(eh-eb)>0.05:
            v = 'BETTER' if eh<eb else 'WORSE'
            imp+= v=='BETTER'; reg+= v=='WORSE'
        else: same+=1
        print(f'{doc[:52]:52} {rs:16.2f} {bs:16.2f} {hs:16.2f}  {v} (err {eb:.2f} -> {eh:.2f})')
print(f'\n{tot} reference pie/doughnut charts matched: {imp} better, {reg} worse, {same} unchanged')
