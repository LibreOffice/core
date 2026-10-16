#!/usr/bin/env python3
"""Author the ### probe: one variable at a time against a control, at two or more widths.

Writes a flat ODS. Every sheet is a strip of one-cell columns whose widths step in small
increments, so a single render of the file by 26.2.4.2 gives the whole width sweep for one
variable.  Nothing here is measured; the measurement is reading the rendered PDF back.
"""
import sys

HEAD = """<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0"
 xmlns:of="urn:oasis:names:tc:opendocument:xmlns:of:1.2"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
<office:font-face-decls>
 <style:font-face style:name="Liberation Sans" svg:font-family="Liberation Sans"
  xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"/>
</office:font-face-decls>
<office:automatic-styles>
"""

TAIL = """</office:automatic-styles>
<office:body><office:spreadsheet>
%s
</office:spreadsheet></office:body></office:document>
"""


class Doc:
    def __init__(self):
        self.styles = []
        self.sheets = []
        self.n = 0

    def colstyle(self, cm):
        self.n += 1
        name = "co%d" % self.n
        self.styles.append(
            '<style:style style:name="%s" style:family="table-column">'
            '<style:table-column-properties style:column-width="%.4fcm"/></style:style>' % (name, cm))
        return name

    def numstyle(self, body):
        self.n += 1
        name = "N%d" % self.n
        self.styles.append(body % name)
        return name

    def cellstyle(self, datastyle=None, extra=""):
        self.n += 1
        name = "ce%d" % self.n
        ds = ' style:data-style-name="%s"' % datastyle if datastyle else ""
        self.styles.append(
            '<style:style style:name="%s" style:family="table-cell" '
            'style:parent-style-name="Default"%s>%s</style:style>' % (name, ds, extra))
        return name


d = Doc()

# ---- data styles, one per number format under test -------------------------------------
fixed2 = d.numstyle('<number:number-style style:name="%s">'
                    '<number:number number:decimal-places="2" number:min-integer-digits="1"/>'
                    '</number:number-style>')
int0 = d.numstyle('<number:number-style style:name="%s">'
                  '<number:number number:decimal-places="0" number:min-integer-digits="1"/>'
                  '</number:number-style>')
datef = d.numstyle('<number:date-style style:name="%s">'
                   '<number:day number:style="long"/><number:text>/</number:text>'
                   '<number:month number:style="long"/><number:text>/</number:text>'
                   '<number:year number:style="long"/></number:date-style>')
pct = d.numstyle('<number:percentage-style style:name="%s">'
                 '<number:number number:decimal-places="2" number:min-integer-digits="1"/>'
                 '<number:text>%%</number:text></number:percentage-style>')

CE_GENERAL = d.cellstyle()
CE_FIXED2 = d.cellstyle(fixed2)
CE_INT0 = d.cellstyle(int0)
CE_DATE = d.cellstyle(datef)
CE_PCT = d.cellstyle(pct)
CE_SHRINK = d.cellstyle(None, '<style:table-cell-properties style:shrink-to-fit="true"/>')
CE_WRAP = d.cellstyle(None, '<style:table-cell-properties fo:wrap-option="wrap"/>')
CE_WRAPDATE = d.cellstyle(datef, '<style:table-cell-properties fo:wrap-option="wrap"/>')
CE_LEFT = d.cellstyle(None, '<style:table-cell-properties/>'
                            '<style:paragraph-properties fo:text-align="start"/>')

# ---- the width sweeps ------------------------------------------------------------------
# Each row is one variant; each column one width. Widths in cm.
WIDTHS = [0.10, 0.15, 0.20, 0.25, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80,
          0.90, 1.00, 1.20, 1.40, 1.60, 1.80, 2.00, 2.50, 3.00, 4.00]


