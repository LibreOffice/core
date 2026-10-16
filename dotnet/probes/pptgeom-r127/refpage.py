"""One page's content stream out of a multi-page PDF, through the real page tree (C10)."""
import re, sys, zlib
def obj_offsets(data):
    offs={}
    for m in re.finditer(rb'(?m)^(\d+)\s+(\d+)\s+obj\b', data):
        offs[int(m.group(1))]=m.start()
    return offs
def body(data, offs, num):
    st=offs[num]; en=data.find(b'endobj', st)
    return data[st:en]
def stream_of(data, offs, num):
    b=body(data,offs,num)
    m=re.search(rb'stream\r?\n', b)
    if not m: return b''
    raw=b[m.end():b.rfind(b'endstream')]
    try: return zlib.decompress(raw)
    except Exception: return raw
def page_stream(path, index):
    data=open(path,'rb').read(); offs=obj_offsets(data)
    pages=[]
    for n in sorted(offs):
        b=body(data,offs,n)
        if re.search(rb'/Type\s*/Page\b(?!s)', b): pages.append(n)
    n=pages[index]
    b=body(data,offs,n)
    cm=re.search(rb'/Contents\s+(\d+)\s+\d+\s+R', b)
    if cm: return stream_of(data,offs,int(cm.group(1))).decode('latin-1'), len(pages)
    cm=re.search(rb'/Contents\s*\[([^\]]*)\]', b)
    out=b''
    for r in re.finditer(rb'(\d+)\s+\d+\s+R', cm.group(1)):
        out+=stream_of(data,offs,int(r.group(1)))
    return out.decode('latin-1'), len(pages)
if __name__=='__main__':
    s,n=page_stream(sys.argv[1], int(sys.argv[2])-1)
    print(f'# pages={n}', file=sys.stderr)
    sys.stdout.write(s)
