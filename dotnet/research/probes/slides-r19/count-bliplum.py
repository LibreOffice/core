#!/usr/bin/env python3
"""How many corpus decks put an <a:lum> inside an <a:blip>, and how many blips carry one.

A blip's <a:lum bright= contrast=> is PowerPoint's picture recolour — "Washout" is
bright="70000" contrast="-70000" — and is a different element from the <a:lum> that
transforms a *colour*, which shares its name and sits under a colour element instead.
Counted by parsing rather than by grepping the tag, because the two are indistinguishable
to a grep.
"""
import collections
import os
import re
import sys
import zipfile

NS_A = 'http://schemas.openxmlformats.org/drawingml/2006/main'
BLIP = f'{{{NS_A}}}blip'
LUM = f'{{{NS_A}}}lum'

import xml.etree.ElementTree as ET


def scan(path):
    """(blips, blips carrying a lum, the (bright, contrast) pairs seen)."""
    blips = lums = 0
    pairs = collections.Counter()
    try:
        with zipfile.ZipFile(path) as z:
            for name in z.namelist():
                if not name.endswith('.xml'):
                    continue
                try:
                    root = ET.fromstring(z.read(name))
                except ET.ParseError:
                    continue
                for blip in root.iter(BLIP):
                    blips += 1
                    lum = blip.find(LUM)
                    if lum is not None:
                        lums += 1
                        pairs[(lum.get('bright', '0'), lum.get('contrast', '0'))] += 1
    except (zipfile.BadZipFile, OSError):
        return None
    return blips, lums, pairs


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else '/workspace/sample-files'
    total = collections.Counter()
    documents = 0
    carriers = []
    for dirpath, _, names in os.walk(root):
        for name in names:
            if not name.lower().endswith(('.pptx', '.pptm', '.potx', '.ppsx')):
                continue
            path = os.path.join(dirpath, name)
            result = scan(path)
            if result is None:
                continue
            documents += 1
            blips, lums, pairs = result
            total.update(pairs)
            if lums:
                carriers.append((lums, blips, os.path.relpath(path, root)))
    carriers.sort(reverse=True)
    print(f'{documents} pptx-family documents under {root}')
    print(f'{len(carriers)} carry a blip <a:lum>')
    for lums, blips, rel in carriers:
        print(f'  {lums:3d} of {blips:3d} blips  {rel}')
    print('(bright, contrast) pairs, by blip count:')
    for pair, count in total.most_common():
        print(f'  {pair}  x{count}')


if __name__ == '__main__':
    main()
