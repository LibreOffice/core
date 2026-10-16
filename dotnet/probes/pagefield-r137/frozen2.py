import subprocess, re, os, sys, glob
PATS = [('of',   re.compile(r'(?<![\d.\-/])(\d{1,3})\s+of\s+(\d{1,3})(?![\d.\-/])', re.I)),
        ('slash',re.compile(r'(?<![\d.\-/])(\d{1,3})\s*/\s*(\d{1,3})(?![\d.\-/])')),
        ('page', re.compile(r'\bpage\s+(\d{1,3})\b', re.I))]
refdir, outp = sys.argv[1], sys.argv[2]
out=open(outp,'w'); out.write("file\tpages\tpat\tverdict\tdetail\n")
for f in sorted(glob.glob(os.path.join(refdir,'*.pdf'))):
    try:
        txt=subprocess.run(['pdftotext','-layout',f,'-'],capture_output=True,timeout=120).stdout.decode('utf-8','replace')
    except Exception as e:
        out.write("%s\t?\t-\terror\t%s\n"%(os.path.basename(f),e)); continue
    pages=txt.split('\f')
    if pages and pages[-1].strip()=='': pages=pages[:-1]
    n=len(pages)
    if n<3: continue
    for name,pat in PATS:
        vals=[]
        for p in pages:
            m=pat.search(p); vals.append(m.group(1) if m else None)
        present=[v for v in vals if v is not None]
        if len(present) < max(3, n*0.8): continue
        uniq=sorted(set(present),key=int)
        seq=sum(1 for i,v in enumerate(vals,1) if v is not None and int(v)==i)
        if len(uniq)==1:
            out.write("%s\t%d\t%s\tFROZEN\tvalue=%s on %d/%d\n"%(os.path.basename(f),n,name,uniq[0],len(present),n))
        elif seq>=len(present)*0.8:
            out.write("%s\t%d\t%s\tsequential\t%d/%d\n"%(os.path.basename(f),n,name,seq,len(present)))
    out.flush()
out.close(); print('done')
