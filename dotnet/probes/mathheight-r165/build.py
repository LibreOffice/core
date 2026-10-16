#!/usr/bin/env python3
"""One DOCX per OMML construct, so 26.2.4.2's own resolved `svg:height` can be read for each.

The parts of the real document are copied verbatim (styles, settings, fontTable, numbering,
theme) so the fixtures inherit the same OOXML compatibility defaults -- the trap named in
`office-corpus/SKILL.md`: a DOCX without `word/settings.xml` answers a different question.
"""
import os, re, sys, zipfile

SRC = "/home/user/sample-files/words/ceiling-001/docx/ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx"
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "fixtures")
os.makedirs(OUT, exist_ok=True)

with zipfile.ZipFile(SRC) as z:
    PARTS = {i.filename: z.read(i.filename) for i in z.infolist()}
doc = PARTS["word/document.xml"].decode("utf-8")
SECT = re.sub(r"<w:(?:header|footer)Reference[^>]*/>", "",
              re.search(r"<w:sectPr [^>]*>.*?</w:sectPr>", doc, re.S).group(0))

HDR = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
       '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'
       ' xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"'
       ' xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math"><w:body>')

M = "http://schemas.openxmlformats.org/officeDocument/2006/math"


def r(t, sz=None):
    rpr = ('<w:rPr><w:rFonts w:ascii="Cambria Math" w:hAnsi="Cambria Math"/>'
           + (('<w:sz w:val="%d"/>' % (sz * 2)) if sz else '') + '</w:rPr>')
    return '<m:r>%s<m:t>%s</m:t></m:r>' % (rpr, t)


def sub(e, s):      return '<m:sSub><m:e>%s</m:e><m:sub>%s</m:sub></m:sSub>' % (e, s)
def sup(e, s):      return '<m:sSup><m:e>%s</m:e><m:sup>%s</m:sup></m:sSup>' % (e, s)
def subsup(e, a, b):return '<m:sSubSup><m:e>%s</m:e><m:sub>%s</m:sub><m:sup>%s</m:sup></m:sSubSup>' % (e, a, b)
def pre(e, a, b):   return '<m:sPre><m:sub>%s</m:sub><m:sup>%s</m:sup><m:e>%s</m:e></m:sPre>' % (a, b, e)
def frac(n, d, ty=None):
    pr = ('<m:fPr><m:type m:val="%s"/></m:fPr>' % ty) if ty else ''
    return '<m:f>%s<m:num>%s</m:num><m:den>%s</m:den></m:f>' % (pr, n, d)
def rad(e, deg=None):
    return '<m:rad>%s<m:e>%s</m:e></m:rad>' % (
        ('<m:deg>%s</m:deg>' % deg) if deg else '<m:radPr><m:degHide m:val="1"/></m:radPr><m:deg/>', e)
def nary(chr_, s, sp, e, hide=''):
    pr = '<m:naryPr>' + (('<m:chr m:val="%s"/>' % chr_) if chr_ else '') + hide + '</m:naryPr>'
    return '<m:nary>%s<m:sub>%s</m:sub><m:sup>%s</m:sup><m:e>%s</m:e></m:nary>' % (pr, s, sp, e)
def delim(e, beg=None, end=None):
    pr = '<m:dPr>' + (('<m:begChr m:val="%s"/>' % beg) if beg else '') + \
         (('<m:endChr m:val="%s"/>' % end) if end else '') + '</m:dPr>'
    return '<m:d>%s<m:e>%s</m:e></m:d>' % (pr, e)
def func(n, e):     return '<m:func><m:fName>%s</m:fName><m:e>%s</m:e></m:func>' % (n, e)
def acc(e, ch=None):
    pr = ('<m:accPr><m:chr m:val="%s"/></m:accPr>' % ch) if ch else ''
    return '<m:acc>%s<m:e>%s</m:e></m:acc>' % (pr, e)
def bar(e, pos=None):
    pr = ('<m:barPr><m:pos m:val="%s"/></m:barPr>' % pos) if pos else ''
    return '<m:bar>%s<m:e>%s</m:e></m:bar>' % (pr, e)
def limlow(e, l):   return '<m:limLow><m:e>%s</m:e><m:lim>%s</m:lim></m:limLow>' % (e, l)
def limupp(e, l):   return '<m:limUpp><m:e>%s</m:e><m:lim>%s</m:lim></m:limUpp>' % (e, l)
def mat(rows):
    return '<m:m>%s</m:m>' % "".join(
        '<m:mr>%s</m:mr>' % "".join('<m:e>%s</m:e>' % c for c in row) for row in rows)
def grp(e, ch=None):
    pr = ('<m:groupChrPr><m:chr m:val="%s"/></m:groupChrPr>' % ch) if ch else ''
    return '<m:groupChr>%s<m:e>%s</m:e></m:groupChr>' % (pr, e)
def box(e):         return '<m:box><m:e>%s</m:e></m:box>' % e
def phant(e):       return '<m:phant><m:e>%s</m:e></m:phant>' % e
def eqarr(es):      return '<m:eqArr>%s</m:eqArr>' % "".join('<m:e>%s</m:e>' % e for e in es)


x, y, a, b, c, n1, n2, nn = r("x"), r("y"), r("a"), r("b"), r("c"), r("1"), r("2"), r("n")

