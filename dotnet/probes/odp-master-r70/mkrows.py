"""Assemble the probe's stored TSVs, each with a header naming the binary and the font set."""
import csv, subprocess

HDR = ("# ours   = XRay.Office.Cli @ {ours}\n"
       "# ref    = /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2 (TDF tarball)\n"
       "# fonts  = system fontconfig; ALL FOUR tarball confounds moved aside on 2026-09-07 --\n"
       "#          Carlito/Caladea/Liberation/DejaVu duplicates, the Latin Noto, and the four\n"
       "#          LiberationSansNarrow. 38 faces in .duplicates-aside, 71 left in the directory.\n"
       "#          fc-match \"DejaVu Sans\" -> DejaVuSans.ttf. fc-list | grep -i narrow is empty.\n"
       "# rule   = batch-check.sh of 2026-09-05: page count, then alphanumeric characters within\n"
       "#          max(2%, 15), then unembedded fonts.\n"
       "# corpus = {corpus}\n"
       "# note   = the reference half here reproduces probes/odf-gate-01/rows.tsv's ref columns on\n"
       "#          302 of 302 .odp rows, pages and glyphs, so the Narrow move did not touch them.\n")

def emit(out, verdicts, ours_tsv, ref_tsv, ourscommit, corpus):
    O = {}
    with open(ours_tsv) as fh:
        fh.readline()
        for r in csv.DictReader(fh, delimiter='\t'): O[r['path']] = r
    R = {}
    with open(ref_tsv) as fh:
        fh.readline()
        for r in csv.DictReader(fh, delimiter='\t'): R[r['path']] = r
    with open(out, 'w', newline='') as fh:
        fh.write(HDR.format(ours=ourscommit, corpus=corpus))
        w = csv.writer(fh, delimiter='\t')
        w.writerow(['path', 'ext', 'pages', 'words', 'fonts', 'unemb', 'verdict', 'rawwords', 'glyphs'])
        with open(verdicts) as vh:
            vh.readline()
            for v in csv.DictReader(vh, delimiter='\t'):
                p = v['path']; o, r = O[p], R[p]
                w.writerow([p, p.rsplit('.', 1)[-1].lower(),
                            '%s/%s' % (o['pages'], r['pages']),
                            '%s/%s' % (o['words'], r['words']),
                            '%s/%s' % (o['fonts'], r['fonts']),
                            o['unemb'], v['verdict'],
                            '%s/%s' % (o['rawwords'], r['rawwords']),
                            '%s/%s' % (o['glyphs'], r['glyphs'])])

BASE = 'f4b8c38255a92caf1f05cc8acdf8d8288a5ceac4 (base, before this round)'
HEAD = 'f4b8c38255a92caf1f05cc8acdf8d8288a5ceac4 + this round'
emit('rows-odp-before.tsv', 'verdicts-before.tsv', 'ours-before.tsv', 'ref.tsv', BASE, '/home/user/corpus-odf')
emit('rows-odp-after.tsv',  'verdicts-after.tsv',  'ours-after.tsv',  'ref.tsv', HEAD, '/home/user/corpus-odf')
emit('rows-slides-before.tsv','slides-verdicts-before.tsv','slides-before.tsv','slides-ref-bank.tsv', BASE, '/home/user/sample-files (reference columns from /home/user/gate-2f47 @ 2f4709c08)')
emit('rows-slides-after.tsv', 'slides-verdicts-after.tsv', 'slides-after.tsv', 'slides-ref-bank.tsv', HEAD, '/home/user/sample-files (reference columns from /home/user/gate-2f47 @ 2f4709c08)')
print('written')
