import zipfile, re, sys, xml.etree.ElementTree as ET

W='{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
def q(t): return W+t

def walk(el, path, out, ctx):
    tag=el.tag
    local=tag.split('}')[-1]
    ns=tag.split('}')[0][1:] if '}' in tag else ''
    nctx=dict(ctx)
    if local=='tbl' and ns.endswith('wordprocessingml/2006/main'): nctx['tbl']=nctx.get('tbl',0)+1
    if local=='txbxContent': nctx['txbx']=True
    if local=='anchor': nctx['anchor']=True
    if local=='inline': nctx['inline']=True
    if local=='wgp': nctx['wgp']=True
    if local=='AlternateContent': nctx['ac']=True
    if local=='Fallback': nctx['fallback']=True
    if local=='pict': nctx['pict']=True
    if local=='instrText':
        out.append(('instr', (el.text or ''), nctx))
    if local=='fldSimple':
        out.append(('fldSimple', el.get(q('instr'),''), nctx))
    if local=='fldChar':
        out.append(('fldChar', el.get(q('fldCharType'),'')+('|LOCK' if el.get(q('fldLock'))=='true' else '')+('|DIRTY' if el.get(q('dirty'))=='true' else ''), nctx))
    if local=='t':
        out.append(('t', (el.text or ''), nctx))
    for c in el:
        walk(c, path+'/'+local, out, nctx)

def report(path):
    z=zipfile.ZipFile(path)
    parts=[n for n in z.namelist() if re.match(r'word/(header|footer)\d*\.xml$',n)]
    print("#### "+path)
    for n in sorted(parts):
        root=ET.fromstring(z.read(n))
        out=[]; walk(root,'',out,{})
        # find field runs containing PAGE/NUMPAGES
        interesting=False
        seq=[]
        state=0
        for kind,val,ctx in out:
            if kind=='fldChar' and val.startswith('begin'): state=1; buf=[]; cbuf=[]; bctx=ctx
            elif kind=='instr' and state==1: buf.append(val)
            elif kind=='fldChar' and val.startswith('separate'): state=2
            elif kind=='t' and state==2: cbuf.append(val)
            elif kind=='fldChar' and val.startswith('end'):
                if state>=1:
                    ins=''.join(buf).strip()
                    if re.search(r'\b(page|numpages|sectionpages)\b',ins,re.I):
                        seq.append((ins, ''.join(cbuf), bctx)); interesting=True
                state=0
            elif kind=='fldSimple':
                if re.search(r'\b(page|numpages|sectionpages)\b',val,re.I):
                    seq.append((val.strip(),'(fldSimple)',ctx)); interesting=True
        if interesting:
            print("  "+n)
            for ins,cache,ctx in seq:
                flags=[]
                if ctx.get('txbx'): flags.append('TXBX')
                if ctx.get('tbl'): flags.append('tbl%d'%ctx['tbl'])
                if ctx.get('anchor'): flags.append('anchor')
                if ctx.get('inline'): flags.append('inline')
                if ctx.get('wgp'): flags.append('wgp')
                if ctx.get('fallback'): flags.append('FALLBACK')
                if ctx.get('pict'): flags.append('pict')
                print("    instr=%-22r cache=%-8r  %s" % (ins, cache, ','.join(flags)))
    z.close()

for p in sys.argv[1:]:
    report(p)
