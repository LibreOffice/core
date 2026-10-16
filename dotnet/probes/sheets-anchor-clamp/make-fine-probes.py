import zipfile, os
SRC='/home/user/wt-shapes/dotnet/tests/corpus/features/sheet-rich-text.xlsx'
src=zipfile.ZipFile(SRC); names=src.namelist(); data={n:src.read(n) for n in names}; src.close()
OLD='<xdr:to><xdr:col>1</xdr:col><xdr:colOff>640080</xdr:colOff><xdr:row>1</xdr:row><xdr:rowOff>640080</xdr:rowOff></xdr:to>'
os.makedirs('probes2',exist_ok=True)
def write(tag, v, colwidth=None):
    d=data['xl/drawings/drawing1.xml'].decode()
    d=d.replace(OLD, f'<xdr:to><xdr:col>1</xdr:col><xdr:colOff>{v}</xdr:colOff><xdr:row>1</xdr:row><xdr:rowOff>{v}</xdr:rowOff></xdr:to>')
    s3=data['xl/worksheets/sheet3.xml'].decode()
    if colwidth is not None:
        s3=s3.replace('width="9.08"', f'width="{colwidth}"')
    out=f'probes2/{tag}.xlsx'
    z=zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED)
    for n in names:
        if n=='xl/drawings/drawing1.xml': z.writestr(n, d.encode())
        elif n=='xl/worksheets/sheet3.xml': z.writestr(n, s3.encode())
        else: z.writestr(n, data[n])
    z.close()
for v in range(636000, 641001, 500):
    write(f'a{v}', v)
# wide column: 20 chars
for v in [1200000, 1400000, 1410000, 1420000, 1430000, 1440000]:
    write(f'w{v}', v, colwidth='20')
