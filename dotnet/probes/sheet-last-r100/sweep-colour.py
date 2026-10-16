#!/usr/bin/env python3
"""Render every sheets document at two binaries, hash each PDF, delete it, report movers."""
import os, subprocess, sys, hashlib, glob, shutil, tempfile

ROOTS = ['/home/user/sample-files/sheets']
EXT = {'.xls'}
LEGS = {'base': '/home/user/r100-work/cli-head/XRay.Office.Cli',
        'head': '/home/user/r100-work/cli-colour/XRay.Office.Cli'}

docs = []
for r in ROOTS:
    for p in sorted(glob.glob(r + '/**/*', recursive=True)):
        if os.path.isfile(p) and os.path.splitext(p)[1].lower() in EXT:
            docs.append(p)
print('documents %d' % len(docs), flush=True)

env = dict(os.environ, SOURCE_DATE_EPOCH='1757462400')
out = open(sys.argv[1], 'w')
out.write('path\tbase\thead\n')
moved = fail = 0
for i, doc in enumerate(docs):
    h = {}
    for leg, cli in LEGS.items():
        d = tempfile.mkdtemp(prefix='r100-', dir='/home/user/r100-work/tmp')
        try:
            subprocess.run([cli, 'render', doc, '--outdir', d],
                           env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                           timeout=900)
            pdfs = glob.glob(d + '/*.pdf')
            if not pdfs:
                h[leg] = 'FAILED'
            else:
                b = open(pdfs[0], 'rb').read()
                h[leg] = 'TRUNC' if b'%%EOF' not in b[-2048:] else hashlib.md5(b).hexdigest()
        except Exception as e:
            h[leg] = 'ERR'
        finally:
            shutil.rmtree(d, ignore_errors=True)
    out.write('%s\t%s\t%s\n' % (doc, h['base'], h['head']))
    out.flush()
    if 'FAILED' in h.values() or 'ERR' in h.values() or 'TRUNC' in h.values():
        fail += 1
        print('  !! %s %s' % (doc, h), flush=True)
    elif h['base'] != h['head']:
        moved += 1
        print('  MOVED %s' % doc, flush=True)
    if (i + 1) % 50 == 0:
        print('  ... %d/%d moved=%d fail=%d' % (i + 1, len(docs), moved, fail), flush=True)
print('TOTAL %d moved %d failures %d' % (len(docs), moved, fail))
out.close()
