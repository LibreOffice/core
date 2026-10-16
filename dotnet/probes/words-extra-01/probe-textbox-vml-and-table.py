import sys, os
sys.path.insert(0,'/tmp/claude-0/-c-sandbox-workdir/def86b95-446e-4afd-b708-2956130227d4/scratchpad')
from mkdocx import build, para, PGSZ
from probe_box import textbox, box_paras
OUT='/tmp/claude-0/-c-sandbox-workdir/def86b95-446e-4afd-b708-2956130227d4/scratchpad/probesbox3'
os.makedirs(OUT, exist_ok=True)
EMU_PT=12700
def vml(h_pt, n, fit=''):
    return (f'<w:r><w:pict><v:shape id="s1" type="#_x0000_t202" '
            f'style="position:absolute;margin-left:0;margin-top:0;width:400pt;height:{h_pt}pt;'
            f'z-index:1;visibility:visible;mso-wrap-style:square{fit}">'
            f'<v:textbox inset="7.2pt,3.6pt,7.2pt,3.6pt"><w:txbxContent>{box_paras(n)}</w:txbxContent>'
            f'</v:textbox></v:shape></w:pict></w:r>')
def tbl_rows(n):
    rows=''
    for i in range(n):
        rows+=('<w:tr><w:tc><w:tcPr><w:tcW w:w="4000" w:type="dxa"/></w:tcPr>'
               f'<w:p><w:pPr><w:rPr><w:sz w:val="16"/></w:rPr></w:pPr>'
               f'<w:r><w:rPr><w:sz w:val="16"/></w:rPr><w:t>BOXLINE{i:02d}</w:t></w:r></w:p></w:tc></w:tr>')
    return ('<w:tbl><w:tblPr><w:tblW w:w="4000" w:type="dxa"/></w:tblPr>'
            '<w:tblGrid><w:gridCol w:w="4000"/></w:tblGrid>'+rows+'</w:tbl><w:p/>')
cases={}
for h in [10,15,20,30,40,60]:
    cases[f'v-h{h}']=vml(h,6)
cases['v-h15-fit']=vml(15,6,fit=';mso-fit-shape-to-text:t')
# table inside a wps text box
for h in [15,20,30,40,60,100]:
    tb=textbox(h,6)
    cases[f't-h{h}']=tb.replace(box_paras(6), tbl_rows(6))
for name,r in cases.items():
    body=f'<w:p>{r}</w:p>'+''.join(para(f'BODY{i}') for i in range(2))+f'<w:sectPr>{PGSZ}</w:sectPr>'
    build(os.path.join(OUT,name+'.docx'), body, {})
print('built',len(cases))
