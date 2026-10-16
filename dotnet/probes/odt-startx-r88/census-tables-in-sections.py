#!/usr/bin/env python3
"""Which columned text:sections hold a table, which is the case this round does not lay out.

A table is placed against the page's text area rather than against the column the flow is in, so a
columned section holding one puts its table across both columns and resumes the flow in the second.
"""
import pathlib, sys, zipfile
from xml.etree import ElementTree as ET

T = '{urn:oasis:names:tc:opendocument:xmlns:text:1.0}'
TABLE = '{urn:oasis:names:tc:opendocument:xmlns:table:1.0}table'
STYLE = '{urn:oasis:names:tc:opendocument:xmlns:style:1.0}'
FO = '{urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0}'

def columned(root):
    names = set()
    for style in root.iter(f'{STYLE}style'):
        if style.get(f'{STYLE}family') != 'section':
            continue
        for props in style.iter(f'{STYLE}section-properties'):
            for cols in props.iter(f'{STYLE}columns'):
                if int(cols.get(f'{FO}column-count') or 1) > 1:
                    names.add(style.get(f'{STYLE}name'))
    return names

for path in sorted(pathlib.Path(sys.argv[1]).rglob('*.odt')):
    try:
        with zipfile.ZipFile(path) as z:
            root = ET.fromstring(z.read('content.xml'))
    except Exception:
        continue
    names = columned(root)
    if not names:
        continue
    sections = tables = 0
    for section in root.iter(f'{T}section'):
        if section.get(f'{T}style-name') not in names:
            continue
        sections += 1
        tables += sum(1 for _ in section.iter(TABLE))
    if sections:
        print(f"{sections:>3} columned sections {tables:>4} tables in them   {path.name}")
