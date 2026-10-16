#!/usr/bin/env python3
"""Where does 26.2.4.2 get `013`'s page-1 camera picture from?

Round 54 varied the *DrawingML* side of `013_Contextures_chart_sample` -- `editAs`,
`a:ext/@cx`, the anchor's `to` column -- and the reference did not move for any of them.
A null result from an instrument that cannot register the effect is not evidence of
absence, so this probe varies the **other** side: the worksheet's legacy VML drawing.

Reading of `oox`'s source that this is meant to confirm or refute on the binary:

  * `ContextHandler2Helper::prepareMceContext` (oox/source/core/contexthandler2.cxx)
    lists the MCE namespaces oox supports and `a14` is explicitly **not** on it
    (`// u"a14", // We do not currently support ...`). `013`'s whole `xdr:twoCellAnchor`
    sits inside `mc:Choice Requires="a14"` beside an **empty** `mc:Fallback`, so on this
    reading Calc reads *no* DrawingML anchor at all -- which is exactly why round 54's
    three variants were inert.
  * The picture then comes from `xl/drawings/vmlDrawing1.vml`, reached by the sheet's
    `legacyDrawing` relationship. `VmlDrawing::isShapeSupported` (sc) excludes only
    `XML_Note`, so a `Pict` is imported.
  * `ShapeBase::calcShapeRectangle` (oox/source/vml/vmlshape.cxx:509-516) prefers the
    `x:ClientData/x:Anchor` cell anchor over the shape's `style` width/height.

Each variant changes exactly one thing, and each has a *stated* expected direction, so a
"nothing moved" answer refutes rather than merely failing to confirm.
"""
import os, re, shutil, subprocess, sys, zipfile

SRC = "/c/sandbox/workdir/sample-files/sheets/chartset-012/xlsx/013_Contextures_chart_sample_21b98e22.xlsx"
CLI = "/c/sandbox/workdir/wt-sheets-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
WORK = "/c/sandbox/workdir/scratch-r55-sheets/vmlprobe"

ENV = dict(os.environ, SOURCE_DATE_EPOCH="1700000000", TZ="UTC")


def build(name, edits):
    """edits: {part-name: fn(str)->str}; a value of None deletes the part."""
    out = os.path.join(WORK, name + ".xlsx")
    zin = zipfile.ZipFile(SRC)
    zo = zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED)
    for it in zin.infolist():
        if it.filename in edits and edits[it.filename] is None:
            continue
        data = zin.read(it.filename)
        if it.filename in edits:
            data = edits[it.filename](data.decode("utf8")).encode("utf8")
        zo.writestr(it, data)
    zo.close()
    return out


VML = "xl/drawings/vmlDrawing1.vml"
RELS = "xl/worksheets/_rels/sheet1.xml.rels"
DRAW = "xl/drawings/drawing1.xml"

CASES = {
    # control: the corpus file, unedited.
    "base": ({}, "reference 133.8 / 534.3, ours 129.5 / 414.5 (round 54's figures)"),

    # If the picture is the VML shape, removing the sheet's legacyDrawing relationship
    # must make it disappear from the reference and leave ours untouched.
    "no-vml-rel": ({RELS: lambda s: re.sub(
        r'<Relationship Id="rId3"[^>]*vmlDrawing[^>]*/>', '', s)},
        "reference loses the picture entirely; ours unchanged"),

    # The VML client anchor's `to` column 6 -> 9. Round 54 moved the *DrawingML* `to`
    # column and nothing happened. If the VML anchor is the one that counts, this widens
    # the reference and leaves ours alone.
    "vml-tocol9": ({VML: lambda s: s.replace(
        "1, 0, 7, 0, 6, 6, 21, 1", "1, 0, 7, 0, 9, 6, 21, 1")},
        "reference wider; ours unchanged"),

    # The VML `style` width, which `calcShapeRectangle` should ignore in favour of the
    # client anchor. A *negative* control with a direction: if the style were what Calc
    # used, halving it would halve the reference's picture.
    "vml-halfstyle": ({VML: lambda s: s.replace("width:326.25pt;", "width:163pt;")},
        "reference unchanged (client anchor wins over style)"),

    # The client anchor removed altogether: now `calcShapeRectangle` must fall back to
    # the style, which is 326.25pt -- i.e. the reference should shrink to *our* current
    # width.
    "vml-noanchor": ({VML: lambda s: re.sub(
        r"<x:Anchor>.*?</x:Anchor>", "", s, flags=re.S)},
        "reference narrows to the 326.25pt style width"),

    # And the DrawingML side once more, with the a14 Choice *unwrapped* so that a reader
    # honouring MCE would now see the anchor. If oox ignores a14, this is the variant
    # that makes the reference notice the DrawingML anchor for the first time.
    "unwrap-a14": ({DRAW: lambda s: re.sub(
        r'<mc:AlternateContent[^>]*>\s*<mc:Choice[^>]*>', '', s).replace(
        "</mc:Choice><mc:Fallback/></mc:AlternateContent>", "")},
        "reference now reads the DrawingML anchor: picture drawn twice, or at 326.25pt"),
}


def measure(pdf):
    if not os.path.exists(pdf):
        return "NO-PDF"
    out = subprocess.run(["pdftotext", "-q", "-f", "1", "-l", "1", "-bbox", pdf, "-"],
                         capture_output=True, text=True).stdout
    def x(word):
        m = re.search(r'<word xMin="([\d.]+)"[^>]*>%s</word>' % word, out)
        return round(float(m.group(1)), 1) if m else None
    n = len(re.findall(r"<word ", out))
    return "1000@%s Jan@%s words=%d" % (x("1000"), x("Jan"), n)


def main():
    shutil.rmtree(WORK, ignore_errors=True)
    os.makedirs(WORK)
    print("%-16s %-34s %-34s %s" % ("case", "reference", "ours", "expected"))
    for name, (edits, expect) in CASES.items():
        path = build(name, edits)
        prof = os.path.join(WORK, "prof-" + name)
        subprocess.run(["soffice", "-env:UserInstallation=file://" + prof, "--headless",
                        "--convert-to", "pdf", "--outdir", os.path.join(WORK, "ref"), path],
                       capture_output=True, env=ENV)
        subprocess.run([CLI, "render", path, "--format", "pdf", "--outdir",
                        os.path.join(WORK, "ours")], capture_output=True, env=ENV)
        print("%-16s %-34s %-34s %s" % (
            name,
            measure(os.path.join(WORK, "ref", name + ".pdf")),
            measure(os.path.join(WORK, "ours", name + ".pdf")),
            expect))


main()
