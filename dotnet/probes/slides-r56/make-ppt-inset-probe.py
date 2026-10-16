#!/usr/bin/env python3
"""An asymmetric-inset .ppt family for the txflTextFlow turn.

Round 55's rule -- *cite the C++ for intent, measure soffice for truth* -- applies twice here:
its own inset derivation for `a:bodyPr/@vert` was wrong by two slots until an asymmetric
10/20/30/40 pt fixture corrected it, and the binary path is a DIFFERENT code path
(`svdfppt.cxx` sets the four `SdrText*DistItem`s straight from `dxText*`, where
`oox`'s `TextBodyProperties::pushTextDistances` cyclically shifts them first).  So the two
formats' answers cannot be assumed equal.

The fixture is authored as flat ODF, converted to .ppt by the reference, and then the ONE
property under test is patched byte-for-byte -- the conversion supplies the insets and the
anchor, both of which are read back out of the produced .ppt and checked before anything is
measured, and the flow value is never the reference's own choice.

    make-ppt-inset-probe.py <outdir>
"""
import os, subprocess, sys

BOX = """  <draw:frame draw:style-name="box{n}" draw:text-style-name="P1"
      svg:width="{w}" svg:height="{h}" svg:x="{x}" svg:y="{y}">
   <draw:text-box>
    <text:p text:style-name="P1">Ag</text:p>
   </draw:text-box>
  </draw:frame>
"""

STYLE = """  <style:style style:name="box{n}" style:family="graphic">
   <style:graphic-properties fo:padding-left="{l}pt" fo:padding-top="{t}pt"
      fo:padding-right="{r}pt" fo:padding-bottom="{b}pt"
      style:writing-mode="{wm}"
      draw:textarea-vertical-align="{va}" draw:textarea-horizontal-align="{ha}"
      draw:auto-grow-width="false" draw:auto-grow-height="false"
      draw:fill="none" draw:stroke="solid" svg:stroke-color="#000000"/>
  </style:style>
"""

HEAD = """<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 xmlns:presentation="urn:oasis:names:tc:opendocument:xmlns:presentation:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.presentation">
 <office:automatic-styles>
  <style:style style:name="P1" style:family="paragraph">
   <style:paragraph-properties fo:text-align="start"/>
   <style:text-properties fo:font-size="16pt" style:font-name-complex="Liberation Sans"
      fo:font-family="'Liberation Sans'"/>
  </style:style>
  <style:page-layout style:name="PM1">
   <style:page-layout-properties fo:page-width="10in" fo:page-height="7.5in"
      fo:margin-top="0in" fo:margin-bottom="0in" fo:margin-left="0in" fo:margin-right="0in"/>
  </style:page-layout>
%s  <style:style style:name="dp1" style:family="drawing-page"/>
 </office:automatic-styles>
 <office:master-styles>
  <style:master-page style:name="M1" style:page-layout-name="PM1" draw:style-name="dp1"/>
 </office:master-styles>
 <office:body>
  <office:presentation>
   <draw:page draw:name="p1" draw:master-page-name="M1" draw:style-name="dp1">
%s   </draw:page>
  </office:presentation>
 </office:body>
</office:document>
"""

# `escherex.cxx:731-745` maps a VERTICAL box's horizontal adjust onto `anchorText`:
# LEFT -> AnchorBottom (2), CENTER -> AnchorMiddle (1), RIGHT/BLOCK -> AnchorTop (0).  So the
# three arms below produce the three values of the property whose reading is under test, and
# `style:writing-mode="tb-rl"` is what makes the exporter emit `txflTextFlow` at all
# (`escherex.cxx:730,779`) -- the value is then patched, so no arm keeps the one it chose.
# (name, vertical-align, horizontal-align, writing-mode)
ANCHORS = [("top", "top", "right", "tb-rl"),
           ("mid", "top", "center", "tb-rl"),
           ("bot", "top", "left", "tb-rl")]

# One inset at a time, against an all-zero control.  Reading the mapping off a single
# 10/20/30/40 box needs the first line's ascent as a known constant and it is not one; a
# DIFFERENCE against the zero arm cancels it exactly, so each of these five files answers
# "which edge of the turned frame does this shape inset feed" on its own.
QUADS = {
    "zero": (0, 0, 0, 0),
    "l40": (40, 0, 0, 0),
    "t40": (0, 40, 0, 0),
    "r40": (0, 0, 40, 0),
    "b40": (0, 0, 0, 40),
}


def main(out):
    os.makedirs(out, exist_ok=True)
    for tag, (l, t, r, b) in QUADS.items():
        styles, boxes = "", ""
        for n, (_name, va, ha, wm) in enumerate(ANCHORS):
            styles += STYLE.format(n=n, l=l, t=t, r=r, b=b, va=va, ha=ha, wm=wm)
            boxes += BOX.format(n=n, w="2.4in", h="1.6in", x=f"{0.5 + n * 3.0}in", y="1in")
        path = os.path.join(out, f"{tag}.fodp")
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(HEAD % (styles, boxes))
        subprocess.run(
            ["soffice", f"-env:UserInstallation=file://{os.path.abspath(out)}/prof",
             "--headless", "--convert-to", "ppt", "--outdir", out, path],
            check=True, capture_output=True, timeout=300)
        print(os.path.join(out, f"{tag}.ppt"))


if __name__ == "__main__":
    main(sys.argv[1])
