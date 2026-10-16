#!/usr/bin/env python3
"""Render a list of documents with one output directory per document."""
import concurrent.futures as cf, hashlib, os, pathlib, subprocess, sys

CLI = '/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli'
docs = [l.strip() for l in open(sys.argv[1]) if l.strip()]
out = pathlib.Path(sys.argv[2]); out.mkdir(parents=True, exist_ok=True)
env = dict(os.environ, SOURCE_DATE_EPOCH='0')

def one(path):
    key = hashlib.md5(path.encode()).hexdigest()[:12]
    d = out / key; d.mkdir(exist_ok=True)
    r = subprocess.run([CLI, 'render', path, '--outdir', str(d)],
                       capture_output=True, env=env, timeout=600)
    return path, r.returncode

with cf.ThreadPoolExecutor(max_workers=3) as pool:
    bad = [p for p, rc in pool.map(one, docs) if rc != 0]
print(f'{len(docs)} rendered, {len(bad)} failed')
for p in bad: print('  FAILED', p)
