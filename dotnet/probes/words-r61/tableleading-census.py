#!/usr/bin/env python3
"""How many paragraph-then-table boundaries in the words corpus carry proportional line spacing?

    python3 tableleading-census.py [corpus-root]

`SwFlowFrame::CalcUpperSpace` adds `nPrevLineSpacing` — the previous frame's proportional
line-spacing excess — to the upper space of **whatever follows**, and `pOwn` being a text frame is
only consulted for the *own* term. A `SwTabFrame` therefore takes the leading of the paragraph
above it. We hand that leading between paragraphs and drop it at a table, which is what this census
sizes.

Counted **per boundary**, never summed per document into one number and never counted per
paragraph: a document with forty such boundaries is worth forty, and the two figures answer
different questions. The per-document column is printed beside the per-boundary one so neither can
be read as the other.

Resolution is the real one and not a grep: a paragraph's `w:spacing/@w:line` and `@w:lineRule` are
taken from its own `w:pPr`, else from its `w:pStyle` chain (following `w:basedOn` to the root), else
from `w:docDefaults/w:pPrDefault`. A grep for `w:lineRule="auto"` over `document.xml` alone sees
neither the style chain nor the document default, and both carry it in this corpus.

**What this census cannot see**, written down before the change rather than after:

  * `.doc` (66 paths) and `.odt` (0 paths) — the WW8 reader resolves `sprmPDyaLine` and the ODF
    reader `fo:line-height` through code this script does not run. `.doc` is counted only by its
    table count, as an upper bound on where the change *could* reach.
  * a paragraph inside a table cell followed by a nested table — the same boundary one level down,
    which `FlowLayouter` reaches and this walk does count, but whose reach in *page* terms is
    bounded by the cell rather than by the page.
  * whether the extra pt actually moves anything: a boundary in the middle of a page changes the
    y of everything below it and moves no verdict at all. This is a count of *sites*, and the
    prediction has to say separately what it expects of them.
"""
import os
import sys
import zipfile
import xml.etree.ElementTree as ET
from collections import defaultdict

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'


def styles_of(z):
    """(by-id line/lineRule/basedOn, docDefaults line/lineRule)."""
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


def proportional_over_100(line, rule):
    if line is None:
        return False
    if rule not in (None, 'auto'):
        return False
    try:
        return int(line) > 240
    except ValueError:
        return False


def boundaries(path):
    """(sites, tables, paragraphs-before-a-table) for one .docx."""
    with zipfile.ZipFile(path) as z:
        table, defaults = styles_of(z)
        sites = 0
        tables = 0
        befores = 0
        for name in z.namelist():
            if not name.startswith('word/') or not name.endswith('.xml'):
                continue
            if os.path.basename(name) not in (
                    'document.xml',) and not os.path.basename(name).startswith(
                        ('header', 'footer')):
                continue
            try:
                root = ET.fromstring(z.read(name))
            except ET.ParseError:
                continue
            # every container whose children can be a paragraph then a table
            for parent in root.iter():
                kids = [k for k in parent if k.tag in (W + 'p', W + 'tbl')]
                for a, b in zip(kids, kids[1:]):
                    if b.tag != W + 'tbl':
                        continue
                    tables += 1
                    if a.tag != W + 'p':
                        continue
                    befores += 1
                    line, rule = resolve(a, table, defaults)
                    if proportional_over_100(line, rule):
                        sites += 1
    return sites, tables, befores


if __name__ == '__main__':
    root = sys.argv[1] if len(sys.argv) > 1 else '/c/sandbox/workdir/sample-files'
    man = os.path.join(root, 'MANIFEST.tsv')
    paths = []
    with open(man, encoding='utf-8') as f:
        next(f)
        for line in f:
            c = line.rstrip('\n').split('\t')
            if c[0] == 'words':
                paths.append((c[2], c[3], c[7]))

    docx = [(p, e, s) for p, e, s in paths if e in ('docx', 'docm', 'dotx', 'dotm')]
    other = defaultdict(int)
    for p, e, s in paths:
        if e not in ('docx', 'docm', 'dotx', 'dotm'):
            other[e] += 1

    per_doc = {}
    failed = []
    for p, e, s in docx:
        full = os.path.join(root, p)
        try:
            per_doc[p] = boundaries(full) + (s,)
        except Exception as exc:                                     # noqa: BLE001
            failed.append((p, repr(exc)))

    if failed:
        print('REFUSING TO PRINT — %d of %d packages could not be read:' % (len(failed), len(docx)))
        for p, e in failed[:10]:
            print('   ', p, e)
        sys.exit(2)

    sites = sum(v[0] for v in per_doc.values())
    with_sites = [p for p, v in per_doc.items() if v[0]]
    tables = sum(v[1] for v in per_doc.values())
    befores = sum(v[2] for v in per_doc.values())
    open_with = [p for p in with_sites if per_doc[p][3] == 'open']

    print('%d words paths in the manifest; %d read as OOXML packages, %d read'
          % (len(paths), len(docx), len(per_doc)))
    print('other extensions the walk cannot resolve:', dict(other))
    print()
    print('paragraph-then-table boundaries        : %5d in %d documents'
          % (befores, len({p for p, v in per_doc.items() if v[2]})))
    print('  ... of them proportional over 100%%   : %5d in %d documents   <- the sites'
          % (sites, len(with_sites)))
    print('all table starts (any predecessor)     : %5d' % tables)
    print('sites in documents the gate calls open : %5d in %d documents'
          % (sum(per_doc[p][0] for p in open_with), len(open_with)))
    print()
    print('the twenty largest, per document (never summed into the figure above):')
    for p in sorted(with_sites, key=lambda p: -per_doc[p][0])[:20]:
        v = per_doc[p]
        print('   %5d  %-8s %s' % (v[0], v[3], p))
