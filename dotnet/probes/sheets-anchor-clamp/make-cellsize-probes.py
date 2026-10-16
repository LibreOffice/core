import zipfile, os
SRC='/home/user/wt-shapes/dotnet/tests/corpus/features/sheet-rich-text.xlsx'
src=zipfile.ZipFile(SRC); names=src.namelist(); data={n:src.read(n) for n in names}; src.close()
OLD='<xdr:to><xdr:col>1</xdr:col><xdr:colOff>640080</xdr:colOff><xdr:row>1</xdr:row><xdr:rowOff>640080</xdr:rowOff></xdr:to>'
os.makedirs('probes3',exist_ok=True)
def write(tag, v, colwidth, rowht):
    d=data['xl/drawings/drawing1.xml'].decode().replace(OLD, f'<xdr:to><xdr:col>1</xdr:col><xdr:colOff>{v}</xdr:colOff><xdr:row>1</xdr:row><xdr:rowOff>{v}</xdr:rowOff></xdr:to>')
    s3=data['xl/worksheets/sheet3.xml'].decode().replace('width="9.08"', f'width="{colwidth}"').replace('ht="50.4"', f'ht="{rowht}"')
    z=zipfile.ZipFile(f'probes3/{tag}.xlsx','w',zipfile.ZIP_DEFLATED)
    for n in names:
        z.writestr(n, d.encode() if n=='xl/drawings/drawing1.xml' else (s3.encode() if n=='xl/worksheets/sheet3.xml' else data[n]))
    z.close()
for i,(W,H) in enumerate([(5,17.3),(7,23.7),(11,31.1),(13,37.9),(15,41.3),(17,44.7)]):
    write(f'c{i}s', 100000, W, H)
    write(f'c{i}b', 3000000, W, H)
