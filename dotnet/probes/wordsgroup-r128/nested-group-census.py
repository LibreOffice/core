#!/usr/bin/env python3
"""Nested DrawingML groups in the corpus's word-processing documents, and their child scales.

Round 124 attributed two documents' misdrawn frames to a nested `wpg:grpSp` with a
non-identity child transform. This measures how many such groups exist and how far from
the identity each one's `ext / chExt` is, so the class the hypothesis named can be sized
whether or not it turns out to be the cause.

Usage: nested-group-census.py <corpus-root>
"""
import collections
import os
import sys
import xml.etree.ElementTree as ET
import zipfile

A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
GROUPS = ('wgp', 'grpSp', 'wpc')
TOLERANCE = 0.02          # 2 per cent, the threshold `words-group-transform` censused at


def transform(group):
    """(sx, sy) of a group's own child transform, or None when it states none."""
    props = next((c for c in group if c.tag.split('}')[-1] in ('grpSpPr', 'spPr')), None)
    if props is None:
        return None
    xfrm = props.find(f'{{{A}}}xfrm')
    if xfrm is None:
        return None
    ext = xfrm.find(f'{{{A}}}ext')
    chext = xfrm.find(f'{{{A}}}chExt')
    if ext is None or chext is None:
        return None
    try:
        cx, cy = int(ext.get('cx')), int(ext.get('cy'))
        hx, hy = int(chext.get('cx')), int(chext.get('cy'))
    except (TypeError, ValueError):
        return None
    if hx == 0 or hy == 0:
        return None
    return cx / hx, cy / hy


def walk(node, depth, out):
    for child in node:
        local = child.tag.split('}')[-1]
        if local in GROUPS:
            out.append((depth, local, transform(child)))
            walk(child, depth + 1, out)
        else:
            walk(child, depth, out)


def scan(path):
    groups = []
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return groups
    with z:
        for name in z.namelist():
            if not (name.startswith('word/') and name.endswith('.xml')):
                continue
            try:
                root = ET.fromstring(z.read(name))
            except Exception:
                continue
            for anchor in root.iter():
                if anchor.tag.split('}')[-1] not in ('anchor', 'inline'):
                    continue
                found = []
                walk(anchor, 0, found)
                groups.extend(found)
    return groups


def main():
    root = sys.argv[1]
    outer = nested = nested_docs = 0
    nested_scaled = 0
    docs_with_nested = set()
    docs_with_scaled = set()
    rows = []
    total_docs = 0
    for base, _dirs, files in os.walk(root):
        for f in sorted(files):
            low = f.lower()
            if not low.endswith(('.docx', '.docm', '.dotx', '.dotm')):
                continue
            total_docs += 1
            groups = scan(os.path.join(base, f))
            if not groups:
                continue
            o = sum(1 for d, _k, _t in groups if d == 0)
            n = sum(1 for d, _k, _t in groups if d > 0)
            scaled = [(d, k, t) for d, k, t in groups
                      if d > 0 and t is not None
                      and (abs(t[0] - 1) > TOLERANCE or abs(t[1] - 1) > TOLERANCE)]
            outer += o
            nested += n
            nested_scaled += len(scaled)
            if n:
                docs_with_nested.add(f)
            if scaled:
                docs_with_scaled.add(f)
                worst = max(scaled, key=lambda g: max(abs(g[2][0] - 1), abs(g[2][1] - 1)))
                rows.append((max(abs(worst[2][0] - 1), abs(worst[2][1] - 1)),
                             len(scaled), n, f,
                             f'{worst[2][0]:.4f}x{worst[2][1]:.4f}'))
    rows.sort(reverse=True)
    print('worst\tscaled\tnested\tdocument\tworst-scale')
    for r in rows:
        print(f'{r[0]:.4f}\t{r[1]}\t{r[2]}\t{r[3]}\t{r[4]}')
    print(f'# docx scanned: {total_docs}')
    print(f'# outermost groups: {outer}   nested groups: {nested}')
    print(f'# documents holding a nested group: {len(docs_with_nested)}')
    print(f'# nested groups whose child scale is off the identity by >2%: {nested_scaled}'
          f' in {len(docs_with_scaled)} documents')


main()
