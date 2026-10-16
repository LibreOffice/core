#!/usr/bin/env python3
"""Which columned `text:section`s hold a section of their own -- an index or a nested section.

Writer inserts a nested section frame *behind* its parent rather than inside it
(`sw/source/core/layout/frmtool.cxx`:1795-1803), so the parent's columns do not reach it and the
parent is split around it.  An ODF index -- `text:table-of-content` and its seven siblings -- is a
section like any other, which is why counting `text:section` alone answers "none is nested".
"""
import pathlib, sys, zipfile
from xml.etree import ElementTree as ET

T = '{urn:oasis:names:tc:opendocument:xmlns:text:1.0}'
STYLE = '{urn:oasis:names:tc:opendocument:xmlns:style:1.0}'
FO = '{urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0}'

INDEXES = ('table-of-content', 'alphabetical-index', 'illustration-index', 'table-index',
           'object-index', 'user-index', 'bibliography', 'section')


def columned(root):
    names = {}
    for style in root.iter(f'{STYLE}style'):
        if style.get(f'{STYLE}family') != 'section':
            continue
        for props in style.iter(f'{STYLE}section-properties'):
            for cols in props.iter(f'{STYLE}columns'):
                if int(cols.get(f'{FO}column-count') or 1) > 1:
                    names[style.get(f'{STYLE}name')] = True
    return names


totals = [0, 0, 0, 0]
docs = set()
for path in sorted(pathlib.Path(sys.argv[1]).rglob('*.odt')):
    try:
        with zipfile.ZipFile(path) as z:
            root = ET.fromstring(z.read('content.xml'))
    except Exception:
        continue
    names = columned(root)
    if not names:
        continue
    sections = nested = 0
    kinds = {}
    for section in root.iter(f'{T}section'):
        if section.get(f'{T}style-name') not in names:
            continue
        sections += 1
        for kind in INDEXES:
            n = sum(1 for _ in section.iter(f'{T}{kind}'))
            if kind == 'section':
                n -= 1  # itself
            if n:
                kinds[kind] = kinds.get(kind, 0) + n
                nested += n
    totals[0] += sections
    totals[1] += nested
    if sections:
        totals[2] += 1
    if nested:
        totals[3] += 1
        docs.add(path.name)
        print(f"{sections:>3} columned sections {nested:>3} nested   {path.name}   {kinds}")

print(f"TOTAL {totals[0]} columned sections in {totals[2]} documents; "
      f"{totals[1]} nested sections in {totals[3]} documents")
