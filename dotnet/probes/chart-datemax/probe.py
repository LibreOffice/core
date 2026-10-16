#!/usr/bin/env python3
"""Where `055_Project_timeline`'s date axis gets its maximum from, by variation.

Each variant changes exactly one thing in `xl/charts/chart11.xml`, renders the workbook
through 26.2.4.2 and reads the date-axis labels off the PDF's own text. The axis' first
and last label, and the step between the first two, are what the axis' minimum, maximum
and increment are.

    probe.py [outdir]
"""
import re, shutil, subprocess, sys, zipfile
from pathlib import Path
import pymupdf

SRC = Path('/home/user/sample-files/sheets/chartset-008/xlsx/'
           '055_Project_timeline_with_milestones_Use_this_template_546cecc0.xlsx')
SOFF = '/opt/libreoffice26.2/program/soffice'
PART = 'xl/charts/chart11.xml'
OUT = Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/wt-chartfit/.work/datemax')
OUT.mkdir(parents=True, exist_ok=True)

MONTHS = dict(Jan=1, Feb=2, Mar=3, Apr=4, May=5, Jun=6,
              Jul=7, Aug=8, Sep=9, Oct=10, Nov=11, Dec=12)


def variant(name, edit):
    """Write a copy of the workbook with `edit` applied to the chart part."""
    path = OUT / f'{name}.xlsx'
    with zipfile.ZipFile(SRC) as zin, zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as zout:
        for info in zin.infolist():
            data = zin.read(info.filename)
            if info.filename == PART:
                data = edit(data.decode('utf-8')).encode('utf-8')
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
    """The date labels sitting on one horizontal row — the axis' own — left to right."""
    page = pymupdf.open(pdf)[0]
    spans = []
    for block in page.get_text('dict')['blocks']:
        for line in block.get('lines', []):
            for span in line['spans']:
                text = span['text'].strip()
                if re.fullmatch(r'\d{1,2} [A-Z][a-z]{2}', text):
                    spans.append((round(span['bbox'][1], 0), round(span['bbox'][0], 2), text))
    if not spans:
        return []
    rows = {}
    for y, x, t in spans:
        rows.setdefault(y, []).append((x, t))
    y = max(rows, key=lambda k: len(rows[k]))
    return [t for _, t in sorted(rows[y])]


def step(labels):
    """Days between the first two labels, using 2023 as the base year."""
    if len(labels) < 2:
        return None
    import datetime
    def day(t, year):
        d, m = t.split()
        return datetime.date(year, MONTHS[m], int(d))
    a = day(labels[0], 2023)
    b = day(labels[1], 2023 if MONTHS[labels[1].split()[1]] >= MONTHS[labels[0].split()[1]] else 2024)
    return (b - a).days


def cut_cache(xml, formula, count):
    """Truncate one cached sequence to `count` points and restate its ptCount."""
    def one(match):
        body = match.group(0)
        if formula not in body:
            return body
        body = re.sub(r'<c:ptCount val="\d+"/>', f'<c:ptCount val="{count}"/>', body)
        pts = list(re.finditer(r'<c:pt idx="(\d+)"[^>]*>.*?</c:pt>', body, re.S))
        for pt in reversed(pts):
            if int(pt.group(1)) >= count:
                body = body[:pt.start()] + body[pt.end():]
        return body
    return re.sub(r'<c:cat>.*?</c:cat>', one, xml, flags=re.S)


def shift_dates(xml, delta, scale=1.0):
    """Move every cached serial in the C range by `delta`, after scaling its offset."""
    base = 45021.0
    def one(match):
        body = match.group(0)
        if "$C$20:$C$36" not in body:
            return body
        def pt(m):
            v = float(m.group(1))
            return f'<c:v>{base + (v - base) * scale + delta:.0f}</c:v>'
        return re.sub(r'<c:v>(4\d{4}(?:\.\d+)?)</c:v>', pt, body)
    return re.sub(r'<c:cat>.*?</c:cat>', one, xml, flags=re.S)


VARIANTS = [
    ('base', lambda s: s),
    ('major5', lambda s: s.replace('<c:majorUnit val="10"/>', '<c:majorUnit val="5"/>')),
    ('major40', lambda s: s.replace('<c:majorUnit val="10"/>', '<c:majorUnit val="40"/>')),
    ('nomajor', lambda s: s.replace(
        '<c:majorUnit val="10"/><c:majorTimeUnit val="days"/>', '')),
    ('lastdate45100', lambda s: s.replace('<c:v>45169</c:v>', '<c:v>45100</c:v>')),
    ('ptcount13', lambda s: cut_cache(s, "$C$20:$C$36", 13)),
    ('nobar', lambda s: re.sub(r'<c:barChart>.*?</c:barChart>', '', s, flags=re.S)),
    ('noline', lambda s: re.sub(r'<c:lineChart>.*?</c:lineChart>', '', s, flags=re.S)),
    ('shift1000', lambda s: shift_dates(s, 1000.0)),
    ('range2x', lambda s: shift_dates(s, 0.0, 2.0)),
    ('range10x', lambda s: shift_dates(s, 0.0, 10.0)),
    ('rangehalf', lambda s: shift_dates(s, 0.0, 0.5)),
]

print('variant\tlabels\tfirst\tlast\tstep_days')
for name, edit in VARIANTS:
    pdf = render(variant(name, edit))
    if pdf is None:
        print(f'{name}\tRENDER-FAILED')
        continue
    labels = axis_labels(pdf)
    print(f'{name}\t{len(labels)}\t{labels[0] if labels else "-"}'
          f'\t{labels[-1] if labels else "-"}\t{step(labels)}')
    sys.stdout.flush()
