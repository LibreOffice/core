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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:aml="http://schemas.microsoft.com/aml/2001/core" xmlns:office="http://openoffice.org/2000/office" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:style="http://openoffice.org/2000/style" xmlns:text="http://openoffice.org/2000/text" exclude-result-prefixes="w aml">
    <xsl:template name="create-default-paragraph-styles">
        <xsl:variable name="default-paragraph-style" select="w:style[@w:default = 'on' and @w:type = 'paragraph']"/>
        <xsl:if test="$default-paragraph-style">
            <style:default-style style:family="paragraph">
                <style:properties>
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
                    <xsl:if test="$default-paragraph-style/w:rPr/w:sz">
                        <xsl:attribute name="fo:font-size"><xsl:value-of select="$default-paragraph-style/w:rPr/w:sz/@w:val div 2"/>pt</xsl:attribute>
                        <xsl:attribute name="fo:font-size-asian"><xsl:value-of select="$default-paragraph-style/w:rPr/w:sz/@w:val div 2"/>pt</xsl:attribute>
                    </xsl:if>
                    <xsl:if test="$default-paragraph-style/w:rPr/w:sz-cs">
                        <xsl:attribute name="fo:font-size-complex"><xsl:value-of select="$default-paragraph-style/w:rPr/w:sz-cs/@w:val div 2"/>pt</xsl:attribute>
                    </xsl:if>
                    <!-- if not defined default font size in Word, make it out as 10pt. glu -->
                    <xsl:if test="not($default-paragraph-style/w:rPr/w:sz or w:rPr/w:sz-cs)">
                        <xsl:attribute name="fo:font-size">10pt</xsl:attribute>
                        <xsl:attribute name="fo:font-size-asian">10pt</xsl:attribute>
                        <xsl:attribute name="fo:font-size-complex">10pt</xsl:attribute>
                    </xsl:if>
                    <xsl:if test="$default-paragraph-style/w:rPr/w:lang">
                        <xsl:if test="$default-paragraph-style/w:rPr/w:lang/@w:val">
                            <xsl:attribute name="fo:language"><xsl:value-of select="substring-before( $default-paragraph-style/w:rPr/w:lang/@w:val, '-')"/></xsl:attribute>
                            <xsl:attribute name="fo:country"><xsl:value-of select="substring-after( $default-paragraph-style/w:rPr/w:lang/@w:val, '-')"/></xsl:attribute>
                        </xsl:if>
                        <xsl:if test="$default-paragraph-style/w:rPr/w:lang/@w:fareast">
                            <xsl:attribute name="fo:language-asian"><xsl:value-of select="substring-before( $default-paragraph-style/w:rPr/w:lang/@w:fareast, '-')"/></xsl:attribute>
                            <xsl:attribute name="fo:country-asian"><xsl:value-of select="substring-after( $default-paragraph-style/w:rPr/w:lang/@w:fareast, '-')"/></xsl:attribute>
                        </xsl:if>
                        <xsl:if test="$default-paragraph-style/w:rPr/w:lang/@w:bidi">
                            <xsl:attribute name="fo:language-complex"><xsl:value-of select="substring-before( $default-paragraph-style/w:rPr/w:lang/@w:bidi, '-')"/></xsl:attribute>
                            <xsl:attribute name="fo:country-complex"><xsl:value-of select="substring-after( $default-paragraph-style/w:rPr/w:lang/@w:bidi, '-')"/></xsl:attribute>
                        </xsl:if>
                    </xsl:if>
                    <xsl:attribute name="style:tab-stop-distance"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(/w:wordDocument/w:docPr/w:defaultTabStop/@w:val,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                </style:properties>
            </style:default-style>
        </xsl:if>
    </xsl:template>
    <xsl:template name="create-default-text-styles">
        <style:style style:name="Numbering Symbols" style:family="text"/>
        <style:style style:name="Bullet Symbols" style:family="text">
            <style:properties style:font-name="StarSymbol" fo:font-size="12pt" style:font-name-asian="StarSymbol" style:font-size-asian="12pt" style:font-name-complex="StarSymbol" style:font-size-complex="12pt"/>
        </style:style>
    </xsl:template>
    <xsl:template match="w:p" mode="style">
        <xsl:variable name="paragraph-number">
            <xsl:number from="/w:wordDocument/w:body" level="any" count="w:p"/>
        </xsl:variable>
        <xsl:variable name="section-property-number" select="count(preceding::w:sectPr)"/>
        <xsl:variable name="last-section-property" select="preceding::w:pPr/w:sectPr[1]"/>
        <style:style style:family="paragraph" style:name="P{$paragraph-number}">
            <xsl:if test="w:pPr/w:pStyle">
                <xsl:attribute name="style:parent-style-name"><xsl:value-of select="w:pPr/w:pStyle/@w:val"/></xsl:attribute>
            </xsl:if>
            <xsl:choose>
                <xsl:when test="generate-id($last-section-property[last()]/following::w:p[1]) = generate-id(.) and not(ancestor::w:sectPr)">
                    <xsl:attribute name="style:master-page-name">Standard<xsl:value-of select="$section-property-number + 1"/></xsl:attribute>
                </xsl:when>
                <xsl:when test="$paragraph-number = 1">
                    <xsl:attribute name="style:master-page-name">Standard1</xsl:attribute>
                </xsl:when>
            </xsl:choose>
            <style:properties>
                <xsl:apply-templates select="w:pPr"/>
            </style:properties>
        </style:style>
        <xsl:if test="w:r/w:br/@w:type='page'">
            <style:style style:family="paragraph" style:name="P{$paragraph-number}page-break">
                <xsl:if test="w:pPr/w:pStyle">
                    <xsl:attribute name="style:parent-style-name"><xsl:value-of select="w:pPr/w:pStyle/@w:val"/></xsl:attribute>
                </xsl:if>
                <xsl:choose>
                    <xsl:when test="generate-id($last-section-property[last()]/following::w:p[1]) = generate-id(.) and not(ancestor::w:sectPr)">
                        <xsl:attribute name="style:master-page-name">Standard<xsl:value-of select="$section-property-number + 1"/></xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$paragraph-number = 1">
                        <xsl:attribute name="style:master-page-name">Standard1</xsl:attribute>
                    </xsl:when>
                </xsl:choose>
                <style:properties fo:break-before="page">
                    <xsl:apply-templates select="w:pPr"/>
                </style:properties>
            </style:style>
        </xsl:if>
        <xsl:if test="w:r/w:br/@w:type='column'">
            <style:style style:family="paragraph" style:name="P{$paragraph-number}column-break">
                <xsl:if test="w:pPr/w:pStyle">
                    <xsl:attribute name="style:parent-style-name"><xsl:value-of select="w:pPr/w:pStyle/@w:val"/></xsl:attribute>
                </xsl:if>
                <xsl:choose>
                    <xsl:when test="generate-id($last-section-property[last()]/following::w:p[1]) = generate-id(.) and not(ancestor::w:sectPr)">
                        <xsl:attribute name="style:master-page-name">Standard<xsl:value-of select="$section-property-number + 1"/></xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$paragraph-number = 1">
                        <xsl:attribute name="style:master-page-name">Standard1</xsl:attribute>
                    </xsl:when>
                </xsl:choose>
                <style:properties fo:break-before="column">
                    <xsl:apply-templates select="w:pPr"/>
                </style:properties>
            </style:style>
        </xsl:if>
    </xsl:template>
    <xsl:template match="w:pPr">
        <xsl:if test="w:ind/@w:left">
            <xsl:attribute name="fo:margin-left"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:ind/@w:left, 'dxa')"/></xsl:call-template>cm</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:ind/@w:right">
            <xsl:attribute name="fo:margin-right"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:ind/@w:right, 'dxa')"/></xsl:call-template>cm</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:ind/@w:first-line">
            <xsl:attribute name="fo:text-indent"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:ind/@w:first-line, 'dxa')"/></xsl:call-template>cm</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:ind/@w:hanging">
            <xsl:attribute name="fo:text-indent"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat('-',w:ind/@w:hanging, 'dxa')"/></xsl:call-template>cm</xsl:attribute>
        </xsl:if>
        <!-- bi-directional support-->
        <xsl:if test="w:bidi">
            <xsl:attribute name="style:writing-mode">rl-tb</xsl:attribute>
            <xsl:attribute name="fo:text-align">end</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:jc">
            <xsl:choose>
                <xsl:when test="w:jc/@w:val = 'center'">
                    <xsl:attribute name="fo:text-align">center</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:jc/@w:val = 'left'">
                    <xsl:choose>
                        <xsl:when test="w:bidi">
                            <xsl:attribute name="fo:text-align">end</xsl:attribute>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:attribute name="fo:text-align">start</xsl:attribute>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:when>
                <xsl:when test="w:jc/@w:val = 'right'">
                    <xsl:choose>
                        <xsl:when test="w:bidi">
                            <xsl:attribute name="fo:text-align">start</xsl:attribute>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:attribute name="fo:text-align">end</xsl:attribute>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:attribute name="fo:text-align">justify</xsl:attribute>
                    <xsl:attribute name="style:justify-single-word">false</xsl:attribute>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
        <xsl:attribute name="style:auto-text-indent">false</xsl:attribute>
        <xsl:if test="w:spacing">
            <xsl:choose>
                <xsl:when test="w:spacing/@w:line-rule = 'at-least'">
                    <xsl:attribute name="style:line-height-at-least"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:spacing/@w:line, 'dxa')"/></xsl:call-template>cm</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:spacing/@w:line-rule = 'auto'">
                    <xsl:attribute name="fo:line-height"><xsl:value-of select="round(w:spacing/@w:line div 240 * 100)"/>%</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:spacing/@w:line-rule = 'exact'">
                    <xsl:attribute name="fo:line-height"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:spacing/@w:line, 'dxa')"/></xsl:call-template>cm</xsl:attribute>
                </xsl:when>
            </xsl:choose>
            <xsl:if test="w:spacing/@w:before">
                <xsl:attribute name="fo:margin-top"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:spacing/@w:before, 'dxa')"/></xsl:call-template>cm</xsl:attribute>
            </xsl:if>
            <xsl:if test="w:spacing/@w:after">
                <xsl:attribute name="fo:margin-bottom"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:spacing/@w:after, 'dxa')"/></xsl:call-template>cm</xsl:attribute>
            </xsl:if>
        </xsl:if>
        <xsl:if test="w:shd">
            <xsl:variable name="background-color">
                <xsl:choose>
                    <xsl:when test="string-length(w:shd/@w:fill) = 6">
                        <xsl:value-of select="concat('#', w:shd/@w:fill)"/>
                    </xsl:when>
                    <xsl:otherwise>#000000</xsl:otherwise>
                </xsl:choose>
            </xsl:variable>
            <xsl:choose>
                <xsl:when test="w:shd/@w:val = 'solid'">
                    <xsl:attribute name="fo:background-color"><xsl:value-of select="$background-color"/></xsl:attribute>
                </xsl:when>
                <!-- patterns are necessary in the future. glu -->
                <xsl:otherwise/>
            </xsl:choose>
        </xsl:if>
        <xsl:if test="w:pageBreakBefore">
            <xsl:attribute name="fo:break-before">page</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:tabs">
            <xsl:element name="style:tab-stops">
                <xsl:for-each select="w:tabs/w:tab">
                    <xsl:element name="style:tab-stop">
                        <xsl:attribute name="style:position"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(@w:pos, 'dxa')"/></xsl:call-template>cm</xsl:attribute>
                        <xsl:choose>
                            <xsl:when test="@w:val = 'decimal'">
                                <xsl:attribute name="style:type">char</xsl:attribute>
                                <xsl:attribute name="style:char"/>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="style:type"><xsl:value-of select="@w:val"/></xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                        <xsl:if test="@w:leader">
                            <xsl:attribute name="style:leader-char"><xsl:choose><xsl:when test="hyphen">-</xsl:when><xsl:when test="underscore">_</xsl:when><xsl:when test="dot">.</xsl:when></xsl:choose></xsl:attribute>
                        </xsl:if>
                        <xsl:if test="@style:leader-char">
                            <xsl:choose>
                                <xsl:when test="@style:leader-char='-'">
                                    <xsl:attribute name="w:leader">hyphen</xsl:attribute>
                                </xsl:when>
                                <xsl:when test="@style:leader-char='_'">
                                    <xsl:attribute name="w:leader">underscore</xsl:attribute>
                                </xsl:when>
                                <xsl:when test="@style:leader-char='.'">
                                    <xsl:attribute name="w:leader">dot</xsl:attribute>
                                </xsl:when>
                                <xsl:when test="@style:leader-char='·'">
                                    <xsl:attribute name="w:leader">middle-dot</xsl:attribute>
                                </xsl:when>
                            </xsl:choose>
                        </xsl:if>
                    </xsl:element>
                </xsl:for-each>
            </xsl:element>
        </xsl:if>
        <xsl:if test="w:keepNext">
            <xsl:attribute name="fo:keep-with-next">true</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:keepLines">
            <xsl:attribute name="style:break-inside">avoid</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:widowControl='on'">
            <xsl:attribute name="fo:widows">2</xsl:attribute>
            <xsl:attribute name="fo:orphans">2</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:suppressAutoHyphens">
            <xsl:attribute name="fo:hyphenate">false</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:kinsoku/@w:val='off'">
            <xsl:attribute name="style:line-break">normal</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:overflowPunct/@w:val='off'">
            <xsl:attribute name="style:punctuation-wrap">simple</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:autoSpaceDE/@w:val='off' or w:autoSpaceDN/@w:val='off'">
            <xsl:attribute name="style:text-autospace">none</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:textAlignment">
            <xsl:choose>
                <xsl:when test="w:textAlignment/@w:val='center'">
                    <xsl:attribute name="style:vertical-align">middle</xsl:attribute>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:attribute name="style:vertical-align"><xsl:value-of select="w:textAlignment/@w:val"/></xsl:attribute>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
        <xsl:if test="w:pBdr">
            <xsl:if test="w:pBdr/w:top">
                <xsl:call-template name="get-table-border">
                    <xsl:with-param name="style-pos" select="'top'"/>
                    <xsl:with-param name="style-position-0" select="w:pBdr/w:top"/>
                </xsl:call-template>
                <xsl:attribute name="fo:padding-top"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pBdr/w:top/@w:space,'pt')"/></xsl:call-template>cm</xsl:attribute>
            </xsl:if>
            <xsl:if test="w:pBdr/w:left">
                <xsl:call-template name="get-table-border">
                    <xsl:with-param name="style-pos" select="'left'"/>
                    <xsl:with-param name="style-position-0" select="w:pBdr/w:left"/>
                </xsl:call-template>
                <xsl:attribute name="fo:padding-left"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pBdr/w:left/@w:space,'pt')"/></xsl:call-template>cm</xsl:attribute>
            </xsl:if>
            <xsl:if test="w:pBdr/w:right">
                <xsl:call-template name="get-table-border">
                    <xsl:with-param name="style-pos" select="'right'"/>
                    <xsl:with-param name="style-position-0" select="w:pBdr/w:right"/>
                </xsl:call-template>
                <xsl:attribute name="fo:padding-right"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pBdr/w:right/@w:space,'pt')"/></xsl:call-template>cm</xsl:attribute>
            </xsl:if>
            <xsl:if test="w:pBdr/w:bottom">
                <xsl:call-template name="get-table-border">
                    <xsl:with-param name="style-pos" select="'bottom'"/>
                    <xsl:with-param name="style-position-0" select="w:pBdr/w:bottom"/>
                </xsl:call-template>
                <xsl:attribute name="fo:padding-bottom"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pBdr/w:bottom/@w:space,'pt')"/></xsl:call-template>cm</xsl:attribute>
            </xsl:if>
            <xsl:if test="w:pBdr/*/@w:shadow='on'">
                <xsl:attribute name="style:shadow">#000000 0.15cm 0.15cm</xsl:attribute>
            </xsl:if>
        </xsl:if>
        <xsl:if test="w:snapToGrid/@w:val='off'">
            <xsl:attribute name="style:snap-to-layout-grid">false</xsl:attribute>
        </xsl:if>
        <xsl:apply-templates select="w:rPr"/>
    </xsl:template>
    <xsl:template match="w:rPr" mode="style">
        <xsl:element name="style:style">
            <xsl:attribute name="style:name">T<xsl:number from="/w:wordDocument/w:body" level="any" count="w:rPr"/></xsl:attribute>
            <xsl:attribute name="style:family">text</xsl:attribute>
            <xsl:if test="w:rStyle">
                <xsl:attribute name="style:parent-style-name"><xsl:value-of select="w:rStyle/@w:val"/></xsl:attribute>
            </xsl:if>
            <xsl:element name="style:properties">
                <xsl:apply-templates select="current()"/>
            </xsl:element>
        </xsl:element>
    </xsl:template>
    <xsl:template match="w:rPr">
        <xsl:if test="w:rFonts">
            <xsl:if test="w:rFonts/@w:ascii">
                <xsl:attribute name="style:font-name"><xsl:value-of select="w:rFonts/@w:ascii"/></xsl:attribute>
                <xsl:if test="ancestor::w:body">
                    <xsl:attribute name="style:font-name-asian"><xsl:value-of select="w:rFonts/@w:ascii"/></xsl:attribute>
                    <xsl:attribute name="style:font-name-complex"><xsl:value-of select="w:rFonts/@w:ascii"/></xsl:attribute>
                </xsl:if>
            </xsl:if>
            <xsl:if test="ancestor::w:styles">
                <xsl:if test="w:rFonts/@w:fareast">
                    <xsl:attribute name="style:font-name-asian"><xsl:value-of select="w:rFonts/@w:fareast"/></xsl:attribute>
                </xsl:if>
                <xsl:if test="w:rFonts/@w:cs">
                    <xsl:attribute name="style:font-name-complex"><xsl:value-of select="w:rFonts/@w:cs"/></xsl:attribute>
                </xsl:if>
            </xsl:if>
        </xsl:if>
        <xsl:if test="w:b">
            <xsl:attribute name="fo:font-weight">bold</xsl:attribute>
            <xsl:attribute name="fo:font-weight-asian">bold</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:b-cs">
            <xsl:attribute name="fo:font-weight-complex">bold</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:i">
            <xsl:attribute name="fo:font-style">italic</xsl:attribute>
            <xsl:attribute name="fo:font-style-asian">italic</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:i-cs">
            <xsl:attribute name="fo:font-style-complex">italic</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:caps">
            <xsl:attribute name="fo:text-transform">uppercase</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:smallCaps">
            <xsl:attribute name="fo:font-variant">small-caps</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:strike">
            <xsl:attribute name="style:text-crossing-out">single-line</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:dstrike">
            <xsl:attribute name="style:text-crossing-out">double-line</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:outline">
            <xsl:attribute name="style:text-outline">true</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:shadow">
            <xsl:attribute name="style:text-shadow">1pt 1pt</xsl:attribute>
            <xsl:attribute name="fo:text-shadow">1pt 1pt</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:imprint">
            <xsl:attribute name="style:font-relief">engraved</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:emboss">
            <xsl:attribute name="style:font-relief">embossed</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:vanish">
            <xsl:attribute name="text:display">true</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:color[not(@w:val = 'auto')]">
            <xsl:attribute name="fo:color">#<xsl:value-of select="w:color/@w:val"/></xsl:attribute>
        </xsl:if>
        <xsl:if test="w:spacing">
            <xsl:attribute name="fo:letter-spacing"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:spacing/@w:val,'dxa')"/></xsl:call-template>cm</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:w/@w:val">
            <xsl:attribute name="style:text-scale"><xsl:value-of select="w:w/@w:val"/></xsl:attribute>
        </xsl:if>
        <xsl:if test="w:vertAlign or w:position">
            <xsl:variable name="height">
                <xsl:choose>
                    <xsl:when test="w:vertAlign[@w:val = 'superscript' or @w:val = 'subscript']">58%</xsl:when>
                    <xsl:otherwise>100%</xsl:otherwise>
                </xsl:choose>
            </xsl:variable>
            <xsl:variable name="position">
                <xsl:choose>
                    <xsl:when test="w:position">
                        <!-- con't get font height easily, so just set w:val as percentage. glu -->
                        <xsl:value-of select="concat( w:position/@w:val, '%')"/>
                    </xsl:when>
                    <xsl:when test="w:vertAlign[@w:val = 'superscript']">super</xsl:when>
                    <xsl:when test="w:vertAlign[@w:val = 'subscript']">sub</xsl:when>
                </xsl:choose>
            </xsl:variable>
            <xsl:attribute name="style:text-position"><xsl:value-of select="concat($position, ' ', $height)"/></xsl:attribute>
        </xsl:if>
        <xsl:if test="w:sz">
            <xsl:attribute name="fo:font-size"><xsl:value-of select="w:sz/@w:val div 2"/>pt</xsl:attribute>
            <xsl:attribute name="fo:font-size-asian"><xsl:value-of select="w:sz/@w:val div 2"/>pt</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:sz-cs">
            <xsl:attribute name="fo:font-size-complex"><xsl:value-of select="w:sz-cs/@w:val div 2"/>pt</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:highlight">
            <xsl:choose>
                <xsl:when test="w:highlight/@w:val='black'">
                    <xsl:attribute name="style:text-background-color">#000000</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='yellow'">
                    <xsl:attribute name="style:text-background-color">#ffff00</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='green'">
                    <xsl:attribute name="style:text-background-color">#00ff00</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='cyan'">
                    <xsl:attribute name="style:text-background-color">#00ffff</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='magenta'">
                    <xsl:attribute name="style:text-background-color">#ff00ff</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='blue'">
                    <xsl:attribute name="style:text-background-color">#0000ff</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='red'">
                    <xsl:attribute name="style:text-background-color">#ff0000</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='dark-blue'">
                    <xsl:attribute name="style:text-background-color">#000080</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='dark-cyan'">
                    <xsl:attribute name="style:text-background-color">#008080</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='dark-green'">
                    <xsl:attribute name="style:text-background-color">#008000</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='dark-magenta'">
                    <xsl:attribute name="style:text-background-color">#800080</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='dark-red'">
                    <xsl:attribute name="style:text-background-color">#800000</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='dark-yellow'">
                    <xsl:attribute name="style:text-background-color">#808000</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='dark-gray'">
                    <xsl:attribute name="style:text-background-color">#808080</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:highlight/@w:val='light-gray'">
                    <xsl:attribute name="style:text-background-color">#c0c0c0</xsl:attribute>
                </xsl:when>
            </xsl:choose>
        </xsl:if>
        <xsl:if test="w:u">
            <xsl:choose>
                <xsl:when test="w:u/@w:val = 'words'">
                    <xsl:attribute name="style:text-underline">single</xsl:attribute>
                    <xsl:attribute name="fo:score-spaces">false</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:u/@w:val = 'thick'">
                    <xsl:attribute name="style:text-underline">bold</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:u/@w:val = 'dotted-heavy'">
                    <xsl:attribute name="style:text-underline">bold-dotted</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:u/@w:val = 'dashed-heavy'">
                    <xsl:attribute name="style:text-underline">bold-dash</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:u/@w:val = 'dash-long'">
                    <xsl:attribute name="style:text-underline">long-dash</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:u/@w:val = 'dash-long-heavy'">
                    <xsl:attribute name="style:text-underline">bold-long-dash</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:u/@w:val = 'dash-dot-heavy'">
                    <xsl:attribute name="style:text-underline">bold-dot-dash</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:u/@w:val = 'dash-dot-dot-heavy'">
                    <xsl:attribute name="style:text-underline">bold-dot-dot-dash</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:u/@w:val = 'wavy-heavy'">
                    <xsl:attribute name="style:text-underline">bold-wave</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:u/@w:val = 'wavy-double'">
                    <xsl:attribute name="style:text-underline">double-wave</xsl:attribute>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:attribute name="style:text-underline"><xsl:value-of select="w:u/@w:val"/></xsl:attribute>
                </xsl:otherwise>
            </xsl:choose>
            <xsl:if test="w:u/@w:color and not(w:u/@w:color = 'auto')">
                <xsl:attribute name="style:text-underline-color">#<xsl:value-of select="w:u/@w:color"/></xsl:attribute>
            </xsl:if>
        </xsl:if>
        <xsl:if test="w:effect[@w:val = 'blink-background']">
            <xsl:attribute name="style:text-blinking">true</xsl:attribute>
            <xsl:attribute name="style:text-background-color">#000000</xsl:attribute>
        </xsl:if>
        <xsl:if test="w:shd and not(w:highlight)">
            <xsl:if test="string-length(w:shd/@w:fill) = 6">
                <xsl:attribute name="style:text-background-color">#<xsl:value-of select="w:shd/@w:fill"/></xsl:attribute>
            </xsl:if>
        </xsl:if>
        <xsl:if test="w:em">
            <xsl:choose>
                <xsl:when test="w:em/@w:val = 'comma'">
                    <xsl:attribute name="style:text-emphasize">accent below</xsl:attribute>
                </xsl:when>
                <xsl:when test="w:em/@w:val = 'under-dot'">
                    <xsl:attribute name="style:text-emphasize">disc below</xsl:attribute>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:attribute name="style:text-emphasize"><xsl:value-of select="concat( w:em/@w:val, ' below')"/></xsl:attribute>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
        <xsl:if test="w:lang">
            <xsl:if test="w:lang/@w:val">
                <xsl:attribute name="fo:language"><xsl:value-of select="substring-before( w:lang/@w:val, '-')"/></xsl:attribute>
                <xsl:attribute name="fo:country"><xsl:value-of select="substring-after( w:lang/@w:val, '-')"/></xsl:attribute>
            </xsl:if>
            <xsl:if test="w:lang/@w:fareast">
                <xsl:attribute name="fo:language-asian"><xsl:value-of select="substring-before( w:lang/@w:fareast, '-')"/></xsl:attribute>
                <xsl:attribute name="fo:country-asian"><xsl:value-of select="substring-after( w:lang/@w:fareast, '-')"/></xsl:attribute>
            </xsl:if>
            <xsl:if test="w:lang/@w:bidi">
                <xsl:attribute name="fo:language-complex"><xsl:value-of select="substring-before( w:lang/@w:bidi, '-')"/></xsl:attribute>
                <xsl:attribute name="fo:country-complex"><xsl:value-of select="substring-after( w:lang/@w:bidi, '-')"/></xsl:attribute>
            </xsl:if>
        </xsl:if>
    </xsl:template>
    <xsl:template match="w:p">
        <xsl:choose>
            <!-- because word treats page breaks as separate tags, we must split some paragraphs up so that we can 
			give the sub para a fo:break-before ="page" or column attribute. -->
            <xsl:when test="w:r[w:br/@w:type='page' or w:br/@w:type='column']">
                <xsl:call-template name="process-breaks-in-paragraph"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:call-template name="process-common-paragraph"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="process-breaks-in-paragraph">
        <xsl:variable name="textruns-with-break" select="w:r[w:br/@w:type='page' or w:br/@w:type='column']"/>
        <xsl:call-template name="create-sub-paragraph">
            <xsl:with-param name="textruns" select="$textruns-with-break[1]/preceding-sibling::w:r"/>
        </xsl:call-template>
        <xsl:for-each select="$textruns-with-break">
            <xsl:variable name="break-position" select="position()"/>
            <xsl:call-template name="create-sub-paragraph">
                <xsl:with-param name="textruns" select="following-sibling::w:r[not(w:br/@w:type='page' or w:br/@w:type='column') and (count(preceding::w:r[w:br/@w:type='page' or w:br/@w:type='column']) = $break-position)]"/>
                <xsl:with-param name="textruns-with-break" select="current()"/>
            </xsl:call-template>
        </xsl:for-each>
    </xsl:template>
    <xsl:template name="create-sub-paragraph">
        <xsl:param name="textruns"/>
        <xsl:param name="textruns-with-break"/>
        <xsl:if test="$textruns or $textruns-with-break">
            <xsl:variable name="curr-num">
                <xsl:number from="/w:wordDocument/w:body" level="any" count="w:p"/>
            </xsl:variable>
            <text:p>
                <xsl:choose>
                    <xsl:when test="$textruns-with-break">
                        <xsl:attribute name="text:style-name"><xsl:value-of select="concat('P',$curr-num,w:br/@w:type, '-break')"/></xsl:attribute>
                        <xsl:apply-templates select="$textruns-with-break"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="text:style-name"><xsl:value-of select="concat( 'P', $curr-num)"/></xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
                <xsl:if test="$textruns">
                    <xsl:apply-templates select="$textruns"/>
                </xsl:if>
            </text:p>
        </xsl:if>
    </xsl:template>
    <xsl:template name="process-common-paragraph">
        <xsl:variable name="heading-or-paragraph">
            <xsl:choose>
                <xsl:when test="key('heading-style', w:pPr/w:pStyle/@w:val)">text:h</xsl:when>
                <xsl:otherwise>text:p</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:element name="{$heading-or-paragraph}">
            <xsl:if test="$heading-or-paragraph = 'text:h'">
                <xsl:attribute name="text:level"><xsl:value-of select="key('heading-style',w:pPr/w:pStyle/@w:val)/w:pPr/w:outlineLvl/@w:val + 1"/></xsl:attribute>
            </xsl:if>
            <xsl:variable name="curr-num">
                <xsl:number from="/w:wordDocument/w:body" level="any" count="w:p"/>
            </xsl:variable>
            <xsl:attribute name="text:style-name"><xsl:value-of select="concat( 'P', $curr-num)"/></xsl:attribute>
            <!-- call the template to generate bookmark-start matching nodes before current. cp tom chen. -->
            <xsl:variable name="bm-node" select="preceding::aml:annotation[@w:type = 'Word.Bookmark.Start']"/>
            <xsl:variable name="wp-node" select="preceding::w:p[1]"/>
            <xsl:call-template name="start-end-bookmark">
                <xsl:with-param name="type" select="'start'"/>
                <xsl:with-param name="bm-node" select="$bm-node"/>
                <xsl:with-param name="wp-node" select="$wp-node"/>
            </xsl:call-template>
            <xsl:apply-templates select=".//w:r | w:fldSimple | w:hlink"/>
            <xsl:variable name="bm-node2" select="following::aml:annotation[@w:type = 'Word.Bookmark.End']"/>
            <xsl:variable name="wp-node2" select="following::w:p[position() = 1]"/>
            <xsl:call-template name="start-end-bookmark">
                <xsl:with-param name="type" select="'end'"/>
                <xsl:with-param name="bm-node" select="$bm-node2"/>
                <xsl:with-param name="wp-node" select="$wp-node2"/>
            </xsl:call-template>
        </xsl:element>
    </xsl:template>
    <!-- WordML contains multiple w:t within one w:r, so in Star Writer text:span should correspond to w:t glu -->
    <xsl:template match="w:r">
        <!-- handling another type of Hyperlink address in w:body.cp tom chen. 
            Acutally they're from Field features, a complex one. glu 
            <xsl:when test="w:r/w:instrText and contains(w:r/w:instrText , 'HYPERLINK')">
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
                <text:p>
                    <text:a>
                        <xsl:attribute name="xlink:type">simple</xsl:attribute>
                        <xsl:attribute name="xlink:href"><xsl:value-of select="concat( $hyper-dest, $hyper-bookmark)"/></xsl:attribute>
                        <xsl:apply-templates/>
                    </text:a>
                </text:p>
            </xsl:when>
            -->
        <xsl:if test="w:br[@w:type='text-wrapping' or not(@w:type)]">
            <text:line-break/>
        </xsl:if>
        <xsl:choose>
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
                            <xsl:variable name="position">
                                <xsl:number from="/w:wordDocument/w:body" level="any" count="w:rPr"/>
                            </xsl:variable>
                            <xsl:attribute name="text:style-name">T<xsl:value-of select="$position + 1"/></xsl:attribute>
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
    <!-- strange enough, WordML allows bookmarks out of w:p, but this isn't supported by StarWriter. To be patched. :( glu -->
    <xsl:template match="aml:annotation[@w:type = 'Word.Bookmark.Start']">
        <xsl:variable name="id" select="@aml:id"/>
        <text:bookmark-start text:name="{@w:name}"/>
    </xsl:template>
    <xsl:template match="aml:annotation[@w:type = 'Word.Bookmark.End']">
        <xsl:variable name="id" select="@aml:id"/>
        <text:bookmark-end text:name="{preceding::aml:annotation[@aml:id = $id]/@w:name}"/>
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
    <xsl:template name="start-end-bookmark">
        <!--?? generate bookmark before/after the tag w:p by recursion untill another type of aml:annotation. type: start/end. cp tom chen. -->
        <xsl:param name="type"/>
        <xsl:param name="bm-node"/>
        <xsl:param name="wp-node"/>
        <xsl:for-each select="$bm-node">
            <xsl:variable name="curr" select="."/>
            <xsl:for-each select="$wp-node[1]">
                <xsl:if test="not(preceding::aml:annotation/@aml:id = $curr/@aml:id) and not(./aml:annotation[@aml:id = $curr/@aml:id]) and ($type = 'start')">
                    <text:bookmark-start text:name="{$bm-node[@aml:id = $curr/@aml:id]/@w:name}"/>
                </xsl:if>
                <xsl:if test="not(following::aml:annotation/@aml:id = $curr/@aml:id) and not(./aml:annotation[@aml:id = $curr/@aml:id]) and ($type = 'end')">
                    <text:bookmark-end text:name="{preceding::aml:annotation[@aml:id = $curr/@aml:id]/@w:name}"/>
                </xsl:if>
            </xsl:for-each>
        </xsl:for-each>
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
    <xsl:template match="w:tab">
        <xsl:element name="text:tab-stop"/>
    </xsl:template>
    <xsl:template match="w:instrText"/>
    <xsl:template match="w:fldChar"/>
</xsl:stylesheet>
