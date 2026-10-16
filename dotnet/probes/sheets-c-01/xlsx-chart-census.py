import zipfile, re, os, sys, glob, collections
root='/c/sandbox/workdir/sample-files/sheets'
files=[]
for dp,dn,fn in os.walk(root):
    for f in fn: files.append(os.path.join(dp,f))
files.sort()
NS='{http://schemas.openxmlformats.org/drawingml/2006/chart}'
import xml.etree.ElementTree as ET
def cells(ref):
    # ref like 'Sheet'!$A$4:$A$16 -> count
    if '!' in ref: ref=ref.rsplit('!',1)[1]
    ref=ref.replace('$','')
    if ':' not in ref: return 1
    a,b=ref.split(':',1)
    def sp(x):
        m=re.match(r'^([A-Za-z]*)(\d*)$',x)
        if not m: return None
        col=0
        for ch in m.group(1).upper(): col=col*26+ord(ch)-64
        return (col, int(m.group(2)) if m.group(2) else None)
    A,B=sp(a),sp(b)
    if not A or not B or A[1] is None or B[1] is None: return None
    return (abs(B[0]-A[0])+1)*(abs(B[1]-A[1])+1)
tot=0; withchart=0; rows=[]
for path in files:
    if not zipfile.is_zipfile(path): continue
    try: z=zipfile.ZipFile(path)
    except Exception: continue
    names=[n for n in z.namelist() if re.match(r'xl/charts/chart\d+\.xml$',n)]
    if not names: 
        z.close(); continue
    withchart+=1
    nmis=0; nchart=0; nstatedmax=0; nnof=0
    for n in names:
        nchart+=1
        try: root_e=ET.fromstring(z.read(n))
        except Exception: continue
        s=z.read(n).decode('utf-8','replace')
        if re.search(r'<c:max ',s): nstatedmax+=1
        for tag in ('numRef','strRef','multiLvlStrRef'):
            for ref in root_e.iter(NS+tag):
                f=ref.find(NS+'f')
                if f is None or not (f.text or '').strip(): nnof+=1; continue
                cache=None
                for c in ref:
                    if c.tag.endswith('Cache'): cache=c
                if cache is None: continue
                pc=cache.find(NS+'ptCount')
                declared=int(pc.get('val')) if pc is not None else None
                want=cells(f.text.strip())
                if want is None or declared is None: continue
                if want!=declared: nmis+=1
    rows.append((os.path.basename(path), nchart, nmis, nstatedmax, nnof))
    z.close()
print("zip workbooks with chart parts:", withchart)
print(f"{'file':70s} charts mismatch statedMax noF")
for r in sorted(rows, key=lambda r:-r[2]):
    print(f"{r[0][:70]:70s} {r[1]:6d} {r[2]:8d} {r[3]:9d} {r[4]:4d}")
