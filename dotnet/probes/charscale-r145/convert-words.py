#!/usr/bin/env python3
"""Convert the corpus's 337 word-processing documents with 26.2.4.2's own filters.

    convert-words.py <ext>            # ext in {odt, rtf}

The `.odt`/`.rtf` columns of /home/user/corpus-odf described in dotnet/CLAUDE.md were not
present in this container (only `ods`, 307).  This rebuilds them the same way: every input is
staged under `md5(abspath)[:12] + '-' + stem` so no two conversions collide on a stem
(`libreoffice-reference` gotcha 1), a private user profile is used, and the output file is
CHECKED FOR EXISTENCE because soffice exits 0 when it converts nothing.
"""
import csv, hashlib, pathlib, shutil, subprocess, sys

SOFFICE = '/opt/libreoffice26.2/program/soffice'
CORPUS = pathlib.Path('/home/user/sample-files')
OUTROOT = pathlib.Path('/home/user/corpus-odf')
STAGE = pathlib.Path('/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad/stage-words')
PROFILE = '/tmp/claude-0/-home-user/bb4a221c-b846-5451-ba79-f27935c68360/scratchpad/loprofile'

ext = sys.argv[1]
out = OUTROOT / ext
out.mkdir(parents=True, exist_ok=True)
STAGE.mkdir(parents=True, exist_ok=True)

rows = []
with (CORPUS / 'MANIFEST.tsv').open() as fh:
    for row in csv.DictReader(fh, delimiter='\t'):
        if row['family'] != 'words':
            continue
        src = CORPUS / row['path']
        key = hashlib.md5(str(src).encode()).hexdigest()[:12]
        rows.append((key, src))

staged = []
for key, src in rows:
    dst = STAGE / ('%s-%s%s' % (key, src.stem, src.suffix.lower()))
    if not dst.exists():
        shutil.copy2(src, dst)
    staged.append((key, src, dst))

log = open(OUTROOT / ('convert-%s.log' % ext), 'w')
ok = failed = 0
BATCH = 12
for i in range(0, len(staged), BATCH):
    chunk = staged[i:i + BATCH]
    cmd = [SOFFICE, '--headless', '--norestore',
           '-env:UserInstallation=file://%s' % PROFILE,
           '--convert-to', ext, '--outdir', str(out)] + [str(d) for _, _, d in chunk]
    try:
        subprocess.run(cmd, capture_output=True, text=True, timeout=1800)
    except subprocess.TimeoutExpired:
        pass
    for key, src, d in chunk:
        produced = out / (d.stem + '.' + ext)
        if produced.exists() and produced.stat().st_size > 0:
            ok += 1
            log.write('ok\t%s\t%s\n' % (key, src))
        else:
            failed += 1
            log.write('FAIL\t%s\t%s\n' % (key, src))
    log.flush()
    print('%d/%d' % (i + len(chunk), len(staged)), flush=True)

log.write('DONE %d ok, %d failed\n' % (ok, failed))
log.close()
print('DONE %d ok, %d failed' % (ok, failed))
