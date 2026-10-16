import sys, os
sys.path.insert(0,'/tmp/claude-0/-c-sandbox-workdir/def86b95-446e-4afd-b708-2956130227d4/scratchpad')
from mkdocx import build, para, PGSZ
from probe_box import textbox, box_paras
OUT='/tmp/claude-0/-c-sandbox-workdir/def86b95-446e-4afd-b708-2956130227d4/scratchpad/probesbox2'
os.makedirs(OUT, exist_ok=True)
cases={}
# fine sweep, default insets (3.6pt each)
for h in [1,2,4,6,9,9.5,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,26,28,29,30,31]:
    cases[f'f-h{h}'] = textbox(h,6)
# zero insets
for h in [9,10,11,12,13,14,15,16,17,18,19,20,21,22]:
    cases[f'z-h{h}'] = textbox(h,6,tIns=0,bIns=0)
# large insets (10pt each = 127000)
for h in [15,20,25,30,35,40]:
    cases[f'i-h{h}'] = textbox(h,6,tIns=127000,bIns=127000)
for name,r in cases.items():
    body=f'<w:p>{r}</w:p>'+''.join(para(f'BODY{i}') for i in range(2))+f'<w:sectPr>{PGSZ}</w:sectPr>'
    build(os.path.join(OUT,name.replace('.','p')+'.docx'), body, {})
print('built',len(cases))
