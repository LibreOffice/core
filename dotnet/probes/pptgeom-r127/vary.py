"""Vary the FIRST Escher adjustment (DFF_Prop_adjustValue, property 327) and ask whether this
tree's preset, fed the same value rescaled by 100000/21600, draws what 26.2.4.2 draws.

That is the conversion O73 needs, measured per preset against the reference's own output rather
than argued from the two tables.
"""
import json, subprocess, sys, os
cells=json.load(open('cells.json'))
mods=json.load(open('ref_modifiers.json'))
VALUES=[3000, 8000, 15000]
head=open('census.fodg').read().split('  <draw:page')[0]
for v in VALUES:
    parts=[]
    for idx,c in enumerate(cells):
        d=mods[str(c['spt'])][1]
        if not d: continue
        vals=d.split(); vals[0]=str(v)
        x=1+30*idx
        parts.append(f'   <draw:custom-shape draw:style-name="gr1" svg:width="4cm" svg:height="4cm" svg:x="{x}cm" svg:y="1cm">'
                     f'<draw:enhanced-geometry draw:type="{c["odf"]}" draw:modifiers="{" ".join(vals)}"/></draw:custom-shape>')
    open(f'vary{v}.fodg','w').write(head+'  <draw:page draw:name="p1" draw:master-page-name="M1">\n'+'\n'.join(parts)+
        '\n  </draw:page>\n </office:drawing></office:body>\n</office:document>\n')
