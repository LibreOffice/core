#!/usr/bin/env python3
"""Census: how many decks draw a chart series whose colour resolves through a theme line style.

Walks the OPC parts and parses them with ElementTree. No regex over markup — the rule this
project keeps re-learning is that an alternation without a backreference measures itself
(`slides-c-01` §4: 16 decks reported, 1 real).

The question is deliberately phrased as *what a series resolves to*, not *what a part declares*:
a theme that states a `phClr` transform only matters if some drawn series actually resolves its
stroke through that entry. So the walk is slide -> graphicFrame -> chart part -> series, and the
theme is the one that slide inherits (or the chart's own themeOverride).
"""

import sys, os, zipfile, csv
import xml.etree.ElementTree as ET

A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
C = 'http://schemas.openxmlformats.org/drawingml/2006/chart'
R = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
PR = 'http://schemas.openxmlformats.org/package/2006/relationships'

LINEAR = {'lineChart', 'line3DChart', 'scatterChart', 'radarChart', 'stockChart'}
FILLED = {'barChart', 'bar3DChart', 'pieChart', 'pie3DChart', 'doughnutChart',
          'areaChart', 'area3DChart', 'ofPieChart', 'surfaceChart', 'surface3DChart',
          'bubbleChart'}


def parse(z, name):
    try:
        with z.open(name) as f:
            return ET.parse(f).getroot()
    except Exception:
        return None


def rels_of(z, part):
    d, b = os.path.split(part)
    name = f'{d}/_rels/{b}.rels' if d else f'_rels/{b}.rels'
    root = parse(z, name)
    if root is None:
        return {}
    out = {}
    for rel in root:
        out[rel.get('Id')] = (rel.get('Type', ''), rel.get('Target', ''), rel.get('TargetMode', ''))
    return out


def resolve(part, target):
    if target.startswith('/'):
        return target[1:]
    return os.path.normpath(os.path.join(os.path.dirname(part), target)).replace('\\', '/')


def theme_for_slide(z, slide):
    """slide -> layout -> master -> theme, following relationship types."""
    for rid, (typ, tgt, mode) in rels_of(z, slide).items():
        if typ.endswith('/slideLayout'):
            layout = resolve(slide, tgt)
            for _, (t2, g2, _m) in rels_of(z, layout).items():
                if t2.endswith('/slideMaster'):
                    master = resolve(layout, g2)
                    for _, (t3, g3, _m3) in rels_of(z, master).items():
                        if t3.endswith('/theme'):
                            return resolve(master, g3)
    return None


def style_entry(theme_root, list_name, index):
    """The one-based entry of a:lnStyleLst / a:fillStyleLst, clamped like lclGetStyleElement."""
    if theme_root is None:
        return None
    els = theme_root.find(f'{{{A}}}themeElements')
    if els is None:
        return None
    fmt = els.find(f'{{{A}}}fmtScheme')
    if fmt is None:
        return None
    lst = fmt.find(f'{{{A}}}{list_name}')
    if lst is None or len(lst) == 0:
        return None
    return lst[min(index, len(lst)) - 1]


def phclr_transforms(entry):
    """The transform children of the first a:schemeClr val="phClr" under this style entry.

    Returns None when the entry states no phClr at all (then the accent passes through
    unchanged), and a list of (localName, val) when it does.
    """
    if entry is None:
        return None
    for el in entry.iter(f'{{{A}}}schemeClr'):
        if el.get('val') == 'phClr':
            return [(c.tag.split('}')[-1], c.get('val')) for c in el]
    return None


IDENTITY = {'shade': '100000', 'tint': '100000', 'satMod': '100000',
            'lumMod': '100000', 'alpha': '100000'}


def changes_colour(transforms):
    """True when the transforms would move the substituted accent off its own value."""
    if not transforms:
        return False
    return any(IDENTITY.get(k) != v for k, v in transforms)


