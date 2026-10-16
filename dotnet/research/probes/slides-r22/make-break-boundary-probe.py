#!/usr/bin/env python3
"""Does the autofit device grid reach *line breaking*, or only the line height?

    make-break-boundary-probe.py <out.pptx> [firstWidthPt] [stepPt] [count]

Round twenty-one established that an autofitted shape measures its lines at the reference
device's realisation of the em rather than at the em itself — `ImpEditEngine::SeekCursor` puts
the whole font at the device and reads the size back out of its metric
(`editeng/source/editeng/impedit3.cxx`:2985-3062). Only the *line height* was changed, because
only the line height had been measured. `SetFontSize` sets the whole size, so the advance widths
at format time are taken at the realised size too — and whether that ever moves a line break was
left open.

The drawn widths cannot answer it. Measured on `slide-autofit-device-grid.pptx`: every word in
the autofitted column is the same width to four decimal places as the same word in the plain
column, at all five sizes, *including* the four where the line pitch differs. That is expected —
at paint time both go through the PDF export device, which realises them identically — and it
means the question can only be settled at a line *boundary*.

So: one slide per box width, the same phrase in a plain box at y = 60 and in an autofitted box at
y = 300 too tall to shrink, the width stepped across a wrapping boundary. Stacked rather than
side by side because the widths under test run to 400 points and two of those do not fit across
a 720 point slide — which the first version did, and its two columns then overlapped and were
read as one. Whichever box is measuring at a larger em wraps first.

At 20 pt the realised em is 707 hundredths of a millimetre against the stated 705.55, which is
+0.206 per cent — about 0.4 pt on a 200 pt line, or four steps of this probe. Predicted, if the
grid reaches advance widths: the autofitted column wraps at a box **wider** than the plain one by
about four steps. Predicted, if it does not: the two columns wrap at the same step.
"""
import os
import re
import shutil
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
SHELL = os.path.join(HERE, '..', '..', '..', 'tests', 'corpus', 'features',
                     'slide-autofit-device-grid.pptx')

EMU_PER_POINT = 12700
FACE = 'Liberation Sans'
SIZE = 2000                       # hundredths of a point
PHRASE = 'Alpha Bravo Charlie Delta Echo Foxtrot Golf'

NS = ('<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
      'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
      'xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">')


def emu(points):
    return int(round(points * EMU_PER_POINT))


def box(shape_id, name, y_pt, width_pt, autofit):
    fit = '<a:normAutofit/>' if autofit else '<a:noAutofit/>'
    rpr = f'<a:rPr lang="en-GB" sz="{SIZE}"><a:latin typeface="{FACE}"/></a:rPr>'
    return (
        f'<p:sp><p:nvSpPr><p:cNvPr id="{shape_id}" name="{name}"/>'
        '<p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr><p:spPr>'
        f'<a:xfrm><a:off x="{emu(20)}" y="{emu(y_pt)}"/>'
        f'<a:ext cx="{emu(width_pt)}" cy="{emu(200)}"/></a:xfrm>'
        '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>'
        '<p:txBody><a:bodyPr wrap="square" lIns="0" tIns="0" rIns="0" bIns="0" anchor="t">'
        f'{fit}</a:bodyPr><a:lstStyle/>'
        f'<a:p><a:pPr algn="l"/><a:r>{rpr}<a:t>{PHRASE}</a:t></a:r></a:p>'
        '</p:txBody></p:sp>')


def warmup():
    """LibreOffice's shared draw outliner formats the first text object on a page before
    SetFixedCellHeight takes hold, so nothing under test may be first."""
    rpr = f'<a:rPr lang="en-GB" sz="1200"><a:latin typeface="{FACE}"/></a:rPr>'
    return (
        '<p:sp><p:nvSpPr><p:cNvPr id="2" name="warmup"/><p:cNvSpPr txBox="1"/><p:nvPr/>'
        f'</p:nvSpPr><p:spPr><a:xfrm><a:off x="{emu(20)}" y="{emu(5)}"/>'
        f'<a:ext cx="{emu(200)}" cy="{emu(30)}"/></a:xfrm>'
        '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>'
        '<p:txBody><a:bodyPr wrap="square" lIns="0" tIns="0" rIns="0" bIns="0" anchor="t">'
        '<a:noAutofit/></a:bodyPr><a:lstStyle/>'
        f'<a:p><a:pPr algn="l"/><a:r>{rpr}<a:t>Warm</a:t></a:r></a:p></p:txBody></p:sp>')