def numcell(style, value, shown, kind="float"):
    """`shown` is the display text the authoring application would have cached.

    It has to be present: a reader that takes the cached text is measuring the cached text,
    and an empty <text:p/> makes the cell invisible on that reader while LibreOffice
    recomputes it from office:value. That is a property of the fixture, not of either
    renderer, and leaving it out voided the first run of this probe.
    """
    if kind == "date":
        return ('<table:table-cell table:style-name="%s" office:value-type="date" '
                'office:date-value="%s"><text:p>%s</text:p></table:table-cell>'
                % (style, value, shown))
    return ('<table:table-cell table:style-name="%s" office:value-type="float" '
            'office:value="%s"><text:p>%s</text:p></table:table-cell>' % (style, value, shown))


def strcell(style, text):
    return ('<table:table-cell table:style-name="%s" office:value-type="string">'
            '<text:p>%s</text:p></table:table-cell>' % (style, text))


def empty(n=1):
    return '<table:table-cell table:number-columns-repeated="%d"/>' % n if n > 1 \
        else '<table:table-cell/>'


# Each variant is (label, cellfactory). One row per variant; row 1 holds no header so that
# the extracted reading order is one variant per line.
VARIANTS = [
    ("general-1", lambda: numcell(CE_GENERAL, "1", "1")),
    ("general-12345", lambda: numcell(CE_GENERAL, "12345", "12345")),
    ("general-123456789012", lambda: numcell(CE_GENERAL, "123456789012", "123456789012")),
    ("general-1.5", lambda: numcell(CE_GENERAL, "1.5", "1.5")),
    ("general-neg1", lambda: numcell(CE_GENERAL, "-1", "-1")),
    ("fixed2-1", lambda: numcell(CE_FIXED2, "1", "1.00")),
    ("int0-1", lambda: numcell(CE_INT0, "1", "1")),
    ("pct-0.5", lambda: numcell(CE_PCT, "0.5", "50.00%")),
    ("date-2022-02-28", lambda: numcell(CE_DATE, "2022-02-28", "28/02/2022", kind="date")),
    ("string-XX", lambda: strcell(CE_GENERAL, "XX")),
    ("shrink-general-12345", lambda: numcell(CE_SHRINK, "12345", "12345")),
    ("wrap-general-12345", lambda: numcell(CE_WRAP, "12345", "12345")),
    ("wrapdate", lambda: numcell(CE_WRAPDATE, "2022-02-28", "28/02/2022", kind="date")),
    ("left-general-12345", lambda: numcell(CE_LEFT, "12345", "12345")),
]

cols = "".join('<table:table-column table:style-name="%s"/>' % d.colstyle(w) for w in WIDTHS)

rows = []
for label, make in VARIANTS:
    # column 0 is a wide label column? No — keep the grid pure: the label is a separate sheet.
    rows.append("<table:table-row>%s</table:table-row>"
                % "".join(make() for _ in WIDTHS))

sheet1 = ('<table:table table:name="sweep">%s%s</table:table>'
          % (cols, "".join(rows)))

# ---- sheet 2: the value-vs-string asymmetry, with an occupied neighbour -----------------
w_narrow = d.colstyle(0.30)
w_wide = d.colstyle(3.00)
rows2 = [
    # a value beside an empty cell: still clipped, because a value never spills
    "<table:table-row>%s%s</table:table-row>" % (numcell(CE_GENERAL, "12345", "12345"), empty()),
    # a string beside an empty cell: spills, no hash
    "<table:table-row>%s%s</table:table-row>" % (strcell(CE_GENERAL, "ABCDEFGH"), empty()),
    # a string beside an occupied cell: clipped, still no hash
    "<table:table-row>%s%s</table:table-row>" % (strcell(CE_GENERAL, "ABCDEFGH"),
                                                 strcell(CE_GENERAL, "Z")),
    # a fixed-format value beside an empty cell
    "<table:table-row>%s%s</table:table-row>" % (numcell(CE_FIXED2, "12345", "12345.00"), empty()),
]
sheet2 = ('<table:table table:name="spill">'
          '<table:table-column table:style-name="%s"/>'
          '<table:table-column table:style-name="%s"/>%s</table:table>'
          % (w_narrow, w_wide, "".join(rows2)))

open(sys.argv[1], "w", encoding="utf8").write(
    HEAD + "".join(d.styles) + TAIL % (sheet1 + sheet2))
print("wrote", sys.argv[1], "widths:", WIDTHS)
