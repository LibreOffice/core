#!/usr/bin/env python3
"""O91's reach, counted by what the rule PAINTS.

A `<w:br/>` inside a shape body whose run size equals the paragraph mark's costs nothing, so the
population is the subset where they DIFFER -- and, beyond that, the subset where the difference
can reach the page at all, which needs the shape's own height.

Two things this census does that a string count cannot.  It skips the `mc:Fallback` VML twin of
every `mc:AlternateContent`, which carries a second copy of the same `w:txbxContent` and doubles
every figure taken with `grep`; and it resolves each size through rPr -> rStyle -> pStyle ->
docDefaults rather than reading the literal attribute.
"""
import collections
import pathlib
import re
import sys
import xml.etree.ElementTree as ET
import zipfile

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
MC = '{http://schemas.openxmlformats.org/markup-compatibility/2006}'
A = '{http://schemas.openxmlformats.org/drawingml/2006/main}'
WPS = '{http://schemas.microsoft.com/office/word/2010/wordprocessingShape}'

CORPUS = pathlib.Path('/home/user/sample-files')
PARTS = re.compile(r'^word/(document|header\d*|footer\d*|footnotes|endnotes)\.xml$')


def sz_of(rpr, styles, default):
    """Effective half-point size of a w:rPr, following rStyle then falling back."""
    if rpr is None:
        return default
    el = rpr.find(W + 'sz')
    if el is not None and el.get(W + 'val'):
        try:
            return int(el.get(W + 'val'))
        except ValueError:
            return default
    ref = rpr.find(W + 'rStyle')
    if ref is not None:
        return styles.get(ref.get(W + 'val'), default)
    return default


def read_styles(pkg):
    """{styleId: half-points} plus the docDefaults size."""
    out, default = {}, 20            # Word's own fallback when nothing states one
    if 'word/styles.xml' not in pkg.namelist():
        return out, default
    root = ET.fromstring(pkg.read('word/styles.xml'))
    dd = root.find(W + 'docDefaults/' + W + 'rPrDefault/' + W + 'rPr/' + W + 'sz')
    if dd is not None and dd.get(W + 'val'):
        default = int(dd.get(W + 'val'))
    for st in root.findall(W + 'style'):
        el = st.find(W + 'rPr/' + W + 'sz')
        if el is not None and el.get(W + 'val'):
            try:
                out[st.get(W + 'styleId')] = int(el.get(W + 'val'))
            except ValueError:
                pass
    return out, default


VML_H = re.compile(r'height:\s*([-\d.]+)pt')
VML = '{urn:schemas-microsoft-com:vml}'


def live_bodies(root):
    """Every w:txbxContent that LibreOffice actually reads -- i.e. not under an mc:Fallback.

    Returns the body with the inner height of the shape that holds it, in pt, from either the
    DrawingML `wps:wsp` or the VML `v:rect`/`v:shape` that some of these templates use instead,
    and whether that shape's height is fixed (no autofit).
    """
    parent = {c: p for p in root.iter() for c in p}
    for body in root.iter(W + 'txbxContent'):
        node, fallback, shape, vml = body, False, None, None
        while node is not None:
            if node.tag == MC + 'Fallback':
                fallback = True
                break
            if node.tag == WPS + 'wsp' and shape is None:
                shape = node
            if node.tag.startswith(VML) and vml is None and node.get('style'):
                m = VML_H.search(node.get('style'))
                if m:
                    vml = node
            node = parent.get(node)
        if fallback:
            continue
        inner, fixed = None, True
        if shape is not None:
            ext = shape.find(WPS + 'spPr/' + A + 'xfrm/' + A + 'ext')
            bpr = shape.find(WPS + 'bodyPr')
            if ext is not None and emu(ext, 'cy'):
                ins = 91440
                if bpr is not None:
                    ins = ((emu(bpr, 'tIns') if emu(bpr, 'tIns') is not None else 45720)
                           + (emu(bpr, 'bIns') if emu(bpr, 'bIns') is not None else 45720))
                inner = (emu(ext, 'cy') - ins) / 12700.0
            if bpr is not None and bpr.find(A + 'spAutoFit') is not None:
                fixed = False
        elif vml is not None:
            inner = float(VML_H.search(vml.get('style')).group(1)) - 7.2
            tb = vml.find(VML + 'textbox')
            if tb is not None and 'mso-fit-shape-to-text:t' in (tb.get('style') or ''):
                fixed = False
        yield body, inner, fixed


def emu(el, attr):
    try:
        return int(el.get(attr))
    except (TypeError, ValueError):
        return None


