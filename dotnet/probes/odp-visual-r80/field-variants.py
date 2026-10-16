#!/usr/bin/env python3
"""Author one-attribute .fodp variants that isolate what a `text:a` does to a slide's text.

A `text:a` inside a draw shape's text is imported as an EditEngine *field*
(`xmloff/source/text/txtparai.cxx`:1352-1370 picks XMLUrlFieldImportContext when the
cursor has no `HyperLinkURL` property, which is every Draw/Impress text), and a field
portion is broken and drawn by rules of its own.  Each variant below changes exactly
one thing so the rule can be read off the reference's own baselines.

Everything is authored in flat ODF so a variant is one string substitution, and every
`draw:frame` names a parent style, because an automatic graphic style with no parent
is a Draw shape rather than a Writer frame (`#i51726#`) -- the trap `dotnet/CLAUDE.md`
records against authored ODF probes.
"""
import os, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[1] if len(sys.argv) > 1 else "/home/user/r80work/fields"
FILS = sys.argv[2] if len(sys.argv) > 2 else "false"

NS = (
 'xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" '
 'xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" '
 'xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" '
 'xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" '
 'xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" '
 'xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" '
 'xmlns:presentation="urn:oasis:names:tc:opendocument:xmlns:presentation:1.0" '
 'xmlns:xlink="http://www.w3.org/1999/xlink" '
 'xmlns:loext="urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0"')

URL = ("https://www.example.org/working-with-us/hr-connect/organisational-development/"
       "career-and-development-planning-framework/leadership/")

def doc(boxes):
    """boxes: list of (name, y_cm, anchor, body_xml)."""
    frames = []
    for i, (name, y, anchor, body) in enumerate(boxes):
        frames.append(
            f'<style:style style:name="gr{i}" style:family="graphic" style:parent-style-name="standard">'
            f'<style:graphic-properties draw:fill="none" draw:stroke="none" '
            f'draw:auto-grow-height="false" draw:auto-grow-width="false" '
            f'draw:textarea-vertical-align="{anchor}" '
            f'fo:padding-top="0cm" fo:padding-bottom="0cm" fo:padding-left="0cm" fo:padding-right="0cm" '
            f'style:font-independent-line-spacing="true"/>'
            f'<style:paragraph-properties fo:margin-top="0cm" fo:margin-bottom="0cm" fo:line-height="100%"/>'
            f'<style:text-properties fo:font-size="16pt" style:font-name-asian="Liberation Sans" '
            f'fo:font-family="Liberation Sans"/></style:style>')
    styles = "".join(frames)
    pages = []
    for i, (name, y, anchor, body) in enumerate(boxes):
        pages.append(
            f'<draw:frame draw:style-name="gr{i}" draw:text-style-name="P0" draw:layer="layout" '
            f'svg:width="14cm" svg:height="5cm" svg:x="1cm" svg:y="{y}cm">'
            f'<draw:text-box>{body}</draw:text-box></draw:frame>')
    return (f'<?xml version="1.0" encoding="UTF-8"?>\n'
        f'<office:document {NS} office:version="1.3" '
        f'office:mimetype="application/vnd.oasis.opendocument.presentation">'
        f'<office:styles>'
        f'<style:style style:name="standard" style:family="graphic">'
        f'<style:graphic-properties draw:fill="none" draw:stroke="none"/>'
        f'<style:text-properties fo:font-size="16pt" fo:font-family="Liberation Sans"/></style:style>'
        f'<style:style style:name="P0" style:family="paragraph">'
        f'<style:paragraph-properties fo:margin-top="0cm" fo:margin-bottom="0cm" '
        f'style:font-independent-line-spacing="{FILS}"/>'
        f'<style:text-properties fo:font-size="16pt" fo:font-family="Liberation Sans"/></style:style>'
        f'</office:styles>'
        f'<office:automatic-styles>{styles}'
        f'<style:style style:name="PC" style:family="paragraph">'
        f'<style:paragraph-properties fo:text-align="center" fo:margin-top="0cm" fo:margin-bottom="0cm"/>'
        f'<style:text-properties fo:font-size="16pt" fo:font-family="Liberation Sans"/></style:style>'
        f'<style:style style:name="PR" style:family="paragraph">'
        f'<style:paragraph-properties fo:text-align="end" fo:margin-top="0cm" fo:margin-bottom="0cm"/>'
        f'<style:text-properties fo:font-size="16pt" fo:font-family="Liberation Sans"/></style:style>'
        f'<style:style style:name="T16" style:family="text">'
        f'<style:text-properties fo:font-size="16pt" fo:font-family="Liberation Sans"/></style:style>'
        f'</office:automatic-styles>'
        f'<office:body><office:presentation>'
        f'<draw:page draw:name="p1" draw:master-page-name="">{"".join(pages)}</draw:page>'
        f'</office:presentation></office:body></office:document>')

