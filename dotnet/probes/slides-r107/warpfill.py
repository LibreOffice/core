#!/usr/bin/env python3
"""How many corpus WordArt shapes need a fill a solid colour cannot draw.

Half (b) of O13 is `SlideFontwork.Read` ending `Paint.Solid(stated.Colour)`. For a DrawingML
shape the colour is the *run's*, because `lcl_copyCharPropsToShape` (`oox/source/drawingml/
shape.cxx`:721-905) copies the first non-empty run's fill onto the shape before the Fontwork
engine sees it -- so what a non-solid case looks like is an `a:rPr` carrying `a:gradFill`,
`a:blipFill`, `a:pattFill` or `a:grpFill` rather than `a:solidFill`.

This walks every slide, layout and master of every .pptx in the corpus, finds each text body
whose `a:bodyPr/a:prstTxWarp` states a warp other than `textNoShape`, takes its first non-blank
run the way `shape.cxx`:748-756 does, and reports what kind of fill that run states.

    warpfill.py <file.pptx>...
"""
import sys, zipfile
from xml.etree import ElementTree as ET

A = '{http://schemas.openxmlformats.org/drawingml/2006/main}'
FILLS = ['noFill', 'solidFill', 'gradFill', 'blipFill', 'pattFill', 'grpFill']


def bodies(root):
    for body in root.iter():
        if not body.tag.endswith('}txBody') and not body.tag.endswith('}txbxContent'):
            continue
        pr = body.find(A + 'bodyPr')
        if pr is None:
            continue
        warp = pr.find(A + 'prstTxWarp')
        if warp is None:
            continue
        prst = warp.get('prst')
        if not prst or prst == 'textNoShape':
            continue
        yield prst, body


def first_run(body):
    """The first run the reference would take its character properties from."""
    for para in body.findall(A + 'p'):
        for run in para.findall(A + 'r'):
            text = run.findtext(A + 't') or ''
            if len(text) == 0:
                continue
            if len(text) == 1 and text in (' ', ' '):
                continue
            return run
    return None


def main(paths):
    counts = {k: 0 for k in FILLS}
    counts['(none stated)'] = 0
    total = 0
    docs = set()
    nonsolid_docs = set()
    for path in paths:
        try:
            z = zipfile.ZipFile(path)
        except Exception as e:
            print(f'{path}\tERROR {e}')
            continue
        for name in z.namelist():
            if not (name.startswith('ppt/slides/') or name.startswith('ppt/slideLayouts/')
                    or name.startswith('ppt/slideMasters/')) or not name.endswith('.xml'):
                continue
            try:
                root = ET.fromstring(z.read(name))
            except ET.ParseError:
                continue
            for prst, body in bodies(root):
                total += 1
                docs.add(path)
                run = first_run(body)
                pr = run.find(A + 'rPr') if run is not None else None
                kind = '(none stated)'
                if pr is not None:
                    for f in FILLS:
                        if pr.find(A + f) is not None:
                            kind = f
                            break
                counts[kind] += 1
                if kind in ('gradFill', 'blipFill', 'pattFill', 'grpFill'):
                    nonsolid_docs.add(path)
                    print(f'{path.rsplit("/", 1)[-1]}\t{name}\t{prst}\t{kind}')
        z.close()
    print(f'# warped bodies: {total} in {len(docs)} documents')
    for k in FILLS + ['(none stated)']:
        print(f'#   {k:16s} {counts[k]}')
    print(f'# documents with a non-solid first-run fill: {len(nonsolid_docs)}')


if __name__ == '__main__':
    main(sys.argv[1:])
