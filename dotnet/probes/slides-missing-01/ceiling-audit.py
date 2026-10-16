#!/usr/bin/env python3
"""Re-measure every `ceiling` row of raster-ceiling-pages.tsv against the banked references.

    ceiling-audit.py <ours-dir-for-slides> <ours-dir-for-the-rest>

The table's rows name the pre-2026-08-14 batch paths, which no longer resolve; each is
followed forward through MANIFEST.tsv by basename. The metric is the one the table's own
generator uses -- `pdftotext -f N -l N | str.split()`, pages 1-based -- so the columns are
comparable to the stored ones and not merely similar.

Written for round slides-missing-01, which found one row with its sign inverted.
"""
import csv, os, subprocess, sys

MANIFEST = '/c/sandbox/workdir/sample-files/MANIFEST.tsv'
TABLE = '/c/sandbox/workdir/wt-s-missing/dotnet/raster-ceiling-pages.tsv'
REFS = '/c/sandbox/workdir/refpdfs-26.2.4.2-fonts'
MIN_EXTRA_WORDS, MIN_EXTRA_RATIO = 8, 0.25


def run(command):
    try:
        return subprocess.run(command, capture_output=True, timeout=300).stdout.decode('utf8', 'replace')
    except Exception:
        return ''


def words(pdf, page):
    return len(run(['pdftotext', '-f', str(page), '-l', str(page), pdf, '-']).split())


def main():
    slides_dir, other_dir = sys.argv[1], sys.argv[2]
    manifest = {os.path.basename(r['path']): r
                for r in csv.DictReader(open(MANIFEST), delimiter='\t')}
    print('document\tpage\tfiled_ours\tfiled_ref\tours\tref\tnote')
    for row in csv.DictReader(open(TABLE), delimiter='\t'):
        if row.get('verdict') != 'ceiling':
            continue
        base = os.path.basename(row['document'])
        entry = manifest[base]
        stem, extension = os.path.splitext(base)
        identity = f"{stem}__{extension.lstrip('.').lower()}"
        family = entry['family']
        ours = os.path.join(slides_dir if family == 'slides' else other_dir, identity + '.pdf')
        reference = os.path.join(REFS, family, identity + '.pdf')
        page = int(row['page'])
        our_words, reference_words = words(ours, page), words(reference, page)
        filed = int(row['our_words']) - int(row['ref_words'])
        now = our_words - reference_words
        if (filed > 0) != (now > 0):
            note = 'SIGN-FLIPPED'
        elif now < MIN_EXTRA_WORDS or now < MIN_EXTRA_RATIO * reference_words:
            note = 'below-threshold'
        else:
            note = ''
        print(f"{base}\t{page}\t{row['our_words']}\t{row['ref_words']}\t{our_words}\t{reference_words}\t{note}")


if __name__ == '__main__':
    sys.exit(main())
