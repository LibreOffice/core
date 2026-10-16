"""Every content stream of a PDF, decompressed, concatenated.  Enough for reading path
operators off a one-page rendering; it does not walk the page tree (C10) and must therefore
only be used on a PDF that holds ONE page."""
import re, sys, zlib
def streams(path):
    data=open(path,'rb').read()
    out=[]
    for m in re.finditer(rb'stream\r?\n', data):
        start=m.end()
        end=data.find(b'endstream', start)
        if end<0: continue
        raw=data[start:end]
        try: out.append(zlib.decompress(raw).decode('latin-1'))
        except Exception:
            try: out.append(raw.decode('latin-1'))
            except Exception: pass
    return out
if __name__=='__main__':
    for s in streams(sys.argv[1]):
        sys.stdout.write(s)
