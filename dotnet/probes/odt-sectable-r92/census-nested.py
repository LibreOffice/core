#!/usr/bin/env python3
"""How many ODF sections sit inside another, counting an index as the section it is.

A `text:table-of-content` and its six siblings import as a `SwSectionNode` exactly as a
`text:section` does, so a census that counts only `text:section` answers *nothing is nested* for a
corpus that has one.  Prints, over every `.odt` under the root:

  * sections and indexes, and how many of each are inside another section of any kind;
  * of those, how many are inside a *columned* one, which is the only case that is visible.

`probes/odt-sectable-r89/census-nested.py` is the ancestor; this one counts the whole corpus rather
than only the columned sections, so the denominator is stated.
"""
import pathlib, sys, zipfile
from xml.etree import ElementTree as ET

T = '{urn:oasis:names:tc:opendocument:xmlns:text:1.0}'
STYLE = '{urn:oasis:names:tc:opendocument:xmlns:style:1.0}'
FO = '{urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0}'
OFFICE = '{urn:oasis:names:tc:opendocument:xmlns:office:1.0}'

INDEXES = ('table-of-content', 'alphabetical-index', 'illustration-index', 'table-index',
           'object-index', 'user-index', 'bibliography')
KINDS = ('section',) + INDEXES


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


def walk(node, root, cols, depth, inside, incol, tally):
    for child in node:
        name = child.tag.split('}')[-1]
        ns = child.tag.split('}')[0] + '}'
        if ns == T and name in KINDS:
            tally['seen'] += 1
            if inside:
                tally['nested'] += 1
            if incol:
                tally['in-columned'] += 1
                tally['docs-in-columned'].add(root)
            walk(child, root, cols, depth + 1, True,
                 incol or child.get(f'{T}style-name') in cols, tally)
        else:
            walk(child, root, cols, depth + 1, inside, incol, tally)


tally = {'seen': 0, 'nested': 0, 'in-columned': 0, 'docs-in-columned': set(), 'docs': 0}
for path in sorted(pathlib.Path(sys.argv[1]).rglob('*.odt')):
    try:
        with zipfile.ZipFile(path) as z:
            root = ET.fromstring(z.read('content.xml'))
    except Exception:
        continue
    tally['docs'] += 1
    body = root.find(f'{OFFICE}body')
    text = body.find(f'{OFFICE}text') if body is not None else None
    if text is None:
        continue
    before = tally['in-columned']
    walk(text, path.name, columned(root), 0, False, False, tally)
    if tally['in-columned'] > before:
        print(f"{tally['in-columned'] - before:>3} inside a columned section   {path.name}")

print(f"TOTAL {tally['docs']} documents, {tally['seen']} sections and indexes, "
      f"{tally['nested']} of them inside another, "
      f"{tally['in-columned']} inside a columned one in {len(tally['docs-in-columned'])} documents")
