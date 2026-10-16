import re, sys, zipfile

TOK = re.compile(r'<w:p(?:\s[^>]*?)?(/?)>|</w:p>')

def outer_para_containing(xml, pos):
    stack=[]
    for m in TOK.finditer(xml):
        if m.start() > pos and stack:
            break
        t=m.group(0)
        if t.startswith('</w:p'):
            st=stack.pop()
            if st < pos < m.end() and not stack:
                return (st, m.end())
        elif m.group(1)=='/':
            pass
        else:
            stack.append(m.start())
    # continue scanning to closing
    stack=[]
    for m in TOK.finditer(xml):
        t=m.group(0)
        if t.startswith('</w:p'):
            st=stack.pop()
            if st <= pos <= m.end() and not stack:
                return (st, m.end())
        elif m.group(1)=='/':
            continue
        else:
            stack.append(m.start())
    raise RuntimeError('not found')

def run(src,dst,part,mode):
    zin=zipfile.ZipFile(src); xml=zin.read(part).decode('utf-8')
    d=xml.index('<w:drawing>')
    s,e=outer_para_containing(xml,d)
    para=xml[s:e]
    rest=xml[:s]+'<w:p/>'+xml[e:]
    if mode=='untable':
        t=rest.index('<w:tbl>'); out=rest[:t]+para+rest[t:]
    elif mode=='drop':
        out=rest
    else: raise RuntimeError(mode)
    zo=zipfile.ZipFile(dst,'w',zipfile.ZIP_DEFLATED)
    for it in zin.infolist():
        data=zin.read(it.filename)
        if it.filename==part: data=out.encode('utf-8')
        zo.writestr(it,data)
    zo.close(); zin.close()
    print('wrote',dst,'para bytes',len(para))

run(sys.argv[1],sys.argv[2],sys.argv[3],sys.argv[4])
