#!/usr/bin/env python3
"""The 24.2.7.2 audit re-check for `SlideDrawing.FillReachesThePage`, on 26.2.4.2.

Two claims live at that site and they need different fixtures:

  A. a metafile the PACKAGE carries as an entry of its own loses its picture frame's fill,
     whatever the frame states;
  B. the same metafile INLINE -- ODF's `office:binary-data`, a `.ppt`'s Escher blip -- keeps it.

A is checked by rendering a corpus deck three ways: as found, with the frame's fill changed to
red, and with it replaced by `a:noFill`.  Three identical page images means the fill never
reached the page.

B needs a DISCRIMINATING PAIR, because a single flat-ODF rendering only shows what one storage
does: the same bytes as `office:binary-data` in a flat `.fodp`, and as a `Pictures/` entry in a
zipped `.odp` written from it.  If the two agree, storage is not the variable and the site's
rule is wrong; if they differ, it is the rule.

    audit_picturefill.py <emf> <outdir>
"""
import base64, os, shutil, subprocess, sys, zipfile

FODP = """<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 xmlns:xlink="http://www.w3.org/1999/xlink"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.presentation">
 <office:automatic-styles>
  <style:style style:name="pic" style:family="graphic">
   <style:graphic-properties draw:fill="solid" draw:fill-color="#FF0000"
      draw:stroke="none"/>
  </style:style>
 </office:automatic-styles>
 <office:body><office:presentation>
  <draw:page draw:name="p1">
   <draw:frame draw:style-name="pic" svg:width="4in" svg:height="3in" svg:x="1in" svg:y="1in">
    <draw:image>
     <office:binary-data>%s</office:binary-data>
    </draw:image>
   </draw:frame>
  </draw:page>
 </office:presentation></office:body>
</office:document>
"""


def render(path, out, profile):
    subprocess.run(
        ["soffice", f"-env:UserInstallation=file://{profile}", "--headless",
         "--convert-to", "pdf", "--outdir", out, path],
        check=False, capture_output=True, timeout=600)
    stem = os.path.splitext(os.path.basename(path))[0]
    return os.path.join(out, stem + ".pdf")


if __name__ == "__main__":
    emf, out = sys.argv[1], os.path.abspath(sys.argv[2])
    os.makedirs(out, exist_ok=True)
    profile = os.path.join(out, "prof")

    inline = os.path.join(out, "inline.fodp")
    with open(inline, "w", encoding="utf-8") as fh:
        fh.write(FODP % base64.b64encode(open(emf, "rb").read()).decode("ascii"))

    # The zipped counterpart, written by the reference itself so the two hold the same fill and
    # the same frame and differ only in where the bytes live.
    render(inline, out, profile)
    subprocess.run(
        ["soffice", f"-env:UserInstallation=file://{profile}", "--headless",
         "--convert-to", "odp", "--outdir", out, inline],
        check=False, capture_output=True, timeout=600)
    zipped = os.path.join(out, "inline.odp")
    with zipfile.ZipFile(zipped) as z:
        print("odp entries:", [n for n in z.namelist() if "Picture" in n or n.endswith(".emf")])
    shutil.move(zipped, os.path.join(out, "zipped.odp"))
    render(os.path.join(out, "zipped.odp"), out, profile)
    print(os.path.join(out, "inline.pdf"), os.path.join(out, "zipped.pdf"))