def style_of(chart_root):
    for el in chart_root.iter(f'{{{C}}}style'):
        v = el.get('val')
        try:
            n = int(v)
        except (TypeError, ValueError):
            continue
        if 1 <= n <= 48:
            return n
    return 2


def series_states_line_colour(ser):
    """Whether c:ser/c:spPr/a:ln carries a colour of its own (so automatic never applies)."""
    sp = ser.find(f'{{{C}}}spPr')
    if sp is None:
        return False
    ln = sp.find(f'{{{A}}}ln')
    if ln is None:
        return False
    for kind in ('solidFill', 'gradFill', 'pattFill', 'noFill'):
        if ln.find(f'{{{A}}}{kind}') is not None:
            return True
    return False


def stroke_visible(style, linear):
    """Whether the automatic table gives this object a stroke at all (spFilledSeriesLines)."""
    if linear:
        return True
    return 9 <= style <= 16 or 33 <= style <= 40


def census_one(path):
    row = {'charts': 0, 'series_auto_stroke': 0, 'series_reaching': 0,
           'themes_with_transform': 0, 'transform': ''}
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return None  # not a zip: a .ppt, which has no DrawingML theme at all
    names = set(z.namelist())
    slides = sorted(n for n in names
                    if n.startswith('ppt/slides/slide') and n.endswith('.xml'))
    seen_transform = set()
    for slide in slides:
        theme_part = theme_for_slide(z, slide)
        for rid, (typ, tgt, mode) in rels_of(z, slide).items():
            pass
        # chart parts referenced from this slide, directly or through a graphicFrame's r:id
        chart_parts = [resolve(slide, tgt) for (typ, tgt, mode) in rels_of(z, slide).values()
                       if typ.endswith('/chart') and mode != 'External']
        for chart in chart_parts:
            croot = parse(z, chart)
            if croot is None:
                continue
            row['charts'] += 1
            # a chart may override the theme entirely
            over = [resolve(chart, t) for (ty, t, m) in rels_of(z, chart).values()
                    if ty.endswith('/themeOverride')]
            troot = parse(z, over[0]) if over else (parse(z, theme_part) if theme_part else None)
            entry = style_entry(troot, 'lnStyleLst', 1)
            tf = phclr_transforms(entry)
            moves = changes_colour(tf)
            if moves:
                seen_transform.add(str(tf))
            style = style_of(croot)
            for group in croot.iter():
                ln = group.tag.split('}')[-1]
                if ln not in LINEAR and ln not in FILLED:
                    continue
                linear = ln in LINEAR
                for ser in group.findall(f'{{{C}}}ser'):
                    if series_states_line_colour(ser):
                        continue
                    if not stroke_visible(style, linear):
                        continue
                    row['series_auto_stroke'] += 1
                    if moves:
                        row['series_reaching'] += 1
    row['themes_with_transform'] = len(seen_transform)
    row['transform'] = ' | '.join(sorted(seen_transform))[:120]
    return row


def main(root, out):
    rows = []
    for dirpath, _dirs, files in os.walk(root):
        for f in sorted(files):
            p = os.path.join(dirpath, f)
            rows.append((os.path.relpath(p, root), p))
    rows.sort()
    w = csv.writer(open(out, 'w', newline=''), delimiter='\t')
    w.writerow(['doc', 'charts', 'series_auto_stroke', 'series_reaching',
                'distinct_transforms', 'transform'])
    tot = reach = zips = 0
    for rel, p in rows:
        r = census_one(p)
        tot += 1
        if r is None:
            w.writerow([rel, 'n/a', 'n/a', 'n/a', 'n/a', 'not-a-zip'])
            continue
        zips += 1
        if r['series_reaching'] > 0:
            reach += 1
        w.writerow([rel, r['charts'], r['series_auto_stroke'], r['series_reaching'],
                    r['themes_with_transform'], r['transform']])
    print(f'documents {tot}, zip (pptx) {zips}, decks with a series reaching a '
          f'transforming theme line style: {reach}')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
