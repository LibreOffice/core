#!/usr/bin/env python3
"""Page count and alphanumeric-character count for every document the sweep moved.

`batch-check.sh`'s three checks are page count, extractable characters (its column 9,
`glyphs` -- alphanumeric characters, not tokens) and font embedding.  Only the first two can
move here: no chart change embeds or un-embeds a face.  Every document the confinement sweep
reported byte-identical has, by construction, the identical count on both sides, so this runs
only over the movers and says so.

The reference side is the banked 26.2.4.2 rendering rather than a fresh `soffice`, so no
reference render can time out under load and be scored as a regression.

    gate-movers.py <rows.tsv> <cli-base> <cli-head> <workdir>
"""
import concurrent.futures, hashlib, os, pathlib, re, shutil, subprocess, sys

CORPUS = pathlib.Path('/home/user/sample-files')
BANK = pathlib.Path('/home/user/gate-orig-r83/ref')
ALNUM = re.compile(r'[^\w]|_', re.UNICODE)


def counts(pdf):
    if pdf is None or not pdf.exists():
        return None, None
    pages = subprocess.run(['pdfinfo', str(pdf)], capture_output=True, text=True, check=False)
    n = None
    for line in pages.stdout.splitlines():
        if line.startswith('Pages:'):
            n = int(line.split()[1])
    text = subprocess.run(['pdftotext', '-q', str(pdf), '-'],
                          capture_output=True, text=True, check=False).stdout
    return n, sum(1 for c in text if c.isalnum())


def render(cli, doc, out):
    out.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, SOURCE_DATE_EPOCH='0')
    subprocess.run([cli, 'render', str(doc), '--format', 'pdf', '--outdir', str(out)],
                   capture_output=True, env=env, timeout=600, check=False)
    pdfs = sorted(out.glob('*.pdf'))
    return pdfs[0] if pdfs else None


def one(name, work, base_cli, head_cli):
    hits = [p for p in CORPUS.rglob(name) if p.is_file()]
    if not hits:
        return f'{name}\tnot-found'
    doc = hits[0]
    d = work / hashlib.sha1(name.encode()).hexdigest()[:16]
    shutil.rmtree(d, ignore_errors=True)
    try:
        bp, bg = counts(render(base_cli, doc, d / 'base'))
        hp, hg = counts(render(head_cli, doc, d / 'head'))
        ref = BANK / f'{doc.stem}__{doc.suffix[1:].lower()}.pdf'
        rp, rg = counts(ref if ref.exists() else None)
        return (f'{name}\t{rp}\t{bp}\t{hp}\t{rg}\t{bg}\t{hg}\t'
                f'{"PAGES-MOVED" if bp != hp else "pages-same"}\t'
                f'{"GLYPHS-MOVED" if bg != hg else "glyphs-same"}')
    finally:
        shutil.rmtree(d, ignore_errors=True)


def main():
    rows, base_cli, head_cli, work = sys.argv[1], sys.argv[2], sys.argv[3], pathlib.Path(sys.argv[4])
    work.mkdir(parents=True, exist_ok=True)
    names = [line.split('\t')[0] for line in open(rows).read().splitlines()[1:]
             if line.split('\t')[2] == 'moved']
    print('document\tref_pages\tbase_pages\thead_pages\tref_glyphs\tbase_glyphs\thead_glyphs\tpages\tglyphs')
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
        for line in pool.map(lambda n: one(n, work, base_cli, head_cli), names):
            print(line, flush=True)


if __name__ == '__main__':
    main()
