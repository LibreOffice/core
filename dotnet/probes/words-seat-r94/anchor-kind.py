#!/usr/bin/env python3
"""What each wp:anchor holds, which is what decides whether 26.2.4.2 captures it.

`SwAnchoredObjectPosition`'s constructor (anchoredobjectposition.cxx:125-144) sets
`mbDoNotCaptureAnchoredObj = bConsidered && !mbFollowTextFlow && DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE`,
and `bConsidered` is asked differently of the two object kinds:

    fly  (a picture, a frame)   bWrapThrough && !bTextBox
    draw (a shape)              bWrapThrough || !bTextBox

so with DOCX's DO_NOT_CAPTURE set:
    a picture           captured unless wrap-through
    a shape WITH text   captured unless wrap-through   (it is a textbox pair)
    a shape WITHOUT text  never captured, whatever the wrap
"""
import sys, zipfile, collections
from xml.etree import ElementTree as ET
W='{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
WP='{http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing}'
A='{http://schemas.openxmlformats.org/drawingml/2006/main}'
PIC='{http://schemas.openxmlformats.org/drawingml/2006/picture}'
WPS='{http://schemas.microsoft.com/office/word/2010/wordprocessingShape}'
WPG='{http://schemas.microsoft.com/office/word/2010/wordprocessingGroup}'

def kind(anchor):
    tags = {e.tag for e in anchor.iter()}
    if PIC + 'pic' in tags:  return 'picture'
    if WPS + 'txbx' in tags: return 'shape+text'
    if WPG + 'wgp' in tags:  return 'group'
    if WPS + 'wsp' in tags:  return 'shape'
    return 'other'

def wrap(a):
    for c in a:
        n = c.tag.split('}')[1]
        if n.startswith('wrap'): return n
    return None

for path in sys.argv[1:]:
    c = collections.Counter()
    with zipfile.ZipFile(path) as z:
        for n in z.namelist():
            if not (n.startswith('word/') and n.endswith('.xml')): continue
            try: root = ET.fromstring(z.read(n))
            except ET.ParseError: continue
            for a in root.iter(WP + 'anchor'):
                c[(kind(a), wrap(a))] += 1
    print(path.split('/')[-1])
    for k, v in sorted(c.items()):
        cap = ('not captured' if k[1] == 'wrapNone' or k[0] == 'shape'
               else 'CAPTURED')
        print(f"   {v:>3}  {k[0]:<11} {str(k[1]):<12} -> {cap}")
