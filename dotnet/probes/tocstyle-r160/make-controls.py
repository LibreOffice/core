#!/usr/bin/env python3
"""The three controls that cannot share a document with the fixture's arms.

The registration a `TOC \\t` switch makes is document-wide, so a control asking "what happens
WITHOUT such a switch naming this style" needs a document of its own.  Each case here is one
file; run it and read the printed `fo:margin-bottom`, which is `0in` when the paragraph's own
`<w:spacing w:after="0"/>` survived and `0.0835in` when the style's 6 pt won.
"""
import os, re, subprocess, sys, zipfile, importlib.util

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location('fixture', os.path.join(HERE, 'make-probe.py'))
fixture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixture)
W = fixture.W

CASES = [
    ('F-other-level',    'Heading 2,1'),   # a \t naming a heading the paragraph is not in
    ('H-outline-switch', None),            # a \o switch and no \t at all
    ('L-no-toc',         ''),              # no TOC field in the document
]


def document(template):
    field = '' if template == '' else fixture.toc(template)
    return ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f'<w:document xmlns:w="{W}"><w:body>'
            '<w:p><w:r><w:t>lead in</w:t></w:r></w:p>' + field
            + '<w:p><w:pPr><w:pStyle w:val="Heading3"/><w:spacing w:after="0"/></w:pPr>'
              '<w:r><w:t>ZZMARKER</w:t></w:r></w:p>'
              '<w:p><w:r><w:t>follower</w:t></w:r></w:p>'
              '<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>'
              '<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/></w:sectPr>'
              '</w:body></w:document>')


def main(outdir, soffice='/opt/libreoffice26.2/program/soffice'):
    os.makedirs(outdir, exist_ok=True)
    for label, template in CASES:
        parts = dict(fixture.PARTS)
        parts['word/document.xml'] = document(template)
        path = os.path.join(outdir, label + '.docx')
        with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
            for name, text in parts.items():
                z.writestr(name, text)

        subprocess.run([soffice, '--headless', '--norestore',
                        '-env:UserInstallation=file:///tmp/claude-0/lo-tocstyle',
                        '--convert-to', 'fodt', '--outdir', outdir, path],
                       capture_output=True, timeout=600,
                       env={**os.environ, 'HOME': '/tmp/claude-0/lo-tocstyle-home'})
        f = open(path[:-5] + '.fodt', encoding='utf-8').read()
        at = f.find('ZZMARKER')
        start = max(f.rfind('<text:h', 0, at), f.rfind('<text:p', 0, at))
        name = re.search(r'text:style-name="([^"]+)"', f[start:f.find('>', start) + 1]).group(1)
        m = re.search(r'<style:style style:name="%s"[^>]*>.*?</style:style>' % re.escape(name),
                      f, re.S)
        own = re.sub(r'\s+', ' ', m.group(0)) if m else ''
        bottom = re.search(r'fo:margin-bottom="([^"]+)"', own)
        print(f'{label:18} style={name:16} margin-bottom={bottom and bottom.group(1)}')


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '/tmp/claude-0/o110/controls')
