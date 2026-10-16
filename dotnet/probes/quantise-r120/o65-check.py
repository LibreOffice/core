#!/usr/bin/env python3
"""Count the link-coloured and file-coloured text spans on both sides of every O65 mover."""
import collections, glob, os, shutil, subprocess, sys, tempfile
import pymupdf
SOFFICE='/opt/libreoffice26.2/program/soffice'
CLI=sys.argv[1]
DOCS=[l.strip() for l in open(sys.argv[2]) if l.strip()]
NAVY='#000080'
def census(pdf):
    d=pymupdf.open(pdf); c=collections.Counter()
    for i in range(d.page_count):
        for b in d[i].get_text('dict')['blocks']:
            for l in b.get('lines',[]):
                for s in l['spans']:
                    if s['text'].strip(): c['#%06X'%s['color']]+=1
    d.close(); return c
print('document\tnavy_ref\tnavy_ours\tother_coloured_ref\tother_coloured_ours\tidentical_census')
for path in DOCS:
    tmp=tempfile.mkdtemp(prefix='o65c-')
    try:
        legs={}
        for n in (1,2):
            subprocess.run(['timeout','-k','30','900',SOFFICE,
                '-env:UserInstallation=file://'+tmp+'/p%d'%n,'--headless','--norestore',
                '--convert-to','pdf','--outdir',tmp+'/r%d'%n,path],capture_output=True)
            g=glob.glob(tmp+'/r%d/*.pdf'%n)
            legs['ref%d'%n]=census(g[0]) if g else collections.Counter()
        subprocess.run(['timeout','-k','30','900',CLI,'render',path,'--format','pdf',
                        '--outdir',tmp+'/o'],capture_output=True,
                       env=dict(os.environ,SOURCE_DATE_EPOCH='1700000000'))
        g=glob.glob(tmp+'/o/*.pdf')
        ours=census(g[0]) if g else collections.Counter()
        ref=legs['ref1']
        def other(c): return sum(v for k,v in c.items() if k not in ('#000000',NAVY,'#FFFFFF'))
        print('%s\t%d\t%d\t%d\t%d\t%s'%(os.path.basename(path),ref[NAVY],ours[NAVY],
              other(ref),other(ours),'yes' if legs['ref1']==legs['ref2'] else 'NO'))
    finally:
        shutil.rmtree(tmp,ignore_errors=True)
