#!/usr/bin/env python3
"""Author the grid probe: which column's vertical rule does Calc split per row, and why.

`ScOutputData::DrawGrid` (`sc/source/ui/view/output.cxx:456-513`) takes a per-row branch for
column nX when **any** of three things holds — the next column has zero width, some row has
`cellInfo(nX+1).bHOverlapped` (a merge), or some row has `cellInfo(nX).bHideGrid` (a string
that overflowed across it). One sheet per candidate, one variable each, against a control.
"""
import sys

HEAD = """<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
<office:automatic-styles>
 <style:style style:name="co" style:family="table-column">
  <style:table-column-properties style:column-width="2cm"/></style:style>
 <style:style style:name="cohid" style:family="table-column">
  <style:table-column-properties style:column-width="2cm"/></style:style>
 <style:page-layout style:name="pm1"><style:page-layout-properties
  fo:margin-left="2cm" fo:margin-right="2cm" fo:margin-top="2cm" fo:margin-bottom="2cm"
  style:print="charts drawings grid objects zero-values"/>
 </style:page-layout>
 <style:style style:name="Default" style:family="table-cell"/>
</office:automatic-styles>
<office:master-styles>
 <style:master-page style:name="Default" style:page-layout-name="pm1"/>
</office:master-styles>
<office:body><office:spreadsheet>
"""
TAIL = "</office:spreadsheet></office:body></office:document>\n"

ROWS = 6


def cell(t=""):
    if t == "":
        return "<table:table-cell/>"
    return ('<table:table-cell office:value-type="string"><text:p>%s</text:p>'
            '</table:table-cell>' % t)


def sheet(name, columns, rows):
    return ('<table:table table:name="%s" table:style-name="ta1">%s%s</table:table>'
            % (name, columns, rows))


COL = '<table:table-column table:style-name="co"/>'
COLHID = '<table:table-column table:style-name="cohid" table:visibility="collapse"/>'

sheets = []

# control: five plain 2 cm columns, nothing else
rows = "".join("<table:table-row>%s</table:table-row>" % (cell("x") * 5) for _ in range(ROWS))
sheets.append(sheet("control", COL * 5, rows))

# variant A: column C hidden. Predicted trigger for the split on column B's right rule.
sheets.append(sheet("hidden-C", COL * 2 + COLHID + COL * 2, rows))

# variant B: a merge in the middle of column C, on row 3 only.
rows_b = []
for r in range(ROWS):
    if r == 2:
        rows_b.append("<table:table-row>%s%s%s</table:table-row>"
                      % (cell("x") * 2,
                         '<table:table-cell table:number-columns-spanned="2" '
                         'table:number-rows-spanned="1" office:value-type="string">'
                         '<text:p>m</text:p></table:table-cell>'
                         '<table:covered-table-cell/>',
                         cell("x")))
    else:
        rows_b.append("<table:table-row>%s</table:table-row>" % (cell("x") * 5))
sheets.append(sheet("merge-CD", COL * 5, "".join(rows_b)))

# variant C: a long string in column B on row 3 only, overflowing across C.
rows_c = []
for r in range(ROWS):
    if r == 2:
        rows_c.append("<table:table-row>%s%s%s</table:table-row>"
                      % (cell("x"), cell("OVERFLOWING" * 4), cell() + cell("x") * 2))
    else:
        rows_c.append("<table:table-row>%s</table:table-row>" % (cell("x") * 5))
sheets.append(sheet("overflow-B", COL * 5, "".join(rows_c)))

STYLE = ('<style:style style:name="ta1" style:family="table" '
         'style:master-page-name="Default">'
         '<style:table-properties table:display="true" style:writing-mode="lr-tb"/>'
         '</style:style>')

out = HEAD.replace("</office:automatic-styles>", STYLE + "</office:automatic-styles>") \
    + "".join(sheets) + TAIL
open(sys.argv[1], "w", encoding="utf8").write(out)
print("wrote", sys.argv[1])
