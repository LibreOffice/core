import re, sys, zipfile
TOK=re.compile(r'<w:p(?:\s[^>]*?)?(/?)>|</w:p>')
def outer_para(xml,pos):
    stack=[]
    for m in TOK.finditer(xml):
        t=m.group(0)
        if t.startswith('</w:p'):
            st=stack.pop()
            if st<=pos<=m.end() and not stack: return (st,m.end())
        elif m.group(1)=='/': continue
        else: stack.append(m.start())
    raise RuntimeError('nf')
TBL_OPEN=('<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/>'
          '<w:tblLayout w:type="fixed"/><w:tblLook w:val="04A0"/></w:tblPr>'
          '<w:tblGrid><w:gridCol w:w="9639"/></w:tblGrid>'
          '<w:tr><w:tc><w:tcPr><w:tcW w:w="9639" w:type="dxa"/></w:tcPr>')
TBL_CLOSE='</w:tc></w:tr></w:tbl><w:p/>'
src,dst,part=sys.argv[1],sys.argv[2],sys.argv[3]
zin=zipfile.ZipFile(src); xml=zin.read(part).decode('utf-8')
d=xml.index('<w:drawing>')
s,e=outer_para(xml,d)
out=xml[:s]+TBL_OPEN+xml[s:e]+TBL_CLOSE+xml[e:]
zo=zipfile.ZipFile(dst,'w',zipfile.ZIP_DEFLATED)
for it in zin.infolist():
    data=zin.read(it.filename)
    if it.filename==part: data=out.encode('utf-8')
    zo.writestr(it,data)
zo.close(); print('wrote',dst)
