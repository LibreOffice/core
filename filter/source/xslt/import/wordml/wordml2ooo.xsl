<?xml version="1.0" encoding="UTF-8"?>
<!--
   The Contents of this file are made available subject to the terms of
   either of the following licenses

          - GNU Lesser General Public License Version 2.1
          - Sun Industry Standards Source License Version 1.1

   Sun Microsystems Inc., October, 2000

   GNU Lesser General Public License Version 2.1
   =============================================
   Copyright 2000 by Sun Microsystems, Inc.
   901 San Antonio Road, Palo Alto, CA 94303, USA

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2.1, as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
   MA  02111-1307  USA


   Sun Industry Standards Source License Version 1.1
   =================================================
   The contents of this file are subject to the Sun Industry Standards
   Source License Version 1.1 (the "License"); You may not use this file
   except in compliance with the License. You may obtain a copy of the
   License at http://www.openoffice.org/license.html.

   Software provided under this License is provided on an "AS IS" basis,
   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
   See the License for the specific provisions governing your rights and
   obligations concerning the Software.

   The Initial Developer of the Original Code is: Sun Microsystems, Inc.

   Copyright © 2002 by Sun Microsystems, Inc.

   All Rights Reserved.

   Contributor(s): _______________________________________
   
 -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:wx="http://schemas.microsoft.com/office/word/2003/auxHint" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:w10="urn:schemas-microsoft-com:office:word" xmlns:SL="http://schemas.microsoft.com/schemaLibrary/2003/core" xmlns:aml="http://schemas.microsoft.com/aml/2001/core" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:dt="uuid:C2F41010-65B3-11d1-A29F-00AA00C14882" xmlns:office="http://openoffice.org/2000/office" xmlns:table="http://openoffice.org/2000/table" xmlns:style="http://openoffice.org/2000/style" xmlns:text="http://openoffice.org/2000/text" xmlns:draw="http://openoffice.org/2000/drawing" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:number="http://openoffice.org/2000/datastyle" xmlns:meta="http://openoffice.org/2000/meta" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:svg="http://www.w3.org/2000/svg" xmlns:chart="http://openoffice.org/2000/chart" xmlns:dr3d="http://openoffice.org/2000/dr3d" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="http://openoffice.org/2000/form" xmlns:script="http://openoffice.org/2000/script" xmlns:config="http://openoffice.org/2001/config" exclude-result-prefixes="w wx v w10 SL aml o dt" office:version="1.0">
    <xsl:output method="xml" indent="no" encoding="UTF-8" version="1.0"/>
    <xsl:template match="w:wordDocument">
        <office:document office:class="text" office:version="1.0">
            <xsl:apply-templates select="o:DocumentProperties"/>
            <xsl:apply-templates select="w:docPr"/>
            <xsl:apply-templates select="w:fonts"/>
            <xsl:apply-templates select="w:styles"/>
            <xsl:apply-templates select="w:body"/>
        </office:document>
    </xsl:template>
    <xsl:template match="o:DocumentProperties">
        <office:meta>
            <meta:generator>Microsoft Word 2003</meta:generator>
            <dc:title>
                <xsl:value-of select="o:Title"/>
            </dc:title>
            <dc:description>
                <xsl:value-of select="o:Description"/>
            </dc:description>
            <dc:subject>
                <xsl:value-of select="o:Subject"/>
            </dc:subject>
            <meta:initial-creator>
                <xsl:value-of select="o:Author"/>
            </meta:initial-creator>
            <meta:creation-date>
                <xsl:value-of select="substring-before( o:Created, 'Z')"/>
            </meta:creation-date>
            <dc:creator>
                <xsl:value-of select="o:LastAuthor"/>
            </dc:creator>
            <dc:date>
                <xsl:value-of select="substring-before( o:LastSaved, 'Z')"/>
            </dc:date>
            <meta:printed-by/>
            <meta:print-date>
                <xsl:value-of select="substring-before( o:LastPrinted, 'Z')"/>
            </meta:print-date>
            <meta:keywords>
                <meta:keyword>
                    <xsl:value-of select="o:Keywords"/>
                </meta:keyword>
            </meta:keywords>
            <dc:language/>
            <meta:editing-cycles>
                <xsl:value-of select="o:Revision"/>
            </meta:editing-cycles>
            <meta:editing-duration>
                <xsl:if test="o:TotalTime">
                    <xsl:value-of select="concat('PT', floor(o:TotalTime div 60), 'H', o:TotalTime mod 60, 'M0S')"/>
                </xsl:if>
            </meta:editing-duration>
            <meta:user-defined meta:name="Category">
                <xsl:value-of select="o:Category"/>
            </meta:user-defined>
            <meta:user-defined meta:name="Manager">
                <xsl:value-of select="o:Manager"/>
            </meta:user-defined>
            <meta:user-defined meta:name="Company">
                <xsl:value-of select="o:Company"/>
            </meta:user-defined>
            <meta:user-defined meta:name="Version">
                <xsl:value-of select="o:Version"/>
            </meta:user-defined>
            <meta:user-defined meta:name="HyperlinkBase">
                <xsl:value-of select="o:HyperlinkBase"/>
            </meta:user-defined>
            <xsl:apply-templates select="../o:CustomDocumentProperties"/>
            <meta:document-statistic meta:page-count="{o:Pages}" meta:paragraph-count="{o:Paragraphs}" meta:word-count="{o:Words}" meta:character-count="{o:Characters}"/>
        </office:meta>
    </xsl:template>
    <xsl:template match="o:CustomDocumentProperties">
        <xsl:for-each select="node()[@dt:dt]">
            <meta:user-defined meta:name="{local-name()}">
                <xsl:value-of select="."/>
            </meta:user-defined>
        </xsl:for-each>
    </xsl:template>
    <xsl:template match="w:docPr">
        <office:settings>
            <config:config-item-set config:name="view-settings">
                <config:config-item config:name="InBrowseMode" config:type="boolean">
                    <xsl:choose>
                        <xsl:when test="w:view/@w:val = 'outline'">true</xsl:when>
                        <xsl:when test="w:view/@w:val = 'print'">false</xsl:when>
                        <!-- others: web, reading, normal, master-pages, none. glu -->
                        <xsl:otherwise>true</xsl:otherwise>
                    </xsl:choose>
                </config:config-item>
                <config:config-item-map-indexed config:name="Views">
                    <config:config-item-map-entry>
                        <xsl:if test="w:zoom">
                            <!-- VisibleRight and VisibleBottom are arbitrary positive numbers. ;) glu -->
                            <config:config-item config:name="VisibleRight" config:type="int">1</config:config-item>
                            <config:config-item config:name="VisibleBottom" config:type="int">1</config:config-item>
                            <xsl:choose>
                                <xsl:when test="w:zoom/@w:val = 'best-fit'">
                                    <config:config-item config:name="ZoomType" config:type="short">3</config:config-item>
                                </xsl:when>
                                <xsl:when test="w:zoom/@w:val = 'full-page'">
                                    <config:config-item config:name="ZoomType" config:type="short">2</config:config-item>
                                </xsl:when>
                                <xsl:when test="w:zoom/@w:val = 'text-fit'">
                                    <config:config-item config:name="ZoomType" config:type="short">1</config:config-item>
                                </xsl:when>
                                <xsl:otherwise>
                                    <config:config-item config:name="ZoomType" config:type="short">0</config:config-item>
                                </xsl:otherwise>
                            </xsl:choose>
                            <config:config-item config:name="ZoomFactor" config:type="short">
                                <xsl:value-of select="w:zoom/@w:percent"/>
                            </config:config-item>
                        </xsl:if>
                    </config:config-item-map-entry>
                </config:config-item-map-indexed>
            </config:config-item-set>
        </office:settings>
    </xsl:template>
    <xsl:template match="w:fonts">
        <xsl:element name="office:font-decls">
            <!-- MS Word's default font declaration, added for Writer automatically. glu -->
            <style:font-decl style:name="Arial" fo:font-family="Arial" style:font-family-generic="roman" style:font-pitch="variable"/>
            <style:font-decl style:name="Times New Roman" fo:font-family="'Times New Roman'" style:font-family-generic="roman" style:font-pitch="variable"/>
            <style:font-decl style:name="Symbol" fo:font-family="Symbol" style:font-family-generic="roman" style:font-pitch="variable" style:font-charset="x-symbol"/>
            <style:font-decl style:name="Courier New" fo:font-family="'Courier New'" style:font-family-generic="modern" style:font-pitch="fixed"/>
            <xsl:if test="not(w:font[@w:name='StarSymbol'])">
                <style:font-decl style:name="StarSymbol" fo:font-family="StarSymbol" style:font-charset="x-symbol"/>
            </xsl:if>
            <xsl:for-each select="w:font">
                <xsl:element name="style:font-decl">
                    <xsl:attribute name="style:name"><xsl:value-of select="@w:name"/></xsl:attribute>
                    <xsl:attribute name="fo:font-family"><xsl:value-of select="@w:name"/></xsl:attribute>
                    <!-- added by glu, for process special fonts e.g. Marlett, -->
                    <xsl:if test="w:charset/@w:val = '02'">
                        <xsl:attribute name="style:font-charset">x-symbol</xsl:attribute>
                    </xsl:if>
                    <xsl:if test="w:family">
                        <xsl:choose>
                            <xsl:when test="w:family/@w:val = 'Swiss'">
                                <xsl:attribute name="style:font-family-generic">swiss</xsl:attribute>
                            </xsl:when>
                            <xsl:when test="w:family/@w:val='Modern'">
                                <xsl:attribute name="style:font-family-generic">modern</xsl:attribute>
                            </xsl:when>
                            <xsl:when test="w:family/@w:val='Roman'">
                                <xsl:attribute name="style:font-family-generic">roman</xsl:attribute>
                            </xsl:when>
                            <xsl:when test="w:family/@w:val='Script'">
                                <xsl:attribute name="style:font-family-generic">script</xsl:attribute>
                            </xsl:when>
                            <xsl:when test="w:family/@w:val='Decorative'">
                                <xsl:attribute name="style:font-family-generic">decorative</xsl:attribute>
                            </xsl:when>
                            <xsl:when test="w:family/@w:val='System'">
                                <xsl:attribute name="style:font-family-generic">system</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="style:font-family-generic">system</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:if>
                    <xsl:if test="w:pitch">
                        <xsl:attribute name="style:font-pitch"><xsl:value-of select="w:pitch/@w:val"/></xsl:attribute>
                    </xsl:if>
                </xsl:element>
            </xsl:for-each>
        </xsl:element>
    </xsl:template>
    <xsl:template match="w:styles">
        <xsl:variable name="default_paragraph_style" select="w:style[@w:default = 'on' and @w:type = 'paragraph']"/>
        <xsl:variable name="default_character_style" select="w:style[@w:default = 'on' and @w:type = 'character']"/>
        <xsl:variable name="default_table_style" select="w:style[@w:default = 'on' and @w:type = 'table']"/>
        <office:styles>
            <!-- make default paragarph  and other styles. glu -->
            <xsl:if test="$default_paragraph_style">
                <style:default-style style:family="paragraph">
                    <xsl:element name="style:properties">
                        <xsl:choose>
                            <xsl:when test="/w:wordDocument/w:fonts/w:defaultFonts">
                                <xsl:attribute name="style:font-name"><xsl:value-of select="/w:wordDocument/w:fonts/w:defaultFonts/@w:ascii"/></xsl:attribute>
                                <xsl:attribute name="style:font-name-asian"><xsl:value-of select="/w:wordDocument/w:fonts/w:defaultFonts/@w:fareast"/></xsl:attribute>
                                <xsl:attribute name="style:font-name-complex"><xsl:value-of select="/w:wordDocument/w:fonts/w:defaultFonts/@w:cs"/></xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="style:font-name">Times New Roman</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                        <xsl:apply-templates select="$default_paragraph_style/w:rPr/w:sz"/>
                        <xsl:apply-templates select="$default_paragraph_style/w:rPr/w:sz-cs"/>
                        <!-- if not defined default font size in Word, make it out as 10pt. glu -->
                        <xsl:if test="not($default_paragraph_style/w:rPr/w:sz or w:rPr/w:sz-cs)">
                            <xsl:attribute name="fo:font-size">10pt</xsl:attribute>
                            <xsl:attribute name="fo:font-size-asian">10pt</xsl:attribute>
                            <xsl:attribute name="fo:font-size-complex">10pt</xsl:attribute>
                        </xsl:if>
                        <xsl:apply-templates select="$default_paragraph_style/w:rPr/w:lang"/>
                        <xsl:attribute name="style:tab-stop-distance"><xsl:value-of select="concat( number(/w:wordDocument/w:docPr/w:defaultTabStop/@w:val)div 1440 *2.54, 'cm')"/></xsl:attribute>
                    </xsl:element>
                </style:default-style>
            </xsl:if>
            <style:style style:name="ColumnBreakPara" style:family="paragraph">
                <style:properties fo:break-after="column"/>
            </style:style>
            <xsl:if test="$default_character_style">
                <style:default-style style:family="text"/>
            </xsl:if>
            <style:style style:name="Numbering Symbols" style:family="text"/>
            <style:style style:name="Bullet Symbols" style:family="text">
                <style:properties style:font-name="StarSymbol" fo:font-size="9pt" style:font-name-asian="StarSymbol" style:font-size-asian="9pt" style:font-name-complex="StarSymbol" style:font-size-complex="9pt"/>
            </style:style>
            <xsl:if test="$default_table_style">
                <style:default-style style:family="table">
                    <xsl:element name="style:properties">
                        <xsl:apply-templates select="$default_table_style/w:rPr/wx:font"/>
                    </xsl:element>
                </style:default-style>
            </xsl:if>
            <!-- StarWriter has no default style family 'list'. glu -->
            <xsl:apply-templates select="w:style"/>
            <!-- primitive list and table styles should be converted to here. glu -->
        </office:styles>
        <xsl:element name="office:automatic-styles">
            <style:style style:name="PageBreak" style:family="paragraph">
                <style:properties fo:break-before="page"/>
            </style:style>
            <xsl:for-each select="/w:wordDocument/w:body//w:p">
                <xsl:choose>
                    <xsl:when test="w:pPr">
                        <xsl:element name="style:style">
                            <xsl:attribute name="style:family">paragraph</xsl:attribute>
                            <xsl:attribute name="style:name">P<xsl:number from="/w:wordDocument/w:body" level="any" count="w:p[w:pPr]"/></xsl:attribute>
                            <xsl:if test="w:pPr/w:pStyle">
                                <xsl:attribute name="style:parent-style-name"><xsl:value-of select="w:pPr/w:pStyle/@w:val"/></xsl:attribute>
                            </xsl:if>
                            <!-- We don't want to change the page style if there are columns being created. The Columns will be handled by using in-line sections.-->
                            <xsl:if test="not(ancestor::wx:sect/descendant::w:sectPr/w:cols[@w:num &gt; 1])">
                                <xsl:variable name="section-property-number">
                                    <xsl:number count="wx:sect" from="/w:wordDocument/w:body" level="any"/>
                                </xsl:variable>
                                <xsl:choose>
                                    <xsl:when test="generate-id(ancestor::wx:sect/descendant::w:p[1]) = generate-id(.)">
                                        <xsl:attribute name="style:master-page-name"><xsl:value-of select="concat('Standard', $section-property-number)"/></xsl:attribute>
                                    </xsl:when>
                                    <xsl:when test="descendant::w:br[@w:type = 'page'] and following::w:sectPr">
                                        <xsl:attribute name="style:master-page-name"><xsl:value-of select="concat('Standard', $section-property-number)"/></xsl:attribute>
                                    </xsl:when>
                                </xsl:choose>
                            </xsl:if>
                            <xsl:element name="style:properties">
                                <xsl:apply-templates select="w:rPr/*"/>
                                <xsl:apply-templates select="w:pPr/*"/>
                                <xsl:call-template name="paragraph-properties"/>
                            </xsl:element>
                        </xsl:element>
                    </xsl:when>
                    <xsl:when test="generate-id(ancestor::wx:sect/descendant::w:p[1]) = generate-id(.)">
                        <xsl:element name="style:style">
                            <xsl:attribute name="style:family">paragraph</xsl:attribute>
                            <xsl:attribute name="style:name">P<xsl:number from="/w:wordDocument/w:body" level="any" count="w:pPr"/>_1</xsl:attribute>
                            <xsl:attribute name="style:master-page-name">Standard<xsl:number count="wx:sect" from="/w:wordDocument/w:body" level="any"/></xsl:attribute>
                        </xsl:element>
                    </xsl:when>
                </xsl:choose>
            </xsl:for-each>
            <xsl:for-each select="/w:wordDocument/w:body//w:rPr">
                <xsl:element name="style:style">
                    <xsl:attribute name="style:name">T<xsl:value-of select="count(preceding::w:rPr)"/></xsl:attribute>
                    <xsl:if test="w:rStyle">
                        <xsl:attribute name="style:parent-style-name"><xsl:value-of select="w:rStyle/@w:val"/></xsl:attribute>
                    </xsl:if>
                    <xsl:choose>
                        <xsl:when test="ancestor::w:pPr">
                            <xsl:attribute name="style:family">paragraph</xsl:attribute>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:attribute name="style:family">text</xsl:attribute>
                        </xsl:otherwise>
                    </xsl:choose>
                    <xsl:element name="style:properties">
                        <xsl:apply-templates select="*"/>
                    </xsl:element>
                </xsl:element>
            </xsl:for-each>
            <xsl:for-each select="/w:wordDocument/w:body//w:tblPr">
                <xsl:element name="style:style">
                    <xsl:attribute name="style:name">Table<xsl:number count="w:tbl" from="/w:wordDocument/w:body" level="any"/></xsl:attribute>
                    <xsl:attribute name="style:family">table</xsl:attribute>
                    <xsl:if test="w:tblStyle">
                        <xsl:attribute name="style:parent-style-name"><xsl:value-of select="w:tblStyle/@w:val"/></xsl:attribute>
                    </xsl:if>
                    <xsl:element name="style:properties">
                        <xsl:choose>
                            <xsl:when test="w:jc/@w:val = 'left' or w:jc/@w:val = 'center' or w:jc/@w:val = 'right'">
                                <xsl:attribute name="table:align"><xsl:value-of select="w:jc/@w:val"/></xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="table:align">margins</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                        <xsl:if test="w:bidiVisual">
                            <xsl:attribute name="style:writing-mode">rl-tb</xsl:attribute>
                        </xsl:if>
                        <!-- Setting table margins using cell padding-->
                        <xsl:if test="w:tblStyle/@w:val">
                            <xsl:variable name="rootStyle" select="w:tblStyle/@w:val"/>
                            <xsl:variable name="rootStyleNode" select="/w:wordDocument/w:styles/w:style[@w:styleId = $rootStyle]"/>
                            <xsl:if test="$rootStyleNode/w:tblPr/w:tblCellMar/w:left">
                                <xsl:attribute name="fo:margin-left">-<xsl:value-of select="($rootStyleNode/w:tblPr/w:tblCellMar/w:left/@w:w div 1440) *2.54"/>cm</xsl:attribute>
                            </xsl:if>
                            <xsl:if test="$rootStyleNode/w:tblPr/w:tblCellMar/w:right">
                                <xsl:attribute name="fo:margin-right">-<xsl:value-of select="($rootStyleNode/w:tblPr/w:tblCellMar/w:right/@w:w div 1440) *2.54"/>cm</xsl:attribute>
                            </xsl:if>
                        </xsl:if>
                        <xsl:choose>
                            <xsl:when test="w:tblW/@w:type ='pct'">
                                <xsl:variable name="sectPr" select="ancestor::wx:sect//w:sectPr"/>
                                <xsl:variable name="pageSize" select="$sectPr/w:pgSz/@w:w"/>
                                <xsl:variable name="pageLeftMar" select="$sectPr/w:pgMar/@w:left"/>
                                <xsl:variable name="pageRightMar" select="$sectPr/w:pgMar/@w:right"/>
                                <xsl:attribute name="style:width"><xsl:value-of select="format-number( ( ($pageSize - $pageLeftMar - $pageRightMar  + (113 *2) ) div 1440) *2.54, '0.###cm')"/></xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:variable name="gridcols" select="../w:tblGrid/w:gridCol"/>
                                <xsl:variable name="tblsize" select="sum($gridcols/@w:w)"/>
                                <xsl:if test="not($tblsize = 0)">
                                    <xsl:choose>
                                        <xsl:when test="w:tblInd">
                                            <xsl:attribute name="style:width"><xsl:value-of select="format-number( (number($tblsize -  w:tblInd/@w:w) div 1440)*2.54, '0.###cm')"/></xsl:attribute>
                                        </xsl:when>
                                        <xsl:otherwise>
                                            <xsl:attribute name="style:width"><xsl:value-of select="format-number( (number($tblsize  ) div 1440)*2.54, '0.###cm')"/></xsl:attribute>
                                        </xsl:otherwise>
                                    </xsl:choose>
                                </xsl:if>
                            </xsl:otherwise>
                        </xsl:choose>
                        <!-- If previous w:p has a page break, the table must have the page break attribute applied to it	 May need this for tables starting on new pages-->
                        <!--	<xsl:if test="parent::w:tbl/preceding-sibling::w:p[1][descendant::w:br/@w:type='page']">
							<xsl:attribute name="fo:break-before">page</xsl:attribute></xsl:if>	-->
                        <!-- initial values for tables-->
                        <xsl:apply-templates/>
                    </xsl:element>
                </xsl:element>
            </xsl:for-each>
            <xsl:for-each select="/w:wordDocument/w:body//w:gridCol ">
                <xsl:element name="style:style">
                    <xsl:attribute name="style:family">table-column</xsl:attribute>
                    <xsl:attribute name="style:name">Table<xsl:number count="w:tbl" from="/w:wordDocument/w:body" level="any"/>.C<xsl:number count="w:gridCol" from="/w:wordDocument/w:body" level="single"/></xsl:attribute>
                    <xsl:element name="style:properties">
                        <xsl:choose>
                            <xsl:when test="../../w:tblPr/w:tblW/@w:type = 'pct'">
                                <xsl:variable name="sectPr" select="ancestor::wx:sect//w:sectPr"/>
                                <xsl:variable name="pageWidth" select="$sectPr/w:pgSz/@w:w"/>
                                <xsl:variable name="pageLeftMar" select="$sectPr/w:pgMar/@w:left"/>
                                <xsl:variable name="pageRightMar" select="$sectPr/w:pgMar/@w:right"/>
                                <xsl:variable name="pctTotal" select="../../w:tblPr/w:tblW/@w:w"/>
                                <xsl:variable name="currCol">
                                    <xsl:number count="w:gridCol" from="/w:wordDocument/w:body" level="single"/>
                                </xsl:variable>
                                <xsl:variable name="relSize" select="../../w:tr[1]/w:tc[$currCol]/w:tcPr/w:tcW/@w:w"/>
                                <xsl:attribute name="style:column-width"><xsl:value-of select="concat( (number(($relSize div $pctTotal) * ($pageWidth - ($pageLeftMar + $pageRightMar))) div 1440) *2.54, 'cm')"/></xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="style:column-width"><xsl:value-of select="concat( (number(@w:w) div 1440) *2.54, 'cm')"/></xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:element>
                </xsl:element>
            </xsl:for-each>
            <xsl:for-each select="/w:wordDocument/w:body//w:tcPr">
                <style:style>
                    <xsl:attribute name="style:name">Table<xsl:number count="w:tbl" from="/w:wordDocument/w:body" level="any"/>.R<xsl:number count="w:tr" from="/w:wordDocument/w:body" level="single"/>C<xsl:number count="w:tc" from="/w:wordDocument/w:body" level="single"/></xsl:attribute>
                    <xsl:attribute name="style:family">table-cell</xsl:attribute>
                    <xsl:variable name="rootStyle" select="ancestor::w:tbl/w:tblPr/w:tblStyle/@w:val"/>
                    <xsl:variable name="rootStyleNode" select="/w:wordDocument/w:styles/w:style[@w:styleId = $rootStyle]"/>
                    <xsl:element name="style:properties">
                        <xsl:if test="ancestor::w:tbl/w:tblPr/w:tblStyle/@w:val">
                            <xsl:if test="$rootStyleNode/w:tcPr/w:shd/@wx:bgcolor">
                                <xsl:attribute name="fo:background-color">#<xsl:value-of select="$rootStyleNode/w:tcPr/w:shd/@wx:bgcolor"/></xsl:attribute>
                            </xsl:if>
                            <xsl:if test="$rootStyleNode/w:tblPr/w:tblCellMar/w:left">
                                <xsl:attribute name="fo:padding-left"><xsl:value-of select="($rootStyleNode/w:tblPr/w:tblCellMar/w:left/@w:w div 1440) *2.54"/>cm</xsl:attribute>
                            </xsl:if>
                            <xsl:if test="$rootStyleNode/w:tblPr/w:tblCellMar/w:right">
                                <xsl:attribute name="fo:padding-right"><xsl:value-of select="($rootStyleNode/w:tblPr/w:tblCellMar/w:right/@w:w div 1440) *2.54"/>cm</xsl:attribute>
                            </xsl:if>
                            <xsl:if test="$rootStyleNode/w:tblPr/w:tblCellMar/w:top">
                                <xsl:attribute name="fo:padding-top"><xsl:value-of select="($rootStyleNode/w:tblPr/w:tblCellMar/w:top/@w:w div 1440) *2.54"/>cm</xsl:attribute>
                            </xsl:if>
                            <xsl:if test="$rootStyleNode/w:tblPr/w:tblCellMar/w:bottom">
                                <xsl:attribute name="fo:padding-bottom"><xsl:value-of select="($rootStyleNode/w:tblPr/w:tblCellMar/w:bottom/@w:w div 1440) *2.54"/>cm</xsl:attribute>
                            </xsl:if>
                        </xsl:if>
                        <xsl:variable name="row-position">
                            <xsl:number count="w:tr" from="/w:wordDocument/w:body" level="single"/>
                        </xsl:variable>
                        <!-- cell borders should be carefully converted. a little complex. glu :( -->
                        <xsl:variable name="Borders" select="ancestor::w:tbl/w:tblPr/w:tblBorders"/>
                        <xsl:choose>
                            <xsl:when test="$row-position &gt; 1">
                                <xsl:call-template name="get-table-border">
                                    <xsl:with-param name="style-pos" select="'top'"/>
                                    <xsl:with-param name="style-position-0" select="w:tcBorders/w:top"/>
                                    <xsl:with-param name="style-position-1" select="$Borders/w:insideH"/>
                                    <xsl:with-param name="style-position-2" select="$rootStyleNode/w:tblPr/w:tblBorders/w:insideH"/>
                                </xsl:call-template>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:call-template name="get-table-border">
                                    <xsl:with-param name="style-pos" select="'top'"/>
                                    <xsl:with-param name="style-position-0" select="w:tcBorders/w:top"/>
                                    <xsl:with-param name="style-position-1" select="$Borders/w:top"/>
                                    <xsl:with-param name="style-position-2" select="$rootStyleNode/w:tblPr/w:tblBorders/w:top"/>
                                </xsl:call-template>
                            </xsl:otherwise>
                        </xsl:choose>
                        <xsl:choose>
                            <xsl:when test="count(ancestor::w:tr/following-sibling::w:tr) &gt; 0">
                                <xsl:call-template name="get-table-border">
                                    <xsl:with-param name="style-pos" select="'bottom'"/>
                                    <xsl:with-param name="style-position-0" select="w:tcBorders/w:bottom"/>
                                    <xsl:with-param name="style-position-1" select="$Borders/w:insideH"/>
                                    <xsl:with-param name="style-position-2" select="$rootStyleNode/w:tblPr/w:tblBorders/w:insideH"/>
                                </xsl:call-template>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:call-template name="get-table-border">
                                    <xsl:with-param name="style-pos" select="'bottom'"/>
                                    <xsl:with-param name="style-position-0" select="w:tcBorders/w:bottom"/>
                                    <xsl:with-param name="style-position-1" select="$Borders/w:bottom"/>
                                    <xsl:with-param name="style-position-2" select="$rootStyleNode/w:tblPr/w:tblBorders/w:bottom"/>
                                </xsl:call-template>
                            </xsl:otherwise>
                        </xsl:choose>
                        <xsl:choose>
                            <xsl:when test="count(ancestor::w:tc/preceding-sibling::w:tc) &gt; 0">
                                <xsl:call-template name="get-table-border">
                                    <xsl:with-param name="style-pos" select="'left'"/>
                                    <xsl:with-param name="style-position-0" select="w:tcBorders/w:left"/>
                                    <xsl:with-param name="style-position-1" select="$Borders/w:insideV"/>
                                    <xsl:with-param name="style-position-2" select="$rootStyleNode/w:tblPr/w:tblBorders/w:insideV"/>
                                </xsl:call-template>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:call-template name="get-table-border">
                                    <xsl:with-param name="style-pos" select="'left'"/>
                                    <xsl:with-param name="style-position-0" select="w:tcBorders/w:left"/>
                                    <xsl:with-param name="style-position-1" select="$Borders/w:left"/>
                                    <xsl:with-param name="style-position-2" select="$rootStyleNode/w:tblPr/w:tblBorders/w:left"/>
                                </xsl:call-template>
                            </xsl:otherwise>
                        </xsl:choose>
                        <xsl:choose>
                            <xsl:when test="count(ancestor::w:tc/following-sibling::w:tc) &gt; 0">
                                <xsl:call-template name="get-table-border">
                                    <xsl:with-param name="style-pos" select="'right'"/>
                                    <xsl:with-param name="style-position-0" select="w:tcBorders/w:right"/>
                                    <xsl:with-param name="style-position-1" select="$Borders/w:insideV"/>
                                    <xsl:with-param name="style-position-2" select="$rootStyleNode/w:tblPr/w:tblBorders/w:insideV"/>
                                </xsl:call-template>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:call-template name="get-table-border">
                                    <xsl:with-param name="style-pos" select="'right'"/>
                                    <xsl:with-param name="style-position-0" select="w:tcBorders/w:right"/>
                                    <xsl:with-param name="style-position-1" select="$Borders/w:right"/>
                                    <xsl:with-param name="style-position-2" select="$rootStyleNode/w:tblPr/w:tblBorders/w:right"/>
                                </xsl:call-template>
                            </xsl:otherwise>
                        </xsl:choose>
                        <xsl:attribute name="fo:vertical-align">top</xsl:attribute>
                        <xsl:apply-templates/>
                    </xsl:element>
                </style:style>
            </xsl:for-each>
            <xsl:for-each select="/w:wordDocument/w:body//w:listPr">
                <xsl:variable name="currlistid" select="w:ilfo/@w:val"/>
                <xsl:variable name="currlist" select="."/>
                <xsl:variable name="rootlistid" select="/w:wordDocument/w:lists/w:list[@w:ilfo=$currlistid]/w:ilst/@w:val"/>
                <xsl:variable name="rootlist" select="/w:wordDocument/w:lists/w:listDef[@w:listDefId =$rootlistid ]"/>
                <xsl:if test="not(ancestor::w:p/preceding-sibling::w:p/w:pPr/w:listPr[1]/w:ilfo/@w:val= $currlistid) ">
                    <xsl:element name="text:list-style">
                        <xsl:attribute name="style:name">List<xsl:value-of select="count(preceding::w:listPr)"/></xsl:attribute>
                        <xsl:for-each select="$rootlist/w:lvl">
                            <xsl:variable name="listtype" select="w:nfc/@w:val"/>
                            <xsl:choose>
                                <xsl:when test="$listtype =23">
                                    <!-- nfcBullet: Bullet character. glu -->
                                    <xsl:element name="text:list-level-style-bullet">
                                        <xsl:call-template name="list-styles-common">
                                            <xsl:with-param name="listtype" select="$listtype"/>
                                            <xsl:with-param name="rootlist" select="$rootlist"/>
                                            <xsl:with-param name="currlist" select="$currlist"/>
                                        </xsl:call-template>
                                    </xsl:element>
                                </xsl:when>
                                <xsl:otherwise>
                                    <!-- all kinds of numbering characters. glu :( -->
                                    <xsl:element name="text:list-level-style-number">
                                        <xsl:call-template name="list-styles-common">
                                            <xsl:with-param name="listtype" select="$listtype"/>
                                            <xsl:with-param name="rootlist" select="$rootlist"/>
                                            <xsl:with-param name="currlist" select="$currlist"/>
                                        </xsl:call-template>
                                    </xsl:element>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:for-each>
                    </xsl:element>
                </xsl:if>
            </xsl:for-each>
            <xsl:for-each select="/w:wordDocument/w:body//w:sectPr">
                <xsl:element name="style:page-master">
                    <xsl:attribute name="style:name">pm<xsl:number from="/w:wordDocument/w:body" level="any"/></xsl:attribute>
                    <xsl:element name="style:properties">
                        <xsl:attribute name="fo:page-width"><xsl:value-of select="(number(w:pgSz/@w:w) div 1440)*2.54"/>cm</xsl:attribute>
                        <xsl:attribute name="fo:page-height"><xsl:value-of select="(number(w:pgSz/@w:h) div 1440)*2.54"/>cm</xsl:attribute>
                        <xsl:choose>
                            <xsl:when test="/w:wordDocument/w:docPr/w:gutterAtTop">
                                <xsl:attribute name="fo:margin-top"><xsl:value-of select="(number(w:pgMar/@w:top + w:pgMar/@w:gutter) div 1440)*2.54"/>cm</xsl:attribute>
                                <xsl:attribute name="fo:margin-left"><xsl:value-of select="(number(w:pgMar/@w:left) div 1440)*2.54"/>cm</xsl:attribute>
                                <xsl:attribute name="fo:margin-right"><xsl:value-of select="(number(w:pgMar/@w:right) div 1440)*2.54"/>cm</xsl:attribute>
                            </xsl:when>
                            <xsl:when test="w:rtlGutter">
                                <xsl:attribute name="fo:margin-top"><xsl:value-of select="(number(w:pgMar/@w:top) div 1440)*2.54"/>cm</xsl:attribute>
                                <xsl:attribute name="fo:margin-left"><xsl:value-of select="(number(w:pgMar/@w:left) div 1440)*2.54"/>cm</xsl:attribute>
                                <xsl:attribute name="fo:margin-right"><xsl:value-of select="(number(w:pgMar/@w:right + w:pgMar/@w:gutter) div 1440)*2.54"/>cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="fo:margin-top"><xsl:value-of select="(number(w:pgMar/@w:top) div 1440)*2.54"/>cm</xsl:attribute>
                                <xsl:attribute name="fo:margin-left"><xsl:value-of select="(number(w:pgMar/@w:left + w:pgMar/@w:gutter) div 1440)*2.54"/>cm</xsl:attribute>
                                <xsl:attribute name="fo:margin-right"><xsl:value-of select="(number(w:pgMar/@w:right) div 1440)*2.54"/>cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                        <xsl:attribute name="fo:margin-bottom"><xsl:value-of select="(number(w:pgMar/@w:bottom) div 1440)*2.54"/>cm</xsl:attribute>
                        <xsl:attribute name="style:footnote-max-height"><xsl:value-of select="(number(w:pgMar/@w:footer) div 1440)*2.54"/></xsl:attribute>
                        <xsl:if test="w:pgSz/@w:orient">
                            <xsl:attribute name="style:print-orientation"><xsl:value-of select="w:pgSz/@w:orient"/></xsl:attribute>
                        </xsl:if>
                    </xsl:element>
                </xsl:element>
                <xsl:if test="w:cols/@w:num">
                    <!-- create sction property-->
                    <xsl:element name="style:style">
                        <xsl:attribute name="style:name">sect<xsl:value-of select="count(preceding::w:sectPr[w:cols/@w:num])"/></xsl:attribute>
                        <xsl:attribute name="style:family">section</xsl:attribute>
                        <xsl:element name="style:properties">
                            <xsl:element name="style:columns">
                                <xsl:attribute name="fo:column-count"><xsl:value-of select="number(w:cols/@w:num)"/></xsl:attribute>
                                <xsl:attribute name="fo:column-gap"><xsl:value-of select="(number(w:cols/@w:space)div 1440)*2.54"/>cm</xsl:attribute>
                            </xsl:element>
                        </xsl:element>
                    </xsl:element>
                </xsl:if>
            </xsl:for-each>
        </xsl:element>
        <office:master-styles>
            <xsl:for-each select="/w:wordDocument/w:body//w:sectPr">
                <!-- style:page-master style:style-->
                <xsl:element name="style:master-page">
                    <xsl:variable name="master-page-name">
                        <xsl:number count="w:sectPr" from="/w:wordDocument/w:body" level="any"/>
                    </xsl:variable>
                    <xsl:attribute name="style:name"><xsl:value-of select="concat('Standard',$master-page-name)"/></xsl:attribute>
                    <xsl:attribute name="style:page-master-name"><xsl:value-of select="concat('pm', $master-page-name)"/></xsl:attribute>
                    <xsl:if test="following::w:sectPr">
                        <xsl:attribute name="style:next-style-name">Standard<xsl:value-of select="$master-page-name +1"/></xsl:attribute>
                    </xsl:if>
                    <!-- Headers and footers-->
                    <xsl:element name="style:header-style">
                        <xsl:element name="style:properties">
                            <xsl:attribute name="fo:min-height"><xsl:value-of select="(number(w:pgMar/@w:header)div 1440)*2.54"/>cm</xsl:attribute>
                            <xsl:attribute name="fo:margin-bottom">0.792cm</xsl:attribute>
                            <xsl:attribute name="style:dynamic-spacing">true</xsl:attribute>
                        </xsl:element>
                    </xsl:element>
                    <xsl:element name="style:footer-style">
                        <xsl:element name="style:properties">
                            <xsl:attribute name="fo:min-height"><xsl:value-of select="(number(w:pgMar/@w:footer)div 1440)*2.54"/>cm</xsl:attribute>
                            <xsl:attribute name="fo:margin-top">0.792cm</xsl:attribute>
                            <xsl:attribute name="style:dynamic-spacing">true</xsl:attribute>
                        </xsl:element>
                    </xsl:element>
                    <xsl:if test="w:hdr and not(w:titlePg)">
                        <xsl:element name="style:header">
                            <xsl:apply-templates select="w:hdr[@w:type='odd']/*"/>
                        </xsl:element>
                        <xsl:element name="style:header-left">
                            <xsl:apply-templates select="w:hdr[@w:type='even']/*"/>
                        </xsl:element>
                    </xsl:if>
                    <xsl:if test="w:ftr and not(w:titlePg)">
                        <xsl:element name="style:footer">
                            <xsl:apply-templates select="w:ftr[@w:type='odd']/*"/>
                        </xsl:element>
                        <xsl:element name="style:footer-left">
                            <xsl:apply-templates select="w:ftr[@w:type='even']/*"/>
                        </xsl:element>
                    </xsl:if>
                </xsl:element>
            </xsl:for-each>
        </office:master-styles>
    </xsl:template>
    <xsl:template match="w:style">
        <xsl:element name="style:style">
            <xsl:attribute name="style:name"><xsl:value-of select="@w:styleId"/></xsl:attribute>
            <xsl:if test="w:basedOn">
                <xsl:attribute name="style:parent-style-name"><xsl:value-of select="w:basedOn/@w:val"/></xsl:attribute>
            </xsl:if>
            <xsl:if test="w:next">
                <xsl:attribute name="style:next-style-name"><xsl:value-of select="w:next/@w:val"/></xsl:attribute>
            </xsl:if>
            <xsl:choose>
                <xsl:when test="@w:type = 'character' ">
                    <xsl:attribute name="style:family">text</xsl:attribute>
                </xsl:when>
                <xsl:when test="@w:type = 'list' ">
                    <xsl:attribute name="style:family">text</xsl:attribute>
                </xsl:when>
                <!-- table, paragraph are the same as in Writer . glu -->
                <xsl:when test="@w:type">
                    <xsl:attribute name="style:family"><xsl:value-of select="@w:type"/></xsl:attribute>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:attribute name="style:family">text</xsl:attribute>
                </xsl:otherwise>
            </xsl:choose>
            <xsl:element name="style:properties">
                <xsl:apply-templates select="w:rPr/*"/>
                <xsl:apply-templates select="w:pPr/*"/>
                <xsl:call-template name="paragraph-properties"/>
            </xsl:element>
        </xsl:element>
    </xsl:template>
    <xsl:template name="get-table-border">
        <xsl:param name="style-pos"/>
        <xsl:param name="style-position-0"/>
        <xsl:param name="style-position-1"/>
        <xsl:param name="style-position-2"/>
        <xsl:variable name="size-style">
            <xsl:choose>
                <xsl:when test="$style-position-0">
                    <xsl:value-of select="$style-position-0/@w:sz"/>
                </xsl:when>
                <xsl:when test="$style-position-1">
                    <xsl:value-of select="$style-position-1/@w:sz"/>
                </xsl:when>
                <xsl:when test="$style-position-2">
                    <xsl:value-of select="$style-position-2/@w:sz"/>
                </xsl:when>
                <xsl:otherwise>2</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="border-style">
            <xsl:choose>
                <xsl:when test="$style-position-0">
                    <xsl:value-of select="$style-position-0/@w:val"/>
                </xsl:when>
                <xsl:when test="$style-position-1">
                    <xsl:value-of select="$style-position-1/@w:val"/>
                </xsl:when>
                <xsl:when test="$style-position-2">
                    <xsl:value-of select="$style-position-2/@w:val"/>
                </xsl:when>
                <xsl:otherwise>single</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <!-- totally basic Word table border styles: nil, none, single, thick, double, dotted, dashed, dot-dash, dot-dot-dash, triple, thin-thick-small-gap, thick-thin-small-gap, thin-thick-thin-small-gap, thin-thick-medium-gap, thick-thin-medium-gap, thin-thick-thin-medium-gap, thin-thick-large-gap, thick-thin-large-gap, thin-thick-thin-large-gap, wave, double-wave, dash-small-gap, dash-dot-stroked, three-d-emboss, three-d-engrave, outset, inset. Others are advanced graphical table border styles. glu -->
        <xsl:variable name="style-border">
            <xsl:choose>
                <xsl:when test="$border-style = 'nil' or $border-style ='none'">hidden</xsl:when>
                <xsl:when test="$border-style = 'single' or $border-style = 'thick' or $border-style = 'dotted' or $border-style = 'dashed' or $border-style = 'dot-dash' or $border-style = 'dot-dot-dash' or $border-style = 'wave' or $border-style = 'dash-small-gap' or $border-style = 'dash-dot-stroked'">solid</xsl:when>
                <xsl:otherwise>double</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="color-border">
            <xsl:choose>
                <xsl:when test="$style-position-0 and string-length($style-position-0/@w:color) = 6">
                    <xsl:value-of select="$style-position-0/@w:color"/>
                </xsl:when>
                <xsl:when test="$style-position-0 and $style-position-0/@w:color = 'auto' and contains($border-style, 'set')">
                    <xsl:text>c0c0c0</xsl:text>
                </xsl:when>
                <xsl:when test="$style-position-1 and string-length($style-position-1/@w:color) = 6">
                    <xsl:value-of select="$style-position-1/@w:color"/>
                </xsl:when>
                <xsl:when test="$style-position-1 and $style-position-1/@w:color = 'auto' and contains($border-style, 'set')">
                    <xsl:text>c0c0c0</xsl:text>
                </xsl:when>
                <xsl:when test="$style-position-2 and string-length($style-position-2/@w:color) = 6">
                    <xsl:value-of select="$style-position-2/@w:color"/>
                </xsl:when>
                <xsl:when test="$style-position-2 and $style-position-2/@w:color = 'auto' and contains($border-style, 'set')">
                    <xsl:text>c0c0c0</xsl:text>
                </xsl:when>
                <xsl:otherwise>000000</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <!-- mapping border line widths. glu -->
        <xsl:choose>
            <xsl:when test="$border-style = 'single'">
                <xsl:choose>
                    <xsl:when test="$size-style &lt; 7">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.002cm solid #', $color-border)"/></xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 20">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.035cm solid #', $color-border)"/></xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 30">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.088cm solid #', $color-border)"/></xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 40">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.141cm solid #', $color-border)"/></xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.176cm solid #', $color-border)"/></xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:when test="$border-style = 'double'">
                <xsl:choose>
                    <xsl:when test="$size-style &lt; 10">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.039cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.035cm 0.002cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 15">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.092cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.002cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 20">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.106cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.035cm 0.035cm 0.035cm</xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.265cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.088cm 0.088cm</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:when test="$border-style = 'triple'">
                <xsl:choose>
                    <xsl:when test="$size-style &lt; 5">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.039cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.035cm 0.002cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 10">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.092cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">.002cm 0.088cm 0.002cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 15">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.106cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.035cm 0.035cm 0.035cm</xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.265cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.088cm 0.088cm</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:when test="$border-style = 'thin-thick-small-gap' or $border-style = 'thick-thin-small-gap'">
                <xsl:choose>
                    <xsl:when test="($border-style = 'thin-thick-small-gap' and ($style-pos = 'left' or $style-pos = 'top')) or ($border-style = 'thick-thin-small-gap' and ($style-pos = 'right' or $style-pos = 'bottom'))">
                        <xsl:choose>
                            <xsl:when test="$size-style &lt; 20">
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.125cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.035cm</xsl:attribute>
                            </xsl:when>
                            <xsl:when test="$size-style &lt; 30">
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.178cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.088cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.231cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.141cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.159cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.035cm 0.035cm</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:when test="$border-style = 'thin-thick-thin-small-gap'">
                <xsl:choose>
                    <xsl:when test="$size-style &lt; 20">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.178cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.088cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 40">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.231cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.141cm</xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.318cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.088cm 0.141cm</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:when test="$border-style = 'thin-thick-medium-gap' or $border-style = 'thick-thin-medium-gap'">
                <xsl:choose>
                    <xsl:when test="$size-style &lt; 10">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.039cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.035cm 0.002cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 15">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.106cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.035cm 0.035cm 0.035cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 30">
                        <xsl:choose>
                            <xsl:when test="($border-style = 'thin-thick-medium-gap' and ($style-pos = 'left' or $style-pos = 'top')) or ($border-style = 'thick-thin-medium-gap' and ($style-pos = 'right' or $style-pos = 'bottom'))">
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.212cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.035cm 0.088cm 0.088cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.159cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.035cm 0.035cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.318cm double #', $color-border)"/></xsl:attribute>
                        <xsl:choose>
                            <xsl:when test="($border-style = 'thin-thick-medium-gap' and ($style-pos = 'left' or $style-pos = 'top')) or ($border-style = 'thick-thin-medium-gap' and ($style-pos = 'right' or $style-pos = 'bottom'))">
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.088cm 0.141cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.141cm 0.088cm 0.088cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:when test="$border-style = 'thin-thick-thin-medium-gap'">
                <xsl:choose>
                    <xsl:when test="$size-style &lt; 10">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.039cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.035cm 0.002cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 15">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.106cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.035cm 0.035cm 0.035cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 30">
                        <xsl:choose>
                            <xsl:when test="$style-pos = 'left' or $style-pos = 'top'">
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.159cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.035cm 0.035cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.212cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.035cm 0.088cm 0.088cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.318cm double #', $color-border)"/></xsl:attribute>
                        <xsl:choose>
                            <xsl:when test="$style-pos = 'left' or $style-pos = 'top'">
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.141cm 0.088cm 0.088cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.088cm 0.141cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:when test="$border-style = 'thin-thick-large-gap' or $border-style = 'thick-thin-large-gap'">
                <xsl:choose>
                    <xsl:when test="$size-style &lt; 7">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.092cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.002cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 10">
                        <xsl:choose>
                            <xsl:when test="($border-style = 'thin-thick-large-gap' and ($style-pos = 'left' or $style-pos = 'top')) or ($border-style = 'thick-thin-large-gap' and ($style-pos = 'right' or $style-pos = 'bottom'))">
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.125cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.035cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.092cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.002cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 15">
                        <xsl:choose>
                            <xsl:when test="($border-style = 'thin-thick-large-gap' and ($style-pos = 'left' or $style-pos = 'top')) or ($border-style = 'thick-thin-large-gap' and ($style-pos = 'right' or $style-pos = 'bottom'))">
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.125cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.035cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.159cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.035cm 0.035cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 30">
                        <xsl:choose>
                            <xsl:when test="($border-style = 'thin-thick-large-gap' and ($style-pos = 'left' or $style-pos = 'top')) or ($border-style = 'thick-thin-large-gap' and ($style-pos = 'right' or $style-pos = 'bottom'))">
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.178cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.088cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.159cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.035cm 0.035cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 40">
                        <xsl:choose>
                            <xsl:when test="($border-style = 'thin-thick-large-gap' and ($style-pos = 'left' or $style-pos = 'top')) or ($border-style = 'thick-thin-large-gap' and ($style-pos = 'right' or $style-pos = 'bottom'))">
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.231cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.141cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.159cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.035cm 0.035cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.318cm double #', $color-border)"/></xsl:attribute>
                        <xsl:choose>
                            <xsl:when test="($border-style = 'thin-thick-large-gap' and ($style-pos = 'left' or $style-pos = 'top')) or ($border-style = 'thick-thin-large-gap' and ($style-pos = 'right' or $style-pos = 'bottom'))">
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.088cm 0.141cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.141cm 0.088cm 0.088cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:when test="$border-style = 'thin-thick-thin-large-gap'">
                <xsl:choose>
                    <xsl:when test="$size-style &lt; 5">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.125cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.035cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 10">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.178cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.088cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 20">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.231cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.141cm</xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.318cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.088cm 0.141cm</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:when test="contains( $border-style, 'wave') or $border-style = 'dash-dot-stroked'">
                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.106cm double #', $color-border)"/></xsl:attribute>
                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.035cm 0.035cm 0.035cm</xsl:attribute>
            </xsl:when>
            <xsl:when test="contains( $border-style, 'three-d')">
                <xsl:choose>
                    <xsl:when test="$size-style &lt; 10">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.035cm solid #', $color-border)"/></xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 20">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.088cm solid #', $color-border)"/></xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 30">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.176cm solid #', $color-border)"/></xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 40">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.265cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.088cm 0.088cm</xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.318cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.088cm 0.141cm</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:when test="contains( $border-style, 'set')">
                <xsl:choose>
                    <xsl:when test="$size-style &lt; 7">
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.092cm double #', $color-border)"/></xsl:attribute>
                        <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.002cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 10">
                        <xsl:choose>
                            <xsl:when test="($border-style = 'outset' and ($style-pos = 'left' or $style-pos = 'top')) or ($border-style = 'inset' and ($style-pos = 'right' or $style-pos = 'bottom'))">
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.092cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.002cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.125cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.035cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 15">
                        <xsl:choose>
                            <xsl:when test="($border-style = 'outset' and ($style-pos = 'left' or $style-pos = 'top')) or ($border-style = 'inset' and ($style-pos = 'right' or $style-pos = 'bottom'))">
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.159cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.035cm 0.035cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.125cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.035cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 30">
                        <xsl:choose>
                            <xsl:when test="($border-style = 'outset' and ($style-pos = 'left' or $style-pos = 'top')) or ($border-style = 'inset' and ($style-pos = 'right' or $style-pos = 'bottom'))">
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.159cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.035cm 0.035cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.178cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.088cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:when test="$size-style &lt; 40">
                        <xsl:choose>
                            <xsl:when test="($border-style = 'outset' and ($style-pos = 'left' or $style-pos = 'top')) or ($border-style = 'inset' and ($style-pos = 'right' or $style-pos = 'bottom'))">
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.159cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.035cm 0.035cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.231cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.002cm 0.088cm 0.141cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.318cm double #', $color-border)"/></xsl:attribute>
                        <xsl:choose>
                            <xsl:when test="($border-style = 'outset' and ($style-pos = 'left' or $style-pos = 'top')) or ($border-style = 'inset' and ($style-pos = 'right' or $style-pos = 'bottom'))">
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.141cm 0.088cm 0.088cm</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="{concat('fo:border-', $style-pos)}"><xsl:value-of select="concat('0.231cm double #', $color-border)"/></xsl:attribute>
                                <xsl:attribute name="{concat('style:border-line-width-',$style-pos)}">0.088cm 0.088cm 0.141cm</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
                <xsl:attribute name="{concat('fo:border-', $style-pos)}">0.002cm solid #000000</xsl:attribute>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="list-styles-common">
        <xsl:param name="listtype"/>
        <xsl:param name="rootlist"/>
        <xsl:param name="currlist"/>
        <xsl:variable name="currlevel" select="number(@w:ilvl)"/>
        <xsl:variable name="startval" select="w:start/@w:val"/>
        <xsl:attribute name="text:level"><xsl:value-of select="$currlevel+1"/></xsl:attribute>
        <xsl:choose>
            <xsl:when test="$listtype = 23">
                <!-- bullet character. glu -->
                <xsl:attribute name="text:style-name">Bullet Symbols</xsl:attribute>
                <xsl:attribute name="text:bullet-char"><xsl:value-of select="w:lvlText/@w:val"/></xsl:attribute>
            </xsl:when>
            <xsl:when test="($listtype &gt; 0) and ($listtype &lt; 6)">
                <xsl:attribute name="text:style-name">Numbering Symbols</xsl:attribute>
                <xsl:if test="$startval">
                    <xsl:attribute name="text:start-value"><xsl:value-of select="$startval"/></xsl:attribute>
                </xsl:if>
                <xsl:attribute name="text:display-levels"><xsl:value-of select="string-length(w:lvlText/@w:val) - string-length(translate(w:lvlText/@w:val,'%','') )"/></xsl:attribute>
                <xsl:choose>
                    <xsl:when test="$listtype = 1">
                        <!-- nfcUCRoman: Uppercase roman -->
                        <xsl:attribute name="style:num-format">I</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$listtype = 2">
                        <!-- nfcLCRoman: Lowercase roman -->
                        <xsl:attribute name="style:num-format">i</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$listtype = 3">
                        <!-- nfcUCLetter: Uppercase alpha -->
                        <xsl:attribute name="style:num-format">A</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$listtype = 4">
                        <!-- nfcLCLetter: Lowercase alpah -->
                        <xsl:attribute name="style:num-format">a</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$listtype = 5">
                        <!-- nfcOrdinal: Ordinal -->
                        <xsl:attribute name="style:num-format">1</xsl:attribute>
                    </xsl:when>
                </xsl:choose>
                <xsl:attribute name="style:num-prefix"><xsl:value-of select="substring-before(w:lvlText/@w:val, '%')"/></xsl:attribute>
                <xsl:attribute name="style:num-suffix"><xsl:value-of select="substring-after(w:lvlText/@w:val, concat('%', $currlevel + 1) )"/></xsl:attribute>
            </xsl:when>
            <xsl:otherwise>
                <xsl:attribute name="text:style-name">Numbering Symbols</xsl:attribute>
                <xsl:if test="$startval">
                    <xsl:attribute name="text:start-value"><xsl:value-of select="$startval"/></xsl:attribute>
                </xsl:if>
                <xsl:attribute name="text:display-levels"><xsl:value-of select="string-length(w:lvlText/@w:val) - string-length(translate(w:lvlText/@w:val,'%','') )"/></xsl:attribute>
                <!-- can't support so many list types (totally 56) as in Word 2003. :( glu -->
                <xsl:attribute name="style:num-format">1</xsl:attribute>
                <xsl:attribute name="style:num-prefix"><xsl:value-of select="substring-before(w:lvlText/@w:val, '%')"/></xsl:attribute>
                <xsl:attribute name="style:num-suffix"><xsl:value-of select="substring-after(w:lvlText/@w:val, concat('%', $currlevel + 1) )"/></xsl:attribute>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:element name="style:properties">
            <!-- Find the node that corresponds to the level being processed. We can use this to determine the indentation to be used-->
            <xsl:variable name="currNode" select="/w:wordDocument/w:body//w:listPr[w:ilvl/@w:val = $currlevel][w:ilfo/@w:val = $currlist/w:ilfo/@w:val]"/>
            <xsl:choose>
                <xsl:when test="($currNode/wx:t/@wx:wTabBefore ) and ($currNode/wx:t/@wx:wTabAfter ) and (not($currNode/following-sibling::w:jc) or $currNode/following-sibling::w:jc/@w:val = 'left')">
                    <xsl:attribute name="text:space-before"><xsl:value-of select="(number($currNode/wx:t/@wx:wTabBefore)div 1440)*2.54"/>cm</xsl:attribute>
                    <xsl:attribute name="text:min-label-distance"><xsl:value-of select="(number($currNode/wx:t/@wx:wTabAfter)div 1440)*2.54"/>cm</xsl:attribute>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:attribute name="text:space-before"><xsl:value-of select="((number(w:pPr/w:ind/@w:left)  div 1440)*2.54) "/>cm</xsl:attribute>
                    <xsl:attribute name="text:min-label-distance"><xsl:value-of select="(number($currlist/wx:t/@wx:wTabAfter) div 1440)*2.54"/>cm</xsl:attribute>
                </xsl:otherwise>
            </xsl:choose>
            <xsl:if test="w:rPr/w:rFonts">
                <xsl:if test="w:rPr/w:rFonts/@w:ascii">
                    <xsl:attribute name="style:font-name"><xsl:value-of select="w:rPr/w:rFonts/@w:ascii"/></xsl:attribute>
                </xsl:if>
                <xsl:if test="w:rPr/w:rFonts/@w:fareast">
                    <xsl:attribute name="style:font-name-asian"><xsl:value-of select="w:rPr/w:rFonts/@w:fareast"/></xsl:attribute>
                </xsl:if>
                <xsl:if test="w:rPr/w:rFonts/@w:cs">
                    <xsl:attribute name="style:font-name-complex"><xsl:value-of select="w:rPr/w:rFonts/@w:cs"/></xsl:attribute>
                </xsl:if>
            </xsl:if>
        </xsl:element>
    </xsl:template>
    <xsl:template match="w:body">
        <xsl:element name="office:body">
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>
    <!-- avoid listPr in textbox. :( glu -->
    <xsl:template match="w:p[w:pPr/w:listPr[w:ilvl and w:ilfo]]">
        <xsl:call-template name="genlist"/>
    </xsl:template>
    <xsl:template match="w:hlink">
        <xsl:element name="text:a">
            <xsl:attribute name="xlink:type">simple</xsl:attribute>
            <xsl:choose>
                <xsl:when test="@w:dest and @w:bookmark">
                    <xsl:attribute name="xlink:href"><xsl:value-of select="concat( @w:dest, concat('#', @w:bookmark) )"/></xsl:attribute>
                </xsl:when>
                <xsl:when test="@w:dest">
                    <xsl:attribute name="xlink:href"><xsl:value-of select="@w:dest"/></xsl:attribute>
                </xsl:when>
                <xsl:when test="@w:bookmark">
                    <xsl:attribute name="xlink:href"><xsl:value-of select="concat('#', @w:bookmark)"/></xsl:attribute>
                </xsl:when>
            </xsl:choose>
            <xsl:if test="@w:target">
                <xsl:attribute name="office:target-frame-name"><xsl:value-of select="@w:target"/></xsl:attribute>
            </xsl:if>
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>
    <xsl:template match="w:p">
        <xsl:choose>
            <!-- because word treats page breaks as separate tags, we must split some paragraphs up so that we can 
			give the sub para a fo:break-before ="page" attribute	-->
            <xsl:when test="w:r[w:br/@w:type='page']  ">
                <xsl:call-template name="processPageBreaks"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:call-template name="commonParagraph"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="processPageBreaks">
        <xsl:variable name="pageBreak" select="w:r[w:br/@w:type='page']"/>
        <xsl:call-template name="createSubParagraph">
            <xsl:with-param name="list" select="$pageBreak[1]/preceding-sibling::w:r"/>
            <xsl:with-param name="pageBreak"/>
            <xsl:with-param name="needsPageBreak">false</xsl:with-param>
        </xsl:call-template>
        <xsl:for-each select="$pageBreak">
            <xsl:call-template name="createSubParagraph">
                <xsl:with-param name="list" select="./following-sibling::w:r[preceding::w:r/w:br = '.']"/>
                <xsl:with-param name="pageBreak" select="."/>
                <xsl:with-param name="needsPageBreak">true</xsl:with-param>
            </xsl:call-template>
        </xsl:for-each>
    </xsl:template>
    <xsl:template name="createSubParagraph">
        <xsl:param name="list"/>
        <xsl:param name="pageBreak"/>
        <xsl:param name="needsPageBreak"/>
        <xsl:if test="(count($list) &gt; 0) or ($needsPageBreak ='true') ">
            <xsl:element name="text:p">
                <xsl:choose>
                    <xsl:when test="$needsPageBreak = 'true'">
                        <xsl:choose>
                            <xsl:when test="ancestor::w:p/w:pPr">
                                <xsl:attribute name="text:style-name">P<xsl:number from="/w:wordDocument/w:body" level="any" count="w:pPr"/></xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="text:style-name">PageBreak</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                </xsl:choose>
                <xsl:if test="$pageBreak">
                    <xsl:apply-templates select="$pageBreak"/>
                </xsl:if>
                <xsl:apply-templates select="$list"/>
            </xsl:element>
        </xsl:if>
    </xsl:template>
    <!-- strange enough, WordML allows bookmarks out of w:p, but this isn't supported by StarWriter. To be patched. :( glu -->
    <xsl:template match="aml:annotation[@w:type = 'Word.Bookmark.Start']">
        <text:bookmark-start text:name="{@w:name}"/>
    </xsl:template>
    <xsl:template match="aml:annotation[@w:type = 'Word.Bookmark.End']">
        <xsl:variable name="id" select="@aml:id"/>
        <text:bookmark-end text:name="{preceding::aml:annotation[@aml:id = $id]/@w:name}"/>
    </xsl:template>
    <xsl:template name="commonParagraph">
        <!-- Heading 1 to 9  or normal paragraph-->
        <xsl:choose>
            <xsl:when test="starts-with(w:pPr/w:pStyle/@w:val, 'Heading')">
                <xsl:variable name="styleid" select="w:pPr/w:pStyle/@w:val"/>
                <xsl:element name="text:h">
                    <xsl:variable name="outlinelevel" select="number(/w:wordDocument/w:styles/w:style[@w:styleId = $styleid]/w:pPr/w:outlineLvl/@w:val)+1"/>
                    <xsl:if test="$outlinelevel">
                        <xsl:attribute name="text:level"><xsl:value-of select="$outlinelevel"/></xsl:attribute>
                    </xsl:if>
                    <xsl:call-template name="commonParagraphAttributes"/>
                    <xsl:apply-templates/>
                </xsl:element>
            </xsl:when>
            <xsl:when test="w:r/w:instrText and contains(w:r/w:instrText , 'HYPERLINK')">
                <!-- handling another type of Hyperlink address in w:body.cp tom chen. Acutally they're from Field features, a complex one. glu -->
                <xsl:variable name="hyper-str" select="normalize-space(w:r/w:instrText)"/>
                <xsl:variable name="hyper-dest" select="substring-before( substring($hyper-str, 12), '&quot;')"/>
                <xsl:variable name="hyper-bookmark">
                    <xsl:choose>
                        <xsl:when test="contains( $hyper-str, ' \l ')">
                            <xsl:value-of select="concat( '#', substring-before( substring-after( substring-after( $hyper-str, ' \l '), '&quot;'), '&quot;') )"/>
                        </xsl:when>
                        <xsl:otherwise/>
                    </xsl:choose>
                </xsl:variable>
                <xsl:element name="text:p">
                    <xsl:element name="text:a">
                        <xsl:attribute name="xlink:type">simple</xsl:attribute>
                        <xsl:attribute name="xlink:href"><xsl:value-of select="concat( $hyper-dest, $hyper-bookmark)"/></xsl:attribute>
                        <xsl:apply-templates/>
                    </xsl:element>
                </xsl:element>
            </xsl:when>
            <xsl:otherwise>
                <xsl:element name="text:p">
                    <xsl:call-template name="commonParagraphAttributes"/>
                    <xsl:apply-templates/>
                </xsl:element>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="commonParagraphAttributes">
        <xsl:choose>
            <xsl:when test="w:pPr">
                <!-- count="w:pPr" doesn't work well, don't know  the reason actually. glu :( -->
                <xsl:attribute name="text:style-name">P<xsl:number from="/w:wordDocument/w:body" level="any" count="w:p[w:pPr]"/></xsl:attribute>
            </xsl:when>
            <xsl:when test="generate-id(ancestor::wx:sect/descendant::w:p[1]) = generate-id(.)">
                <!-- create the leading paragraph style name in one section for master page style application, glu -->
                <xsl:variable name="paragraph-number">
                    <xsl:number from="/w:wordDocument/w:body" level="any" count="w:p[w:pPr]"/>
                </xsl:variable>
                <xsl:attribute name="text:style-name">P<xsl:value-of select="number($paragraph-number)"/>_1</xsl:attribute>
            </xsl:when>
            <xsl:when test="not(w:pPr) and (descendant::w:br[@w:type = 'column'] or ancestor::wx:sect/descendant::w:sectPr[w:cols/@w:num &gt; 1])">
                <xsl:attribute name="text:style-name">ColumnBreakPara</xsl:attribute>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="w:pPr"/>
    <xsl:template match="w:rPr"/>
    <xsl:template match="wx:sub-section">
        <xsl:apply-templates/>
    </xsl:template>
    <xsl:template match="w:t">
        <xsl:choose>
            <xsl:when test="string(.) = ' ' ">
                <xsl:element name="text:s"/>
            </xsl:when>
            <xsl:when test="contains(.,'  ')">
                <xsl:call-template name="replace-spaces">
                    <xsl:with-param name="curr-string" select="."/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:apply-templates/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="replace-spaces">
        <xsl:param name="curr-string"/>
        <xsl:if test="contains($curr-string,'  ')">
            <xsl:value-of select="substring-before($curr-string,'  ')"/>
            <text:s text:c="2"/>
            <xsl:variable name="next-string" select="substring-after($curr-string,'  ')"/>
            <xsl:choose>
                <xsl:when test="contains($next-string, '  ')">
                    <xsl:call-template name="replace-spaces">
                        <xsl:with-param name="curr-string" select="$next-string"/>
                    </xsl:call-template>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$next-string"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
    </xsl:template>
    <xsl:template match="w:tab[parent::w:r]">
        <xsl:element name="text:tab-stop"/>
    </xsl:template>
    <xsl:template match="w:hdr">
        <!-- empty template to ignore inline processing of header. These are handled as part of the master style in styles.xsl	-->
    </xsl:template>
    <xsl:template match="w:ftr">
        <!-- empty template to ignore inline processing of header. These are handled as part of the master style in styles.xsl	-->
    </xsl:template>
    <!-- WordML contains multiple w:t within one w:r, so in Star Writer text:span should correspond to w:t glu -->
    <xsl:template match="w:r">
        <xsl:if test="w:br[@w:type='text-wrapping' or not(@w:type)]">
            <xsl:element name="text:line-break"/>
        </xsl:if>
        <xsl:choose>
            <xsl:when test="w:rPr/w:vanish">
                <text:hidden-text text:is-hidden="true" text:condition="" text:string-value="{.}"/>
            </xsl:when>
            <xsl:when test="aml:annotation/@w:type = 'Word.Comment'">
                <office:annotation office:author="{aml:annotation/@aml:author}" office:create-date="{substring(aml:annotation/@aml:createdate,1,10)}" office:display="true">
                    <xsl:apply-templates/>
                </office:annotation>
            </xsl:when>
            <xsl:when test="(preceding-sibling::w:r) or (w:rPr)">
                <xsl:element name="text:span">
                    <xsl:choose>
                        <xsl:when test="w:rPr/w:rStyle">
                            <xsl:attribute name="text:style-name"><xsl:value-of select="w:rPr/w:rStyle/@w:val"/></xsl:attribute>
                        </xsl:when>
                        <xsl:when test="w:rPr">
                            <xsl:attribute name="text:style-name">T<xsl:value-of select="count(preceding::w:rPr)"/></xsl:attribute>
                        </xsl:when>
                    </xsl:choose>
                    <xsl:apply-templates/>
                </xsl:element>
            </xsl:when>
            <xsl:otherwise>
                <xsl:apply-templates/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <!--
	<xsl:template match="w:br[@w:type='text-wrapping' or not(@w:type)]">
		<text:line-break/>
	</xsl:template>
	<xsl:template match="aml:annotation[@w:type = 'Word.Comment']">
		<office:annotation office:author="{@aml:author}" office:create-date="{substring(@aml:createdate,1,10)}" office:display="true">
			<xsl:apply-templates/>
		</office:annotation>
	</xsl:template>
	<xsl:template match="w:rPr/w:vanish">
		<text:hidden-text text:is-hidden="true" text:condition="" text:string-value="{../..}"/>
	</xsl:template>
	<xsl:template match="w:t">
		<xsl:element name="text:span">
			<xsl:if test="preceding-sibling::w:rPr">
				<xsl:attribute name="text:style-name">T<xsl:value-of select="count(preceding::w:rPr)"/></xsl:attribute>
			</xsl:if>
			<xsl:apply-templates/>
		</xsl:element>
	</xsl:template>
	-->
    <xsl:template match="node()[name() = 'w:vertAlign' or name() = 'w:position']">
        <xsl:variable name="height">
            <xsl:choose>
                <xsl:when test="parent::w:rPr/w:vertAlign[@w:val = 'superscript' or @w:val = 'subscript']">58%</xsl:when>
                <xsl:otherwise>100%</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="position">
            <xsl:choose>
                <xsl:when test="parent::w:rPr/w:position">
                    <!-- con't get font height easily, so just set w:val as percentage. glu -->
                    <xsl:value-of select="concat( parent::w:rPr/w:position/@w:val, '%')"/>
                </xsl:when>
                <xsl:when test="parent::w:rPr/w:vertAlign[@w:val = 'superscript']">super</xsl:when>
                <xsl:when test="parent::w:rPr/w:vertAlign[@w:val = 'subscript']">sub</xsl:when>
            </xsl:choose>
        </xsl:variable>
        <xsl:attribute name="style:text-position"><xsl:value-of select="concat(concat( $position, ' '), $height)"/></xsl:attribute>
    </xsl:template>
    <xsl:template match="w:sz">
        <xsl:attribute name="fo:font-size"><xsl:value-of select="concat( number(@w:val) div 2, 'pt')"/></xsl:attribute>
        <xsl:attribute name="fo:font-size-asian"><xsl:value-of select="concat( number(@w:val) div 2, 'pt')"/></xsl:attribute>
    </xsl:template>
    <xsl:template match="w:sz-cs">
        <xsl:attribute name="fo:font-size-complex"><xsl:value-of select="concat( number(@w:val) div 2, 'pt')"/></xsl:attribute>
    </xsl:template>
    <xsl:template match="w:i">
        <xsl:attribute name="fo:font-style">italic</xsl:attribute>
        <xsl:attribute name="fo:font-style-asian">italic</xsl:attribute>
    </xsl:template>
    <xsl:template match="w:i-cs">
        <xsl:attribute name="fo:font-style-complex">italic</xsl:attribute>
    </xsl:template>
    <xsl:template match="w:b">
        <xsl:attribute name="fo:font-weight">bold</xsl:attribute>
        <xsl:attribute name="fo:font-weight-asian">bold</xsl:attribute>
    </xsl:template>
    <xsl:template match="w:b-cs">
        <xsl:attribute name="fo:font-weight-complex">bold</xsl:attribute>
    </xsl:template>
    <xsl:template match="w:u">
        <xsl:choose>
            <xsl:when test="@w:val = 'words'">
                <xsl:attribute name="style:text-underline">single</xsl:attribute>
                <xsl:attribute name="fo:score-spaces">false</xsl:attribute>
            </xsl:when>
            <xsl:when test="@w:val = 'thick'">
                <xsl:attribute name="style:text-underline">bold</xsl:attribute>
            </xsl:when>
            <xsl:when test="@w:val = 'dotted-heavy'">
                <xsl:attribute name="style:text-underline">bold-dotted</xsl:attribute>
            </xsl:when>
            <xsl:when test="@w:val = 'dashed-heavy'">
                <xsl:attribute name="style:text-underline">bold-dash</xsl:attribute>
            </xsl:when>
            <xsl:when test="@w:val = 'dash-long'">
                <xsl:attribute name="style:text-underline">long-dash</xsl:attribute>
            </xsl:when>
            <xsl:when test="@w:val = 'dash-long-heavy'">
                <xsl:attribute name="style:text-underline">bold-long-dash</xsl:attribute>
            </xsl:when>
            <xsl:when test="@w:val = 'dash-dot-heavy'">
                <xsl:attribute name="style:text-underline">bold-dot-dash</xsl:attribute>
            </xsl:when>
            <xsl:when test="@w:val = 'dash-dot-dot-heavy'">
                <xsl:attribute name="style:text-underline">bold-dot-dot-dash</xsl:attribute>
            </xsl:when>
            <xsl:when test="@w:val = 'wavy-heavy'">
                <xsl:attribute name="style:text-underline">bold-wave</xsl:attribute>
            </xsl:when>
            <xsl:when test="@w:val = 'wavy-double'">
                <xsl:attribute name="style:text-underline">double-wave</xsl:attribute>
            </xsl:when>
            <xsl:otherwise>
                <xsl:attribute name="style:text-underline"><xsl:value-of select="@w:val"/></xsl:attribute>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:if test="@w:color and not(@w:color = 'auto')">
            <xsl:attribute name="style:text-underline-color"><xsl:value-of select="concat( '#', @w:color)"/></xsl:attribute>
        </xsl:if>
    </xsl:template>
    <xsl:template match="w:shadow">
        <xsl:attribute name="style:text-shadow">1pt 1pt</xsl:attribute>
        <xsl:attribute name="fo:text-shadow">1pt 1pt</xsl:attribute>
    </xsl:template>
    <xsl:template match="w:strike">
        <xsl:attribute name="style:text-crossing-out">single-line</xsl:attribute>
    </xsl:template>
    <xsl:template match="w:dstrike">
        <xsl:attribute name="style:text-crossing-out">double-line</xsl:attribute>
    </xsl:template>
    <xsl:template match="w:color[not(@w:val = 'auto')]">
        <xsl:attribute name="fo:color">#<xsl:value-of select="@w:val"/></xsl:attribute>
    </xsl:template>
    <xsl:template match="w:smallCaps">
        <xsl:attribute name="fo:font-variant">small-caps</xsl:attribute>
    </xsl:template>
    <xsl:template match="w:caps">
        <xsl:attribute name="fo:text-transform">uppercase</xsl:attribute>
    </xsl:template>
    <xsl:template match="w:imprint">
        <xsl:attribute name="style:font-relief">engraved</xsl:attribute>
    </xsl:template>
    <xsl:template match="w:emboss">
        <xsl:attribute name="style:font-relief">embossed</xsl:attribute>
    </xsl:template>
    <xsl:template match="w:outline">
        <xsl:attribute name="style:text-outline">true</xsl:attribute>
    </xsl:template>
    <xsl:template match="w:w[@w:val]">
        <xsl:attribute name="style:text-scale"><xsl:value-of select="@w:val"/></xsl:attribute>
    </xsl:template>
    <xsl:template match="w:em">
        <xsl:choose>
            <xsl:when test="@w:val = 'comma'">
                <xsl:attribute name="style:text-emphasize">accent below</xsl:attribute>
            </xsl:when>
            <xsl:when test="@w:val = 'under-dot'">
                <xsl:attribute name="style:text-emphasize">disc below</xsl:attribute>
            </xsl:when>
            <xsl:otherwise>
                <xsl:attribute name="style:text-emphasize"><xsl:value-of select="concat( @w:val, ' below')"/></xsl:attribute>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="w:spacing[parent::w:rPr]">
        <xsl:attribute name="fo:letter-spacing"><xsl:value-of select="concat( floor(number(@w:val div 1440) * 2540) div 1000, 'cm')"/></xsl:attribute>
    </xsl:template>
    <xsl:template match="w:effect[@w:val = 'blink-background']">
        <xsl:attribute name="style:text-blinking">true</xsl:attribute>
        <xsl:attribute name="style:text-background-color">#000000</xsl:attribute>
    </xsl:template>
    <xsl:template match="w:lang">
        <xsl:if test="@w:val">
            <xsl:attribute name="fo:language"><xsl:value-of select="substring-before( @w:val, '-')"/></xsl:attribute>
            <xsl:attribute name="fo:country"><xsl:value-of select="substring-after( @w:val, '-')"/></xsl:attribute>
        </xsl:if>
        <xsl:if test="@w:fareast">
            <xsl:attribute name="fo:language-asian"><xsl:value-of select="substring-before( @w:fareast, '-')"/></xsl:attribute>
            <xsl:attribute name="fo:country-asian"><xsl:value-of select="substring-after( @w:fareast, '-')"/></xsl:attribute>
        </xsl:if>
        <xsl:if test="@w:bidi">
            <xsl:attribute name="fo:language-complex"><xsl:value-of select="substring-before( @w:bidi, '-')"/></xsl:attribute>
            <xsl:attribute name="fo:country-complex"><xsl:value-of select="substring-after( @w:bidi, '-')"/></xsl:attribute>
        </xsl:if>
    </xsl:template>
    <xsl:template match="wx:font[not(parent::w:listPr)]">
        <xsl:attribute name="style:font-name"><xsl:value-of select="@wx:val"/></xsl:attribute>
        <xsl:attribute name="style:font-name-asian"><xsl:value-of select="@wx:val"/></xsl:attribute>
        <xsl:attribute name="style:font-name-complex"><xsl:value-of select="@wx:val"/></xsl:attribute>
    </xsl:template>
    <xsl:template match="wx:sect">
        <!-- We handle Columns as seperate Sections to allow for different numbers of columns on the same page-->
        <xsl:choose>
            <xsl:when test="descendant::w:sectPr[w:cols/@w:num &gt; 1]">
                <xsl:element name="text:section">
                    <xsl:attribute name="text:style-name">sect<xsl:value-of select="count(preceding::w:sectPr[w:cols/@w:num &gt;1])"/></xsl:attribute>
                    <xsl:attribute name="text:name">sect<xsl:value-of select="count(preceding::w:sectPr[w:cols/@w:num &gt;1])"/></xsl:attribute>
                    <xsl:apply-templates/>
                </xsl:element>
            </xsl:when>
            <xsl:otherwise>
                <xsl:apply-templates/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="paragraph-properties">
        <xsl:choose>
            <xsl:when test="descendant::w:ind[@w:left]">
                <xsl:attribute name="fo:margin-left"><xsl:value-of select="number(((descendant::w:ind/@w:left)div 1440) *2.54)"/>cm</xsl:attribute>
            </xsl:when>
            <xsl:otherwise>
                <xsl:attribute name="fo:margin-left">0cm</xsl:attribute>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:choose>
            <xsl:when test="descendant::w:ind[@w:right]">
                <xsl:attribute name="fo:margin-right"><xsl:value-of select="number(((descendant::w:ind/@w:right)div 1440) *2.54)"/>cm</xsl:attribute>
            </xsl:when>
            <xsl:otherwise>
                <xsl:attribute name="fo:margin-right">0cm</xsl:attribute>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:choose>
            <xsl:when test="descendant::w:ind[@w:first-line]">
                <xsl:attribute name="fo:text-indent"><xsl:value-of select="number(((descendant::w:ind/@w:first-line)div 1440) *2.54)"/>cm</xsl:attribute>
            </xsl:when>
            <xsl:otherwise>
                <xsl:attribute name="fo:text-indent">0cm</xsl:attribute>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:call-template name="bidi"/>
        <!-- line spacing-->
        <xsl:if test="descendant::w:spacing">
            <xsl:choose>
                <xsl:when test="descendant::w:spacing/@w:line-rule='at-least'">
                    <xsl:attribute name="style:line-height-at-least"><xsl:value-of select="number(((descendant::w:spacing/@w:line) div 1440)*2.54)"/>cm</xsl:attribute>
                </xsl:when>
                <xsl:when test="descendant::w:spacing/@w:line-rule='auto'">
                    <xsl:attribute name="fo:line-height"><xsl:value-of select="(number((descendant::w:spacing/@w:line) div 240) * 100)"/>%</xsl:attribute>
                </xsl:when>
                <xsl:when test="descendant::w:spacing/@w:line-rule='exact'">
                    <xsl:attribute name="fo:line-height"><xsl:value-of select="number(((descendant::w:spacing/@w:line) div 1440)*2.54)"/>cm</xsl:attribute>
                </xsl:when>
            </xsl:choose>
            <xsl:if test="descendant::w:spacing/@w:before">
                <xsl:attribute name="fo:margin-top"><xsl:value-of select="(number(descendant::w:spacing/@w:before) div 1440 )*2.54"/>cm</xsl:attribute>
            </xsl:if>
            <xsl:if test="descendant::w:spacing/@w:after">
                <xsl:attribute name="fo:margin-bottom"><xsl:value-of select="(number(descendant::w:spacing/@w:after) div 1440 )*2.54"/>cm</xsl:attribute>
            </xsl:if>
        </xsl:if>
        <xsl:if test="descendant::w:shd">
            <xsl:variable name="background-color">
                <xsl:choose>
                    <xsl:when test="string-length(descendant::w:shd/@w:fill) = 6">
                        <xsl:value-of select="concat('#', descendant::w:shd/@w:fill)"/>
                    </xsl:when>
                    <xsl:otherwise>#000000</xsl:otherwise>
                </xsl:choose>
            </xsl:variable>
            <xsl:choose>
                <xsl:when test="descendant::w:shd/@w:val = 'solid'">
                    <xsl:attribute name="fo:background-color"><xsl:value-of select="$background-color"/></xsl:attribute>
                </xsl:when>
                <!-- patterns are necessary in the future. glu -->
                <xsl:otherwise/>
            </xsl:choose>
        </xsl:if>
        <!-- MS Word & Star Writer has different default font size, MS 10pt, Star 12pt. glu -->
        <xsl:attribute name="style:auto-text-indent">false</xsl:attribute>
        <xsl:if test="descendant::w:tabs">
            <xsl:element name="style:tab-stops">
                <xsl:for-each select="descendant::w:tabs/w:tab">
                    <xsl:element name="style:tab-stop">
                        <xsl:attribute name="style:position"><xsl:value-of select="(number(@w:pos)div 1440)*2.54"/>cm</xsl:attribute>
                        <xsl:choose>
                            <xsl:when test="@w:val = 'decimal'">
                                <xsl:attribute name="style:type">char</xsl:attribute>
                                <xsl:attribute name="style:char"/>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="style:type"><xsl:value-of select="@w:val"/></xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:element>
                </xsl:for-each>
            </xsl:element>
        </xsl:if>
    </xsl:template>
    <xsl:template name="bidi">
        <!-- bi-directional support-->
        <xsl:if test="descendant::w:bidi">
            <xsl:attribute name="style:writing-mode">rl-tb</xsl:attribute>
            <xsl:attribute name="fo:text-align">end</xsl:attribute>
        </xsl:if>
        <xsl:choose>
            <xsl:when test="descendant::w:jc[@w:val = 'center']">
                <xsl:attribute name="fo:text-align">center</xsl:attribute>
            </xsl:when>
            <xsl:when test="descendant::w:jc[@w:val = 'left']">
                <xsl:choose>
                    <xsl:when test="descendant::w:bidi">
                        <xsl:attribute name="fo:text-align">end</xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="fo:text-align">start</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:when test="descendant::w:jc[@w:val = 'right']">
                <xsl:choose>
                    <xsl:when test="descendant::w:bidi">
                        <xsl:attribute name="fo:text-align">start</xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="fo:text-align">end</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:when test="descendant::w:jc[@w:val = 'both']">
                <xsl:attribute name="fo:text-align">justify</xsl:attribute>
                <xsl:attribute name="style:justify-single-word">false</xsl:attribute>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="w:pict">
        <xsl:choose>
            <xsl:when test="descendant::w:binData">
                <!-- Cannot load zipped Wmf file. glu <xsl:if test="not(contains(w:binData/@w:name,'.wmz'))"> -->
                <xsl:element name="draw:image">
                    <xsl:if test="v:shape/@style">
                        <xsl:if test="contains( v:shape/@style, 'margin-left:')">
                            <xsl:attribute name="svg:x"><xsl:value-of select="concat( number(substring-before( substring-after( v:shape/@style, 'margin-left:'), 'pt'))*0.0353, 'cm')"/></xsl:attribute>
                        </xsl:if>
                        <xsl:if test="contains( v:shape/@style, 'margin-top:')">
                            <xsl:attribute name="svg:y"><xsl:value-of select="concat( number(substring-before( substring-after( v:shape/@style, 'margin-top:'), 'pt'))*0.0353, 'cm')"/></xsl:attribute>
                        </xsl:if>
                        <xsl:attribute name="svg:width"><xsl:value-of select="concat( number(substring-before( substring-after( v:shape/@style, 'width:'), 'pt'))*0.0353, 'cm')"/></xsl:attribute>
                        <xsl:attribute name="svg:height"><xsl:value-of select="concat( number(substring-before( substring-after( v:shape/@style, 'height:'), 'pt'))*0.0353, 'cm')"/></xsl:attribute>
                        <xsl:variable name="anchor-type">
                            <xsl:if test="contains( v:shape/@style, 'mso-position-horizontal-relative')">
                                <xsl:value-of select="substring-before( substring-after( v:shape/@style, 'mso-position-horizontal-relative:'), ';')"/>
                            </xsl:if>
                        </xsl:variable>
                        <xsl:choose>
                            <xsl:when test="$anchor-type = 'margin'">
                                <xsl:attribute name="text:anchor-type">frame</xsl:attribute>
                            </xsl:when>
                            <xsl:when test="$anchor-type = 'page'">
                                <xsl:attribute name="text:anchor-type">page</xsl:attribute>
                            </xsl:when>
                            <xsl:when test="$anchor-type = 'char'">
                                <xsl:attribute name="text:anchor-type">as-char</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="text:anchor-type">paragraph</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                        <xsl:if test="v:shape/v:imagedata/@o:title">
                            <xsl:attribute name="draw:name"><xsl:value-of select="v:shape/v:imagedata/@o:title"/></xsl:attribute>
                        </xsl:if>
                        <xsl:element name="office:binary-data">
                            <xsl:value-of select="w:binData"/>
                        </xsl:element>
                    </xsl:if>
                </xsl:element>
            </xsl:when>
            <xsl:otherwise>
                <xsl:apply-templates/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="w:binData">
        <xsl:element name="office:binary-data">
            <xsl:value-of select="."/>
        </xsl:element>
    </xsl:template>
    <xsl:template match="v:group">
        <xsl:apply-templates/>
    </xsl:template>
    <xsl:template match="v:oval">
        <xsl:element name="draw:ellipse">
            <xsl:variable name="style" select="@style"/>
            <xsl:variable name="position" select="substring-after($style,'position:')"/>
            <xsl:variable name="positionval" select="substring-before($position,';')"/>
            <xsl:variable name="left" select="substring-after($style,'left:')"/>
            <xsl:variable name="leftval" select="substring-before($left,';')"/>
            <xsl:variable name="top" select="substring-after($style,'top:')"/>
            <xsl:variable name="topval" select="substring-before($top,';')"/>
            <xsl:variable name="width" select="substring-after($style,'width:')"/>
            <xsl:variable name="widthval" select="substring-before($width,';')"/>
            <xsl:variable name="height" select="substring-after($style,'height:')"/>
            <xsl:variable name="heightval" select="$height"/>
            <xsl:if test="$positionval = 'absolute'">
                <xsl:variable name="parentgroup" select="parent::v:group"/>
                <xsl:variable name="origin" select="$parentgroup/@coordorigin"/>
                <xsl:variable name="groupleft" select="substring-before($origin,',')"/>
                <xsl:variable name="grouptop" select="substring-after($origin,',')"/>
                <xsl:variable name="currsect" select="ancestor::wx:sect"/>
                <xsl:variable name="currsectPr" select="$currsect/descendant::w:sectPr/w:pgMar"/>
                <xsl:variable name="leftmar" select="$currsectPr/@w:left"/>
                <xsl:variable name="topmar" select="$currsectPr/@w:top"/>
                <xsl:attribute name="text:anchor-type">page</xsl:attribute>
                <xsl:attribute name="svg:width"><xsl:value-of select="(number($widthval) div 1440)*2.54"/>cm</xsl:attribute>
                <xsl:attribute name="svg:height"><xsl:value-of select="(number($heightval) div 1440)*2.54"/>cm</xsl:attribute>
                <xsl:attribute name="svg:x"><xsl:value-of select="(number($topval + $topmar) div 1440)*2.54"/>cm</xsl:attribute>
                <xsl:attribute name="svg:y"><xsl:value-of select="(number($leftval + $leftmar) div 1440)*2.54"/>cm</xsl:attribute>
            </xsl:if>
        </xsl:element>
    </xsl:template>
    <xsl:template match="v:rect">
        <xsl:element name="draw:rect">
            <xsl:variable name="style" select="@style"/>
            <xsl:variable name="position" select="substring-after($style,'position:')"/>
            <xsl:variable name="positionval" select="substring-before($position,';')"/>
            <xsl:variable name="left" select="substring-after($style,'left:')"/>
            <xsl:variable name="leftval" select="substring-before($left,';')"/>
            <xsl:variable name="top" select="substring-after($style,'top:')"/>
            <xsl:variable name="topval" select="substring-before($top,';')"/>
            <xsl:variable name="width" select="substring-after($style,'width:')"/>
            <xsl:variable name="widthval" select="substring-before($width,';')"/>
            <xsl:variable name="height" select="substring-after($style,'height:')"/>
            <xsl:variable name="heightval" select="$height"/>
            <xsl:if test="$positionval = 'absolute'">
                <xsl:variable name="parentgroup" select="parent::v:group"/>
                <xsl:variable name="origin" select="$parentgroup/@coordorigin"/>
                <xsl:variable name="groupleft" select="substring-before($origin,',')"/>
                <xsl:variable name="grouptop" select="substring-after($origin,',')"/>
                <xsl:variable name="currsect" select="ancestor::wx:sect"/>
                <xsl:variable name="currsectPr" select="$currsect/descendant::w:sectPr/w:pgMar"/>
                <xsl:variable name="leftmar" select="$currsectPr/@w:left"/>
                <xsl:variable name="topmar" select="$currsectPr/@w:top"/>
                <xsl:attribute name="text:anchor-type">page</xsl:attribute>
                <xsl:attribute name="svg:width"><xsl:value-of select="(number($widthval) div 1440)*2.54"/>cm</xsl:attribute>
                <xsl:attribute name="svg:height"><xsl:value-of select="(number($heightval) div 1440)*2.54"/>cm</xsl:attribute>
                <xsl:attribute name="svg:x"><xsl:value-of select="(number($topval + $topmar) div 1440)*2.54"/>cm</xsl:attribute>
                <xsl:attribute name="svg:y"><xsl:value-of select="(number($leftval + $leftmar) div 1440)*2.54"/>cm</xsl:attribute>
            </xsl:if>
        </xsl:element>
    </xsl:template>
    <xsl:template match="w:tbl">
        <xsl:element name="table:table">
            <xsl:if test="w:tblPr">
                <xsl:attribute name="table:style-name">Table<xsl:number count="w:tbl" from="/w:wordDocument/w:body" level="any"/></xsl:attribute>
            </xsl:if>
            <xsl:apply-templates select="./*"/>
        </xsl:element>
    </xsl:template>
    <xsl:template match="w:tblPr">
        <!-- commented tempararily by glu -->
        <!-- <xsl:attribute name="table:style-name">Table<xsl:value-of select="count(preceding::w:tblPr)"/></xsl:attribute> -->
    </xsl:template>
    <xsl:template match="w:tblGrid">
        <xsl:apply-templates/>
    </xsl:template>
    <xsl:template match="w:gridCol">
        <xsl:element name="table:table-column">
            <xsl:attribute name="table:style-name">Table<xsl:number count="w:tbl" from="/w:wordDocument/w:body" level="any"/>.C<xsl:number count="w:gridCol" from="/w:wordDocument/w:body" level="single"/></xsl:attribute>
        </xsl:element>
    </xsl:template>
    <xsl:template match="w:tr">
        <xsl:element name="table:table-row">
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>
    <xsl:template match="w:tc">
        <xsl:element name="table:table-cell">
            <xsl:attribute name="table:style-name">Table<xsl:number count="w:tbl" from="/w:wordDocument/w:body" level="any"/>.R<xsl:number count="w:tr" from="/w:wordDocument/w:body" level="single"/>C<xsl:number count="w:tc" from="/w:wordDocument/w:body" level="single"/></xsl:attribute>
            <xsl:if test="w:tcPr/w:gridSpan">
                <xsl:attribute name="table:number-columns-spanned"><xsl:value-of select="w:tcPr/w:gridSpan/@w:val"/></xsl:attribute>
            </xsl:if>
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>
    <!--
	<xsl:template match="w:tcBorders">
		<xsl:apply-templates/>
	</xsl:template>
	<xsl:template match="w:top">
		<xsl:choose>
			<xsl:when test="@w:val='nil' ">
				<xsl:attribute name="fo:border-top">none</xsl:attribute>
			</xsl:when>
			<xsl:when test="@wx:bdrwidth and @w:color">
				<xsl:attribute name="fo:border-top"><xsl:value-of select="(number(@wx:bdrwidth) div 1440 )*2.54"/><xsl:text>cm </xsl:text><xsl:choose><xsl:when test="@w:color ='auto'"><xsl:text>solid #000000</xsl:text></xsl:when><xsl:otherwise><xsl:text>solid #</xsl:text><xsl:value-of select="@w:color"/></xsl:otherwise></xsl:choose></xsl:attribute>
			</xsl:when>
			<xsl:when test="@wx:bdrwidth">
				<xsl:attribute name="fo:border-top"><xsl:value-of select="(number(@wx:bdrwidth) div 1440 )*2.54"/>cm</xsl:attribute>
			</xsl:when>
		</xsl:choose>
	</xsl:template>
	<xsl:template match="w:bottom">
		<xsl:choose>
			<xsl:when test="@w:val='nil' ">
				<xsl:attribute name="fo:border-bottom">none</xsl:attribute>
			</xsl:when>
			<xsl:when test="@wx:bdrwidth and @w:color">
				<xsl:attribute name="fo:border-bottom"><xsl:value-of select="(number(@wx:bdrwidth) div 1440 )*2.54"/><xsl:text>cm </xsl:text><xsl:choose><xsl:when test="@w:color ='auto'"><xsl:text>solid #000000</xsl:text></xsl:when><xsl:otherwise><xsl:text>solid #</xsl:text><xsl:value-of select="@w:color"/></xsl:otherwise></xsl:choose></xsl:attribute>
			</xsl:when>
			<xsl:when test="@wx:bdrwidth">
				<xsl:attribute name="fo:border-bottom"><xsl:value-of select="(number(@wx:bdrwidth) div 1440 )*2.54"/>cm</xsl:attribute>
			</xsl:when>
		</xsl:choose>
	</xsl:template>
	<xsl:template match="w:left">
		<xsl:choose>
			<xsl:when test="@w:val='nil' ">
				<xsl:attribute name="fo:border-left">none</xsl:attribute>
			</xsl:when>
			<xsl:when test="@wx:bdrwidth and @w:color">
				<xsl:attribute name="fo:border-left"><xsl:value-of select="(number(@wx:bdrwidth) div 1440 )*2.54"/><xsl:text>cm </xsl:text><xsl:choose><xsl:when test="@w:color ='auto'"><xsl:text>solid #000000</xsl:text></xsl:when><xsl:otherwise><xsl:text>solid #</xsl:text><xsl:value-of select="@w:color"/></xsl:otherwise></xsl:choose></xsl:attribute>
			</xsl:when>
			<xsl:when test="@wx:bdrwidth">
				<xsl:attribute name="fo:border-left"><xsl:value-of select="(number(@wx:bdrwidth) div 1440 )*2.54"/>cm</xsl:attribute>
			</xsl:when>
		</xsl:choose>
	</xsl:template>
	<xsl:template match="w:right">
		<xsl:choose>
			<xsl:when test="@w:val='nil' ">
				<xsl:attribute name="fo:border-right">none</xsl:attribute>
			</xsl:when>
			<xsl:when test="@wx:bdrwidth and @w:color">
				<xsl:attribute name="fo:border-right"><xsl:value-of select="(number(@wx:bdrwidth) div 1440 )*2.54"/><xsl:text>cm </xsl:text><xsl:choose><xsl:when test="@w:color ='auto'"><xsl:text>solid #000000</xsl:text></xsl:when><xsl:otherwise><xsl:text>solid #</xsl:text><xsl:value-of select="@w:color"/></xsl:otherwise></xsl:choose></xsl:attribute>
			</xsl:when>
			<xsl:when test="@wx:bdrwidth">
				<xsl:attribute name="fo:border-right"><xsl:value-of select="(number(@wx:bdrwidth) div 1440 )*2.54"/>cm</xsl:attribute>
			</xsl:when>
		</xsl:choose>
	</xsl:template>
	<xsl:template match="w:tblBorders">
		<xsl:apply-templates/>
	</xsl:template>
	<xsl:template match="w:tcBorders">
		<xsl:apply-templates/>
	</xsl:template>
	-->
    <xsl:template match="w:tblInd">
        <!-- commented tempararily by glu -->
        <!-- <xsl:attribute name="fo:margin-left"><xsl:value-of select="(number(@w:w) div 1440)*2.54"/>cm</xsl:attribute> -->
    </xsl:template>
    <xsl:template match="w:tcPr"/>
    <xsl:template name="genlist">
        <xsl:variable name="currlistid" select="w:pPr/w:listPr/w:ilfo/@w:val"/>
        <xsl:variable name="currlistlvl" select="w:pPr/w:listPr/w:ilvl/@w:val"/>
        <xsl:variable name="firstoccur" select="/descendant::w:pPr[w:listPr/w:ilfo/@w:val = $currlistid][1]"/>
        <xsl:element name="text:ordered-list">
            <xsl:attribute name="text:style-name">List<xsl:value-of select="count($firstoccur/preceding::w:listPr)"/></xsl:attribute>
            <xsl:choose>
                <xsl:when test="preceding::w:pPr/w:listPr/w:ilfo/@w:val = w:pPr/w:listPr/w:ilfo/@w:val">
                    <xsl:attribute name="text:continue-numbering">true</xsl:attribute>
                    <xsl:element name="text:list-item">
                        <xsl:call-template name="levels">
                            <xsl:with-param name="level" select="$currlistlvl"/>
                        </xsl:call-template>
                    </xsl:element>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:element name="text:list-item">
                        <xsl:call-template name="levels">
                            <xsl:with-param name="level" select="$currlistlvl"/>
                        </xsl:call-template>
                    </xsl:element>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:element>
    </xsl:template>
    <xsl:template name="levels">
        <xsl:param name="level"/>
        <xsl:choose>
            <xsl:when test="$level = '0'">
                <xsl:call-template name="commonParagraph"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:element name="text:ordered-list">
                    <xsl:element name="text:list-item">
                        <xsl:call-template name="levels">
                            <xsl:with-param name="level" select="$level -1"/>
                        </xsl:call-template>
                    </xsl:element>
                </xsl:element>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="w:instrText"/>
    <xsl:template match="w:fldChar"/>
</xsl:stylesheet>
