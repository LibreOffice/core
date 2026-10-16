#!/usr/bin/env python3
"""What sizes the line an empty ODF paragraph occupies, one variant at a time.

The subtitle placeholder of `0335fab9`'s page 6 is replaced with a known three- or
four-paragraph sequence, everything else about the document left alone, and both renderers
read back.  Patching a real corpus document rather than authoring one keeps the placeholder's
own default size -- which is the thing the paragraph's cascade would wrongly supply -- in
play, and it is the difference between the two that the round is about.

  empty-paragraph-variants.py [outdir]

XRAY_OFFICE_CLI names the binary to measure; REF_SOFFICE the reference.
"""
import os, re, subprocess, sys, zipfile

SRC = "/home/user/corpus-odf/slides/done-004/odp/0335fab9-79f0-4944-b92c-f223837ca2d8.odp"
OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/odp-empty-variants"
PAGE = 5
HERE = os.path.dirname(os.path.abspath(__file__))
CLI = os.environ.get(
    "XRAY_OFFICE_CLI",
    "/home/user/wt-odpvis/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli")

# Two extra text styles, so that a variant can state a size the placeholder does not.
EXTRA = ('<style:style style:name="TBIG" style:family="text">'
         '<style:text-properties fo:font-size="40pt"/></style:style>'
         '<style:style style:name="TSML" style:family="text">'
         '<style:text-properties fo:font-size="8pt"/></style:style>')

def para(text, style="T15"):
    return ('<text:p text:style-name="P20">'
            f'<text:span text:style-name="{style}">{text}</text:span></text:p>')

def empty(style="T15"):
    return f'<text:p text:style-name="P20"><text:span text:style-name="{style}"/></text:p>'

CASES = {
    # The two ends of the question: no empty paragraph at all, and one holding a 16pt span.
    "e0-none":            [para("Alpha one"), para("Beta two")],
    "e1-one-16pt":        [para("Alpha one"), empty(), para("Beta two")],
    # The control that hides the defect: no span, so the paragraph's own default is right.
    "e2-bare":            [para("Alpha one"), '<text:p text:style-name="P20"/>', para("Beta two")],
    "e3-two-16pt":        [para("Alpha one"), empty(), empty(), para("Beta two")],
    # The span's own size decides, in both directions from the placeholder's 32pt.
    "f1-empty-40pt":      [para("Alpha one"), empty("TBIG"), para("Beta two")],
    "f2-empty-8pt":       [para("Alpha one"), empty("TSML"), para("Beta two")],
    # ... and not the previous paragraph's, which f4 separates from it.
    "f4-prev-40pt":       [para("Alpha one", "TBIG"), empty(), para("Beta two")],
    # Which span, when there are several: the last ENTERED, not the largest and not the outermost.
    "f3-small-then-big":  [para("Alpha one"),
                           '<text:p text:style-name="P20"><text:span text:style-name="TSML"/>'
                           '<text:span text:style-name="TBIG"/></text:p>', para("Beta two")],
    "g1-big-then-small":  [para("Alpha one"),
                           '<text:p text:style-name="P20"><text:span text:style-name="TBIG"/>'
                           '<text:span text:style-name="TSML"/></text:p>', para("Beta two")],
    "g2-nested":          [para("Alpha one"),
                           '<text:p text:style-name="P20"><text:span text:style-name="TBIG">'
                           '<text:span text:style-name="TSML"/></text:span></text:p>',
                           para("Beta two")],
}

def build():
    with zipfile.ZipFile(SRC) as zin:
        entries = [(item, zin.read(item.filename)) for item in zin.infolist()]

    content = dict((i.filename, d) for i, d in entries)["content.xml"].decode("utf-8")
    content = content.replace("</office:automatic-styles>", EXTRA + "</office:automatic-styles>")
    page = re.findall(r"<draw:page\b.*?</draw:page>", content, re.S)[PAGE]
    body = re.findall(r"<draw:frame\b.*?</draw:frame>", page, re.S)[1]
    box = re.search(r"<draw:text-box>.*</draw:text-box>", body, re.S)
    return entries, content, page, body, box

def main():
    os.makedirs(os.path.join(OUT, "ours"), exist_ok=True)
    entries, content, page, body, box = build()

    for name, paragraphs in CASES.items():
        replaced = (body[:box.start()]
                    + '<draw:text-box><text:list text:style-name="L1"><text:list-header>'
                    + "".join(paragraphs)
                    + "</text:list-header></text:list></draw:text-box>"
                    + body[box.end():])
        patched = content.replace(page, page.replace(body, replaced))
        document = os.path.join(OUT, name + ".odp")

        with zipfile.ZipFile(document, "w", zipfile.ZIP_DEFLATED) as zout:
            for item, data in entries:
                if item.filename == "content.xml":
                    data = patched.encode("utf-8")
                if item.filename == "mimetype":
                    zout.writestr(zipfile.ZipInfo("mimetype"), data, zipfile.ZIP_STORED)
                else:
                    zout.writestr(item.filename, data)

        subprocess.run([os.path.join(HERE, "ref-render.sh"), document, OUT], check=False)
        subprocess.run([CLI, "render", document, "--format", "pdf",
                        "--outdir", os.path.join(OUT, "ours")], capture_output=True, check=False)

    import pymupdf
    print("%-20s %-24s %-24s" % ("variant", "26.2.4.2", "ours"))
    agree = 0
    for name in CASES:
        sides = []
        for path in (os.path.join(OUT, name + ".pdf"), os.path.join(OUT, "ours", name + ".pdf")):
            if not os.path.exists(path):
                sides.append(None)
                continue
            baselines = sorted(
                line["spans"][0]["origin"][1]
                for block in pymupdf.open(path)[PAGE].get_text("dict")["blocks"]
                if block["type"] == 0
                for line in block["lines"]
                if "".join(s["text"] for s in line["spans"]).strip())
            sides.append([round(y, 2) for y in baselines])
        agree += sides[0] is not None and sides[0] == sides[1]
        print("%-20s %-24s %-24s %s" % (name, sides[0], sides[1],
                                        "" if sides[0] == sides[1] else "DIFFER"))
    print("%d of %d agree" % (agree, len(CASES)))

if __name__ == "__main__":
    main()
