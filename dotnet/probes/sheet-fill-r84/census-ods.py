#!/usr/bin/env python3
"""What an ODF sheet shape's graphic style declares for its fill and outline.

    census-ods.py <corpus-root>

Walks every `.ods`, descends through the transparent `draw:g`/`draw:a` wrappers exactly as
`OdsDrawings.Shapes` does, and resolves each shape's `draw:style-name` through its parent
chain — because `draw:fill` is very often on the *parent* style rather than on the automatic
one the shape names.
"""
import collections, os, sys, zipfile
import xml.etree.ElementTree as ET

NS = {
    'office': 'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
    'style': 'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
    'table': 'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
    'draw': 'urn:oasis:names:tc:opendocument:xmlns:drawing:1.0',
    'text': 'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
    'svg': 'urn:oasis:names:tc:opendocument:xmlns:svg-compatible-draw:1.0',
}
D = '{' + NS['draw'] + '}'
T = '{' + NS['table'] + '}'
TX = '{' + NS['text'] + '}'
S = '{' + NS['style'] + '}'
O = '{' + NS['office'] + '}'
SVG = '{' + NS['svg'] + '}'


def styles_of(root):
    """name -> (parent, {qname: value}) over graphic-properties, both style containers."""
    out = {}
    for container in ('automatic-styles', 'styles'):
        c = root.find(O + container)
        if c is None:
            continue
        for st in c.findall(S + 'style'):
            if st.get(S + 'family') != 'graphic':
                continue
            props = {}
            gp = st.find(S + 'graphic-properties')
            if gp is not None:
                props = dict(gp.attrib)
            out[st.get(S + 'name')] = (st.get(S + 'parent-style-name'), props)
    return out


def resolve(styles, name, attr, depth=0):
    while name and depth < 12:
        entry = styles.get(name)
        if entry is None:
            return None
        parent, props = entry
        if attr in props:
            return props[attr]
        name, depth = parent, depth + 1
    return None


def shapes(container):
    for child in container:
        if not child.tag.startswith(D):
            continue
        if child.tag in (D + 'g', D + 'a'):
            yield from shapes(child)
        else:
            yield child


def has_ink(shape):
    for p in shape.iter(TX + 'p'):
        if ''.join(p.itertext()).strip():
            return True
    return False


def main(root):
    kinds = collections.Counter()
    fills = collections.Counter()
    strokes = collections.Counter()
    docs = collections.defaultdict(set)
    inked_textless = collections.Counter()
    presets = collections.Counter()
    for dirpath, _, files in os.walk(root):
        for name in sorted(files):
            if not name.lower().endswith('.ods'):
                continue
            path = os.path.join(dirpath, name)
            rel = os.path.relpath(path, root)
            try:
                z = zipfile.ZipFile(path)
                tree = ET.fromstring(z.read('content.xml'))
            except Exception:
                continue
            st = styles_of(tree)
            try:
                st.update(styles_of(ET.fromstring(z.read('styles.xml'))))
            except Exception:
                pass
            for cell in tree.iter():
                if cell.tag not in (T + 'table-cell', T + 'covered-table-cell', T + 'shapes'):
                    continue
                for sh in shapes(cell):
                    tag = sh.tag.replace(D, '')
                    kinds[tag] += 1
                    docs[tag].add(rel)
                    sn = sh.get(D + 'style-name')
                    f = resolve(st, sn, D + 'fill') or 'absent'
                    s = resolve(st, sn, D + 'stroke') or 'absent'
                    fills[f] += 1
                    strokes[s] += 1
                    ink = f not in ('absent', 'none') or s not in ('absent', 'none')
                    if ink:
                        docs['ink'].add(rel)
                        if not has_ink(sh):
                            inked_textless[tag] += 1
                            docs['inked-textless'].add(rel)
                        else:
                            inked_textless['(with text) ' + tag] += 1
                    geo = sh.find(D + 'enhanced-geometry')
                    if geo is not None and geo.get(D + 'type'):
                        presets[geo.get(D + 'type')] += 1
    print('shapes in cells / table:shapes, by kind')
    for k, v in kinds.most_common():
        print(f'  {k:16s} {v:5d} in {len(docs[k]):3d} documents')
    print('\ndraw:fill  :', fills.most_common())
    print('draw:stroke:', strokes.most_common())
    print(f'\nshapes with ink (fill or stroke) in {len(docs["ink"])} documents')
    for k, v in inked_textless.most_common():
        print(f'  {k:28s} {v:5d}')
    print(f'  ...textless inked shapes live in {len(docs["inked-textless"])} documents')
    print('\npresets:', presets.most_common(15))


if __name__ == '__main__':
    main(sys.argv[1])
