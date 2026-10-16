#!/usr/bin/env python3
"""The same question asked of the *sheet* rather than of the chart's caches.

An `.xlsx` chart's data comes from the worksheet: LibreOffice reads the cells and ignores
`c:numCache`, which is why every variation of the cache in `probe.py` changed nothing at
all. These variants edit `xl/worksheets/sheet11.xml`.

    probe2.py [outdir]
"""
import datetime, re, subprocess, sys, zipfile
from pathlib import Path
import pymupdf

SRC = Path('/home/user/sample-files/sheets/chartset-008/xlsx/'
           '055_Project_timeline_with_milestones_Use_this_template_546cecc0.xlsx')
SOFF = '/opt/libreoffice26.2/program/soffice'
SHEET = 'xl/worksheets/sheet11.xml'
CHART = 'xl/charts/chart11.xml'
OUT = Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/wt-chartfit/.work/datemax2')
OUT.mkdir(parents=True, exist_ok=True)
MONTHS = dict(Jan=1, Feb=2, Mar=3, Apr=4, May=5, Jun=6,
              Jul=7, Aug=8, Sep=9, Oct=10, Nov=11, Dec=12)


def edit_cells(xml, column, rows, change):
    """Apply `change` to the numeric value of each named cell."""
    def one(match):
        ref, body = match.group(1), match.group(0)
        col = re.match(r'([A-Z]+)(\d+)', ref)
        if col.group(1) != column or int(col.group(2)) not in rows:
            return body
        return re.sub(r'<v>([-\d.]+)</v>',
                      lambda m: f'<v>{change(float(m.group(1))):.0f}</v>', body)
    return re.sub(r'<c r="([A-Z]+\d+)"[^>]*?(?:/>|>.*?</c>)', one, xml, flags=re.S)


def variant(name, sheet_edit=None, chart_edit=None):
    path = OUT / f'{name}.xlsx'
    with zipfile.ZipFile(SRC) as zin, zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as zout:
        for info in zin.infolist():
            data = zin.read(info.filename)
            if info.filename == SHEET and sheet_edit:
                data = sheet_edit(data.decode('utf-8')).encode('utf-8')
            if info.filename == CHART and chart_edit:
                data = chart_edit(data.decode('utf-8')).encode('utf-8')
            zout.writestr(info, data)
    return path


def render(path):
    out = OUT / (path.stem + '.out')
    out.mkdir(exist_ok=True)
    subprocess.run([SOFF, '-env:UserInstallation=file://' + str(out / 'prof'), '--headless',
                    '--norestore', '--convert-to', 'pdf', '--outdir', str(out), str(path)],
                   capture_output=True, timeout=900)
    pdf = out / (path.stem + '.pdf')
    return pdf if pdf.exists() else None


def axis_labels(pdf):
    page = pymupdf.open(pdf)[0]
    rows = {}
    for block in page.get_text('dict')['blocks']:
        for line in block.get('lines', []):
            for span in line['spans']:
                text = span['text'].strip()
                if re.fullmatch(r'\d{1,2} [A-Z][a-z]{2}', text):
                    rows.setdefault(round(span['bbox'][1], 0), []).append(
                        (round(span['bbox'][0], 2), text))
    if not rows:
        return []
    y = max(rows, key=lambda k: len(rows[k]))
    return [t for _, t in sorted(rows[y])]


def span(labels):
    """First label, last label, and the days between the first two."""
    if len(labels) < 2:
        return None
    def day(t, year):
        d, m = t.split()
        return datetime.date(year, MONTHS[m], int(d))
    a = day(labels[0], 2023)
    later = MONTHS[labels[1].split()[1]] >= MONTHS[labels[0].split()[1]]
    return (day(labels[1], 2023 if later else 2024) - a).days


