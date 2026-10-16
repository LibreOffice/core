#!/usr/bin/env python3
r"""What the reference resolves for every paragraph of a real document whose style chain passes
through NAME -- size, slant, weight and the four paragraph properties, with the level each was
found at.

`readfodt.py` answers the same question for a probe, by looking for the word HEAD. A corpus
document has no marker, so this one selects on the *chain* instead and collapses identical rows
with a count, which is what makes seventeen paragraphs readable as two lines.

It shares `readfodt.py`'s one hard-won rule: a character property is resolved from the first
`text:span` of the paragraph **before** the paragraph-style chain, because that is where the
reference writes the reset it puts over a style's own `\fs`, and no walk up
`style:parent-style-name` can see it. A value found there is reported `@direct`.

  corpusfodt.py <file.fodt> <style name to select on>
"""
import sys, xml.etree.ElementTree as ET
NS = {'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
      'style':'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
      'text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
      'fo':'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0'}
Q = {k:'{%s}'%v for k,v in NS.items()}
ATTRS = [('size',Q['fo']+'font-size'),('ital',Q['fo']+'font-style'),
         ('weight',Q['fo']+'font-weight')]
PATTRS = [('above',Q['fo']+'margin-top'),('below',Q['fo']+'margin-bottom'),
          ('keep',Q['fo']+'keep-with-next'),('align',Q['fo']+'text-align')]

def collect(root, family):
    out = {}
    for holder in ('styles','automatic-styles'):
        node = root.find(Q['office']+holder)
        if node is None: continue
        for st in node.findall(Q['style']+'style'):
            if st.get(Q['style']+'family') != family: continue
            out[st.get(Q['style']+'name')] = st
    return out

path, want = sys.argv[1], sys.argv[2]
root = ET.parse(path).getroot()
paras = collect(root,'paragraph'); texts = collect(root,'text')
def chain(n):
    out, seen = [], set()
    while n and n in paras and n not in seen:
        seen.add(n); out.append(n)
        n = paras[n].get(Q['style']+'parent-style-name')
    return out
body = root.find(Q['office']+'body/'+Q['office']+'text')
seen_rows = {}
for p in body.iter(Q['text']+'p'):
    c = chain(p.get(Q['text']+'style-name'))
    if want not in c: continue
    span = next((s for s in p.iter(Q['text']+'span') if ''.join(s.itertext())), None)
    direct = texts.get(span.get(Q['text']+'style-name')) if span is not None else None
    dtp = direct.find(Q['style']+'text-properties') if direct is not None else None
    row = ['>'.join(c)]
    for key, a in ATTRS:
        v = dtp.get(a) if dtp is not None else None
        if v is not None: row.append(f'{key}={v}@direct'); continue
        for d,nm in enumerate(c):
            tp = paras[nm].find(Q['style']+'text-properties')
            if tp is not None and tp.get(a) is not None:
                row.append(f'{key}={tp.get(a)}@{nm}'); break
        else: row.append(f'{key}=-')
    for key, a in PATTRS:
        for d,nm in enumerate(c):
            pp = paras[nm].find(Q['style']+'paragraph-properties')
            if pp is not None and pp.get(a) is not None:
                row.append(f'{key}={pp.get(a)}@{nm}'); break
        else: row.append(f'{key}=-')
    key = ' '.join(row)
    seen_rows[key] = seen_rows.get(key,0)+1
for k,v in sorted(seen_rows.items(), key=lambda kv:-kv[1]):
    print(f'{v:>4}x  {k}')
