#!/usr/bin/env python3
"""How many paragraphs in the words corpus state `w:lineRule="atLeast"`?

    python3 atleast-census.py [corpus-root]

`tableleading.py` arm 4 measured a second, independent defect beside the one it was written for.
`SvxLineSpaceRule::Min` — OOXML's `atLeast` — is applied in `SwTextFormatter::CalcRealHeight`
*outside* the `if( !IsParaLine() )` guard (`sw/source/core/text/itrform2.cxx`:2397 against :2425), so
it raises every line of a paragraph including its first. We put the whole raise into the line box's
`SpaceAbove`, and `ParagraphLeading.AsDrawn` then strips `SpaceAbove` from a paragraph's first line
and from a frame's first line because that is where proportional leading lives. A one-line `atLeast`
paragraph therefore loses its raise outright: `w:line="400" w:lineRule="atLeast"` on 11 pt Cambria
draws a 20.00 pt line on the reference and a 12.65 pt line here.

Counted per **paragraph**, and separately per document, because a document with 900 of them is not
900 documents. Resolved through the style chain and `w:docDefaults`, not grepped: the rule is
inherited in this corpus far more often than it is stated.

**Blind spots**, stated before the change:

  * the raise is only visible when the stated value exceeds the paragraph's own natural line height,
    which depends on the resolved face and size and is not computed here — so this is an
    **upper bound** on the sites, not a count of them;
  * `.doc` (66 paths) resolves `sprmPDyaLine`'s negative/positive sign convention in the WW8 reader
    and is not counted at all;
  * a paragraph inside a table cell counts the same as one in the body, and the two do not have the
    same consequence for pagination.
"""
import os
import sys
import zipfile
import xml.etree.ElementTree as ET

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'


def styles_of(z):
    try:
        root = ET.fromstring(z.read('word/styles.xml'))
    except KeyError:
        return {}, (None, None)
    table = {}
    for st in root.findall(W + 'style'):
        if st.get(W + 'type') not in (None, 'paragraph'):
            continue
        sp = st.find(W + 'pPr/' + W + 'spacing')
        based = st.find(W + 'basedOn')
        table[st.get(W + 'styleId')] = (
            sp.get(W + 'line') if sp is not None else None,
            sp.get(W + 'lineRule') if sp is not None else None,
            based.get(W + 'val') if based is not None else None)
    dd = root.find(W + 'docDefaults/' + W + 'pPrDefault/' + W + 'pPr/' + W + 'spacing')
    return table, ((dd.get(W + 'line'), dd.get(W + 'lineRule')) if dd is not None else (None, None))


def resolve(p, table, defaults):
    sp = p.find(W + 'pPr/' + W + 'spacing')
    if sp is not None and sp.get(W + 'line') is not None:
        return sp.get(W + 'line'), sp.get(W + 'lineRule')
    ps = p.find(W + 'pPr/' + W + 'pStyle')
    sid = ps.get(W + 'val') if ps is not None else None
    seen = set()
    while sid and sid in table and sid not in seen:
        seen.add(sid)
        line, rule, based = table[sid]
        if line is not None:
            return line, rule
        sid = based
    return defaults


def count(path):
    with zipfile.ZipFile(path) as z:
        table, defaults = styles_of(z)
        atleast = 0
        atleast_over_20 = 0
        total = 0
        for name in z.namelist():
            base = os.path.basename(name)
            if not name.startswith('word/') or not name.endswith('.xml'):
                continue
            if base != 'document.xml' and not base.startswith(('header', 'footer')):
                continue
            try:
                root = ET.fromstring(z.read(name))
            except ET.ParseError:
                continue
            for p in root.iter(W + 'p'):
                total += 1
                line, rule = resolve(p, table, defaults)
                if rule == 'atLeast' and line is not None:
                    atleast += 1
                    try:
                        if int(line) >= 400:
                            atleast_over_20 += 1
                    except ValueError:
                        pass
    return atleast, atleast_over_20, total


if __name__ == '__main__':
    root = sys.argv[1] if len(sys.argv) > 1 else '/c/sandbox/workdir/sample-files'
    paths = []
    with open(os.path.join(root, 'MANIFEST.tsv'), encoding='utf-8') as f:
        next(f)
        for line in f:
            c = line.rstrip('\n').split('\t')
            if c[0] == 'words':
                paths.append((c[2], c[3], c[7]))
    docx = [(p, e, s) for p, e, s in paths if e in ('docx', 'docm', 'dotx', 'dotm')]
    per = {}
    failed = []
    for p, e, s in docx:
        try:
            per[p] = count(os.path.join(root, p)) + (s,)
        except Exception as exc:                                     # noqa: BLE001
            failed.append((p, repr(exc)))
    if failed:
        print('REFUSING TO PRINT — %d of %d packages unreadable: %s'
              % (len(failed), len(docx), failed[:5]))
        sys.exit(2)

    total_at = sum(v[0] for v in per.values())
    docs = [p for p, v in per.items() if v[0]]
    opens = [p for p in docs if per[p][3] == 'open']
    print('%d .docx read of %d words paths' % (len(per), len(paths)))
    print('paragraphs resolving to lineRule="atLeast" : %6d in %d documents'
          % (total_at, len(docs)))
    print('  ... of them stating 400 twips or more    : %6d'
          % sum(v[1] for v in per.values()))
    print('all paragraphs in those packages           : %6d'
          % sum(v[2] for v in per.values()))
    print('documents the gate calls open, among them  : %6d' % len(opens))
    print()
    print('the fifteen largest, per document:')
    for p in sorted(docs, key=lambda p: -per[p][0])[:15]:
        v = per[p]
        print('   %6d (%4d >= 20 pt)  %-6s %s' % (v[0], v[1], v[3], p))
