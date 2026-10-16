#!/usr/bin/env python3
"""How many `.ods` hold a cell whose *firing* conditional format would change its font.

Round 98. The mechanism this counts is `lcl_populateresult`
(sc/source/core/data/patattr.cxx:608-637 in the reference C++ checkout): when a condition
fires, `ScPatternAttr::fillFontOnly` takes the font item from the applied conditional
style's item set searched WITH parent inheritance (`GetItemIfSet`'s bSrchInParent
defaults to true, include/svl/itemset.hxx:201), so a conditional style stating no font
hands back whatever its ancestors state — terminating at the `Default` cell style, which
every parentless Calc cell style is re-parented to on import.

Reports both the numerator and the base rate: of all cells inside a conditional range,
how many fire, and of the firing ones how many would be measured in a different font
from their own.
"""
import sys, os, zipfile, re, csv
import xml.etree.ElementTree as ET

OFFICE='urn:oasis:names:tc:opendocument:xmlns:office:1.0'
STYLE='urn:oasis:names:tc:opendocument:xmlns:style:1.0'
TABLE='urn:oasis:names:tc:opendocument:xmlns:table:1.0'
FO='urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0'
CALCEXT='urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0'
def q(ns,l): return '{%s}%s'%(ns,l)

UNIT=re.compile(r'^([-0-9.]+)([a-z]*)$')
def pt(s):
    if not s: return None
    m=UNIT.match(s.strip())
    if not m: return None
    v=float(m.group(1)); u=m.group(2)
    return {'pt':1.0,'in':72.0,'cm':72/2.54,'mm':7.2/2.54,'pc':12.0,'':1.0}[u]*v

def collect_styles(root, out):
    for st in root.iter(q(STYLE,'style')):
        if st.get(q(STYLE,'family'))!='table-cell': continue
        n=st.get(q(STYLE,'name'))
        tp=st.find(q(STYLE,'text-properties'))
        out[n]={'parent':st.get(q(STYLE,'parent-style-name')),
                'size':pt(tp.get(q(FO,'font-size'))) if tp is not None else None,
                'face':tp.get(q(STYLE,'font-name')) if tp is not None else None,
                'maps':[(m.get(q(STYLE,'condition')),m.get(q(STYLE,'apply-style-name')))
                        for m in st.findall(q(STYLE,'map'))]}

def resolve(sty,name,key):
    seen=set()
    n=name
    while n and n not in seen:
        seen.add(n); d=sty.get(n)
        if d is None: break
        if d.get(key) is not None: return d[key]
        n=d['parent'] or ('Default' if n!='Default' else None)
    d=sty.get('Default')
    return d.get(key) if d else None

CELLREF=re.compile(r'^\$?([^.]*)\.\$?([A-Z]+)\$?([0-9]+)$')
def colnum(s):
    v=0
    for c in s: v=v*26+ord(c)-64
    return v-1
def parse_range(part):
    if ':' in part:
        a,b=part.split(':',1)
        if '.' not in b: b=a.split('.')[0]+'.'+b
    else:
        a=b=part
    ma,mb=CELLREF.match(a),CELLREF.match(b)
    if not ma or not mb: return None
    return (ma.group(1).strip("'"), colnum(ma.group(2)), int(ma.group(3))-1,
            colnum(mb.group(2)), int(mb.group(3))-1)

NUMOP=re.compile(r'^(<=|>=|!=|<>|<|>|=)\s*(-?[0-9.]+)$')
def fires(op, vt, val):
    """Reproduce ScConditionEntry::IsCellValid for a numeric-valued condition.

    lcl_GetCellContent (conditio.cxx:766-800) yields a number for a value cell and for a
    formula cell whose result IsValue() — an error result is a value, 0 — and a string
    otherwise; IsValidStr then returns false for a numeric condition unless the operator
    is `not equal` (conditio.cxx:1181-1183).
    """
    m=NUMOP.match(op or '')
    if not m: return None                      # formula/date/between conditions: not counted
    o,rhs=m.group(1),float(m.group(2))
    if vt=='error': arg=0.0
    elif vt=='float' or vt=='percentage' or vt=='currency':
        arg=val
    elif vt in ('string',): return o in ('!=','<>')
    elif vt is None: return None               # empty: never scanned for a row height
    else: return None
    return {'<':arg<rhs,'>':arg>rhs,'<=':arg<=rhs,'>=':arg>=rhs,
            '=':arg==rhs,'!=':arg!=rhs,'<>':arg!=rhs}[o]

