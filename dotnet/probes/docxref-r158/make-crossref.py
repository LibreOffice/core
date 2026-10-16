#!/usr/bin/env python3
"""Does a DOCX bookmark named like a Writer cross-reference behave as one when collapsed?"""
import sys, zipfile
sys.path.insert(0, '/home/user/libreoffice-core/dotnet/probes/docxref-r158')
W='http://schemas.openxmlformats.org/wordprocessingml/2006/main'
def run(t): return f'<w:r><w:t xml:space="preserve">{t}</w:t></w:r>'
def fld(i,c): return ('<w:r><w:fldChar w:fldCharType="begin"/></w:r>'
    f'<w:r><w:instrText xml:space="preserve"> {i} </w:instrText></w:r>'
    '<w:r><w:fldChar w:fldCharType="separate"/></w:r>'+run(c)+'<w:r><w:fldChar w:fldCharType="end"/></w:r>')
names=['__RefHeading__1234_567890','__RefNumPara__1234_567890','_Toc12345','_Ref999']
arms=[];targets=[];i=100
for n in names:
    arms.append('<w:p>'+run(f'{n}: ')+fld(f'REF {n} \\h','stale')+run(' .')+'</w:p>')
    targets.append(f'<w:p>{run("Target for "+n)}<w:bookmarkStart w:id="{i}" w:name="{n}"/><w:bookmarkEnd w:id="{i}"/></w:p>')
    i+=1
doc=('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
 f'<w:document xmlns:w="{W}"><w:body>'+''.join(arms)+''.join(targets)+
 '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/><w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/></w:sectPr></w:body></w:document>')
import importlib.util
spec=importlib.util.spec_from_file_location('mp','/home/user/libreoffice-core/dotnet/probes/docxref-r158/make-probe.py')
mp=importlib.util.module_from_spec(spec); spec.loader.exec_module(mp)
parts=dict(mp.PARTS); parts['word/document.xml']=doc
with zipfile.ZipFile(sys.argv[1],'w',zipfile.ZIP_DEFLATED) as z:
    for k,v in parts.items(): z.writestr(k,v)
print('written',sys.argv[1])
