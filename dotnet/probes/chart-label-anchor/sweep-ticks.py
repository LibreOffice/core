import os,re,subprocess,importlib.util
SP="/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad/charts2/tickprobe"
spec=importlib.util.spec_from_file_location("mk", SP+"/make.py"); mk=importlib.util.module_from_spec(spec); spec.loader.exec_module(mk)
LO="/opt/libreoffice26.2/program/soffice"; WORK=SP+"/w4"; os.makedirs(WORK,exist_ok=True)
def conv(tag,h,size,family,want):
    name=f"{tag}-{h:.5f}".replace('.','_'); d=os.path.join(WORK,name); os.makedirs(d,exist_ok=True)
    f=os.path.join(d,"src.fods")
    if not os.path.exists(f): open(f,"w").write(mk.variant(h,size,family))
    o=os.path.join(d,want); os.makedirs(o,exist_ok=True); out=os.path.join(o,"src."+want)
    if not os.path.exists(out):
        subprocess.run([LO,f"-env:UserInstallation=file://{d}/prof","--headless","--convert-to",want,"--outdir",o,f],capture_output=True,timeout=300)
    return out if os.path.exists(out) else None
def nine(tag,h,size,family):
    p=conv(tag,h,size,family,"pdf")
    return "62" in subprocess.run(["pdftotext",p,"-"],capture_output=True,text=True).stdout.split()
def pa(tag,h,size,family):
    f=conv(tag,h,size,family,"fods"); s=open(f).read()
    m=re.search(r'<chart:plot-area[^>]*svg:height="([-\d.]+)cm"',s)
    return float(m.group(1))*72/2.54
for tag,size,family in (("y-libs10",10,"Liberation Sans"),("y-libs24",24,"Liberation Sans"),
                        ("y-dv10",10,"DejaVu Sans"),("y-dv24",24,"DejaVu Sans")):
    lo,hi=2.0,30.0
    if nine(tag,lo,size,family) or not nine(tag,hi,size,family):
        print(f"{tag}: no bracket", flush=True); continue
    for _ in range(13):
        mid=(lo+hi)/2
        if nine(tag,mid,size,family): hi=mid
        else: lo=mid
    a,b=pa(tag,lo,size,family), pa(tag,hi,size,family)
    print(f"{tag} size={size} {family}: frame [{lo:.5f},{hi:.5f}]cm  plot-area [{a:.3f},{b:.3f}]pt  /9=[{a/9:.4f},{b/9:.4f}]pt", flush=True)
