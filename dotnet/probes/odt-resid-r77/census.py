#!/usr/bin/env python3
"""Census the converted `.odt` corpus for the five things this round read.

    census.py <what> [corpus]

`what` is one of

    shrink      documents stating JustifyLinesWithShrinking, and how many also justify
    relwidth    tables whose style:rel-width counts, and columns stating a proportion
    loext       documents holding a loext:table
    nested      shape text nested inside a draw:frame's own content
    furniture   dynamic headers and footers whose content plainly outruns their floor

Every figure quoted in `results.md` comes from here.  The corpus defaults to
`/home/user/corpus-odf/words`, which is 26.2.4.2's own `--convert-to odt` of the words track.
"""
import collections
import glob
import os
import re
import sys
import xml.etree.ElementTree as ET
import zipfile

D = '{urn:oasis:names:tc:opendocument:xmlns:drawing:1.0}'
O = '{urn:oasis:names:tc:opendocument:xmlns:office:1.0}'
S = '{urn:oasis:names:tc:opendocument:xmlns:style:1.0}'
T = '{urn:oasis:names:tc:opendocument:xmlns:table:1.0}'
FO = '{urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0}'
SVG = '{urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0}'
TXT = '{urn:oasis:names:tc:opendocument:xmlns:text:1.0}'


def parts(path):
    z = zipfile.ZipFile(path)
    names = z.namelist()
    return {n: z.read(n).decode('utf-8', 'replace') for n in ('content.xml', 'styles.xml', 'settings.xml')
            if n in names}


def length(value):
    if not value:
        return None
    m = re.match(r'^(-?[\d.]+)(in|cm|mm|pt|pc)$', value.strip())
    if not m:
        return None
    return float(m.group(1)) * {'in': 72, 'cm': 28.3465, 'mm': 2.83465, 'pt': 1, 'pc': 12}[m.group(2)]


def main(what, root):
    files = sorted(glob.glob(os.path.join(root, '**', 'odt', '*.odt'), recursive=True))
    print(f'{len(files)} documents under {root}')
    tally = collections.Counter()

    for path in files:
        p = parts(path)
        content = p.get('content.xml', '')
        styles = p.get('styles.xml', '')
        settings = p.get('settings.xml', '')

        if what == 'shrink':
            m = re.search(r'name="JustifyLinesWithShrinking"[^>]*>([^<]*)<', settings)
            on = m is not None and m.group(1) == 'true'
            justify = 'text-align="justify"' in content + styles
            tally['setting true'] += on
            tally['justifies'] += justify
            tally['both'] += on and justify
            tally['states word-spacing-minimum'] += 'word-spacing-minimum="' in content + styles

        elif what == 'relwidth':
            oriented = 0
            for part in (content, styles):
                if not part:
                    continue
                for st in ET.fromstring(part).iter(S + 'style'):
                    if st.get(S + 'family') != 'table':
                        continue
                    props = st.find(S + 'table-properties')
                    if props is None:
                        continue
                    if props.get(S + 'rel-width') and props.get(T + 'align') in ('left', 'center', 'right'):
                        oriented += 1
            tally['oriented rel-width tables'] += oriented
            tally['documents with an oriented rel-width table'] += oriented > 0

            proportional = 0
            for pr in re.findall(r'<style:table-column-properties[^>]*>', content + styles):
                absolute = 'style:column-width=' in pr
                relative = 'style:rel-column-width=' in pr
                tally['columns stating both'] += absolute and relative
                tally['columns stating a proportion only'] += relative and not absolute
                tally['columns stating a length only'] += absolute and not relative
                proportional += relative and not absolute
            tally['documents with a proportion-only column'] += proportional > 0

        elif what == 'loext':
            if re.search(r'<loext:table[ >]', content):
                tally['documents'] += 1
                tally['tables'] += len(re.findall(r'<loext:table[ >]', content))

        elif what == 'nested':
            body = ET.fromstring(content).find(O + 'body')
            if body is None:
                continue
            nested = 0
            for frame in body.iter(D + 'frame'):
                for shape in frame.iter():
                    if shape is frame:
                        continue
                    if shape.tag in (D + 'custom-shape', D + 'rect', D + 'frame'):
                        nested += sum(1 for c in ''.join(shape.itertext()) if c.isalnum())
            if nested:
                tally['documents'] += 1
                tally['characters'] += nested

        elif what == 'furniture':
            if not styles:
                continue
            root_el = ET.fromstring(styles)
            layouts = {}
            for layout in root_el.iter(S + 'page-layout'):
                kinds = {}
                for kind in ('header', 'footer'):
                    el = layout.find(S + kind + '-style')
                    props = el.find(S + 'header-footer-properties') if el is not None else None
                    if props is None:
                        kinds[kind] = None
                    elif props.get(SVG + 'height'):
                        kinds[kind] = ('fixed', length(props.get(SVG + 'height')))
                    else:
                        kinds[kind] = ('min', length(props.get(FO + 'min-height')) or 0.0)
                layouts[layout.get(S + 'name')] = kinds
            worst = 0.0
            for master in root_el.iter(S + 'master-page'):
                kinds = layouts.get(master.get(S + 'page-layout-name')) or {}
                for kind in ('header', 'footer'):
                    el = master.find(S + kind)
                    if el is None or (kinds.get(kind) or ('fixed', 0))[0] != 'min':
                        continue
                    lines = sum(1 for _ in el.iter(TXT + 'p'))
                    worst = max(worst, lines * 12.0 - kinds[kind][1])
            if worst > 12:
                tally['documents whose furniture outruns its floor'] += 1

        else:
            raise SystemExit(f'unknown census: {what}')

    for key, value in tally.most_common():
        print(f'  {value:8d}  {key}')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else '/home/user/corpus-odf/words')
