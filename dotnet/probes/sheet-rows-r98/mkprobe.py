#!/usr/bin/env python3
"""Write a minimal flat-ODF spreadsheet exercising the conditional-font question.

Arguments: out.fods  default_pt  cell_pt  cond_pt(or '-')  cellface  defface
Row 1 = plain cell, row 2 = conditional cell (style:map), both same cell style
except the map.
"""
import sys
out,dpt,cpt,cond,cface,dface = sys.argv[1:7]
condprops = '' if cond=='-' else ' fo:font-size="%spt" style:font-size-asian="%spt" style:font-size-complex="%spt"'%(cond,cond,cond)
T = '''<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:calcext="urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0" office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
 <office:styles>
  <style:style style:name="Default" style:family="table-cell">
   <style:text-properties style:font-name="{dface}" fo:font-family="{dface}" fo:font-size="{dpt}pt" style:font-size-asian="{dpt}pt" style:font-size-complex="{dpt}pt"/>
  </style:style>
  <style:style style:name="CondSty" style:family="table-cell" style:parent-style-name="Default">
   <style:text-properties fo:color="#ff0000"{condprops}/>
  </style:style>
 </office:styles>
 <office:automatic-styles>
  <style:style style:name="co1" style:family="table-column">
   <style:table-column-properties style:column-width="2in"/>
  </style:style>
  <style:style style:name="ro1" style:family="table-row">
   <style:table-row-properties style:use-optimal-row-height="true"/>
  </style:style>
  <style:style style:name="cePlain" style:family="table-cell" style:parent-style-name="Default">
   <style:table-cell-properties fo:wrap-option="no-wrap"/>
   <style:text-properties style:font-name="{cface}" fo:font-family="{cface}" fo:font-size="{cpt}pt" style:font-size-asian="{cpt}pt" style:font-size-complex="{cpt}pt"/>
  </style:style>
  <style:style style:name="ceCond" style:family="table-cell" style:parent-style-name="Default">
   <style:table-cell-properties fo:wrap-option="no-wrap"/>
   <style:text-properties style:font-name="{cface}" fo:font-family="{cface}" fo:font-size="{cpt}pt" style:font-size-asian="{cpt}pt" style:font-size-complex="{cpt}pt"/>
   <style:map style:condition="cell-content()&lt;40" style:apply-style-name="CondSty" style:base-cell-address="Probe.A2"/>
  </style:style>
 </office:automatic-styles>
 <office:body>
  <office:spreadsheet>
   <table:table table:name="Probe">
    <table:table-column table:style-name="co1" table:number-columns-repeated="2"/>
    <table:table-row table:style-name="ro1"><table:table-cell table:style-name="cePlain" office:value-type="string"><text:p>E</text:p></table:table-cell></table:table-row>
    <table:table-row table:style-name="ro1"><table:table-cell table:style-name="ceCond" office:value-type="string"><text:p>E</text:p></table:table-cell></table:table-row>
   </table:table>
  </office:spreadsheet>
 </office:body>
</office:document>
'''.format(**locals())
open(out,'w',encoding='utf-8').write(T)