DATES = range(20, 33)
VARIANTS = [
    ('base', None, None),
    ('dates_plus1000', lambda s: edit_cells(s, 'C', DATES, lambda v: v + 1000), None),
    ('dates_2x', lambda s: edit_cells(s, 'C', DATES, lambda v: 45021 + (v - 45021) * 2), None),
    ('dates_10x', lambda s: edit_cells(s, 'C', DATES, lambda v: 45021 + (v - 45021) * 10), None),
    ('dates_flat', lambda s: edit_cells(s, 'C', DATES, lambda v: 45021), None),
    ('durations_10x', lambda s: edit_cells(s, 'F', DATES, lambda v: v * 10), None),
    ('durations_zero', lambda s: edit_cells(s, 'F', DATES, lambda v: 0), None),
    ('milestones_100', lambda s: edit_cells(s, 'G', DATES, lambda v: 100), None),
    ('names_5x', lambda s: edit_cells(s, 'D', DATES, lambda v: v * 5), None),
    ('barcat_dates', None, lambda s: s.replace('$D$20:$D$36', '$C$20:$C$36')),
    ('ranges_to_32', None, lambda s: s.replace('$20:$D$36', '$20:$D$32')
                                      .replace('$20:$C$36', '$20:$C$32')
                                      .replace('$20:$F$36', '$20:$F$32')
                                      .replace('$20:$G$36', '$20:$G$32')),
    ('ranges_to_40', None, lambda s: s.replace('$20:$D$36', '$20:$D$40')
                                      .replace('$20:$C$36', '$20:$C$40')
                                      .replace('$20:$F$36', '$20:$F$40')
                                      .replace('$20:$G$36', '$20:$G$40')),
    ('labelsize_2400', None, lambda s: s.replace('sz="1200"', 'sz="2400"')),
    ('labelsize_600', None, lambda s: s.replace('sz="1200"', 'sz="600"')),
    ('crossbetween_between', None,
     lambda s: s.replace('<c:crossBetween val="midCat"/>', '<c:crossBetween val="between"/>')),
    ('no_basetimeunit', None, lambda s: s.replace('<c:baseTimeUnit val="days"/>', '')),
    ('no_errbars', None, lambda s: re.sub(r'<c:errBars>.*?</c:errBars>', '', s, flags=re.S)),
    ('bar_no_ser', None,
     lambda s: re.sub(r'(<c:barChart>.*?)<c:ser>.*?</c:ser>(.*?</c:barChart>)',
                      r'\1\2', s, flags=re.S)),
    ('bar_same_axes', None,
     lambda s: s.replace('<c:axId val="717045280"/><c:axId val="717044888"/>',
                         '<c:axId val="717044104"/><c:axId val="717044496"/>')),
    ('line_no_ser', None,
     lambda s: re.sub(r'(<c:lineChart>.*?)<c:ser>.*?</c:ser>(.*?</c:lineChart>)',
                      r'\1\2', s, flags=re.S)),
    ('dates_minus3000', lambda s: edit_cells(s, 'C', DATES, lambda v: v - 3000), None),
    ('dates_minus10000', lambda s: edit_cells(s, 'C', DATES, lambda v: v - 10000), None),
    ('dates_plus3000', lambda s: edit_cells(s, 'C', DATES, lambda v: v + 3000), None),
    ('year_2023', lambda s: s.replace('YEAR(TODAY())', '2023'), None),
    ('year_2030', lambda s: s.replace('YEAR(TODAY())', '2030'), None),
    ('year_2026', lambda s: s.replace('YEAR(TODAY())', '2026'), None),
] + [
    (f'only_{md.replace(",", "_")}_2026',
     (lambda md: lambda s: s.replace('YEAR(TODAY())', '2023')
                            .replace(f'DATE(2023,{md})', f'DATE(2026,{md})'))(md), None)
    for md in ('4,5)', '4,24)', '5,1)', '5,15)', '6,15)', '6,30)',
               '7,15)', '7,30)', '8,11)', '8,23)', '8,31)')
] + [
    ('bar_cat_dropped', None,
     lambda s: re.sub(r'(<c:barChart>.*?)<c:cat>.*?</c:cat>(.*?</c:barChart>)',
                      r'\1\2', s, flags=re.S)),
]

print('variant\tlabels\tfirst\tlast\tstep_days')
for name, sheet_edit, chart_edit in VARIANTS:
    pdf = render(variant(name, sheet_edit, chart_edit))
    if pdf is None:
        print(f'{name}\tRENDER-FAILED')
        continue
    labels = axis_labels(pdf)
    print(f'{name}\t{len(labels)}\t{labels[0] if labels else "-"}'
          f'\t{labels[-1] if labels else "-"}\t{span(labels)}')
    sys.stdout.flush()
