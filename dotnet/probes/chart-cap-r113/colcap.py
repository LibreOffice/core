#!/usr/bin/env python3
"""O51, the VERTICAL branch: is a value axis' cap taken against the drawn plot height?

Same question as `cat-set-027.py` asks of a bar chart, asked of a column chart, where the
crossing axis is the horizontal category axis and the cap divides by
`m_nMaximumTextHeightSoFar` -- one line of digits, whatever the labels say. So the only
variable left in `nTotalAvailable / nSingleNeeded` is the numerator, and the category labels
are what move it.

`createMaximumLabels` sets `m_bOverlapAllowed = true` and `m_bLineBreakAllowed = false`,
and `canAutoAdjustLabelPlacement` returns false the moment either is set
(`VCartesianAxis.cxx`:539-556), so **the maximum pass draws one unwrapped, unturned line**
however long the category names are. If the numerator is the rectangle that pass leaves,
lengthening the category names must not change the value axis' interval at all -- even as
the drawn plot loses a third of its height to turned labels.

The container is `038_Competitive_Advantage_Card`'s slide 1, whose `p:graphicFrame` this
rewrites, with its chart part replaced by the minimal column chart below so that nothing
but the category text and the frame varies.

    colcap.py write <dir> <cy_pt> <label>...
    colcap.py read  <dir>/ref
"""
import glob
import os
import re
import sys
import zipfile

SRC = ("/home/user/sample-files/slides/chartset-008/pptx/"
       "038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.pptx")
CHART = "ppt/charts/chart1.xml"
SLIDE = "ppt/slides/slide1.xml"
FRAME_CX, FRAME_CY = 3585210, 2824797
FRAME_X, FRAME_Y = 5218265, 2300380
EMU = 12700.0

VALUES = [12000, 9000, 5000, 7000, 3000]

CHART_XML = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<c:chartSpace xmlns:c="http://schemas.openxmlformats.org/drawingml/2006/chart"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<c:chart><c:autoTitleDeleted val="1"/><c:plotArea><c:layout/>
<c:barChart><c:barDir val="col"/><c:grouping val="clustered"/><c:varyColors val="0"/>
<c:ser><c:idx val="0"/><c:order val="0"/>
<c:cat><c:strRef><c:f>Sheet1!$A$1</c:f><c:strCache>%(cats)s</c:strCache></c:strRef></c:cat>
<c:val><c:numRef><c:f>Sheet1!$B$1</c:f><c:numCache><c:formatCode>General</c:formatCode>%(vals)s</c:numCache></c:numRef></c:val>
</c:ser><c:gapWidth val="100"/><c:axId val="111"/><c:axId val="222"/></c:barChart>
<c:catAx><c:axId val="111"/><c:scaling><c:orientation val="minMax"/></c:scaling>
<c:delete val="0"/><c:axPos val="b"/><c:numFmt formatCode="General" sourceLinked="1"/>
<c:majorTickMark val="none"/><c:minorTickMark val="none"/><c:tickLblPos val="nextTo"/>
<c:txPr><a:bodyPr rot="0" vert="horz"/><a:lstStyle/><a:p><a:pPr><a:defRPr sz="900"/></a:pPr><a:endParaRPr lang="en-US"/></a:p></c:txPr>
<c:crossAx val="222"/><c:crosses val="autoZero"/><c:auto val="1"/><c:lblAlgn val="ctr"/>
<c:lblOffset val="100"/><c:noMultiLvlLbl val="0"/></c:catAx>
<c:valAx><c:axId val="222"/><c:scaling><c:orientation val="minMax"/></c:scaling>
<c:delete val="0"/><c:axPos val="l"/><c:numFmt formatCode="General" sourceLinked="0"/>
<c:majorTickMark val="none"/><c:minorTickMark val="none"/><c:tickLblPos val="nextTo"/>
<c:txPr><a:bodyPr rot="0" vert="horz"/><a:lstStyle/><a:p><a:pPr><a:defRPr sz="900"/></a:pPr><a:endParaRPr lang="en-US"/></a:p></c:txPr>
<c:crossAx val="111"/><c:crosses val="autoZero"/><c:crossBetween val="between"/></c:valAx>
</c:plotArea><c:plotVisOnly val="1"/><c:dispBlanksAs val="gap"/></c:chart>
</c:chartSpace>"""


def chart_of(labels):
    cats = '<c:ptCount val="%d"/>' % len(labels) + "".join(
        '<c:pt idx="%d"><c:v>%s</c:v></c:pt>' % (i, t) for i, t in enumerate(labels))
    vals = '<c:ptCount val="%d"/>' % len(labels) + "".join(
        '<c:pt idx="%d"><c:v>%d</c:v></c:pt>' % (i, VALUES[i % len(VALUES)])
        for i in range(len(labels)))
    return CHART_XML % {"cats": cats, "vals": vals}


def write(into, name, labels, cy):
    dst = os.path.join(into, name + ".pptx")
    with zipfile.ZipFile(SRC) as zin, \
            zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as out:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == CHART:
                data = chart_of(labels).encode("utf-8")
            elif item.filename == SLIDE:
                text = data.decode("utf-8")
                text = text.replace('<a:ext cx="%d" cy="%d"/>' % (FRAME_CX, FRAME_CY),
                                    '<a:ext cx="%d" cy="%d"/>' % (FRAME_CX, cy))
                data = text.encode("utf-8")
            out.writestr(item, data)
    return dst


def read(refdir):
    import pymupdf
    x0 = FRAME_X / EMU
    print("variant\tvlabels\tstep\tplot_h\tcat_dir\tcat_lines")
    for path in sorted(glob.glob(os.path.join(refdir, "*.pdf"))):
        name = os.path.basename(path)[:-4]
        with pymupdf.open(path) as doc:
            page = doc[0]
            nums, cats = [], []
            for block in page.get_text("dict")["blocks"]:
                if block["type"] != 0:
                    continue
                for line in block["lines"]:
                    for span in line["spans"]:
                        bx0, by0, bx1, by1 = span["bbox"]
                        if bx0 < x0 - 40 or not (8.0 < span["size"] < 10.0):
                            continue
                        t = span["text"].strip()
                        if not t:
                            continue
                        (nums if re.fullmatch(r"[\d,.]+", t) else cats).append(
                            (bx0, by0, bx1, by1, t, tuple(round(v, 3) for v in line["dir"])))
            nums.sort(key=lambda r: -r[1])
            bars = [d["rect"] for d in page.get_drawings()
                    if d["type"] == "f" and d["rect"].x0 > x0 - 40
                    and d["rect"].width > 2 and d["rect"].height > 1]
            tall = max((r.height for r in bars), default=0.0)
            step = nums[1][4] if len(nums) > 1 else "-"
            print("%s\t%d\t%s\t%.2f\t%s\t%d"
                  % (name, len(nums), step, tall * (float(nums[0][4].replace(",", ""))
                                                    / VALUES[0]) if nums else 0,
                     cats[0][5] if cats else "-", len(cats)))


if __name__ == "__main__":
    if sys.argv[1] == "write":
        into = sys.argv[2]
        os.makedirs(into, exist_ok=True)
        cy = int(float(sys.argv[3]) * EMU)
        for spec in sys.argv[4:]:
            tag, label, n = spec.split(":")
            print(write(into, tag, [label] * int(n), cy))
    else:
        read(sys.argv[2])
