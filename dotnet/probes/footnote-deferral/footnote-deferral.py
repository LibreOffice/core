#!/usr/bin/env python3
"""When a footnote will not fit on the page that cites it, what does Writer do?

Why this exists
---------------

`template---tpr-technical-progress-report-with-guidance.docx` renders 8 pages
against 7, with words at 1840/1842 — the same content laid into one extra page.
Measured from the two PDFs' own geometry, the reference puts footnote 2's
*reference* on page 2 (the superscript at y=659.1) and its *text* on page 3
(y=675.3..698.5). We refuse to separate the two, so the bullet that cites it
cannot be placed, and the `Heading2` above it — `keepNext`, `keepLines`, 18 pt
space-before — follows it forward. Everything after shifts by a page.

Our table rows on that page sit 2.6 pt *higher* than the reference's, so we have
more room rather than less, and on page 3, which has no footnote pressure, we
fill to y=698.5 exactly as the reference does. The defect is specific to a page
whose footnote does not fit.

**Two mechanisms produce that output and they are not the same rule.** Writer
might move the whole note to the next page, or split it and leave nothing behind.
On `tpr` they are indistinguishable, because the note is short enough that a
split would have left nothing anyway. They differ as soon as the note is long
enough that *part* of it could have fitted — which is what this measures, and
why it is worth a probe rather than a guess.

The instrument
--------------

One case per page. Each case fills the body with a controlled number of lines and
then cites a footnote long enough to need several lines of its own, so that
sweeping the body-line count walks the citing line down towards the bottom of the
page and past the point where the note stops fitting.

Reading the answer off the output:

- **split** — the note's early lines appear at the foot of the citing page and
  the rest on the next. The count left behind falls as the body grows.
- **moved whole** — the note appears entirely on the following page the moment it
  stops fitting, with nothing left behind at any body length.
- **the citing text moves instead** — what we currently do; the citing line
  itself lands on the next page, which is the control that says the probe is
  actually reaching the interesting regime.

The note is numbered per page so a note's identity can be read straight out of
the extracted text without matching prose.

What it found, 2026-08-15 — Writer SPLITS
------------------------------------------

    case  body   ref cite pg   ref words on cite pg / later      our cite pg
       1    45             1                    59 / 0                     1
       2    50             2                    59 / 0                     2
       3    53             3                    59 / 0                     3
       4    55             4                    59 / 0                     4
       5    56             5                    49 / 10   SPLIT            5
       6    57             6                    17 / 42   SPLIT            7
       7    58             8                    59 / 0                     9
       8    59            10                    59 / 0                    11
       9    60            12                    59 / 0                    13
      10    61            14                    59 / 0                    15

    reference 14 pages, ours 15

**The note is cut at whatever room is left and the remainder flows onto the next
page. The citing text is not pushed forward.** Cases 5 and 6 leave 49 and then
17 of the 59 words behind — a monotone fall as the body grows, which is a split
at the available room and cannot be anything else. "Moved whole" is refuted: it
predicts nought or fifty-nine and never seventeen.

Cases 7 to 10 look like a return to "all on citing page" and are not: the
reference's citing page jumps 6 → 8 → 10 → 12 → 14, two pages per case, because
the body itself has outgrown one page and the citing paragraph lands on a fresh
one where the note fits whole. They are the control, not the phenomenon.

**We diverge from case 6 onward**, and it is exactly the tpr defect in miniature:
the reference keeps the citing paragraph on page 6 and cuts the note; we move the
paragraph to page 7 and keep the note intact under it. One page on this probe,
one page on tpr.

A split may leave **zero** lines behind, which is the tpr case — there, footnote
1 already filled the note area and nothing of footnote 2 fitted, so all of it
moved while its reference stayed on page 2. That is the same rule at its
boundary, not a second rule.

Usage
-----

    XRAY_OFFICE_CLI=... python3 footnote-deferral.py /abs/workdir
"""

import os
import re
import subprocess
import sys
import zipfile

FACE = 'Liberation Serif'
HALF_POINTS = 22

# How many body lines precede the citing paragraph, walking the citing line down
# to the bottom of the page and past the point where the note stops fitting.
#
# The first cut of this probe used 20..42 and every case fitted with room to
# spare — an A4 page at these margins holds about 62 lines of 11 pt, so the note
# was never under pressure and all ten cases reported the same thing. A probe
# that never reaches the regime it is testing reports a clean, uniform, useless
# answer; the tell was that the body-line count made no difference at all.
BODY_LINES = [45, 50, 53, 55, 56, 57, 58, 59, 60, 61]

# Long enough to need several lines at the foot of the page, so that "some of it
# fitted" and "none of it fitted" are different observable outcomes.
NOTE_WORDS = 60

CT = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
      '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
      '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
      '<Default Extension="xml" ContentType="application/xml"/>'
      '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-'
      'officedocument.wordprocessingml.document.main+xml"/>'
      '<Override PartName="/word/footnotes.xml" ContentType="application/vnd.openxmlformats-'
      'officedocument.wordprocessingml.footnotes+xml"/></Types>')
RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
        'relationships/officeDocument" Target="word/document.xml"/></Relationships>')
DOC_RELS = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            '<Relationship Id="rIdFn" Type="http://schemas.openxmlformats.org/officeDocument/2006/'
            'relationships/footnotes" Target="footnotes.xml"/></Relationships>')
