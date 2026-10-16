import shutil, zipfile, subprocess, os, sys, re
SRC='/home/user/wt-shapes/dotnet/tests/corpus/features/sheet-rich-text.xlsx'
vals=[100000,320040,600000,630000,635000,639000,639445,640080,700000,900000]
os.makedirs('probes',exist_ok=True)
src=zipfile.ZipFile(SRC)
names=src.namelist()
data={n:src.read(n) for n in names}
src.close()
for v in vals:
    d=data['xl/drawings/drawing1.xml'].decode()
    d=d.replace('<xdr:to><xdr:col>1</xdr:col><xdr:colOff>640080</xdr:colOff><xdr:row>1</xdr:row><xdr:rowOff>640080</xdr:rowOff></xdr:to>',
                f'<xdr:to><xdr:col>1</xdr:col><xdr:colOff>{v}</xdr:colOff><xdr:row>1</xdr:row><xdr:rowOff>{v}</xdr:rowOff></xdr:to>')
    out=f'probes/p{v}.xlsx'
    z=zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED)
    for n in names:
        z.writestr(n, d.encode() if n=='xl/drawings/drawing1.xml' else data[n])
    z.close()
print('wrote', len(vals))
