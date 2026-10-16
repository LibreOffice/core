import sys,subprocess,re
p=sys.argv[1]
x=subprocess.run(["pdftotext","-bbox","-f","1","-l","1",p,"-"],capture_output=True,text=True).stdout
ws=[]
for m in re.finditer(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>',x):
    ws.append((float(m.group(1)),float(m.group(2)),float(m.group(3)),m.group(5)))
# title = the words with the smallest yMin (topmost text)
if not ws: print("no words"); sys.exit()
ymin=min(w[1] for w in ws)
line=[w for w in ws if abs(w[1]-ymin)<2]
line.sort()
print(f"span={max(w[2] for w in line)-min(w[0] for w in line):8.3f}  n_tokens={len(line):2d}  {' '.join(w[3] for w in line)}")
