import sys, os
sys.path.insert(0, '/tmp/claude-0/-c-sandbox-workdir/def86b95-446e-4afd-b708-2956130227d4/scratchpad')
from mkdocx import build, para
OUT='/tmp/claude-0/-c-sandbox-workdir/def86b95-446e-4afd-b708-2956130227d4/scratchpad/probes2'
os.makedirs(OUT, exist_ok=True)
HA=para('HEADERALPHA'); HB=para('HEADERBETA'); HC=para('HEADERGAMMA'); HD=para('HEADERDELTA')
EMPTY='<w:p><w:pPr><w:pStyle w:val="Header"/></w:pPr></w:p>'

def pg(w=11906,h=16838,l=1440,r=1440,t=1440,b=1440,extra=''):
    return (f'<w:pgSz w:w="{w}" w:h="{h}"{extra}/>'
            f'<w:pgMar w:top="{t}" w:right="{r}" w:bottom="{b}" w:left="{l}"'
            ' w:header="708" w:footer="708" w:gutter="0"/>')

def mk(name, s1refs, s1pg, s2refs, s2pg, settings='', hdrs=None):
    def refs(rs): return ''.join(f'<w:{k}Reference w:type="{t}" r:id="rIdH{n}"/>' for k,t,n in rs)
    body=(''.join(para(f'SECTIONONEBODY{i}') for i in range(3))
          + f'<w:p><w:pPr><w:sectPr>{refs(s1refs)}{s1pg}</w:sectPr></w:pPr></w:p>'
          + ''.join(para(f'SECTIONTWOBODY{i}') for i in range(3))
          + f'<w:sectPr>{refs(s2refs)}{s2pg}</w:sectPr>')
    build(os.path.join(OUT,name+'.docx'), body, hdrs or {'A':HA,'B':HB,'C':HC,'D':HD}, settings)

D=(('header','default','A'),)
# control: same geometry, S2 no refs
mk('g-same-norefs', D, pg(), [], pg())
# S2 different margins
mk('g-diffmargin-norefs', D, pg(), [], pg(l=851,r=851))
# S2 landscape
mk('g-landscape-norefs', D, pg(), [], pg(w=16838,h=11906,extra=' w:orient="landscape"'))
# S2 different margins, with even+first empty refs (the real shape)
mk('g-diffmargin-evenfirst', D, pg(),
   [('header','even','B'),('header','first','C')], pg(l=851,r=851),
   hdrs={'A':HA,'B':EMPTY,'C':EMPTY,'D':HD})
# S2 same margins, with even+first EMPTY refs
mk('g-same-evenfirst-empty', D, pg(),
   [('header','even','B'),('header','first','C')], pg(),
   hdrs={'A':HA,'B':EMPTY,'C':EMPTY,'D':HD})
# three sections: S1 default, S2 norefs, S3 default
def mk3(name):
    def refs(rs): return ''.join(f'<w:{k}Reference w:type="{t}" r:id="rIdH{n}"/>' for k,t,n in rs)
    body=(''.join(para(f'SECTIONONEBODY{i}') for i in range(3))
          + f'<w:p><w:pPr><w:sectPr>{refs(D)}{pg()}</w:sectPr></w:pPr></w:p>'
          + ''.join(para(f'SECTIONTWOBODY{i}') for i in range(3))
          + f'<w:p><w:pPr><w:sectPr>{refs([])}{pg(l=851,r=851)}</w:sectPr></w:pPr></w:p>'
          + ''.join(para(f'SECTIONTHREEBODY{i}') for i in range(3))
          + f'<w:sectPr>{refs([("header","default","D")])}{pg()}</w:sectPr>')
    build(os.path.join(OUT,name+'.docx'), body, {'A':HA,'B':HB,'C':HC,'D':HD}, '')
mk3('g-three-sections')
print('built', sorted(os.listdir(OUT)))
