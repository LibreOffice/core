#!/usr/bin/env python3
"""Every BIFF workbook in the corpus, rendered at the round's base and again after.

The change is in `XlsChartReader`, which only a BIFF workbook reaches, so the reach is bounded
by code to the corpus's `.xls`/`.xlt` set -- this measures it rather than asserting it.  Each
document is rendered twice under `SOURCE_DATE_EPOCH=0`, compared byte for byte, and where the
bytes move it is scored against its banked 26.2.4.2 rendering with `pdf-image-diff.py`, summing
its `|ink|%` column over the document's pages, once for each side.

    xls-confine.py <workdir> <cli-base> <cli-head>
"""
import hashlib, os, pathlib, re, shutil, subprocess, sys

CORPUS = pathlib.Path('/home/user/sample-files')
BANK = pathlib.Path('/home/user/gate-orig-r83/ref')
DIFF = '/home/user/wt-chartgeom/.claude/skills/render-comparison/scripts/pdf-image-diff.py'
ROW = re.compile(r'^(\d+)\t([-\d.]+)\t([-\d.]+)\t([-\d.]+)\t')


def bank_for(path):
    name = f'{path.stem}__{path.suffix[1:].lower()}.pdf'
    p = BANK / name
    return p if p.exists() else None


def render(cli, doc, out):
    out.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH='0')
    subprocess.run([cli, 'render', str(doc), '--format', 'pdf', '--outdir', str(out)],
                   capture_output=True, env=env, timeout=600, check=False)
    pdfs = sorted(out.glob('*.pdf'))
    return pdfs[0] if pdfs else None


def ink(ours, ref, tmp):
    if tmp.exists():
        shutil.rmtree(tmp)
    r = subprocess.run(['python3', DIFF, str(ours), str(ref), '--outdir', str(tmp)],
                       capture_output=True, text=True, check=False, timeout=900)
    if tmp.exists():
        shutil.rmtree(tmp)
    total = None
    for line in r.stdout.splitlines():
        m = ROW.match(line)
        if m:
            total = (total or 0.0) + float(m.group(4))
    return None if total is None else round(total, 2)


def main():
    work = pathlib.Path(sys.argv[1]).resolve()
    base_cli, head_cli = sys.argv[2], sys.argv[3]
    work.mkdir(parents=True, exist_ok=True)
    docs = sorted(p for p in CORPUS.rglob('*')
                  if p.is_file() and p.suffix.lower() in ('.xls', '.xlt'))
    only = set(sys.argv[4:])
    if only:
        docs = [p for p in docs if p.name in only]
    print('document\tmoved\tbase_ink\thead_ink\tdelta')
    moved = same = 0
    for doc in docs:
        d = work / doc.stem[:60].replace('/', '_')
        for side, cli in (('b', base_cli), ('h', head_cli)):
            out = d / side
            if out.exists():
                shutil.rmtree(out)
        pb = render(base_cli, doc, d / 'b')
        ph = render(head_cli, doc, d / 'h')
        if pb is None or ph is None:
            print(f'{doc.name}\tNO PDF\t\t\t')
            shutil.rmtree(d, ignore_errors=True)
            continue
        hb = hashlib.md5(pb.read_bytes()).hexdigest()
        hh = hashlib.md5(ph.read_bytes()).hexdigest()
        if hb == hh:
            same += 1
            print(f'{doc.name}\tno\t\t\t')
            shutil.rmtree(d, ignore_errors=True)
            continue
        moved += 1
        ref = bank_for(doc)
        if ref is None:
            print(f'{doc.name}\tyes\tno bank\t\t')
            shutil.rmtree(d, ignore_errors=True)
            continue
        ib = ink(pb, ref, d / 'cb')
        ih = ink(ph, ref, d / 'ch')
        delta = '' if ib is None or ih is None else f'{ih - ib:+.2f}'
        print(f'{doc.name}\tyes\t{ib}\t{ih}\t{delta}', flush=True)
        shutil.rmtree(d, ignore_errors=True)
    print(f'# moved {moved}, byte-identical {same}, of {len(docs)}')


if __name__ == '__main__':
    main()
