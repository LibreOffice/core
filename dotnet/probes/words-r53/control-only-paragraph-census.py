#!/usr/bin/env python3
"""Every paragraph in the corpus whose whole laid-out text is format-control characters.

    python3 control-only-paragraph-census.py <corpus-root> [family ...]

`TextItemiser.IsFormatControl` cuts U+0000, U+200E..U+200F, U+2028..U+202E, U+2060,
U+206A..U+206F, U+FEFF, U+FFFE and U+FFFF out of every sub-run, so a paragraph made of
nothing else reaches `MeasuredParagraph` with **no runs at all** and every one of its lines
measures 0 pt.  The commonest member by far is U+2028, which is what all four word-processing
readers and both DrawingML readers emit for a manual line break.

What this counts, per document and per part:

  ooxml words   `w:p` holding at least one `w:br` that is not `w:type="page"`, and no other
                content: no non-empty `w:t`, no `w:tab`, `w:sym`, `w:noBreakHyphen`,
                `w:drawing`, `w:pict`, `w:object`, `w:fldSimple` or note reference.
  drawingml     `a:p` holding at least one `a:br` and no `a:r` with non-empty `a:t`, no
                `a:fld`.  Reached from `ppt/**`, `xl/drawings/**` and `word/**` alike.
  odf           `text:p`/`text:h` holding at least one `text:line-break` and no other text.

**What it cannot see**, and this is written down before the sweep rather than after it:

  * `.doc`, `.rtf` and the legacy binary formats.  Their readers emit the same U+2028 (see
    `Ww8DocumentReader.Layout.cs:1187`, `RtfDocumentReader.State.cs:945`) and this census
    reads neither.  So the words figure is a floor.
  * a paragraph made of *other* control characters — a lone U+200E, a lone U+FEFF — which is
    the same defect and is not counted here.
  * whether the paragraph reaches the per-run path at all: the single-face path measures the
    same paragraph correctly, and a reader that emits no runs takes it.  DOCX always emits
    runs; the other readers were not checked.
  * inheritance of any kind: this is a shape census over parts, which is exactly the thing
    HANDOVER §7 says under-reaches.
"""
import os, re, sys, zipfile
from xml.etree import ElementTree as ET

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
A = '{http://schemas.openxmlformats.org/drawingml/2006/main}'
TEXT = '{urn:oasis:names:tc:opendocument:xmlns:text:1.0}'

WORD_CONTENT = {'tab', 'sym', 'noBreakHyphen', 'object', 'fldSimple',
                'footnoteReference', 'endnoteReference', 'ptab', 'softHyphen'}

WP = '{http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing}'


def is_inline_drawing(node):
    """True for a drawing that puts a character in the text.

    An as-character `wp:inline` reaches the reader as U+0001, which is C0 and so survives
    itemisation — the paragraph keeps a run and is not affected.  A `wp:anchor` is a floating
    frame and contributes no character at all, so a paragraph holding one and a break is
    still a paragraph whose whole text is one control character.
    """
    return any(child.tag == WP + 'inline' for child in node)


def word_paragraphs(root):
    breaks = other = 0
    for p in root.iter(W + 'p'):
        breaks = other = 0
        for node in p.iter():
            if not node.tag.startswith(W):
                continue
            name = node.tag[len(W):]
            if name == 'br':
                if node.get(W + 'type') != 'page':
                    breaks += 1
            elif name == 't':
                if (node.text or '') != '':
                    other += 1
            elif name == 'instrText':
                other += 1
            elif name == 'drawing':
                if is_inline_drawing(node):
                    other += 1
            elif name == 'pict':
                # VML: an inline shape sits in a run and takes a character; a floating one does not.
                if node.find('.//' + W + 'binData') is not None or 'inline' in (node.get('layout') or ''):
                    other += 1
            elif name in WORD_CONTENT:
                other += 1
        if breaks and not other:
            yield p


def drawingml_paragraphs(root):
    for p in root.iter(A + 'p'):
        breaks = other = 0
        for node in p:
            name = node.tag[len(A):] if node.tag.startswith(A) else ''
            if name == 'br':
                breaks += 1
            elif name == 'r':
                t = node.find(A + 't')
                if t is not None and (t.text or '') != '':
                    other += 1
            elif name == 'fld':
                other += 1
        if breaks and not other:
            yield p


def odf_paragraphs(root):
    for p in root.iter():
        if p.tag not in (TEXT + 'p', TEXT + 'h'):
            continue
        breaks = 0
        other = (p.text or '').strip()
        for node in p.iter():
            if node is p:
                continue
            if node.tag == TEXT + 'line-break':
                breaks += 1
            else:
                other += (node.text or '').strip()
            other += (node.tail or '').strip()
        if breaks and not other:
            yield p


def scan(path):
    """(word, drawingml, odf) counts for one file, or None when it cannot be opened."""
    counts = [0, 0, 0]
    try:
        with zipfile.ZipFile(path) as zf:
            for name in zf.namelist():
                if not name.endswith('.xml'):
                    continue
                try:
                    root = ET.fromstring(zf.read(name))
                except ET.ParseError:
                    continue
                if name.startswith('word/'):
                    counts[0] += sum(1 for _ in word_paragraphs(root))
                counts[1] += sum(1 for _ in drawingml_paragraphs(root))
                if name in ('content.xml', 'styles.xml'):
                    counts[2] += sum(1 for _ in odf_paragraphs(root))
    except (zipfile.BadZipFile, OSError):
        return None
    return counts


def main():
    root = sys.argv[1]
    families = sys.argv[2:] or ['words', 'slides', 'sheets']
    manifest = os.path.join(root, 'MANIFEST.tsv')
    wanted = {}
    with open(manifest) as f:
        head = f.readline().rstrip('\n').split('\t')
        for line in f:
            row = dict(zip(head, line.rstrip('\n').split('\t')))
            if row['family'] in families:
                wanted[row['path']] = row

    totals = {}
    for rel, row in sorted(wanted.items()):
        counts = scan(os.path.join(root, rel))
        if counts is None or not any(counts):
            continue
        totals[rel] = (row['family'], row['status'], counts)

    for family in families:
        rows = [(p, v) for p, v in totals.items() if v[0] == family]
        n = sum(sum(v[2]) for _, v in rows)
        done = sum(1 for _, v in rows if v[1] == 'done')
        print(f'{family}: {n} paragraphs in {len(rows)} documents ({done} of them passing)')
        for p, v in sorted(rows, key=lambda kv: -sum(kv[1][2]))[:15]:
            print(f'    {sum(v[2]):5d}  w={v[2][0]:4d} d={v[2][1]:4d} o={v[2][2]:4d}  {v[1]:5s}  {p}')


if __name__ == '__main__':
    main()
