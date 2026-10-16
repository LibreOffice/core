#!/usr/bin/env python3
"""Two more probes: a v:line INSIDE a v:group, to settle whether `from`/`to` are read as
bare numbers in the group's own coordinate space (as `getRelRectangle` would) or in real
units. The group is 200pt x 100pt at left:0;top:0 with coordsize 1000,1000."""
import zipfile, pathlib, sys
sys.path.insert(0, '.')
import importlib.util
spec = importlib.util.spec_from_file_location('mp', 'make-probes.py')
mp = importlib.util.module_from_spec(spec); spec.loader.exec_module(mp)

GROUP = ('<v:group id="g" style="position:absolute;left:0;top:0;width:200pt;height:100pt" '
         'coordsize="1000,1000" coordorigin="0,0">'
         '<v:rect id="gm" style="position:absolute;left:0;top:0;width:1000;height:1000" '
         'filled="f" strokecolor="#0000FF" strokeweight="0.5pt"/>'
         '{inner}</v:group>')

CASES = {
  'group-full':  '<v:line id="probe" style="position:absolute" from="0,0" to="1000,1000" '
                 'strokecolor="#FF0000" strokeweight="2pt"/>',
  'group-half':  '<v:line id="probe" style="position:absolute" from="250,250" to="750,250" '
                 'strokecolor="#FF0000" strokeweight="2pt"/>',
  'group-pt':    '<v:line id="probe" style="position:absolute" from="0,0" to="100pt,50pt" '
                 'strokecolor="#FF0000" strokeweight="2pt"/>',
}

outdir = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'probes')
outdir.mkdir(parents=True, exist_ok=True)
R = mp.R
for name, inner in CASES.items():
    body = mp.DOC.format(lead='', marker=mp.MARKER, line=GROUP.format(inner=inner))
    rows = (f'<Relationship Id="rId1" Type="{R}/settings" Target="settings.xml"/>\n'
            f'<Relationship Id="rId2" Type="{R}/styles" Target="styles.xml"/>\n')
    path = outdir / f'{name}.docx'
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', mp.CT.format(settings_ct=mp.SETTINGS_CT))
        z.writestr('_rels/.rels', mp.RELS)
        z.writestr('word/_rels/document.xml.rels', mp.DOCRELS.format(rows=rows))
        z.writestr('word/settings.xml', mp.SETTINGS)
        z.writestr('word/styles.xml', mp.STYLES)
        z.writestr('word/document.xml', body)
    print(path, path.stat().st_size)