def scan(path):
    rows = []
    stats = collections.Counter()
    with zipfile.ZipFile(path) as pkg:
        styles, default = read_styles(pkg)
        for name in pkg.namelist():
            if not PARTS.match(name):
                continue
            try:
                root = ET.fromstring(pkg.read(name))
            except ET.ParseError:
                continue
            for body, inner, fixed in live_bodies(root):
                stats['bodies'] += 1
                shape_extra = 0.0
                for para in body.iter(W + 'p'):
                    stats['paras'] += 1
                    ppr = para.find(W + 'pPr')
                    mark = sz_of(ppr.find(W + 'rPr') if ppr is not None else None,
                                 styles, default)
                    if ppr is not None:
                        ps = ppr.find(W + 'pStyle')
                        if ps is not None and ppr.find(W + 'rPr/' + W + 'sz') is None:
                            mark = styles.get(ps.get(W + 'val'), mark)
                    for run in para.findall(W + 'r'):
                        brs = [b for b in run.findall(W + 'br')
                               if b.get(W + 'type') in (None, 'textWrapping')]
                        if not brs:
                            continue
                        rsz = sz_of(run.find(W + 'rPr'), styles, default)
                        stats['breaks'] += len(brs)
                        if rsz != mark:
                            stats['differ'] += len(brs)
                            shape_extra += len(brs) * (mark - rsz) / 2.0 * 1.164
                            rows.append(dict(mark=mark, run=rsz, n=len(brs),
                                             inner=inner, fixed=fixed))
                if shape_extra:
                    chars = sum(len(t.text or '') for t in body.iter(W + 't'))
                    stats['bodies_differ'] += 1
                    stats['chars_differ'] += chars
                    if inner is not None and fixed:
                        stats['bodies_measurable'] += 1
                        if shape_extra > inner:
                            stats['bodies_forced_over'] += 1
                            stats['chars_forced_over'] += chars
                            if chars:
                                stats['bodies_forced_over_with_text'] += 1
    return rows, stats


def main():
    out = []
    total = collections.Counter()
    per_doc = {}
    files = sorted(p for p in CORPUS.rglob('*')
                   if p.suffix.lower() in ('.docx', '.docm', '.dotx', '.dotm'))
    for path in files:
        try:
            rows, stats = scan(path)
        except zipfile.BadZipFile:
            total['unreadable'] += 1
            continue
        total['docs'] += 1
        for k, v in stats.items():
            total[k] += v
        if stats['breaks']:
            total['docs_with_break'] += 1
        if stats['differ']:
            total['docs_differ'] += 1
            per_doc[path.name] = (rows, stats['bodies_differ'], stats['bodies_forced_over'])
        out.append((path.name, stats['bodies'], stats['paras'], stats['breaks'],
                    stats['differ']))

    print('# base rate')
    print('docx-family documents read          %5d' % total['docs'])
    print('live w:txbxContent bodies           %5d' % total['bodies'])
    print('paragraphs inside them              %5d' % total['paras'])
    print('<w:br/> inside them                 %5d  in %d documents'
          % (total['breaks'], total['docs_with_break']))
    print('... whose run size DIFFERS from the paragraph mark   %5d  in %d documents'
          % (total['differ'], total['docs_differ']))
    print('shape bodies holding at least one such break        %5d  of %d'
          % (total['bodies_differ'], total['bodies']))
    print('  ... of those, fixed-height and measurable         %5d' % total['bodies_measurable'])
    print('  ... whose EXTRA height alone exceeds the box      %5d' % total['bodies_forced_over'])
    print('  ... of THOSE, ones that hold any text at all       %5d  (%d characters)'
          % (total['bodies_forced_over_with_text'], total['chars_forced_over']))
    print('characters inside the 112 differing bodies          %5d' % total['chars_differ'])
    print()
    print('# the differing population, per document')
    for name, (rows, nbodies, nover) in sorted(per_doc.items()):
        n = sum(r['n'] for r in rows)
        # the extra height this tree's reading adds, in pt, at 1.164 em and the 108 % the
        # witness states -- an upper bound is not needed, this is the size term alone
        extra = sum(r['n'] * (r['mark'] - r['run']) / 2.0 * 1.164 for r in rows)
        inner = [r['inner'] for r in rows if r['inner']]
        print('%-62s %3d breaks  mark/run %-12s extra %+7.1f pt  smallest inner %-8s'
              '  bodies %d, forced over %d'
              % (name[:62], n,
                 ','.join(sorted({'%d/%d' % (r['mark'], r['run']) for r in rows})),
                 extra,
                 ('%.1f pt' % min(inner)) if inner else '-', nbodies, nover))


if __name__ == '__main__':
    main()
