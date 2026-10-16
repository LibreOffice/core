#!/usr/bin/env python3
"""Per-page alphanumeric-character diff between our rendering and 26.2.4.2's, for one
document, plus what each side puts on the page in pictures and in fonts.

Environment: ours = dotnet/tools/XRay.Office.Cli (the tree this is run from);
ref = /opt/libreoffice26.2/program/soffice, 26.2.4.2, all four tarball font confounds aside.
"""
import glob, hashlib, os, shutil, subprocess, sys
from pathlib import Path

CLI = os.environ.get('XRAY_OFFICE_CLI',
    '/home/user/wt-odpfoot/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli')
SOF = '/opt/libreoffice26.2/program/soffice'

def render(doc, out, side):
    shutil.rmtree(out, ignore_errors=True); Path(out).mkdir(parents=True)
    if side == 'ours':
        subprocess.run([CLI, 'render', doc, '--outdir', out], capture_output=True, timeout=600)
    else:
        prof = Path(out) / 'prof'
        subprocess.run([SOF, '--headless', '-env:UserInstallation=file://%s' % prof,
                        '--convert-to', 'pdf', '--outdir', out, doc],
                       capture_output=True, timeout=600)
        shutil.rmtree(prof, ignore_errors=True)
    pdfs = [p for p in glob.glob(out + '/*.pdf')]
    assert pdfs, 'no pdf produced for %s (%s)' % (doc, side)
    return pdfs[0]

def npages(p):
    for l in subprocess.run(['pdfinfo', p], capture_output=True).stdout.decode().splitlines():
        if l.startswith('Pages'): return int(l.split()[1])
    return 0

def alnum(p, i):
    t = subprocess.run(['pdftotext', '-f', str(i), '-l', str(i), p, '-'],
                       capture_output=True).stdout.decode('utf-8', 'replace')
    return sum(1 for c in t if c.isalnum())

def images(p, i):
    out = subprocess.run(['pdfimages', '-list', '-f', str(i), '-l', str(i), p],
                         capture_output=True).stdout.decode()
    return max(0, len([l for l in out.splitlines()[2:] if l.strip()]))

def main():
    doc = sys.argv[1]
    tmp = os.environ.get('TMPDIR', '/tmp') + '/pagediff-' + hashlib.sha1(doc.encode()).hexdigest()[:12]
    o = render(doc, tmp + '/ours', 'ours')
    r = render(doc, tmp + '/ref', 'ref')
    no, nr = npages(o), npages(r)
    print('# %s' % doc)
    print('pages ours %d ref %d' % (no, nr))
    to = tr = 0
    rows = []
    for i in range(1, max(no, nr) + 1):
        a = alnum(o, i) if i <= no else 0
        b = alnum(r, i) if i <= nr else 0
        to += a; tr += b
        rows.append((i, a, b, images(o, i) if i <= no else 0, images(r, i) if i <= nr else 0))
    print('alnum ours %d ref %d  (%+d)' % (to, tr, to - tr))
    print('%4s %8s %8s %8s %6s %6s' % ('page', 'ours', 'ref', 'delta', 'imgO', 'imgR'))
    for i, a, b, io, ir in rows:
        if abs(a - b) > 10 or io != ir:
            print('%4d %8d %8d %+8d %6d %6d' % (i, a, b, a - b, io, ir))
    shutil.rmtree(tmp, ignore_errors=True)

main()
