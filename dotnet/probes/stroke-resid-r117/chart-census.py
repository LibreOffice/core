#!/usr/bin/env python3
"""Census: corpus charts whose axis lines and gridlines state no width of their own.

Such a line takes the *automatic* format, whose width is the theme's first
`a:lnStyleLst` entry (`THEMED_STYLE_SUBTLE`) scaled by the auto-format table's
`mnRelLineWidth`.  Only the OOXML (zip) formats are readable here; a `.xls`,
`.doc` or `.ppt` chart is a BIFF/escher record stream and is reported separately.
"""
import os, re, sys, zipfile, collections

CH = '{http://schemas.openxmlformats.org/drawingml/2006/chart}'
A = '{http://schemas.openxmlformats.org/drawingml/2006/main}'
AXES = ('catAx', 'valAx', 'dateAx', 'serAx')

import xml.etree.ElementTree as ET


def tagname(e):
    return e.tag.split('}')[-1]


def line_of(spPr):
    """(states_ln, width_emu or None, nofill)"""
    if spPr is None:
        return (False, None, False)
    ln = spPr.find(A + 'ln')
    if ln is None:
        return (False, None, False)
    w = ln.get('w')
    return (True, int(w) if w and w.lstrip('-').isdigit() else None,
            ln.find(A + 'noFill') is not None)


def chart_census(data):
    """Counts for one chart part."""
    c = collections.Counter()
    try:
        root = ET.fromstring(data)
    except Exception:
        c['parse-error'] += 1
        return c
    for ax in root.iter():
        if tagname(ax) not in AXES:
            continue
        c['axes'] += 1
        states, w, nofill = line_of(ax.find(CH + 'spPr'))
        if nofill:
            c['axis-nofill'] += 1
        elif w is None:
            c['axis-auto-width'] += 1
        else:
            c['axis-stated-width'] += 1
        for which in ('majorGridlines', 'minorGridlines'):
            g = ax.find(CH + which)
            if g is None:
                continue
            c[which] += 1
            states, w, nofill = line_of(g.find(CH + 'spPr'))
            if nofill:
                c[which + '-nofill'] += 1
            elif w is None:
                c[which + '-auto-width'] += 1
            else:
                c[which + '-stated-width'] += 1
    return c


def theme_has_line_styles(z):
    for n in z.namelist():
        ln = n.lower()
        if '/theme/' in ln and ln.endswith('.xml'):
            try:
                d = z.read(n)
            except Exception:
                continue
            m = re.search(rb'<a:lnStyleLst>\s*<a:ln[^>]*\bw="(\d+)"', d)
            if m:
                return int(m.group(1))
    return None


def main(root, out):
    paths = []
    for fam in ('sheets', 'slides', 'words'):
        for dirpath, _, files in os.walk(os.path.join(root, fam)):
            for f in sorted(files):
                paths.append((fam, os.path.join(dirpath, f)))
    paths.sort()
    with open(out, 'w') as fh:
        fh.write('family\text\tzip\tcharts\taxes\taxis_auto_w\taxis_stated_w\t'
                 'majorgrid\tmajor_auto_w\tmajor_stated_w\tminorgrid\tminor_auto_w\t'
                 'minor_stated_w\tsubtle_w\tpath\n')
        for fam, p in paths:
            ext = os.path.splitext(p)[1].lstrip('.').lower()
            if not zipfile.is_zipfile(p):
                fh.write('%s\t%s\t0\t\t\t\t\t\t\t\t\t\t\t\t%s\n'
                         % (fam, ext, os.path.relpath(p, root)))
                continue
            tot = collections.Counter()
            charts = 0
            subtle = None
            try:
                with zipfile.ZipFile(p) as z:
                    subtle = theme_has_line_styles(z)
                    for n in z.namelist():
                        ln = n.lower()
                        if '/charts/chart' in ln and ln.endswith('.xml') and 'colors' not in ln \
                                and 'style' not in ln:
                            charts += 1
                            tot.update(chart_census(z.read(n)))
            except Exception as e:
                fh.write('%s\t%s\tERR\t%s\t\t\t\t\t\t\t\t\t\t\t%s\n'
                         % (fam, ext, type(e).__name__, os.path.relpath(p, root)))
                continue
            fh.write('\t'.join(str(x) for x in [
                fam, ext, 1, charts, tot['axes'], tot['axis-auto-width'],
                tot['axis-stated-width'], tot['majorGridlines'],
                tot['majorGridlines-auto-width'], tot['majorGridlines-stated-width'],
                tot['minorGridlines'], tot['minorGridlines-auto-width'],
                tot['minorGridlines-stated-width'],
                subtle if subtle is not None else '',
                os.path.relpath(p, root)]) + '\n')
    print('wrote', out, len(paths))


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
