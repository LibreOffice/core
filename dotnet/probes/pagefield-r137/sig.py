import zipfile, re, glob, sys, xml.etree.ElementTree as ET
W='{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
def q(t): return W+t

def scan(root):
    """returns list of (has_field, in_table, in_group, in_txbx) for every field found in this part"""
    found=[]
    def walk(el, ctx):
        local=el.tag.split('}')[-1]
        n=dict(ctx)
        if local in ('drawing','pict'): n['indraw']=True
        if local=='tbl' and not n.get('indraw'): n['tbl']=True
        if local=='wgp': n['wgp']=True
        if local=='txbxContent': n['txbx']=True
        if local=='Fallback': n['fb']=True
        if local=='instrText': found.append(((el.text or ''), n))
        if local=='fldSimple': found.append((el.get(q('instr'),''), n))
        for c in el: walk(c, n)
    walk(root, {})
    return found

FLD=re.compile(r'\b(page|numpages|sectionpages|date|time|filename|author|docproperty|styleref|ref|seq)\b', re.I)
PG=re.compile(r'\b(page|numpages|sectionpages)\b', re.I)

n_doc=0; hdr_wgp=0; hdr_wgp_tbl=0; hit_any=[]; hit_page=[]
wgp_anywhere_hf=[]
for p in sorted(glob.glob(sys.argv[1], recursive=True)):
    n_doc+=1
    try: z=zipfile.ZipFile(p)
    except Exception: continue
    parts=[x for x in z.namelist() if re.match(r'word/(header|footer)\d*\.xml$', x)]
    has_wgp=False; has_wgp_tbl=False; anyf=False; pagef=False
    for part in parts:
        try: root=ET.fromstring(z.read(part))
        except Exception: continue
        raw=z.read(part).decode('utf-8','replace')
        if 'wpg:wgp' in raw or '<wpg:wgp' in raw: has_wgp=True
        for instr, ctx in scan(root):
            if ctx.get('fb'): continue
            if ctx.get('wgp') and ctx.get('tbl') and ctx.get('txbx'):
                has_wgp_tbl=True
                if FLD.search(instr): anyf=True
                if PG.search(instr): pagef=True
    if has_wgp: hdr_wgp+=1; wgp_anywhere_hf.append(p)
    if has_wgp_tbl: hdr_wgp_tbl+=1
    if anyf: hit_any.append(p)
    if pagef: hit_page.append(p)
    z.close()
print("documents scanned:", n_doc)
print("with a wpg:wgp group in a header/footer:", hdr_wgp)
print("with a field-bearing textbox inside a wgp group inside a header/footer TABLE:", hdr_wgp_tbl)
print("...of which the field is PAGE/NUMPAGES/SECTIONPAGES:", len(hit_page))
for h in hit_page: print("   PAGE:", h)
for h in hit_any:
    if h not in hit_page: print("   other-field:", h)