def slide(width_pt):
    return (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' + NS +
        '<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/>'
        '</p:nvGrpSpPr><p:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/>'
        '<a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/></a:xfrm></p:grpSpPr>' +
        warmup() +
        box(3, 'plain', 60.0, width_pt, autofit=False) +
        box(4, 'fitted', 300.0, width_pt, autofit=True) +
        '</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sld>'
    ).encode('utf-8')


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/break-boundary.pptx'
    first = float(sys.argv[2]) if len(sys.argv) > 2 else 316.0
    step = float(sys.argv[3]) if len(sys.argv) > 3 else 0.1
    count = int(sys.argv[4]) if len(sys.argv) > 4 else 24

    shell = os.path.abspath(SHELL)
    if not os.path.exists(shell):
        print(f'no shell deck at {shell}', file=sys.stderr)
        return 2

    widths = [first + step * i for i in range(count)]

    with zipfile.ZipFile(shell) as src:
        keep = {n: src.read(n) for n in src.namelist()
                if not re.match(r'ppt/slides/(_rels/)?slide\d+\.xml(\.rels)?$', n)}
        rels = src.read('ppt/slides/_rels/slide1.xml.rels')

    # The shell's presentation.xml and [Content_Types].xml list its five slides by name; both
    # are rewritten wholesale rather than patched, because a stale entry produces a deck that
    # opens and silently drops slides.
    presentation = keep['ppt/presentation.xml'].decode('utf-8')
    sldid = ''.join(
        f'<p:sldId id="{256 + i}" r:id="rIdSlide{i + 1}"/>' for i in range(count))
    presentation = re.sub(r'<p:sldIdLst>.*?</p:sldIdLst>', f'<p:sldIdLst>{sldid}</p:sldIdLst>',
                          presentation, flags=re.S)
    keep['ppt/presentation.xml'] = presentation.encode('utf-8')

    prels = keep['ppt/_rels/presentation.xml.rels'].decode('utf-8')
    prels = re.sub(r'<Relationship[^>]*slides/slide\d+\.xml"[^>]*/>', '', prels)
    added = ''.join(
        f'<Relationship Id="rIdSlide{i + 1}" '
        'Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide" '
        f'Target="slides/slide{i + 1}.xml"/>' for i in range(count))
    prels = prels.replace('</Relationships>', added + '</Relationships>')
    keep['ppt/_rels/presentation.xml.rels'] = prels.encode('utf-8')

    types = keep['[Content_Types].xml'].decode('utf-8')
    types = re.sub(r'<Override PartName="/ppt/slides/slide\d+\.xml"[^>]*/>', '', types)
    added = ''.join(
        f'<Override PartName="/ppt/slides/slide{i + 1}.xml" '
        'ContentType="application/vnd.openxmlformats-officedocument.'
        'presentationml.slide+xml"/>' for i in range(count))
    types = types.replace('</Types>', added + '</Types>')
    keep['[Content_Types].xml'] = types.encode('utf-8')

    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as dst:
        for name, data in keep.items():
            dst.writestr(name, data)
        for i, width in enumerate(widths, start=1):
            dst.writestr(f'ppt/slides/slide{i}.xml', slide(width))
            dst.writestr(f'ppt/slides/_rels/slide{i}.xml.rels', rels)

    print(f'{out}: {count} slides, {widths[0]:.2f} to {widths[-1]:.2f} pt in {step} pt steps')
    return 0


if __name__ == '__main__':
    sys.exit(main())
