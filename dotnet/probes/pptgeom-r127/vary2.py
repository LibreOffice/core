import json
cells=json.load(open('cells.json'))
mods=json.load(open('ref_modifiers.json'))
VALUES=[2000,5000,9000]
head=open('census.fodg').read().split('  <draw:page')[0]
for (tag,w,h) in (('sq',4,4),('wide',8,4)):
    for v in VALUES:
        parts=[]
        for idx,c in enumerate(cells):
            d=mods[str(c['spt'])][1]
            if not d: continue
            vals=d.split(); vals[0]=str(v)
            x=1+30*idx
            parts.append(f'   <draw:custom-shape draw:style-name="gr1" svg:width="{w}cm" svg:height="{h}cm" svg:x="{x}cm" svg:y="1cm">'
                         f'<draw:enhanced-geometry draw:type="{c["odf"]}" draw:modifiers="{" ".join(vals)}"/></draw:custom-shape>')
        open(f'v{tag}{v}.fodg','w').write(head+'  <draw:page draw:name="p1" draw:master-page-name="M1">\n'+'\n'.join(parts)+
            '\n  </draw:page>\n </office:drawing></office:body>\n</office:document>\n')
