#!/usr/bin/env python3
"""The gap an empty paragraph opens, in the body and in a running head."""
import subprocess, sys, os, re, glob

SOF = {'24.2': '/usr/bin/soffice', '26.2': '/opt/libreoffice26.2/program/soffice'}


def words(pdf):
    x = subprocess.run(['pdftotext', '-bbox', pdf, '-'], capture_output=True, text=True).stdout
    out = {}
    for m in re.finditer(
            r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>', x):
        out.setdefault(m.group(5), tuple(float(m.group(i)) for i in (1, 2, 3, 4)))
    return out


def render(who, doc, outdir):
    os.makedirs(outdir, exist_ok=True)
    if who == 'ours':
        subprocess.run([os.environ['XRAY_OFFICE_CLI'], 'render', doc, '--format', 'pdf',
                        '--outdir', outdir], capture_output=True, timeout=300)
    else:
        subprocess.run([SOF[who], f'-env:UserInstallation=file://{outdir}/prof', '--headless',
                        '--norestore', '--convert-to', 'pdf', '--outdir', outdir, doc],
                       capture_output=True, timeout=300)
    return os.path.join(outdir, os.path.basename(doc)[:-5] + '.pdf')


if __name__ == '__main__':
    fx, work = sys.argv[1], sys.argv[2]
    rows = {}
    for doc in sorted(glob.glob(os.path.join(fx, '*.docx'))):
        name = os.path.basename(doc)[:-5]
        rows[name] = {}
        for who in ('24.2', '26.2', 'ours'):
            pdf = render(who, doc, os.path.join(work, who, name))
            if not os.path.exists(pdf):
                rows[name][who] = None; continue
            w = words(pdf)
            if name.startswith('body-'):
                rows[name][who] = (w['BOTLINE'][1] - w['TOPLINE'][1]) if 'BOTLINE' in w and 'TOPLINE' in w else None
            else:
                rows[name][who] = w['BODYLINE'][1] if 'BODYLINE' in w else None

    print(f"{'fixture':<18} {'24.2':>9} {'26.2':>9} {'ours':>9} {'ours-ref':>9}")
    for name in sorted(rows):
        r = rows[name]
        f = lambda v: '        -' if v is None else f'{v:9.2f}'
        d = ('        -' if r['24.2'] is None or r['ours'] is None
             else f"{r['ours'] - r['24.2']:9.2f}")
        print(f"{name:<18} {f(r['24.2'])} {f(r['26.2'])} {f(r['ours'])} {d}")
