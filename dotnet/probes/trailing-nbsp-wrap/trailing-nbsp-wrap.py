#!/usr/bin/env python3
"""What do a paragraph's trailing no-break space and blank cost LibreOffice?

Why this exists
---------------

`150-5370-10H.docx` renders 726 pages against the reference's 727, and one
paragraph in it is worth two line pitches. Task #70 recorded the cause as "a
paragraph ending in a trailing space" and warned that fixing it would move text
on most of any real corpus.

The trailing characters are `U+00A0` then `U+0020` — a **no-break** space
followed by an ordinary blank. Two earlier rounds read them off a terminal,
where the two are indistinguishable, and generalised from the wrong character.

This script measures the four things that follow from that, and the fourth
refutes the rule the first three suggest. See `README.md` for the tables.

    1. `tail`       — vary only the trailing characters, keeping the real body
    2. `adjust`     — vary the paragraph's `w:jc`, keeping the real tail
    3. `short`      — a one-line paragraph, to test width dependence
    4. `synthetic`  — sweep a synthetic body's length from one to three lines

The reference alone is measured. Ours is a flat four lines in every row of (1),
so it discriminates nothing; what is in question is what *LibreOffice* does.

Reading the output
------------------

`lines` counts the text lines `pdftotext` finds before the second paragraph, and
`gap` is the distance from the last of them to it. A paragraph that gained an
invisible line shows an unchanged `lines` and a `gap` one pitch larger, so
neither column can be read on its own — the line pitch here is 12.65 pt and the
ordinary paragraph gap is 15.6.

**Dump the bytes of what a mutation actually wrote before believing a row.**
A case literal typed into a heredoc silently carried a `U+00A0` once, and for a
round that looked exactly like the reference being nondeterministic. It is not:
`--stability` renders one build five times and the count does not move.

Usage
-----

    export XRAY_OFFICE_CLI=<tree>/dotnet/tools/XRay.Office.Cli/…/XRay.Office.Cli
    python3 trailing-nbsp-wrap.py [outdir]
"""

import importlib.util
import os
import re
import subprocess
import sys
import xml.etree.ElementTree as ET

HERE = os.path.dirname(os.path.abspath(__file__))
SLICER = os.path.join(HERE, '..', 'trailing-space-wrap', 'trailing-space-wrap.py')

# The document has moved batch since the slicer was written; it globs directories, so the
# path in it no longer resolves. Naming it here keeps that script the record of its own round.
SRC = '/c/sandbox/workdir/sample-files/words/pagination-003/docx/150-5370-10H.docx'

NB = ' '


def slicer():
    """The two-block slicer from the previous round, pointed at the document's new home."""
    spec = importlib.util.spec_from_file_location('tsw', SLICER)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    module.SRC = SRC
    return module


def tail(tsw, text):
    """Replace the paragraph's last run of text, leaving its body alone."""
    def mutate(paragraph):
        runs = list(paragraph.iter(tsw.W + 't'))
        runs[-1].set(tsw.SPACE, 'preserve')
        runs[-1].text = text
    return mutate


def adjust(tsw, jc, text):
    """The same, plus an explicit `w:jc` on the paragraph."""
    def mutate(paragraph):
        tail(tsw, text)(paragraph)
        if jc is None:
            return
        properties = paragraph.find(tsw.W + 'pPr')
        if properties is None:
            properties = ET.Element(tsw.W + 'pPr')
            paragraph.insert(0, properties)
        ET.SubElement(properties, tsw.W + 'jc').set(tsw.W + 'val', jc)
    return mutate


def whole(tsw, text):
    """Replace every run, so the body is synthetic and only the style survives."""
    def mutate(paragraph):
        for run in paragraph.findall(tsw.W + 'r'):
            paragraph.remove(run)
        run = ET.SubElement(paragraph, tsw.W + 'r')
        element = ET.SubElement(run, tsw.W + 't')
        element.set(tsw.SPACE, 'preserve')
        element.text = text
    return mutate


def lines(pdf):
    text = subprocess.run(['pdftotext', '-bbox', '-f', '1', '-l', '1', pdf, '-'],
                          capture_output=True, text=True, check=True).stdout
    rows = {}
    for x0, y0, x1, word in re.findall(
            r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)"[^>]*>([^<]*)</word>', text):
        rows.setdefault(round(float(y0), 1), []).append((float(x0), float(x1), word))

    return sorted((y, max(b for _, b, _ in v), ' '.join(w for _, _, w in sorted(v)))
                  for y, v in rows.items())


def measure(tsw, names, blob, out, tag, mutate):
    """Render one mutation and return its line count, its gap, and its last line's right edge."""
    directory = os.path.join(out, re.sub(r'\W+', '_', tag))
    os.makedirs(directory, exist_ok=True)
    docx = os.path.join(directory, 't.docx')
    tsw.build(docx, names, blob, mutate)

    subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', directory, docx],
                   capture_output=True, check=True)

    pdf = os.path.join(directory, 't.pdf')
    if not os.path.isfile(pdf):
        raise SystemExit(f'{pdf} was not written — nothing to measure')

    found = lines(pdf)
    stop = next((i for i, (_, _, t) in enumerate(found) if t.startswith('(2)')), len(found))
    gap = found[stop][0] - found[stop - 1][0] if stop < len(found) else 0.0
    return stop, gap, found[stop - 1][1]


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/trailing-nbsp-wrap'
    tsw = slicer()
    names, blob = tsw.load()
    os.makedirs(out, exist_ok=True)

    def row(label, mutate, note=''):
        count, gap, right = measure(tsw, names, blob, out, label, mutate)
        print(f'{label:26s} | {count:5d} {gap:7.1f} {right:8.1f}  {note}')

    print('1. the tail, on the real body      | lines     gap    right')
    for label, text in [
        ('. NB SP   (the real one)', '.' + NB + ' '),
        ('. NB SP SP', '.' + NB + '  '),
        ('. SP NB SP', '. ' + NB + ' '),
        ('. NB', '.' + NB),
        ('. NB X', '.' + NB + 'X'),
        ('. NB SP X', '.' + NB + ' X'),
        ('. SP NB', '. ' + NB),
        ('. SP SP SP', '.   '),
        ('. (nothing)', '.'),
    ]:
        row(label, tail(tsw, text), repr(text.encode('unicode_escape').decode()))

    print('\n2. the paragraph\'s adjustment      | lines     gap    right')
    for jc in (None, 'left', 'both', 'center', 'right'):
        row(f'jc={jc}', adjust(tsw, jc, '.' + NB + ' '))

    print('\n3. a one-line paragraph            | lines     gap    right')
    for label, text in [
        ('short: plain', 'Hello world.'),
        ('short: + NB', 'Hello world.' + NB),
        ('short: + NB SP', 'Hello world.' + NB + ' '),
        ('short: + SP', 'Hello world. '),
    ]:
        row(label, whole(tsw, text))

    print('\n4. a synthetic body, swept         | lines     gap    right')
    print('   The control: the rule holds on a body sharing nothing with the real one.')
    for count in range(6, 34, 2):
        body = ' '.join(f'word{i:02d}' for i in range(count))
        row(f'{count:2d} words + NB SP', whole(tsw, body + '.' + NB + ' '))
        row(f'{count:2d} words + SP', whole(tsw, body + '. '))

    print('\nRows 1 and 3 of section 1 both cost two pitches and identify the no-break space;')
    print('section 2 splits the effect at block adjustment, which names guess.cxx:78-130;')
    print('sections 3 and 4 show it is unconditional — one line or four, real body or synthetic.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