SHAPES = {
    # --- leaves: the same single row however many characters, whatever they are
    "leaf-x":            x,
    "leaf-abc":          r("abc"),
    "leaf-digit":        r("2"),
    "leaf-caps":         r("WMTOM"),
    "leaf-paren":        r("(x)"),
    "leaf-rho":          r("ρ"),          # a Greek lower-case with a descender
    "leaf-beta":         r("β"),
    "leaf-plus":         r("a+b"),
    "leaf-two-runs":     x + y,

    # --- scripts
    "sub":               sub(x, n1),
    "sup":               sup(x, n2),
    "subsup":            subsup(x, n1, n2),
    "pre":               pre(x, n1, n2),
    "sub-deep":          sub(x, sub(y, n1)),
    "sup-deep":          sup(x, sup(y, n2)),
    "sub-of-frac":       sub(frac(a, b), n1),
    "sup-tall-base":     sup(frac(a, b), n2),
    "sub-tall-script":   sub(x, frac(a, b)),

    # --- radicals
    "rad":               rad(x),
    "rad-deg":           rad(x, r("3")),
    "rad-frac":          rad(frac(a, b)),
    "rad-rad":           rad(rad(x)),

    # --- fractions
    "frac":              frac(a, b),
    "frac-nest-num":     frac(frac(a, b), c),
    "frac-nest-den":     frac(a, frac(b, c)),
    "frac-nest-both":    frac(frac(a, b), frac(b, c)),
    "frac-nest3":        frac(frac(frac(a, b), c), c),
    "frac-scripts":      frac(sub(a, n1), sub(b, n2)),
    "frac-lin":          frac(a, b, "lin"),
    "frac-skw":          frac(a, b, "skw"),
    "frac-nobar":        frac(a, b, "noBar"),

    # --- n-ary
    "nary-sum":          nary("∑", r("i=1"), nn, sub(x, r("i"))),
    "nary-int":          nary(None, r("0"), r("∞"), r("f(t)dt")),
    "nary-nolim":        nary("∑", r("i=1"), nn, x, '<m:subHide m:val="1"/><m:supHide m:val="1"/>'),
    "nary-frac":         nary("∑", r("i"), nn, frac(a, b)),

    # --- delimiters
    "delim":             delim(x),
    "delim-frac":        delim(frac(a, b)),
    "delim-sq":          delim(x, "[", "]"),
    "delim-nest":        delim(r("a+") + delim(frac(b, c), "[", "]")),

    # --- functions, accents, bars, limits
    "func":              func(r("sin"), r("(2θ)")),
    "acc":               acc(x),
    "acc-tilde":         acc(x, "̃"),
    "acc-frac":          acc(frac(a, b)),
    "bar-top":           bar(x),
    "bar-bot":           bar(x, "bot"),
    "limlow":            limlow(r("lim"), r("x→0")),
    "limupp":            limupp(x, nn),
    "groupchr":          grp(r("abc"), "⏟"),

    # --- matrices, arrays, boxes
    "mat-1x2":           mat([[a, b]]),
    "mat-2x2":           mat([[a, b], [c, x]]),
    "mat-3x2":           mat([[a, b], [c, x], [y, n1]]),
    "mat-2x2-frac":      mat([[frac(a, b), b], [c, x]]),
    "eqarr-2":           eqarr([a, b]),
    "eqarr-3":           eqarr([a, b, c]),
    "box":               box(x),
    "phant":             phant(x),

    # --- mixed, and the size controls
    "mixed":             frac(rad(r("WMTOM")), sub(r("ρ"), r("0")) + r("S") + sub(r("w"), None) if False else
                              frac(rad(r("WMTOM")), r("ρ0Sw"))),
    "seq-frac-then-x":   frac(a, b) + x,
    "seq-x-then-frac":   x + frac(a, b),
    "sz8":               r("x", 8),
    "sz20":              r("x", 20),
    "sz20-sub":          sub(r("x", 20), r("1", 20)),
    "sz20-frac":         frac(r("a", 20), r("b", 20)),
}


def build(name, body, para=True):
    wrap = ('<m:oMathPara>%s</m:oMathPara>' % ('<m:oMath>%s</m:oMath>' % body)) if para \
           else ('<m:oMath>%s</m:oMath>' % body)
    p = '<w:p>%s</w:p>' % wrap
    xml = (HDR + '<w:p><w:r><w:t>before</w:t></w:r></w:p>' + p
           + '<w:p><w:r><w:t>after</w:t></w:r></w:p>' + SECT + "</w:body></w:document>")
    path = os.path.join(OUT, "%s.docx" % name)
    if os.path.exists(path):
        os.remove(path)
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zo:
        for nm, data in PARTS.items():
            if nm.startswith(("customXml/", "docProps/")):
                continue
            if nm.startswith("word/") and not re.match(
                    r"word/(styles|settings|fontTable|numbering|theme/[^/]+|webSettings)\.xml$", nm):
                continue
            if nm in ("[Content_Types].xml", "word/_rels/document.xml.rels"):
                continue
            zo.writestr(nm, data)
        rels = PARTS["word/_rels/document.xml.rels"].decode()
        kept = [t.group(0) for t in re.finditer(r"<Relationship [^>]*/>", rels)
                if re.match(r"(styles|settings|fontTable|numbering|theme/[^/]+|webSettings)\.xml$",
                            re.search(r'Target="([^"]*)"', t.group(0)).group(1))]
        zo.writestr("word/_rels/document.xml.rels",
                    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
                    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                    + "".join(kept) + "</Relationships>")
        ct = PARTS["[Content_Types].xml"].decode()
        ct = re.sub(r'<Override PartName="/(word/(header|footer|charts|drawings|embeddings|endnotes|footnotes)'
                    r'[^"]*|customXml[^"]*|docProps[^"]*)"[^>]*/>', "", ct)
        zo.writestr("[Content_Types].xml", ct)
        zo.writestr("word/document.xml", xml)


if __name__ == "__main__":
    for nm, bd in SHAPES.items():
        build(nm, bd)
    print("built", len(SHAPES), "fixtures in", OUT)
