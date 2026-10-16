import sys, os
sys.path.insert(0,'/tmp/claude-0/-c-sandbox-workdir/def86b95-446e-4afd-b708-2956130227d4/scratchpad')
from mkdocx import build, para, PGSZ
OUT='/tmp/claude-0/-c-sandbox-workdir/def86b95-446e-4afd-b708-2956130227d4/scratchpad/probeomml'
os.makedirs(OUT, exist_ok=True)
M='xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math"'
eq = ('<m:oMathPara xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math">'
      '<m:oMath><m:sSub><m:e><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/>'
      '</w:rPr><m:t>EQVAR</m:t></m:r></m:e><m:sub><m:r><w:rPr><w:rFonts w:ascii="Cambria Math" '
      'w:hAnsi="Cambria Math"/></w:rPr><m:t>SUB</m:t></m:r></m:sub></m:sSub>'
      '<m:r><w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/></w:rPr>'
      '<m:t>=EQRESULT</m:t></m:r></m:oMath></m:oMathPara>')
body = para('BEFOREEQUATION') + '<w:p>'+eq+'</w:p>' + para('AFTEREQUATION') + f'<w:sectPr>{PGSZ}</w:sectPr>'
build(os.path.join(OUT,'omml-display.docx'), body, {})
print('built')
