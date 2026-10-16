#!/usr/bin/env python3
"""Census text:section, its column count and its margins across a corpus of ODF files.

A text:section names a section-family style; that style's style:section-properties may carry
style:columns and fo:margin-left / fo:margin-right.  Parent chains are resolved, because a
section style may inherit.
"""
import re, sys, zipfile, pathlib, collections

def parts(path):
    with zipfile.ZipFile(path) as z:
        for n in ('content.xml', 'styles.xml'):
            try:
                yield z.read(n).decode('utf-8', 'replace')
            except KeyError:
                pass

def styles(xml):
    """name -> (parent, section-properties attrs, column count, has column ruler)"""
    out = {}
    for m in re.finditer(r'<style:style\b([^>]*)>(.*?)</style:style>', xml, re.S):
        head, body = m.group(1), m.group(2)
        if 'style:family="section"' not in head:
            continue
        name = re.search(r'style:name="([^"]*)"', head)
        parent = re.search(r'style:parent-style-name="([^"]*)"', head)
        sp = re.search(r'<style:section-properties\b([^>]*)>?', body)
        cols = re.search(r'<style:columns\b[^>]*fo:column-count="(\d+)"', body)
        out[name.group(1) if name else ''] = (
            parent.group(1) if parent else None,
            sp.group(1) if sp else '',
            int(cols.group(1)) if cols else 1,
            len(re.findall(r'<style:column\b', body)))
    return out

tot = collections.Counter()
docs = collections.Counter()
rows = []
for p in sorted(pathlib.Path(sys.argv[1]).rglob('*')):
    if p.suffix.lower() not in ('.odt', '.ott', '.fodt', '.odp', '.ods'):
        continue
    try:
        xmls = list(parts(p))
    except Exception:
        continue
    st = {}
    nsec = 0
    used = []
    for x in xmls:
        st.update(styles(x))
        for m in re.finditer(r'<text:section\b([^>]*)>', x):
            nsec += 1
            n = re.search(r'text:style-name="([^"]*)"', m.group(1))
            used.append(n.group(1) if n else '')
    if not nsec:
        continue
    ncol = nmar = 0
    for u in used:
        seen, cur = set(), u
        cols, attrs = 1, ''
        while cur and cur in st and cur not in seen:
            seen.add(cur)
            par, a, c, _ = st[cur]
            if c > 1 and cols == 1: cols = c
            if not attrs and ('fo:margin-left' in a or 'fo:margin-right' in a): attrs = a
            cur = par
        if cols > 1: ncol += 1
        if attrs:
            ml = re.search(r'fo:margin-left="([^"]*)"', attrs)
            mr = re.search(r'fo:margin-right="([^"]*)"', attrs)
            if (ml and ml.group(1) not in ('0in', '0cm', '0')) or (mr and mr.group(1) not in ('0in', '0cm', '0')):
                nmar += 1
    tot['sections'] += nsec; tot['columned'] += ncol; tot['margined'] += nmar
    docs['any'] += 1
    if ncol: docs['columned'] += 1
    if nmar: docs['margined'] += 1
    rows.append((p.name, nsec, ncol, nmar))

print(f"documents holding a text:section: {docs['any']}   with a columned one: {docs['columned']}"
      f"   with a margined one: {docs['margined']}")
print(f"sections {tot['sections']}   columned {tot['columned']}   margined {tot['margined']}")
for name, n, c, m in sorted(rows, key=lambda r: -(r[2] + r[3])):
    if c or m:
        print(f"  {n:>4} sections  {c:>4} columned  {m:>4} margined   {name[:80]}")
