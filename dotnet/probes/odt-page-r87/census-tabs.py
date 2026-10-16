#!/usr/bin/env python3
"""Census the documents where TabsRelativeToIndent can change the layout.

The two rules differ only for a paragraph that (a) contains a tab and (b) has a non-zero
left indent, since the tab origin is the indent under one rule and the text-area edge under
the other.  Style resolution follows style:parent-style-name so an automatic style that
states nothing still inherits its parent's margin.
"""
import zipfile, glob, re, sys, collections
from xml.etree import ElementTree as ET

NS = {
    'office': 'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
    'style':  'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
    'text':   'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
    'fo':     'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0',
}
Q = lambda p: '{%s}%s' % (NS[p.split(':')[0]], p.split(':')[1])

def lengths(v):
    if not v: return 0.0
    m = re.match(r'\s*(-?[\d.]+)\s*(cm|mm|in|pt|pc|px)?\s*$', v)
    if not m: return 0.0
    n = float(m.group(1)); u = m.group(2) or 'pt'
    return n * {'cm':28.3465,'mm':2.83465,'in':72.0,'pt':1.0,'pc':12.0,'px':0.75}[u]

def styles_of(root):
    out = {}
    for st in root.iter(Q('style:style')):
        if st.get(Q('style:family')) != 'paragraph': continue
        name = st.get(Q('style:name'))
        par = st.get(Q('style:parent-style-name'))
        pp = st.find(Q('style:paragraph-properties'))
        ml = pp.get(Q('fo:margin-left')) if pp is not None else None
        out[name] = (par, ml)
    return out

def resolve(styles, name, depth=0):
    while name and depth < 12:
        par, ml = styles.get(name, (None, None))
        if ml is not None: return lengths(ml)
        name = par; depth += 1
    return 0.0

hits = []
files = sorted(glob.glob(sys.argv[1]))
tot_par = 0
for f in files:
    z = zipfile.ZipFile(f)
    styles = {}
    for part in ('styles.xml', 'content.xml'):
        try: styles.update(styles_of(ET.fromstring(z.read(part))))
        except KeyError: pass
    root = ET.fromstring(z.read('content.xml'))
    n = 0
    for p in root.iter():
        if p.tag not in (Q('text:p'), Q('text:h')): continue
        if p.find(Q('text:tab')) is None: continue
        if resolve(styles, p.get(Q('text:style-name'))) > 0.01:
            n += 1
    tot_par += n
    if n: hits.append((n, f))
hits.sort(reverse=True)
print(f"{len(hits)} of {len(files)} documents hold a tabbed paragraph with a non-zero left indent;"
      f" {tot_par} such paragraphs")
for n, f in hits[:25]:
    print(f"{n:7d}  {f.split('/')[-1]}")