W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'


def build(path):
    """One case per page: N body lines, then a paragraph citing a long footnote."""
    run = f'<w:rFonts w:ascii="{FACE}" w:hAnsi="{FACE}"/><w:sz w:val="{HALF_POINTS}"/>'
    body, notes, labels = '', '', []

    for index, count in enumerate(BODY_LINES):
        note_id = index + 1
        brk = '<w:pageBreakBefore/>' if index else ''

        for line in range(count):
            first = brk if line == 0 else ''
            body += (
                f'<w:p><w:pPr>{first}<w:spacing w:before="0" w:after="0"/>'
                f'<w:rPr>{run}</w:rPr></w:pPr><w:r><w:rPr>{run}</w:rPr>'
                f'<w:t>case{note_id} body line {line}</w:t></w:r></w:p>')

        # The citing paragraph, last on the page.
        body += (
            f'<w:p><w:pPr><w:spacing w:before="0" w:after="0"/><w:rPr>{run}</w:rPr></w:pPr>'
            f'<w:r><w:rPr>{run}</w:rPr><w:t>case{note_id} CITES</w:t></w:r>'
            f'<w:r><w:rPr>{run}<w:vertAlign w:val="superscript"/></w:rPr>'
            f'<w:footnoteReference w:id="{note_id}"/></w:r></w:p>')

        # Each note word is tagged with the case so a note's lines can be
        # attributed to their case straight out of the extracted text.
        words = ' '.join(f'n{note_id}w{w}' for w in range(NOTE_WORDS))
        notes += (
            f'<w:footnote w:id="{note_id}"><w:p><w:pPr><w:rPr>{run}</w:rPr></w:pPr>'
            f'<w:r><w:rPr>{run}</w:rPr><w:t xml:space="preserve">{words}</w:t></w:r>'
            f'</w:p></w:footnote>')
        labels.append((note_id, count))

    document = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:document {W}><w:body>{body}'
        '<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="567" w:right="567" w:bottom="567" w:left="567"/></w:sectPr>'
        '</w:body></w:document>')
    footnotes = (
        f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:footnotes {W}>'
        f'<w:footnote w:id="-1" w:type="separator"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>'
        f'<w:footnote w:id="0" w:type="continuationSeparator"><w:p><w:r><w:continuationSeparator/>'
        f'</w:r></w:p></w:footnote>{notes}</w:footnotes>')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', CT)
        z.writestr('_rels/.rels', RELS)
        z.writestr('word/_rels/document.xml.rels', DOC_RELS)
        z.writestr('word/document.xml', document)
        z.writestr('word/footnotes.xml', footnotes)
    return labels


def pages(pdf):
    """Every page's extracted words, as a list of lists."""
    count = int(re.search(
        rb'Pages:\s+(\d+)',
        subprocess.run(['pdfinfo', pdf], capture_output=True, check=True).stdout).group(1))
    out = []
    for page in range(1, count + 1):
        text = subprocess.run(['pdftotext', '-f', str(page), '-l', str(page), pdf, '-'],
                              capture_output=True, text=True, check=True).stdout
        out.append(text.split())
    return out


def report(name, pdf, labels):
    text = pages(pdf)
    print(f'\n=== {name}: {len(text)} pages ===')
    print(f"{'case':>5} {'body':>5} {'cite pg':>8} {'note words on cite pg':>22} "
          f"{'on later pgs':>13}  {'verdict':<22}")

    for note_id, count in labels:
        # The superscript reference merges into the preceding word, so the citing
        # paragraph extracts as `CITES1` rather than `CITES` followed by `1`.
        # Matching on equality finds nothing and silently reports every case as
        # "not on any page", which is how the first run of this probe managed to
        # print ten confident verdicts with no citing page located at all.
        cite = next((i for i, page in enumerate(text)
                     if any(w.startswith('CITES') and w[5:] == str(note_id) for w in page)),
                    None)
        here = sum(1 for w in (text[cite] if cite is not None else []) if w.startswith(f'n{note_id}w'))
        later = sum(1 for p in text[(cite + 1) if cite is not None else 0:]
                    for w in p if w.startswith(f'n{note_id}w'))

        if here and later:
            verdict = 'SPLIT'
        elif here and not later:
            verdict = 'all on citing page'
        elif later and not here:
            verdict = 'MOVED WHOLE'
        else:
            verdict = 'note not found'

        print(f'{note_id:5d} {count:5d} {(cite + 1) if cite is not None else -1:8d} '
              f'{here:22d} {later:13d}  {verdict:<22}')


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/footnote-deferral'
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        raise SystemExit('set XRAY_OFFICE_CLI to the tree you mean to measure')

    os.makedirs(os.path.join(out, 'ours'), exist_ok=True)
    docx = os.path.join(out, 'fn.docx')
    labels = build(docx)

    subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', out, docx],
                   capture_output=True, check=True)
    subprocess.run([cli, 'render', '--quiet', '--outdir', os.path.join(out, 'ours'), docx],
                   check=True)

    reference, ours = os.path.join(out, 'fn.pdf'), os.path.join(out, 'ours', 'fn.pdf')
    for path in (reference, ours):
        if not os.path.isfile(path):
            raise SystemExit(f'{path} was not written — nothing to compare')

    report('reference', reference, labels)
    report('ours', ours, labels)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
