import sys, re, pymupdf
def score(path):
    doc=pymupdf.open(path); tot=0; big=0
    for pg in doc:
        try: c=pg.read_contents().decode('latin-1')
        except Exception: continue
        for arr in re.finditer(r'\[((?:<[0-9A-Fa-f]*>|-?\d+|\s)*)\]\s*TJ', c):
            body=arr.group(1)
            g=sum(len(m)//2 for m in re.findall(r'<([0-9A-Fa-f]*)>', body))
            adj=[int(x) for x in re.findall(r'(?<![0-9A-Fa-f<])(-?\d+)(?![0-9A-Fa-f]*>)', body)]
            tot+=g; big+=sum(1 for a in adj if abs(a)>=20)
    return tot, big
for p in sys.argv[1:]:
    t,b=score(p)
    print(f"{t}\t{b}\t{b/max(t,1):.4f}")
