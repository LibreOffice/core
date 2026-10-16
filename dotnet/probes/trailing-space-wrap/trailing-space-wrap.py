#!/usr/bin/env python3
"""A paragraph ending in a space costs LibreOffice two extra lines. We charge none.

What was measured, 2026-08-16, LibreOffice 26.2.4.2 with the full font set
-------------------------------------------------------------------------

`150-5370-10H.docx` page 289, paragraph `(1) Transverse measurements.` — two
top-level blocks sliced out of the real document reproduce it exactly:

    ref   para 1 starts y= 75.4, para 2 starts y=154.3, span 78.9 pt, 5 text lines
    ours  para 1 starts y= 75.4, para 2 starts y=129.0, span 53.6 pt, 4 text lines

The line pitch is 12.65 pt, so **the reference spends exactly two more line
pitches than we do on the same text** — 25.3 pt.

The paragraph is 20 runs of plain text: no `w:br`, no `w:tab`, no field, no
bookmark, no `w:sectPr`, one `w:pStyle` and nothing else. Its final run is

    <w:r><w:t xml:space="preserve">. </w:t></w:r>

and that trailing space is the whole cause. Ablated four ways on the two-block
slice:

    case               ref lines  ref gap   our lines  our gap
    baseline                   5     28.3           4     15.6
    merged-runs                5     28.3           4     15.6
    preserve-all               5     28.3           4     15.6
    no-trailing-space          4     15.6           4     15.6   <-- matches us

**Stripping the one trailing space makes the reference agree with us exactly**,
and merging all twenty runs into one changes nothing, so this is not a run-
boundary effect and not an `xml:space` reading difference.

What the two extra lines are
----------------------------

Both are visible in the geometry rather than inferred:

- The last word moves to a line of its own. Line 4 ends at x=262.45 with the
  text area running to about x=538 — roughly 275 pt of unused room — and
  `lanes.` sits alone on line 5. So this is **not** a width decision, and the
  advance-width divergence of task #49 does not explain it.
- A sixth, invisible line follows. The gap from the last text line to the next
  paragraph is 28.3 pt against 15.6 pt without the trailing space, and the
  difference is one 12.65 pt pitch.

What is not yet known
---------------------

The mechanism. Reading it off the geometry gives the rule but not the reason,
and "a trailing blank takes a line of its own" does not by itself explain why
the *preceding* word is pushed down with it. The place to look is Writer's own
blank handling on a line break — `SwTextGuess::Guess` and
`SwTextFormatter::NewTextPortion` in `sw/source/core/text/`, and
`SwTextPortion::Format_` where trailing blanks are measured — rather than
another geometric sweep, which has now given everything it can.

Worth knowing before implementing: this is a *line-breaking* change, so it moves
text on every document holding a paragraph that ends in a space, which is a very
large fraction of any real corpus. It needs the full `words/done-*` sweep, and
it should be measured for how many documents it moves before it is believed.

Usage
-----

    XRAY_OFFICE_CLI=... python3 trailing-space-wrap.py /abs/workdir
"""

import os
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
import zipfile

SRC = ('/c/sandbox/workdir/sample-files/words/pagination-002/docx/150-5370-10H.docx')

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
SPACE = '{http://www.w3.org/XML/1998/namespace}space'

# The two adjacent top-level blocks that carry the case: the paragraph that ends
# in a space, and the one after it whose position reveals the invisible line.
FIRST, SECOND = 3633, 3634


def load():
    with zipfile.ZipFile(SRC) as package:
        names = [item.filename for item in package.infolist()]
        return names, {name: package.read(name) for name in names}


def build(path, names, blob, mutate=None):
    raw = blob['word/document.xml'].decode('utf-8')
    header = re.search(r'<w:document[^>]*>', raw, re.S).group(0)
    for prefix, uri in re.findall(r'xmlns:([A-Za-z0-9]+)="([^"]+)"', header):
        ET.register_namespace(prefix, uri)

    root = ET.fromstring(raw)
    body = root.find(W + 'body')
    kids = list(body)
    keep = [kids[FIRST], kids[SECOND], kids[-1]]

    for kid in list(body):
        body.remove(kid)
    for kid in keep:
        body.append(kid)
    if mutate:
        mutate(keep[0])

    document = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\r\n'
                + ET.tostring(root, encoding='unicode')).encode('utf-8')

    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as out:
        for name in names:
            out.writestr(name, document if name == 'word/document.xml' else blob[name])


def merge_runs(paragraph):
    """One run holding the whole text, to rule out a run-boundary effect."""
    texts = [text.text or '' for text in paragraph.iter(W + 't')]
    for run in paragraph.findall(W + 'r'):
        paragraph.remove(run)

    run = ET.SubElement(paragraph, W + 'r')
    text = ET.SubElement(run, W + 't')
    text.set(SPACE, 'preserve')
    text.text = ''.join(texts)


def drop_trailing_space(paragraph):
    texts = list(paragraph.iter(W + 't'))
    texts[-1].text = (texts[-1].text or '').rstrip()


def preserve_all(paragraph):
    """`xml:space` on every run, to rule out a whitespace-stripping difference."""
    for text in paragraph.iter(W + 't'):
        text.set(SPACE, 'preserve')


def lines(pdf):
    text = subprocess.run(['pdftotext', '-bbox', '-f', '1', '-l', '1', pdf, '-'],
                          capture_output=True, text=True, check=True).stdout
    rows = {}
    for x0, y0, x1, word in re.findall(
            r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)"[^>]*>([^<]*)</word>', text):
        rows.setdefault(round(float(y0), 1), []).append((float(x0), float(x1), word))

    return sorted((y, max(x1 for _, x1, _ in v), ' '.join(w for _, _, w in sorted(v)))
                  for y, v in rows.items())


def summarise(rendered):
    """Text lines before the second paragraph, and the gap into it."""
    stop = next((i for i, (_, _, t) in enumerate(rendered) if t.startswith('(2)')), len(rendered))
    gap = rendered[stop][0] - rendered[stop - 1][0] if stop < len(rendered) else 0.0
    return stop, gap


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/trailing-space'
    cli = os.environ.get('XRAY_OFFICE_CLI')
    if not cli:
        raise SystemExit('set XRAY_OFFICE_CLI to the tree you mean to measure')

    names, blob = load()
    cases = [('baseline', None), ('merged-runs', merge_runs),
             ('preserve-all', preserve_all), ('no-trailing-space', drop_trailing_space)]

    print(f"{'case':18s} {'ref lines':>9s} {'ref gap':>8s} {'our lines':>9s} {'our gap':>8s}")

    for label, mutate in cases:
        directory = os.path.join(out, label)
        os.makedirs(os.path.join(directory, 'ours'), exist_ok=True)
        docx = os.path.join(directory, 't.docx')
        build(docx, names, blob, mutate)

        subprocess.run(['soffice', '--headless', '--convert-to', 'pdf', '--outdir', directory,
                        docx], capture_output=True, check=True)
        subprocess.run([cli, 'render', '--quiet', '--outdir', os.path.join(directory, 'ours'),
                        docx], check=True)

        reference = os.path.join(directory, 't.pdf')
        ours = os.path.join(directory, 'ours', 't.pdf')
        for produced in (reference, ours):
            if not os.path.isfile(produced):
                raise SystemExit(f'{produced} was not written — nothing to compare')

        rl, rg = summarise(lines(reference))
        ol, og = summarise(lines(ours))
        print(f'{label:18s} {rl:9d} {rg:8.1f} {ol:9d} {og:8.1f}')

    print('\nno-trailing-space matching ours is the finding: one space, two line pitches.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
