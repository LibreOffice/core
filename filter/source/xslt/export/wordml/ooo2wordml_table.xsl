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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:table="http://openoffice.org/2000/table" xmlns:style="http://openoffice.org/2000/style" xmlns:text="http://openoffice.org/2000/text" exclude-result-prefixes="fo table style text">
    <xsl:key name="table-style" match="style:style[@style:family='table']" use="@style:name"/>
    <xsl:key name="table-column-style" match="style:style[@style:family='table-column']" use="@style:name"/>
    <xsl:key name="table-row-style" match="style:style[@style:family='table-row']" use="@style:name"/>
    <xsl:key name="table-cell-style" match="style:style[@style:family='table-cell']" use="@style:name"/>
    <xsl:template match="style:properties" mode="table">
        <xsl:param name="within-body"/>
        <xsl:if test="$within-body = 'yes'">
            <w:tblW>
                <xsl:choose>
                    <xsl:when test="@style:rel-width">
                        <xsl:attribute name="w:w"><xsl:value-of select="substring-before(@style:rel-width, '%') * 50"/></xsl:attribute>
                        <xsl:attribute name="w:type">pct</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="@style:width">
                        <xsl:attribute name="w:w"><xsl:call-template name="convert2dxa"><xsl:with-param name="value" select="@style:width"/></xsl:call-template></xsl:attribute>
                        <xsl:attribute name="w:type">dxa</xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="w:w">0</xsl:attribute>
                        <xsl:attribute name="w:type">auto</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
            </w:tblW>
        </xsl:if>
        <w:tblInd>
            <xsl:choose>
                <xsl:when test="@fo:margin-left">
                    <xsl:attribute name="w:w"><xsl:call-template name="convert2dxa"><xsl:with-param name="value" select="@fo:margin-left"/></xsl:call-template></xsl:attribute>
                    <xsl:attribute name="w:type">dxa</xsl:attribute>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:attribute name="w:w">0</xsl:attribute>
                    <xsl:attribute name="w:type">auto</xsl:attribute>
                </xsl:otherwise>
            </xsl:choose>
        </w:tblInd>
        <xsl:if test="@table:align">
            <w:jc>
                <xsl:choose>
                    <xsl:when test="@table:align = 'left' or @table:align= 'center' or @table:align = 'right'">
                        <xsl:attribute name="w:val"><xsl:value-of select="@table:align"/></xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="w:val">left</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
            </w:jc>
        </xsl:if>
    </xsl:template>
    <xsl:template match="table:table">
        <w:tbl>
            <w:tblPr>
                <w:tblStyle w:val="{@table:style-name}"/>
                <xsl:apply-templates select="key('table-style', @table:style-name)/style:properties" mode="table">
                    <xsl:with-param name="within-body" select="'yes'"/>
                </xsl:apply-templates>
            </w:tblPr>
            <w:tblGrid>
                <xsl:apply-templates select="table:table-column"/>
            </w:tblGrid>
            <xsl:apply-templates select=".//table:table-row"/>
        </w:tbl>
    </xsl:template>
    <xsl:template match="table:table-column">
        <xsl:variable name="column-width" select="key('table-column-style', @table:style-name)/style:properties/@style:column-width"/>
        <xsl:variable name="column-width-in-twip">
            <xsl:call-template name="convert2dxa">
                <xsl:with-param name="value" select="$column-width"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:choose>
            <!-- if table:table-column has attribute table:number-columns-repeated, then call the recursion
			temple repeat-gridcol to produce multiple w:gridCol in MS word. Gary.Yang   -->
            <xsl:when test="@table:number-columns-repeated">
                <xsl:call-template name="repeat-gridcol">
                    <xsl:with-param name="grid-repeat-count" select="@table:number-columns-repeated"/>
                    <xsl:with-param name="column-width" select="$column-width-in-twip"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <w:gridCol w:w="{$column-width-in-twip}"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <!--recursion template for produce multiple w:gridCol  Gary.Yang-->
    <xsl:template name="repeat-gridcol">
        <xsl:param name="grid-repeat-count"/>
        <xsl:param name="column-width"/>
        <xsl:if test="$grid-repeat-count &gt; 0">
            <w:gridCol w:w="{$column-width}"/>
            <xsl:call-template name="repeat-gridcol">
                <xsl:with-param name="grid-repeat-count" select="$grid-repeat-count - 1"/>
                <xsl:with-param name="column-width" select="$column-width"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>
    <xsl:template match="table:table-row">
        <xsl:element name="w:tr">
            <xsl:element name="w:trPr">
                <xsl:if test="parent::table:table-header-rows">
                    <!-- fix for  Issue 32034-->
                    <w:tblHeader>on</w:tblHeader>
                  </xsl:if>
              
            <xsl:variable name="row-height" select="key('table-row-style', @table:style-name)/style:properties/@style:row-height"/>
            <xsl:if test="$row-height">
                <w:trHeight>
                    <xsl:attribute name="w:val"><xsl:call-template name="convert2dxa"><xsl:with-param name="value" select="$row-height"/></xsl:call-template></xsl:attribute>
                </w:trHeight>
            </xsl:if>
            </xsl:element> <!--end of w:trPr-->
            <xsl:apply-templates select="table:table-cell "/>
        </xsl:element>
    </xsl:template>
    <xsl:template match="table:table-cell ">
        <xsl:element name="w:tc">
            <xsl:element name="w:tcPr">
                <!-- set w:type to auto that makes the cell width auto fit the content. Gary Yang -->
                <w:tcW w:w="0" w:type="auto"/>
                <xsl:if test="@table:number-columns-spanned">
                    <w:gridSpan w:val="{@table:number-columns-spanned}"/>
                </xsl:if>
                <xsl:variable name="cell-style-properties" select="key('table-cell-style', @table:style-name)/style:properties"/>
                <xsl:if test="$cell-style-properties/@fo:background-color">
                    <w:shd w:val="solid" w:color="{substring-after($cell-style-properties/@fo:background-color,'#')}"/>
                </xsl:if>
                <xsl:if test="$cell-style-properties/@fo:vertical-align">
                    <xsl:choose>
                        <xsl:when test="$cell-style-properties/@fo:vertical-align = 'middle'">
                            <w:vAlign w:val="center"/>
                        </xsl:when>
                        <xsl:when test="$cell-style-properties/@fo:vertical-align = 'Automatic'">
                            <w:vAlign w:val="both"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <w:vAlign w:val="{$cell-style-properties/@fo:vertical-align}"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:if>
                <w:tcMar>
                    <xsl:if test="$cell-style-properties/@fo:padding-top">
                        <w:top w:type="dxa">
                            <xsl:attribute name="w:w"><xsl:call-template name="convert2dxa"><xsl:with-param name="value" select="$cell-style-properties/@fo:padding-top"/></xsl:call-template></xsl:attribute>
                        </w:top>
                    </xsl:if>
                    <xsl:if test="$cell-style-properties/@fo:padding-bottom">
                        <w:bottom w:type="dxa">
                            <xsl:attribute name="w:w"><xsl:call-template name="convert2dxa"><xsl:with-param name="value" select="$cell-style-properties/@fo:padding-bottom"/></xsl:call-template></xsl:attribute>
                        </w:bottom>
                    </xsl:if>
                    <xsl:if test="$cell-style-properties/@fo:padding-left">
                        <w:left w:type="dxa">
                            <xsl:attribute name="w:w"><xsl:call-template name="convert2dxa"><xsl:with-param name="value" select="$cell-style-properties/@fo:padding-left"/></xsl:call-template></xsl:attribute>
                        </w:left>
                    </xsl:if>
                    <xsl:if test="$cell-style-properties/@fo:padding-right">
                        <w:right w:type="dxa">
                            <xsl:attribute name="w:w"><xsl:call-template name="convert2dxa"><xsl:with-param name="value" select="$cell-style-properties/@fo:padding-right"/></xsl:call-template></xsl:attribute>
                        </w:right>
                    </xsl:if>
                </w:tcMar>
                <!-- the following code is to get the cell borders if they exsits Gary.Yang-->
                <xsl:variable name="border-top" select="$cell-style-properties/@fo:border-top | $cell-style-properties/@fo:border"/>
                <xsl:variable name="border-bottom" select="$cell-style-properties/@fo:border-bottom | $cell-style-properties/@fo:border"/>
                <xsl:variable name="border-left" select="$cell-style-properties/@fo:border-left | $cell-style-properties/@fo:border"/>
                <xsl:variable name="border-right" select="$cell-style-properties/@fo:border-right | $cell-style-properties/@fo:border"/>
                <xsl:variable name="border-line-width-top" select="$cell-style-properties/@style:border-line-width-top | $cell-style-properties/@style:border-line-width "/>
                <xsl:variable name="border-line-width-bottom" select="$cell-style-properties/@style:border-line-width-bottom | $cell-style-properties/@style:border-line-width"/>
                <xsl:variable name="border-line-width-left" select="$cell-style-properties/@style:border-line-width-left | $cell-style-properties/@style:border-line-width"/>
                <xsl:variable name="border-line-width-right" select="$cell-style-properties/@style:border-line-width-right | $cell-style-properties/@style:border-line-width"/>
                <xsl:element name="w:tcBorders">
                    <xsl:if test="$border-top">
                        <xsl:element name="w:top">
                            <xsl:call-template name="get-border">
                                <xsl:with-param name="so-border" select="$border-top"/>
                                <xsl:with-param name="so-border-line-width" select="$border-line-width-top"/>
                                <xsl:with-param name="so-border-position" select=" 'top' "/>
                            </xsl:call-template>
                        </xsl:element>
                    </xsl:if>
                    <xsl:if test="$border-bottom">
                        <xsl:element name="w:bottom">
                            <xsl:call-template name="get-border">
                                <xsl:with-param name="so-border" select="$border-bottom"/>
                                <xsl:with-param name="so-border-line-width" select="$border-line-width-bottom"/>
                                <xsl:with-param name="so-border-position" select=" 'bottom' "/>
                            </xsl:call-template>
                        </xsl:element>
                    </xsl:if>
                    <xsl:if test="$border-left">
                        <xsl:element name="w:left">
                            <xsl:call-template name="get-border">
                                <xsl:with-param name="so-border" select="$border-left"/>
                                <xsl:with-param name="so-border-line-width" select="$border-line-width-left"/>
                                <xsl:with-param name="so-border-position" select=" 'left' "/>
                            </xsl:call-template>
                        </xsl:element>
                    </xsl:if>
                    <xsl:if test="$border-right">
                        <xsl:element name="w:right">
                            <xsl:call-template name="get-border">
                                <xsl:with-param name="so-border" select="$border-right"/>
                                <xsl:with-param name="so-border-line-width" select="$border-line-width-right"/>
                                <xsl:with-param name="so-border-position" select=" 'right' "/>
                            </xsl:call-template>
                        </xsl:element>
                    </xsl:if>
                </xsl:element>
            </xsl:element>
            <xsl:if test="not (*)">
                <w:p></w:p>
            </xsl:if>
            <xsl:apply-templates select=".//text:p"/>
        </xsl:element>
    </xsl:template>
</xsl:stylesheet>
