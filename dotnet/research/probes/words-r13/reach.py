#!/usr/bin/env python3
"""How many corpus DOCX carry the one-sided-w:spacing pattern, and against which parents?

A style qualifies when it states exactly one of w:before/w:after in its own w:pPr/w:spacing
and its w:basedOn names a style declared LATER in styles.xml (so the parent still stood at
its Writer pool defaults when the child was imported).
"""
import collections, glob, os, re, sys, zipfile

ROOT = '/workspace/sample-files'
NS = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'

style_re = re.compile(r'<w:style ([^>]*?)/?>(.*?)</w:style>', re.S)


def styles_of(data):
    out = []
    for m in style_re.finditer(data):
        attrs, body = m.group(1), m.group(2)
        sid = re.search(r'w:styleId="([^"]*)"', attrs)
        typ = re.search(r'w:type="([^"]*)"', attrs)
        if not sid or (typ and typ.group(1) != 'paragraph'):
            continue
        name = re.search(r'<w:name w:val="([^"]*)"\s*/>', body)
        based = re.search(r'<w:basedOn w:val="([^"]*)"\s*/>', body)
        ppr = re.search(r'<w:pPr>(.*?)</w:pPr>', body, re.S)
        sp = re.search(r'<w:spacing ([^>]*?)/?>', ppr.group(1)) if ppr else None
        before = after = None
        if sp:
            b = re.search(r'w:before="([^"]*)"', sp.group(1))
            a = re.search(r'w:after="([^"]*)"', sp.group(1))
            ba = re.search(r'w:beforeAutospacing="([^"]*)"', sp.group(1))
            aa = re.search(r'w:afterAutospacing="([^"]*)"', sp.group(1))
            before = b.group(1) if b else (ba.group(1) if ba else None)
            after = a.group(1) if a else (aa.group(1) if aa else None)
        out.append(dict(id=sid.group(1), name=name.group(1) if name else None,
                        based=based.group(1) if based else None,
                        before=before, after=after))
    return out


hits = collections.Counter()
docs = set()
per_doc = collections.defaultdict(list)
files = sorted(glob.glob(os.path.join(ROOT, 'words', '*', '*', '*')))
scanned = 0
for f in files:
    if not f.lower().endswith(('.docx', '.docm')):
        continue
    try:
        z = zipfile.ZipFile(f)
        data = z.read('word/styles.xml').decode('utf8', 'replace')
    except Exception:
        continue
    scanned += 1
    st = styles_of(data)
    order = {s['id']: i for i, s in enumerate(st)}
    byid = {s['id']: s for s in st}
    for i, s in enumerate(st):
        one_sided = (s['before'] is None) != (s['after'] is None)
        if not one_sided or not s['based']:
            continue
        p = byid.get(s['based'])
        pname = (p['name'] if p else None) or s['based']
        later = p is not None and order.get(s['based'], -1) > i
        undefined = p is None
        if later or undefined:
            hits[(pname.lower(), 'undefined' if undefined else 'later')] += 1
            docs.add(f)
            per_doc[f].append((s['id'], s['based'], pname, 'undef' if undefined else 'later'))

print(f'scanned {scanned} docx, {len(docs)} carry the pattern')
for (pname, kind), n in hits.most_common(40):
    print(f'  {n:4}  parent={pname!r} ({kind})')
print()
for f in sorted(per_doc):
    print(' ', f.replace(ROOT + '/', ''))
    for row in per_doc[f][:6]:
        print('      ', row)
