#!/usr/bin/env python3
"""How many `.xls` hyperlink cells state a colour of their own — O65's reach.

    xls-linkcolour-census.py <corpus-root> <workdir> > census.tsv

26.2.4.2's own `--convert-to fods` is the instrument, and C15 is what makes it the RIGHT one
here rather than the wrong one: `ScXMLExport` resolves a URL field with both out-parameters
null (`xmlexprt.cxx`:3062), so the flat file prints the cell's *own* stated colour rather than
the LINKS colour the PDF draws.  That is precisely the quantity this census is after — whether
the cell's edit text carries a hard colour at the field, which is what beats the link colour
under `#i1550` (`editeng/source/editeng/impedit3.cxx`:2947-2957).

Columns: document, hyperlink cells, of which stating a colour, of which stating a non-black one.
"""
import os, re, shutil, subprocess, sys, tempfile

SOFFICE = '/opt/libreoffice26.2/program/soffice'
ROOT = sys.argv[1]
WORK = sys.argv[2]

CELL = re.compile(r'<table:table-cell\b.*?(?:/>|</table:table-cell>)', re.S)
STYLE = re.compile(r'<style:style style:name="([^"]+)"[^>]*>(.*?)</style:style>', re.S)
COLOUR = re.compile(r'fo:color="([^"]+)"')


def colours(text):
    return {name: COLOUR.search(body).group(1)
            for name, body in STYLE.findall(text) if COLOUR.search(body)}


def one(path, work):
    tmp = tempfile.mkdtemp(prefix='o65-', dir=work)
    try:
        subprocess.run(['timeout', '-k', '30', '900', SOFFICE,
                        '-env:UserInstallation=file://' + tmp + '/p', '--headless', '--norestore',
                        '--convert-to', 'fods', '--outdir', tmp, path], capture_output=True)
        out = os.path.join(tmp, os.path.splitext(os.path.basename(path))[0] + '.fods')
        if not os.path.exists(out):
            return None
        text = open(out, encoding='utf-8', errors='replace').read()
        named = colours(text)
        linked = stated = nonblack = 0
        for cell in CELL.findall(text):
            if '<text:a ' not in cell:
                continue
            linked += 1
            found = set()
            for m in re.finditer(r'text:style-name="([^"]+)"|table:style-name="([^"]+)"', cell):
                for name in m.groups():
                    if name and name in named:
                        found.add(named[name])
            if found:
                stated += 1
                if any(c.lower() not in ('#000000',) for c in found):
                    nonblack += 1
        return linked, stated, nonblack
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


docs = sorted(os.path.join(d, f)
              for d, _, fs in os.walk(ROOT) for f in fs if f.lower().endswith('.xls'))
print('document\tlinked_cells\tstating_a_colour\tstating_a_non_black_one')
for path in docs:
    got = one(path, WORK)
    if got is None:
        print('%s\tref-failed\t-\t-' % os.path.basename(path))
        continue
    if got[0]:
        print('%s\t%d\t%d\t%d' % ((os.path.basename(path),) + got))
