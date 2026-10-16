import zipfile, re, os, glob
from xml.etree import ElementTree as ET
NS='{http://schemas.openxmlformats.org/spreadsheetml/2006/main}'
def col(ref):
    m=re.match(r'([A-Z]+)(\d+)',ref); c=0
    for ch in m.group(1): c=c*26+ord(ch)-64
    return c-1, int(m.group(2))-1
docs=sorted(p for p in glob.glob('/workspace/sample-files/sheets/batch-*/*/*') if os.path.isfile(p))
diff=0; anyborder=0; readable=0; tot_merges=0; diffm=0
for d in docs:
    try: z=zipfile.ZipFile(d)
    except Exception: continue
    sheets=[n for n in z.namelist() if re.match(r'xl/worksheets/sheet.*\.xml$',n)]
    if not sheets: continue
    # styles: which xf indices carry a border or fill
    decorated=set()
    try:
        st=ET.fromstring(z.read('xl/styles.xml'))
        borders=st.find(NS+'borders'); fills=st.find(NS+'fills')
        bdec=[any(b.find(NS+e) is not None and b.find(NS+e).get('style') not in (None,'none') for e in ('left','right','top','bottom')) for b in (borders or [])]
        fdec=[(f.find(NS+'patternFill') is not None and f.find(NS+'patternFill').get('patternType') not in (None,'none')) for f in (fills or [])]
        xfs=st.find(NS+'cellXfs')
        for i,x in enumerate(xfs or []):
            bi=int(x.get('borderId','0') or 0); fi=int(x.get('fillId','0') or 0)
            if (bi<len(bdec) and bdec[bi]) or (fi<len(fdec) and fdec[fi]): decorated.add(i)
    except Exception: pass
    readable+=1
    dd=False; ab=False
    for n in sheets:
        try: root=ET.fromstring(z.read(n))
        except Exception: continue
        mc=root.find(NS+'mergeCells')
        if mc is None: continue
        styles={}
        for row in root.iter(NS+'row'):
            for c in row:
                r=c.get('r')
                if r: styles[col(r)]=int(c.get('s','0') or 0)
        for m in mc:
            ref=m.get('ref') or ''
            if ':' not in ref: continue
            a,b=ref.split(':')
            try: (c0,r0)=col(a); (c1,r1)=col(b)
            except Exception: continue
            tot_merges+=1
            o=styles.get((c0,r0),0); local=False
            for rr in range(r0,min(r1,r0+200)+1):
                for cc in range(c0,min(c1,c0+50)+1):
                    if (cc,rr)==(c0,r0): continue
                    s=styles.get((cc,rr),0)
                    if s!=o: local=True
                    if s in decorated: ab=True
            if local: dd=True; diffm+=1
    diff+=dd; anyborder+=ab
print("OOXML-readable worksheets:",readable,"of",len(docs))
print("documents with a merge whose covered cell states a different style:",diff)
print("documents with a covered cell whose style paints a border or fill:",anyborder)
print("merges total:",tot_merges,"of which some covered cell differs:",diffm)
