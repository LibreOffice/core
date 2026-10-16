import os, subprocess, glob, concurrent.futures, collections
SC='/tmp/claude-0/-c-sandbox-workdir/def86b95-446e-4afd-b708-2956130227d4/scratchpad'
REF='/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words'
MAN='/c/sandbox/workdir/sample-files/MANIFEST.tsv'
def pdf_of(root,key):
    f=glob.glob(os.path.join(root,key,'*.pdf')); return f[0] if f else None
def npages(p):
    o=subprocess.run(['pdfinfo',p],capture_output=True,text=True).stdout
    for l in o.splitlines():
        if l.startswith('Pages:'): return int(l.split()[1])
    return 0
def words(p):
    o=subprocess.run(['pdftotext',p,'-'],capture_output=True,text=True).stdout
    return len([t for t in o.split() if any(c.isalnum() for c in t)])
def verdict(op,rp,ow,rw):
    if op!=rp: return 'PAGES'
    return 'WORDS' if abs(ow-rw)>rw*0.02+3 else 'pass'
keys=sorted(os.path.basename(d) for d in glob.glob(SC+'/sweep-new/*') if os.path.isdir(d))
def one(key):
    cand=[c for c in os.listdir(REF) if c[:-4]==key]
    if not cand: return (key,None)
    r=os.path.join(REF,cand[0]); rp,rw=npages(r),words(r)
    out={}
    for tag,root in (('old',SC+'/sweep-old'),('new',SC+'/sweep-new')):
        p=pdf_of(root,key)
        out[tag]=verdict(npages(p),rp,words(p),rw)
    return (key,out)
with concurrent.futures.ThreadPoolExecutor(max_workers=8) as ex:
    rows=list(ex.map(one,keys))
# group membership from the corpus tree
group={}
for dirpath,_,names in os.walk('/c/sandbox/workdir/sample-files/words'):
    g=os.path.relpath(dirpath,'/c/sandbox/workdir/sample-files/words').split('/')[0]
    for n in names:
        stem=os.path.splitext(n)[0]; ext=os.path.splitext(n)[1].lstrip('.').lower()
        group[f'{stem}__{ext}']=g
co=collections.Counter(); cn=collections.Counter()
gold=collections.Counter(); gnew=collections.Counter()
noref=[]
for key,v in rows:
    if v is None: noref.append(key); continue
    co[v['old']]+=1; cn[v['new']]+=1
    g=group.get(key,'?')
    if v['old']=='pass': gold[g]+=1
    if v['new']=='pass': gnew[g]+=1
print('no banked reference:',len(noref))
print('BEFORE:',dict(co))
print('AFTER :',dict(cn))
print()
print('per-group passes  before -> after')
for g in sorted(set(list(gold)+list(gnew)+list(set(group.values())))):
    tot=sum(1 for k in group if group[k]==g and k in dict(rows) )
    print(f'  {g:14} {gold[g]:>3} -> {gnew[g]:>3}')