def scan(path):
    z=zipfile.ZipFile(path)
    sty={}
    for part in ('styles.xml','content.xml'):
        try: collect_styles(ET.fromstring(z.read(part)), sty)
        except KeyError: pass
    content=ET.fromstring(z.read('content.xml'))
    body=content.find(q(OFFICE,'body')).find(q(OFFICE,'spreadsheet'))
    tot=fire=diff=cand=0
    detail={}
    for tb in body.findall(q(TABLE,'table')):
        cfs=tb.find(q(CALCEXT,'conditional-formats'))
        if cfs is None: continue
        rules=[]
        for cf in cfs.findall(q(CALCEXT,'conditional-format')):
            tgt=cf.get(q(CALCEXT,'target-range-address')) or ''
            rr=[parse_range(p) for p in re.findall(r"(?:'[^']*'|[^ ])+", tgt)]
            conds=[(c.get(q(CALCEXT,'value')),c.get(q(CALCEXT,'apply-style-name')))
                   for c in cf.findall(q(CALCEXT,'condition'))]
            if conds: rules.append(([r for r in rr if r],conds))
        if not rules: continue
        coldef=[]
        for c in tb.iter(q(TABLE,'table-column')):
            coldef += [c.get(q(TABLE,'default-cell-style-name'))]*int(c.get(q(TABLE,'number-columns-repeated'),'1'))
        ri=0
        for row in tb.iter(q(TABLE,'table-row')):
            rrep=int(row.get(q(TABLE,'number-rows-repeated'),'1'))
            if rrep>1000: ri+=rrep; continue
            ci=0
            for cell in row:
                crep=int(cell.get(q(TABLE,'number-columns-repeated'),'1'))
                if cell.tag==q(TABLE,'table-cell') and crep<=1000:
                    vt=cell.get(q(CALCEXT,'value-type')) or cell.get(q(OFFICE,'value-type'))
                    v=cell.get(q(OFFICE,'value'))
                    v=float(v) if v not in (None,'') else 0.0
                    csty=cell.get(q(TABLE,'style-name'))
                    for rr in range(ri,ri+rrep):
                        for cc in range(ci,ci+crep):
                            hit=None
                            for ranges,conds in rules:
                                for (_,c1,r1,c2,r2) in ranges:
                                    if r1<=rr<=r2 and c1<=cc<=c2: hit=conds; break
                                if hit: break
                            if not hit: continue
                            tot+=1
                            if vt is None: continue
                            eff=csty or (coldef[cc] if cc<len(coldef) else None) or 'Default'
                            # Upper bound: a cell can only move if the applied style's resolved
                            # font differs from its own, whether or not the condition fires.
                            if any(a for _,a in hit) and any(
                                    (resolve(sty,a,'size'),resolve(sty,a,'face'))
                                    != (resolve(sty,eff,'size'),resolve(sty,eff,'face'))
                                    for _,a in hit if a):
                                cand+=1
                            for cv,apply in hit:
                                f=fires(cv,vt,v)
                                if f is None: break
                                if f:
                                    fire+=1
                                    a=(resolve(sty,apply,'size'),resolve(sty,apply,'face'))
                                    b=(resolve(sty,eff,'size'),resolve(sty,eff,'face'))
                                    if a!=b:
                                        diff+=1
                                        detail.setdefault((eff,apply,b,a),0)
                                        detail[(eff,apply,b,a)]+=1
                                    break
                ci+=crep
            ri+=rrep
    return tot,fire,diff,cand,detail

if __name__=='__main__':
    w=csv.writer(sys.stdout,delimiter='\t')
    w.writerow(['document','cells_in_range','font_differs_upper_bound','firing_evaluable',
                'firing_font_differs','example'])
    for root,_,files in sorted(os.walk(sys.argv[1])):
        for f in sorted(files):
            if not f.endswith('.ods'): continue
            p=os.path.join(root,f)
            try: tot,fire,diff,cand,detail=scan(p)
            except Exception as e:
                w.writerow([f,'ERR','','','',str(e)[:80]]); continue
            if tot==0: continue
            ex=''
            if detail:
                k=max(detail,key=detail.get)
                ex='%s->%s  own=%s cond=%s x%d'%(k[0],k[1],k[2],k[3],detail[k])
            w.writerow([f,tot,cand,fire,diff,ex])
