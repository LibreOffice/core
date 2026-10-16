#!/usr/bin/env python3
"""Every corpus document rendered at the round's base and again after, byte for byte.

One output directory per *document* -- never per worker slot, which is the collision
`corpus-batches` records.  The base render is run with `XRAY_OFFICE_CHART_TRACE=1` so the same
pass also says which documents hold a chart at all, which is the population the change can
possibly reach; the head render is scored only where the bytes move.

    confine.py <workdir> <cli-base> <cli-head> [jobs]

Writes `<workdir>/rows.tsv`: document, charts, moved, base_ink, head_ink over the banked
26.2.4.2 rendering.
"""
import concurrent.futures, hashlib, os, pathlib, re, shutil, subprocess, sys

CORPUS = pathlib.Path('/home/user/sample-files')
BANK = pathlib.Path('/home/user/gate-orig-r83/ref')
DIFF = ('/home/user/wt-chartinner/.claude/skills/render-comparison/scripts/pdf-image-diff.py')
ROW = re.compile(r'^(\d+)\t([-\d.]+)\t([-\d.]+)\t([-\d.]+)\t')
KINDS = ('.doc', '.docx', '.docm', '.dot', '.dotx', '.xls', '.xlsx', '.xlsm', '.xlsb',
         '.xlt', '.xltx', '.ppt', '.pptx', '.pptm', '.pps', '.ppsx', '.odt', '.ods',
         '.odp', '.rtf')


def bank_for(path):
    p = BANK / f'{path.stem}__{path.suffix[1:].lower()}.pdf'
    return p if p.exists() else None


def render(cli, doc, out, trace=False):
    out.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH='0')
    if trace:
        env['XRAY_OFFICE_CHART_TRACE'] = '1'
    try:
        r = subprocess.run([cli, 'render', str(doc), '--format', 'pdf', '--outdir', str(out)],
                           capture_output=True, text=True, env=env, timeout=600, check=False)
    except subprocess.TimeoutExpired:
        return None, 0
    pdfs = sorted(out.glob('*.pdf'))
    charts = sum(1 for line in r.stderr.splitlines() if line.startswith('TRACE')) if trace else 0
    return (pdfs[0] if pdfs else None), charts


def ink(ours, ref, tmp):
    shutil.rmtree(tmp, ignore_errors=True)
    try:
        r = subprocess.run(['python3', DIFF, str(ours), str(ref), '--outdir', str(tmp)],
                           capture_output=True, text=True, check=False, timeout=1800)
    except subprocess.TimeoutExpired:
        shutil.rmtree(tmp, ignore_errors=True)
        return None
    shutil.rmtree(tmp, ignore_errors=True)
    total = None
    for line in r.stdout.splitlines():
        m = ROW.match(line)
        if m:
            total = (total or 0.0) + float(m.group(4))
    return None if total is None else round(total, 2)


def one(doc, work, base_cli, head_cli):
    d = work / hashlib.sha1(str(doc).encode()).hexdigest()[:16]
    shutil.rmtree(d, ignore_errors=True)
    try:
        a, charts = render(base_cli, doc, d / 'base', trace=True)
        b, _ = render(head_cli, doc, d / 'head')
        if a is None or b is None:
            return f'{doc.name}\t{charts}\tours-failed\t\t'
        ha = hashlib.sha256(a.read_bytes()).hexdigest()
        hb = hashlib.sha256(b.read_bytes()).hexdigest()
        if ha == hb:
            return f'{doc.name}\t{charts}\tsame\t\t'
        ref = bank_for(doc)
        if ref is None:
            return f'{doc.name}\t{charts}\tmoved\t\t'
        ia = ink(a, ref, d / 'ta')
        ib = ink(b, ref, d / 'tb')
        return (f'{doc.name}\t{charts}\tmoved\t'
                f'{"" if ia is None else ia}\t{"" if ib is None else ib}')
    finally:
        shutil.rmtree(d, ignore_errors=True)


def main():
    work = pathlib.Path(sys.argv[1]).resolve()
    base_cli, head_cli = sys.argv[2], sys.argv[3]
    jobs = int(sys.argv[4]) if len(sys.argv) > 4 else 4
    work.mkdir(parents=True, exist_ok=True)
    docs = sorted(p for p in CORPUS.rglob('*')
                  if p.is_file() and p.suffix.lower() in KINDS)
    rows = work / 'rows.tsv'
    with rows.open('w') as out:
        out.write('document\tcharts\tmoved\tbase_ink\thead_ink\n')
        with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
            for line in pool.map(lambda p: one(p, work, base_cli, head_cli), docs):
                out.write(line + '\n')
                out.flush()
    print('documents', len(docs))


if __name__ == '__main__':
    main()
