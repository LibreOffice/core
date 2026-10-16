#!/usr/bin/env python3
"""Documents whose TOC field names paragraph styles, and what that costs in 26.2.4.2.

26.2.4.2 discards the whole direct `w:pPr` of every paragraph whose style is named in a
`TOC` field's `\\t` switch (see `results.md` SS2). The blast radius of that is:

  * documents with a `TOC ... \\t "Name,level,..."` field at all;
  * of those, the paragraphs carrying that style;
  * of those, the ones that state direct paragraph formatting the reference will throw
    away -- anything in `w:pPr` other than `w:pStyle`, `w:rPr` and `w:sectPr`.

`\\o` and a bare `TOC` are counted separately: they are the control, and the probe shows
they do not fire.

Usage: toc-template-census.py <corpus-root>
"""
import collections
import os
import re
import sys
import xml.etree.ElementTree as ET
import zipfile

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'
IGNORE = {'pStyle', 'rPr', 'sectPr'}
TEMPLATE = re.compile(r'\\t\s+"([^"]*)"')


def styles_by_name(z):
    """styleId -> style name, so a `\\t` name can be matched back to `w:pStyle/@val`."""
    try:
        root = ET.fromstring(z.read('word/styles.xml'))
    except Exception:
        return {}
    out = {}
    for s in root.findall(f'{{{W}}}style'):
        sid = s.get(f'{{{W}}}styleId')
        name = s.find(f'{{{W}}}name')
        if sid and name is not None:
            out[sid] = (name.get(f'{{{W}}}val') or '').strip().lower()
    return out


def scan(path):
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return None
    with z:
        names = styles_by_name(z)
        templates = []
        kinds = collections.Counter()
        paragraphs = collections.Counter()
        direct = collections.Counter()
        for part in z.namelist():
            if not (part.startswith('word/') and part.endswith('.xml')):
                continue
            try:
                root = ET.fromstring(z.read(part))
            except Exception:
                continue
            for instr in root.iter(f'{{{W}}}instrText'):
                text = instr.text or ''
                if 'TOC' not in text:
                    continue
                if '\\t' in text:
                    kinds['t'] += 1
                    for m in TEMPLATE.finditer(text):
                        templates.append(m.group(1))
                elif '\\o' in text:
                    kinds['o'] += 1
                else:
                    kinds['bare'] += 1
        if not templates:
            return (kinds, set(), 0, 0)

        wanted = set()
        for tpl in templates:
            parts = [p.strip() for p in re.split('[,;]', tpl)]
            for p in parts:
                if p and not p.isdigit():
                    wanted.add(p.lower())
        ids = {sid for sid, name in names.items() if name in wanted}
        ids |= {w.replace(' ', '') for w in wanted}

        for part in z.namelist():
            if not (part.startswith('word/') and part.endswith('.xml')):
                continue
            try:
                root = ET.fromstring(z.read(part))
            except Exception:
                continue
            for p in root.iter(f'{{{W}}}p'):
                pr = p.find(f'{{{W}}}pPr')
                if pr is None:
                    continue
                st = pr.find(f'{{{W}}}pStyle')
                if st is None:
                    continue
                sid = st.get(f'{{{W}}}val') or ''
                if sid not in ids:
                    continue
                paragraphs[sid] += 1
                if any(c.tag.split('}')[-1] not in IGNORE for c in pr):
                    direct[sid] += 1
        return (kinds, wanted, sum(paragraphs.values()), sum(direct.values()))


def main():
    root = sys.argv[1]
    total = withtoc = witht = witht_direct = 0
    rows = []
    for base, _dirs, files in os.walk(root):
        for f in sorted(files):
            if not f.lower().endswith(('.docx', '.docm', '.dotx', '.dotm')):
                continue
            total += 1
            got = scan(os.path.join(base, f))
            if got is None:
                continue
            kinds, wanted, paragraphs, direct = got
            if sum(kinds.values()):
                withtoc += 1
            if kinds.get('t'):
                witht += 1
                if direct:
                    witht_direct += 1
                rows.append((direct, paragraphs, f, ';'.join(sorted(wanted))))
    rows.sort(reverse=True)
    print('direct\tstyled\tdocument\tstyles')
    for r in rows:
        print(f'{r[0]}\t{r[1]}\t{r[2]}\t{r[3]}')
    print(f'# docx scanned: {total}')
    print(f'# with any TOC field: {withtoc}')
    print(f'# with a TOC \\t template: {witht}')
    print(f'# of those, holding a paragraph the reference would strip: {witht_direct}')


main()
