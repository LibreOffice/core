import re, subprocess, sys, zipfile
from pathlib import Path
import pymupdf
SRC=Path('/home/user/sample-files/sheets/chartset-008/xlsx/055_Project_timeline_with_milestones_Use_this_template_546cecc0.xlsx')
SOFF='/opt/libreoffice26.2/program/soffice'
OUT=Path('/home/user/wt-chartfit/.work/fmt'); OUT.mkdir(parents=True, exist_ok=True)
FMT=('[$-409]d\\ mmm;@', '[$-409]d\\ mmm\\ yyyy;@')
def build(name, sheet_edit):
    p=OUT/f'{name}.xlsx'
    with zipfile.ZipFile(SRC) as zin, zipfile.ZipFile(p,'w',zipfile.ZIP_DEFLATED) as zo:
        for i in zin.infolist():
            d=zin.read(i.filename)
            if i.filename=='xl/charts/chart11.xml':
                d=d.decode().replace(FMT[0],FMT[1]).encode()
            if i.filename=='xl/worksheets/sheet11.xml' and sheet_edit:
                d=sheet_edit(d.decode()).encode()
            zo.writestr(i,d)
    return p
def render(p):
    o=OUT/(p.stem+'.out'); o.mkdir(exist_ok=True)
    subprocess.run([SOFF,'-env:UserInstallation=file://'+str(o/'prof'),'--headless','--norestore',
                    '--convert-to','pdf','--outdir',str(o),str(p)],capture_output=True,timeout=600)
    return o/(p.stem+'.pdf')
for name, edit in (('base', None), ('y2023', lambda s: s.replace('YEAR(TODAY())','2023'))):
    pdf=render(build(name, edit))
    rows={}
    for b in pymupdf.open(pdf)[0].get_text('dict')['blocks']:
        for l in b.get('lines',[]):
            for s in l['spans']:
                t=s['text'].strip()
                if re.fullmatch(r'\d{1,2} [A-Z][a-z]{2} \d{4}', t):
                    rows.setdefault(round(s['bbox'][1],0),[]).append((round(s['bbox'][0],2),t))
    if not rows: print(name,'no labels'); continue
    y=max(rows,key=lambda k: len(rows[k])); lab=[t for _,t in sorted(rows[y])]
    print(name, len(lab), lab[0], '->', lab[-1], '|', lab[:4])