def p(inner):
    return f'<text:p text:style-name="P0"><text:span text:style-name="T16">{inner}</text:span></text:p>'

def pa(inner):
    """A paragraph whose content is a bare text:a -- no span wrapping it."""
    return f'<text:p text:style-name="P0">{inner}</text:p>'

LINK = f'<text:a xlink:href="{URL}" xlink:type="simple">{URL}</text:a>'

VARIANTS = {
    # 1. the same long URL as plain text and as a link, top anchored
    "v1-plain-top":  [("a", 1, "top", p("Alpha beta gamma.") + p(URL) + p("Omega."))],
    "v2-link-top":   [("a", 1, "top", p("Alpha beta gamma.") + pa(LINK) + p("Omega."))],
    # 2. a link that starts part way along a line
    "v3-link-inline":[("a", 1, "top", p("Alpha beta gamma.") + pa(f'<text:span text:style-name="T16">Have a look: </text:span>{LINK}') + p("Omega."))],
    # 3. the same, middle and bottom anchored: does the block height count the sub-lines?
    "v4-link-middle":[("a", 1, "middle", p("Alpha beta.") + pa(LINK))],
    "v5-plain-middle":[("a", 1, "middle", p("Alpha beta.") + p(URL))],
    "v6-link-bottom":[("a", 1, "bottom", p("Alpha beta.") + pa(LINK))],
    # 4. a short link that fits: does anything at all change?
    "v7-short-link": [("a", 1, "top", p("Alpha beta gamma.") + pa(f'<text:a xlink:href="{URL}" xlink:type="simple">short.example</text:a>') + p("Omega."))],
    # 5. a link inside a span that also carries a size, to see which size the field takes
    # 6. text after the field on the same paragraph, and a field whose first character cannot fit
    "v9-text-after": [("a", 1, "top", p("Alpha beta gamma.") + pa(f'{LINK}<text:span text:style-name="T16"> tail text follows.</text:span>') + p("Omega."))],
    "v10-no-room":   [("a", 1, "top", p("Alpha beta gamma.") + pa(f'<text:span text:style-name="T16">Alpha beta gamma delta epsilon zeta et. </text:span>{LINK}') + p("Omega."))],
    # 7. a centred and a right-aligned paragraph holding the same over-long field
    "v11-centre":    [("a", 1, "top", p("Alpha beta gamma.") + f'<text:p text:style-name="PC">{LINK}</text:p>' + p("Omega."))],
    "v12-right":     [("a", 1, "top", p("Alpha beta gamma.") + f'<text:p text:style-name="PR">{LINK}</text:p>' + p("Omega."))],
    "v13-centre-plain":[("a", 1, "top", p("Alpha beta gamma.") + f'<text:p text:style-name="PC"><text:span text:style-name="T16">{URL}</text:span></text:p>' + p("Omega."))],
    "v8-link-in-span":[("a", 1, "top", p("Alpha beta gamma.") + f'<text:p text:style-name="P0"><text:span text:style-name="T16">{LINK}</text:span></text:p>' + p("Omega."))],
}

os.makedirs(OUT, exist_ok=True)
for name, boxes in VARIANTS.items():
    path = os.path.join(OUT, name + ".fodp")
    open(path, "w", encoding="utf-8").write(doc(boxes))
    subprocess.run([os.path.join(HERE, "ref-render.sh"), path, OUT], check=False)

import pymupdf
for name in VARIANTS:
    pdf = os.path.join(OUT, name + ".pdf")
    if not os.path.exists(pdf):
        print(name, "NO OUTPUT"); continue
    d = pymupdf.open(pdf)
    print("==", name, d.page_count, "page(s)")
    rows = []
    for b in d[0].get_text("dict")["blocks"]:
        if b["type"]: continue
        for l in b["lines"]:
            t = "".join(s["text"] for s in l["spans"])
            if t.strip():
                rows.append((round(l["spans"][0]["origin"][1], 3),
                             round(l["spans"][0]["origin"][0], 3),
                             round(l["spans"][0]["size"], 2), t))
    rows.sort()
    prev = None
    for y, x, sz, t in rows:
        print("   %8.3f %8.3f %+7.3f %5.2f  %s" % (y, x, (y - prev) if prev else 0, sz, t[:60]))
        prev = y
