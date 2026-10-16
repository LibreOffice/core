#!/usr/bin/env python3
"""Does 26.2.4.2 draw each of the corpus's six 3-D charts as vectors or as a raster?

For each of the six documents, list every image the reference's own PDF places, and count the
vector paths that fall inside the chart's own frame.  A 3-D scene the reference rasterises
carries an image over the plot area and no geometry inside it; one it draws as vectors carries
many-segment filled paths and no image.

Run: python3 threed-raster.py > threed.tsv
"""
import pymupdf, sys

BANK = '/home/user/gate-orig-r83/ref/'

# document, reference PDF, page (1-based), the chart's frame in that page's points
CASES = [
    ('words/chartset-001/docx/pie-chart-template.docx',
     'pie-chart-template__docx.pdf', 1, (128.0, 111.0, 510.0, 333.0), '3-D pie (c:pie3DChart)'),
    ('words/chartset-001/docx/pie-chart-result.docx',
     'pie-chart-result__docx.pdf', 1, (128.0, 111.0, 510.0, 333.0), '3-D pie (c:pie3DChart)'),
    ('words/chartset-012/docx/021_Unit_Circle_Chart_3D_Pie_Chart_404247ab.docx',
     '021_Unit_Circle_Chart_3D_Pie_Chart_404247ab__docx.pdf', 1,
     (28.0, 246.0, 552.0, 526.0), '3-D pie (c:pie3DChart)'),
    ('sheets/done-010/xls/TOGAF9-Tool-ConfReqts-CSQ.xls',
     'TOGAF9-Tool-ConfReqts-CSQ__xls.pdf', 21, (72.7, 332.9, 496.5, 514.7), '3-D bar (BIFF CH3D)'),
    # the plot area alone: this sheet's page also carries a pivot table whose cell borders
    # would otherwise be counted as the chart's geometry
    ('sheets/missing-001/xls/orbus_togaf_tool_csq.xls',
     'orbus_togaf_tool_csq__xls.pdf', 27, (216.9, 395.7, 480.9, 442.9), '3-D bar (BIFF CH3D)'),
    ('slides/done-004/ppt/undp_presentation_revised_17_may.ppt',
     'undp_presentation_revised_17_may__ppt.pdf', 19, (40.0, 342.0, 258.0, 460.0),
     '3-D pie (OLE Excel.Sheet.8)'),
]

print('document\tkind\timages over the frame\tvector paths inside it (items)\tverdict')
for rel, pdf, page, frame, kind in CASES:
    d = pymupdf.open(BANK + pdf)
    pg = d[page - 1]
    box = pymupdf.Rect(*frame)

    images = []
    for info in pg.get_image_info(xrefs=True):
        r = pymupdf.Rect(info['bbox'])
        if not r.intersects(box):
            continue
        filt = ''
        if info.get('xref'):
            try:
                filt = d.xref_get_key(info['xref'], 'Filter')[1]
            except Exception:
                filt = '?'
        images.append(f"{info['width']}x{info['height']}{filt} "
                      f"{r.width:.1f}x{r.height:.1f}pt")

    inside = []
    for dr in pg.get_drawings():
        r = pymupdf.Rect(dr['rect'])
        if not r.intersects(box):
            continue
        # the chart's own background is a plain rectangle covering the whole frame
        if len(dr['items']) <= 5 and r.width > box.width * 0.9:
            continue
        inside.append(len(dr['items']))

    raster = bool(images) and not any(n > 8 for n in inside)
    print(f"{rel}\t{kind}\t{'; '.join(images) or 'none'}\t"
          f"{sorted(inside, reverse=True)[:6] or 'none'}\t"
          f"{'RASTER' if raster else 'VECTOR'}")
