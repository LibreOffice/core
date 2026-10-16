"""Read a .fods and report per-cell computed values, conditional formats,
named expressions.  The values are the REFERENCE BINARY's own results."""
import sys, re
import xml.etree.ElementTree as ET

NS = {
 'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
 'table':'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
 'text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
 'calcext':'urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0',
 'style':'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
 'fo':'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0',
}
def q(n):
    p,l = n.split(':'); return '{%s}%s' % (NS[p], l)

def colname(n):
    s=''
    while n>0:
        n,r=divmod(n-1,26); s=chr(65+r)+s
    return s

def cellvals(path):
    t = ET.parse(path); root = t.getroot()
    res = {}
    for tbl in root.iter(q('table:table')):
        name = tbl.get(q('table:name'))
        r = 0
        for row in tbl.iter(q('table:table-row')):   # iter(): also rows nested in table:table-header-rows
            rep = int(row.get(q('table:number-rows-repeated'), 1))
            c = 0
            cur = {}
            for cell in list(row):
                crep = int(cell.get(q('table:number-columns-repeated'), 1))
                c += 1
                f = cell.get(q('table:formula'))
                vt = cell.get(q('office:value-type'))
                v = cell.get(q('office:value'))
                bv = cell.get(q('office:boolean-value'))
                sv = cell.get(q('office:string-value'))
                txt = ''.join(p.itertext() for p in []) # placeholder
                tx = ''.join(''.join(p.itertext()) for p in cell.findall(q('text:p')))
                if f or vt or tx:
                    for k in range(crep):          # a repeated cell fills EVERY column it covers
                        cur[c + k] = dict(f=f, vt=vt, v=v, bv=bv, sv=sv, text=tx)
                c += crep - 1
            for i in range(rep):
                r += 1
                if cur and i == 0:
                    for cc, d in cur.items():
                        res['%s!%s%d' % (name, colname(cc), r)] = d
            if rep > 1 and cur:
                pass
    return res

def dump_cf(path):
    s = open(path, encoding='utf-8').read()
    out = []
    for m in re.finditer(r'<calcext:conditional-format .*?</calcext:conditional-format>', s, re.S):
        out.append(m.group(0).replace('><', '>\n  <'))
    return out

def dump_names(path):
    s = open(path, encoding='utf-8').read()
    return re.findall(r'<table:named-(?:expression|range)[^>]*/>', s)

if __name__ == '__main__':
    path = sys.argv[1]
    which = sys.argv[2] if len(sys.argv) > 2 else 'cells'
    if which == 'cells':
        vals = cellvals(path)
        keys = sys.argv[3:] 
        for k in sorted(vals, key=lambda k:(k.split('!')[0], len(k.split('!')[1].rstrip('0123456789')), k)):
            d = vals[k]
            if keys and not any(k.endswith('!'+x) or x in k for x in keys): continue
            print('%-14s f=%-52s type=%-8s v=%-10s bool=%-6s text=%r' % (
                k, (d['f'] or '')[:52], d['vt'], d['v'], d['bv'], d['text']))
    elif which == 'cf':
        for b in dump_cf(path): print(b)
    elif which == 'names':
        for b in dump_names(path): print(b)
