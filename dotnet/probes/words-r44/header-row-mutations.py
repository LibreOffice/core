#!/usr/bin/env python3
"""Mutate the header table of UG.CAO.00133 by XML navigation and report the drawn geometry
of the header table's second row, from both renderers."""
import copy, os, re, subprocess, sys, zipfile
import xml.etree.ElementTree as ET

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
BASE = os.path.dirname(os.path.abspath(__file__))
SRC  = os.path.join(BASE, 'ug133.docx')
OPS  = '/home/user/libreoffice-core/.claude/worktrees/words-r44/.claude/skills/render-comparison/scripts/pdf-ops.py'
CLI  = '/home/user/libreoffice-core/.claude/worktrees/words-r44/dotnet/tools/Paperless.Cli/bin/Debug/net10.0/linux-x64/Paperless.Cli'
for p, u in (('w','http://schemas.openxmlformats.org/wordprocessingml/2006/main'),):
    ET.register_namespace(p, u)

def parts(root):
    """(level2 table, its row 2, cell A, cell B, level3 table, cell B's trailing paragraph)"""
    t1 = root.find(W+'tbl')
    tc = t1.find(W+'tr').findall(W+'tc')[1]
    t2 = tc.find(W+'tbl')
    tr2 = t2.findall(W+'tr')[1]
    cA, cB = tr2.findall(W+'tc')
    t3 = cB.find(W+'tbl')
    pB = cB.find(W+'p')
    return t1, t2, tr2, cA, cB, t3, pB

def build(name, xform):
    out = os.path.join(BASE, 'mut2', name + '.docx')
    os.makedirs(os.path.dirname(out), exist_ok=True)
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as zo:
        for it in zin.infolist():
            data = zin.read(it.filename)
            if it.filename == 'word/header1.xml':
                root = ET.fromstring(data)
                xform(root)
                data = ET.tostring(root, encoding='utf-8', xml_declaration=True)
            zo.writestr(it, data)
    return out

def rules(pdf):
    txt = subprocess.run([OPS, 'dump', pdf, '--page', '1'], capture_output=True, text=True).stdout
    hs, vs = [], []
    for l in txt.splitlines():
        m = re.match(r'\s*stroke\s+p1\s+\(\s*([\d.]+),\s*([\d.]+)\)-\(\s*([\d.]+),\s*([\d.]+)\)', l)
        if not m: continue
        x0,y0,x1,y1 = map(float, m.groups())
        if max(y0,y1) < 700: continue
        if abs(y1-y0) < 0.5 and x1-x0 > 20: hs.append(round(y0,2))
        elif abs(x1-x0) < 0.5 and y1-y0 > 5: vs.append(round(y1-y0,2))
    return sorted(set(hs), reverse=True), sorted(set(vs))

def texts(pdf):
    txt = subprocess.run([OPS, 'dump', pdf, '--page', '1'], capture_output=True, text=True).stdout
    out = []
    for l in txt.splitlines():
        m = re.match(r'\s*text\s+p1\s+\(\s*([\d.]+),\s*([\d.]+)\)\s+([\d.]+)pt', l)
        if m and float(m.group(2)) > 700:
            out.append((round(float(m.group(2)),2), float(m.group(3))))
    return sorted(set(out), reverse=True)

def measure(name, path):
    ref  = os.path.join(BASE, 'mut2', name + '-ref')
    ours = os.path.join(BASE, 'mut2', name + '-ours')
    os.makedirs(ref, exist_ok=True); os.makedirs(ours, exist_ok=True)
    subprocess.run(['timeout','300','soffice','--headless',
        '-env:UserInstallation=file://' + BASE + '/prof', '--convert-to','pdf','--outdir',ref,path],
        capture_output=True)
    subprocess.run([CLI,'render',path,'--format','pdf','--outdir',ours], capture_output=True)
    stem = os.path.basename(path)[:-5] + '.pdf'
    for side, d in (('ref',ref), ('ours',ours)):
        p = os.path.join(d, stem)
        if not os.path.exists(p): print(f'  {side:4s}: NO OUTPUT'); continue
        h, v = rules(p)
        print(f'  {side:4s} v-rule heights {v}  h-rules {h}  baselines {texts(p)}')

MUTS = {}
def mut(fn): MUTS[fn.__name__] = fn; return fn

@mut
def asis(r): pass

@mut
def cellB_no_trailing_p(r):
    _,_,_,_,cB,_,pB = parts(r); cB.remove(pB)

@mut
def cellB_no_inner_table(r):
    _,_,_,_,cB,t3,_ = parts(r); cB.remove(t3)

@mut
def cellB_one_inner_row(r):
    _,_,_,_,_,t3,_ = parts(r); t3.remove(t3.findall(W+'tr')[1])

@mut
def cellB_three_inner_rows(r):
    _,_,_,_,_,t3,_ = parts(r); t3.append(copy.deepcopy(t3.findall(W+'tr')[1]))

@mut
def cellA_only(r):
    _,_,_,_,cB,t3,pB = parts(r)
    cB.remove(t3)
    for p in cB.findall(W+'p'): cB.remove(p)
    q = ET.SubElement(cB, W+'p')
    cB.append(q)


@mut
def cellB_trailing_p_text(r):
    _,_,_,_,cB,_,pB = parts(r)
    run = ET.SubElement(pB, W+'r'); t = ET.SubElement(run, W+'t'); t.text = 'ZZZZZZ'

@mut
def cellB_two_trailing_p(r):
    _,_,_,_,cB,_,pB = parts(r)
    cB.append(copy.deepcopy(pB))

@mut
def cellB_p_before_table(r):
    _,_,_,_,cB,t3,pB = parts(r)
    cB.insert(list(cB).index(t3), copy.deepcopy(pB))

if __name__ == '__main__':
    for name in (sys.argv[1:] or list(MUTS)):
        print(name); measure(name, build(name, MUTS[name]))
