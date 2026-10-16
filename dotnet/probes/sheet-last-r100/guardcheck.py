#!/usr/bin/env python3
"""Does the repeat-print-column guard change anything? cli-head (no guard) vs cli-head2 (guard),
over every `.xls` and the one document in 550 that declares repeated print columns."""
import glob, os, hashlib, subprocess, tempfile, shutil
docs = sorted(glob.glob('/home/user/sample-files/sheets/**/*.xls', recursive=True))
docs += ['/home/user/sample-files/sheets/chartset-007/xlsx/037_Personal_money_tracker_a57957bb.xlsx']
env = dict(os.environ, SOURCE_DATE_EPOCH='1757462400')
diff = 0
for doc in docs:
    h = {}
    for leg in ('cli-head', 'cli-head2'):
        d = tempfile.mkdtemp(dir='/home/user/r100-work/tmp')
        subprocess.run(['/home/user/r100-work/%s/XRay.Office.Cli' % leg, 'render', doc, '--outdir', d],
                       env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=900)
        p = glob.glob(d + '/*.pdf')
        h[leg] = hashlib.md5(open(p[0], 'rb').read()).hexdigest() if p else 'FAILED'
        shutil.rmtree(d, ignore_errors=True)
    if h['cli-head'] != h['cli-head2']:
        diff += 1
        print('DIFFERS', doc, h)
print('checked %d, differing %d' % (len(docs), diff))
