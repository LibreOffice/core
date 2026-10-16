import re, zipfile, os, subprocess
src="/c/sandbox/workdir/sample-files/sheets/chartset-012/xlsx/013_Contextures_chart_sample_21b98e22.xlsx"
cli="/c/sandbox/workdir/wt-sheets-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
def variant(name, fn):
    zin=zipfile.ZipFile(src); zo=zipfile.ZipFile(name+".xlsx","w",zipfile.ZIP_DEFLATED)
    for it in zin.infolist():
        d=zin.read(it.filename)
        if it.filename=="xl/drawings/drawing1.xml": d=fn(d.decode()).encode()
        zo.writestr(it,d)
    zo.close(); return name+".xlsx"

cases = {
 "base":     lambda s: s,
 "notwocell":lambda s: s.replace(' editAs="oneCell"',''),
 "halfext":  lambda s: s.replace('cx="4143375"','cx="2071687"'),
 "tocol9":   lambda s: re.sub(r'(<xdr:to><xdr:col>)6', r'\g<1>9', s),
}
def span(pdf):
    out=subprocess.run(["pdftotext","-q","-f","1","-l","1","-bbox",pdf,"-"],capture_output=True,text=True).stdout
    a=re.search(r'<word xMin="([\d.]+)"[^>]*>1000</word>',out)
    b=re.search(r'<word xMin="([\d.]+)"[^>]*>Jan</word>',out)
    c=re.search(r'yMin="([\d.]+)"[^>]*>1000</word>',out)
    return (round(float(a.group(1)),1) if a else None, round(float(b.group(1)),1) if b else None,
            round(float(c.group(1)),1) if c else None)
print(f"{'case':>10} {'ref(1000x,Janx,1000y)':>28} {'ours':>28}")
for name,fn in cases.items():
    p=variant(name,fn); prof=os.path.abspath("p_"+name); subprocess.run(["rm","-rf",prof])
    subprocess.run(["soffice",f"-env:UserInstallation=file://{prof}","--headless","--convert-to","pdf","--outdir","ea",p],capture_output=True)
    subprocess.run([cli,"render",p,"--format","pdf","--outdir","ea/ours"],capture_output=True)
    print(f"{name:>10} {str(span('ea/'+name+'.pdf')):>28} {str(span('ea/ours/'+name+'.pdf')):>28}")
