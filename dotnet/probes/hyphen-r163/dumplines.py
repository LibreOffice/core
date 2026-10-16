import sys,re,subprocess
pdf,pg=sys.argv[1],sys.argv[2]
t=subprocess.run(['pdftotext','-f',pg,'-l',pg,'-bbox',pdf,'-'],capture_output=True,text=True).stdout
ws=[(float(m.group(2)),float(m.group(1)),float(m.group(3)),m.group(5)) for m in re.finditer(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>',t)]
ws.sort(key=lambda w:(round(w[0],1),w[1]))
cur=None;line=[]
def flush():
    if line: print(f'{cur:7.2f} x0={line[0][0]:6.1f} x1={line[-1][1]:6.1f} | '+' '.join(x[2] for x in line))
for y,x0,x1,w in ws:
    if cur is None or abs(y-cur)>2:
        flush(); cur=y; line=[]
    line.append((x0,x1,w))
flush()
