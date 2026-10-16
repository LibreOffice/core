import re, subprocess, sys, collections
S='/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py'
pat=re.compile(r'stroke p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(#\w+)')
def rect(pdf, page='5'):
    out=subprocess.run(['python3',S,'dump',pdf,'--page',page,'--only','stroke'],capture_output=True,text=True).stdout
    v=[]
    for m in pat.finditer(out):
        x0,y0,x1,y1=(float(m.group(i)) for i in (1,2,3,4))
        if abs(x1-x0)<0.05 and abs(y1-y0)>50 and m.group(5) in ('#000000','#666666'): v.append((x0,y0,y1))
    v.sort()
    return (v[0][0], v[0][1], v[-1][0], v[0][2]) if v else None
def ink(pdf, word, page='5'):
    out=subprocess.run(['pdftotext','-bbox','-f',page,'-l',page,pdf,'-'],capture_output=True,text=True).stdout
    for line in out.splitlines():
        if f'>{word}<' in line:
            g=re.search(r'xMin="([\d.]+)".*xMax="([\d.]+)"',line)
            return float(g.group(1)), float(g.group(2))
    return None
for label,pdf in [('ours','dj/ours/Demick_JetBlue.pdf'),('ours after','dj/ours2/Demick_JetBlue.pdf'),('ref authored','dj/ref/Demick_JetBlue.pdf'),
                  
                  ('ref no-lead','dj/var/r-no-lead/v-no-lead.pdf'),
                  ('ref no-qq','dj/var/r-no-qq/v-no-qq.pdf'),('ref drop-trail','dj/var/r-drop-trail/v-drop-trail.pdf'),('ref drop-both','dj/var/r-drop-both/v-drop-both.pdf')]:
    r=rect(pdf); i=ink(pdf,'$1,200,000.00')
    print(f'{label:14s} plot x {r[0]:8.2f}..{r[2]:8.2f}  y {r[1]:8.2f}..{r[3]:8.2f}   ink {i[0]:7.2f}..{i[1]:7.2f}  gap {r[0]-i[1]:6.2f}')
