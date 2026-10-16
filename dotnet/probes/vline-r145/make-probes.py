#!/usr/bin/env python3
"""Author one minimal DOCX per question about a top-level `v:line`'s coordinates.

Every probe holds, in the FIRST body paragraph:
  * a marker `v:rect` at style left:0;top:0;width:1pt;height:1pt, stroked green 0.5pt --
    it fixes where the anchor's own origin lands on the page, so the line can be read
    against it rather than against an assumption; and
  * one `v:line` with the attributes the probe is about, stroked red 2pt.

An (almost) empty `word/settings.xml` is included in every probe but `nosettings`, which
exists precisely to show what dropping it costs: without one the importer takes different
OOXML compatibility defaults. See `dotnet/CLAUDE.md` and the `office-corpus` skill.
"""
import zipfile, pathlib, sys

R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'

CT = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
{settings_ct}<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
</Types>'''
SETTINGS_CT = '<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>\n'

RELS = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/officeDocument" Target="word/document.xml"/>
</Relationships>'''

DOCRELS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
{rows}</Relationships>'''

SETTINGS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
            xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml">
<w:compat><w:compatSetting w:name="compatibilityMode"
  w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat>
</w:settings>'''

STYLES = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/><w:sz w:val="24"/>
</w:rPr></w:rPrDefault></w:docDefaults>
</w:styles>'''

MARKER = ('<w:r><w:pict><v:rect id="marker" style="position:absolute;left:0;top:0;'
          'width:1pt;height:1pt" filled="f" strokecolor="#00A000" strokeweight="0.5pt"/>'
          '</w:pict></w:r>')

DOC = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
            xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
            xmlns:v="urn:schemas-microsoft-com:vml"
            xmlns:o="urn:schemas-microsoft-com:office:office"
            xmlns:w10="urn:schemas-microsoft-com:office:word">
<w:body>
{lead}<w:p><w:r><w:t>Anchor.</w:t></w:r>{marker}<w:r><w:pict>{line}</w:pict></w:r></w:p>
<w:p><w:r><w:t>Tail.</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"
         w:header="708" w:footer="708" w:gutter="0"/></w:sectPr>
</w:body></w:document>'''

def line(style_extra='', frm='0,0', to='144pt,0', extra=''):
    style = 'position:absolute;z-index:251658240' + (';' + style_extra if style_extra else '')
    return (f'<v:line id="probe" style="{style}" from="{frm}" to="{to}" '
            f'strokecolor="#FF0000" strokeweight="2pt"{extra}/>')

PROBES = {
    # name              : (style additions, from, to, extra attrs, lead paragraphs, settings?)
    'base':      dict(),
    'lefttop':   dict(style_extra='left:100pt;top:50pt'),
    'wh':        dict(style_extra='width:200pt;height:100pt'),
    'both':      dict(style_extra='left:100pt;top:50pt;width:200pt;height:100pt'),
    'diag':      dict(to='144pt,72pt'),
    'revh':      dict(frm='144pt,0', to='0,0'),
    'revboth':   dict(frm='144pt,72pt', to='0,0'),
    'revv':      dict(frm='0,72pt', to='144pt,0'),
    'px':        dict(frm='0,0', to='96,48'),
    'inch':      dict(frm='1in,0.5in', to='3in,0.5in'),
    'relpage':   dict(style_extra='mso-position-horizontal-relative:page;'
                                  'mso-position-vertical-relative:page'),
    'relmargin': dict(style_extra='mso-position-horizontal-relative:margin;'
                                  'mso-position-vertical-relative:margin'),
    'strokedf':  dict(extra=' stroked="f"'),
    'thirdpara': dict(lead='<w:p><w:r><w:t>One.</w:t></w:r></w:p>'
                           '<w:p><w:r><w:t>Two.</w:t></w:r></w:p>'),
    'nosettings':dict(settings=False),
}

def build(name, spec, outdir):
    settings = spec.pop('settings', True)
    lead = spec.pop('lead', '')
    body = DOC.format(lead=lead, marker=MARKER, line=line(**spec))
    rows = ''
    if settings:
        rows += f'<Relationship Id="rId1" Type="{R}/settings" Target="settings.xml"/>\n'
    rows += f'<Relationship Id="rId2" Type="{R}/styles" Target="styles.xml"/>\n'
    path = outdir / f'{name}.docx'
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml',
                   CT.format(settings_ct=SETTINGS_CT if settings else ''))
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/_rels/document.xml.rels', DOCRELS.format(rows=rows))
        if settings:
            z.writestr('word/settings.xml', SETTINGS)
        z.writestr('word/styles.xml', STYLES)
        z.writestr('word/document.xml', body)
    return path

if __name__ == '__main__':
    outdir = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'probes')
    outdir.mkdir(parents=True, exist_ok=True)
    for name, spec in PROBES.items():
        p = build(name, dict(spec), outdir)
        print(f'{p}  {p.stat().st_size} B')
