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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:wx="http://schemas.microsoft.com/office/word/2003/auxHint" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:aml="http://schemas.microsoft.com/aml/2001/core"  xmlns:dt="uuid:C2F41010-65B3-11d1-A29F-00AA00C14882" xmlns:v="urn:schemas-microsoft-com:vml"  xmlns:office="urn:oasis:names:tc:openoffice:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:openoffice:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:openoffice:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:openoffice:xmlns:table:1.0" xmlns:draw="urn:oasis:names:tc:openoffice:xmlns:drawing:1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:openoffice:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:openoffice:xmlns:datastyle:1.0" xmlns:svg="http://www.w3.org/2000/svg" xmlns:chart="urn:oasis:names:tc:openoffice:xmlns:chart:1.0" xmlns:dr3d="urn:oasis:names:tc:openoffice:xmlns:dr3d:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="urn:oasis:names:tc:openoffice:xmlns:form:1.0" xmlns:script="urn:oasis:names:tc:openoffice:xmlns:script:1.0" xmlns:config="urn:oasis:names:tc:openoffice:xmlns:config:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:dom="http://www.w3.org/2001/xml-events" exclude-result-prefixes="w wx aml o dt fo v">
    <xsl:template match="w:footnotePr" mode="config">
        <text:notes-configuration text:note-class="footnote" text:citation-style-name="Footnote_20_Symbol" >
            <xsl:if test="w:pos">
                <xsl:choose>
                    <xsl:when test="w:pos/@w:val = 'beneath-text'">
                        <xsl:attribute name="text:footnotes-position">document</xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="text:footnotes-position">page</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:if>
            <xsl:if test="w:numStart">
                <xsl:choose>
                    <xsl:when test="w:numStart/@w:val - 1 &gt; 0">
                    <xsl:attribute name="text:start-value"><xsl:value-of select="w:numStart/@w:val - 1"/></xsl:attribute></xsl:when>
                    <xsl:otherwise><xsl:attribute name="text:start-value"><xsl:value-of select=" '1' "/></xsl:attribute></xsl:otherwise>
                </xsl:choose>
                <!--xsl:attribute name="text:start-value"><xsl:value-of select="w:numStart/@w:val - 1"/></xsl:attribute-->
            </xsl:if>
            <xsl:if test="w:numFmt">
                <xsl:call-template name="convert-number-format">
                    <xsl:with-param name="number-format" select="w:numFmt/@w:val"/>
                </xsl:call-template>
            </xsl:if>
            <xsl:if test="w:numRestart">
                <xsl:choose>
                    <xsl:when test="w:numRestart/@w:val = 'continuous'">
                        <xsl:attribute name="text:start-numbering-at">document</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="w:numRestart/@w:val = 'each-sect'">
                        <xsl:attribute name="text:start-numbering-at">chapter</xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="text:start-numbering-at">page</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:if>
            <!--
            <xsl:if test="w:footnote[@w:type = 'continuation-separator']">
                <text:footnote-continuation-notice-backward>
                    <xsl:value-of select="normalize-space(w:footnote[@w:type = 'continuation-separator'])"/>
                </text:footnote-continuation-notice-backward>
            </xsl:if>
            <xsl:if test="w:footnote[@w:type = 'continuation-notice']">
                <text:footnote-continuation-notice-forward>
                    <xsl:value-of select="normalize-space(w:footnote[@w:type = 'continuation-notice'])"/>
                </text:footnote-continuation-notice-forward>
            </xsl:if>
            -->
        </text:notes-configuration>
    </xsl:template>
    <xsl:template match="w:endnotePr" mode="config">
        <text:notes-configuration text:note-class="endnote" text:citation-style-name="Endnote_20_Symbol" >
            <xsl:if test="w:numStart">
                <xsl:choose>
                    <xsl:when test="(w:numStart/@w:val - 1) &gt; 0">
                        <xsl:attribute name="text:start-value"><xsl:value-of select="w:numStart/@w:val - 1"/></xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise >
                            <xsl:attribute name="text:start-value"><xsl:value-of select=" '1' "/></xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
                <!--xsl:attribute name="text:start-value"><xsl:value-of select="w:numStart/@w:val - 1"/></xsl:attribute -->
            </xsl:if>
            <xsl:if test="w:numFmt">
                <xsl:call-template name="convert-number-format">
                    <xsl:with-param name="number-format" select="w:numFmt/@w:val"/>
                </xsl:call-template>
            </xsl:if>
        </text:notes-configuration>
    </xsl:template>
    <xsl:template name="convert-number-format">
        <xsl:param name="number-format"/>
        <xsl:choose>
            <xsl:when test="$number-format = 'decimal' or $number-format = 'decimal-half-width'">
                <xsl:attribute name="style:num-format">1</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'decimal-zero'">
                <xsl:attribute name="style:num-format">1</xsl:attribute>
                <xsl:attribute name="style:num-prefix">0</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'decimal-enclosed-fullstop'">
                <xsl:attribute name="style:num-format">1</xsl:attribute>
                <xsl:attribute name="style:num-prefix">.</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'decimal-enclosed-paren'">
                <xsl:attribute name="style:num-format">1</xsl:attribute>
                <xsl:attribute name="style:num-prefix">(</xsl:attribute>
                <xsl:attribute name="style:num-suffix">)</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'number-in-dash'">
                <xsl:attribute name="style:num-format">1</xsl:attribute>
                <xsl:attribute name="style:num-prefix">-</xsl:attribute>
                <xsl:attribute name="style:num-suffix">-</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'upper-letter'">
                <xsl:attribute name="style:num-format">A</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'lower-letter'">
                <xsl:attribute name="style:num-format">a</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'upper-roman'">
                <xsl:attribute name="style:num-format">I</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'lower-roman'">
                <xsl:attribute name="style:num-format">i</xsl:attribute>
            </xsl:when>
           
            <!-- ordinal, cardinal-text, ordinal-text, hex, chicago, bullet, ideograph-zodiac-traditional, 
            vietnamese-counting, russian-lower, russian-upper, hindi-vowels, hindi-consonants, hindi-numbers, hindi-counting -->
            <xsl:otherwise>
                <xsl:attribute name="style:num-format">1</xsl:attribute>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="w:sectPr" mode="page-layout">
        <style:page-layout >
            <xsl:attribute name="style:name">pm<xsl:number from="/w:wordDocument/w:body" level="any" count="w:sectPr" format="1"/></xsl:attribute>
            <style:page-layout-properties>
                <xsl:attribute name="fo:page-width"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgSz/@w:w,'twip')"/></xsl:call-template>cm</xsl:attribute>
                <xsl:attribute name="fo:page-height"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgSz/@w:h,'twip')"/></xsl:call-template>cm</xsl:attribute>
                <xsl:choose>
                    <xsl:when test="/w:wordDocument/w:docPr/w:gutterAtTop">
                        <xsl:attribute name="fo:margin-top"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgMar/@w:top + w:pgMar/@w:gutter,'twip')"/></xsl:call-template>cm</xsl:attribute>
                        <xsl:attribute name="fo:margin-left"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgMar/@w:left,'twip')"/></xsl:call-template>cm</xsl:attribute>
                        <xsl:attribute name="fo:margin-right"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgMar/@w:right,'twip')"/></xsl:call-template>cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="w:rtlGutter">
                        <xsl:attribute name="fo:margin-top"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgMar/@w:top,'twip')"/></xsl:call-template>cm</xsl:attribute>
                        <xsl:attribute name="fo:margin-left"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgMar/@w:left,'twip')"/></xsl:call-template>cm</xsl:attribute>
                        <xsl:attribute name="fo:margin-right"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgMar/@w:right + w:pgMar/@w:gutter,'twip')"/></xsl:call-template>cm</xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="fo:margin-top"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgMar/@w:top,'twip')"/></xsl:call-template>cm</xsl:attribute>
                        <xsl:attribute name="fo:margin-left"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgMar/@w:left + w:pgMar/@w:gutter,'twip')"/></xsl:call-template>cm</xsl:attribute>
                        <xsl:attribute name="fo:margin-right"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgMar/@w:right,'twip')"/></xsl:call-template>cm</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
                <xsl:attribute name="fo:margin-bottom"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgMar/@w:bottom,'twip')"/></xsl:call-template>cm</xsl:attribute>
                <xsl:attribute name="style:footnote-max-height"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgMar/@w:footer,'twip')"/></xsl:call-template>cm</xsl:attribute>
                <xsl:if test="w:pgSz/@w:orient">
                    <xsl:attribute name="style:print-orientation"><xsl:value-of select="w:pgSz/@w:orient"/></xsl:attribute>
                </xsl:if>
                <xsl:if test="w:cols/@w:num">
                    <!-- create sction property-->
                    <style:columns>
                        <xsl:attribute name="fo:column-count"><xsl:value-of select="w:cols/@w:num"/></xsl:attribute>
                        <xsl:attribute name="fo:column-gap"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:cols/@w:space,'twip')"/></xsl:call-template>cm</xsl:attribute>
                    </style:columns>
                </xsl:if>
            </style:page-layout-properties>
        </style:page-layout >
    </xsl:template>
    <xsl:template match="w:sectPr" mode="master-page">
        <!-- style:page-layout style:style-->
        <style:master-page>
            <xsl:variable name="master-page-name">
                <xsl:number count="w:sectPr" from="/w:wordDocument/w:body" level="any" format="1"/>
            </xsl:variable>
            <xsl:attribute name="style:name">Standard<xsl:value-of select="$master-page-name"/></xsl:attribute>
            <xsl:attribute name="style:page-layout-name"><xsl:value-of select="concat('pm', $master-page-name)"/></xsl:attribute>
            <!-- Headers and footers-->
            <!--
            <style:header-style>
                <style:header-footer-properties>
                    <xsl:attribute name="fo:min-height"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgMar/@w:header,'twip')"/></xsl:call-template>cm</xsl:attribute>
                    <xsl:attribute name="fo:margin-bottom">0.792cm</xsl:attribute>
                    <xsl:attribute name="style:dynamic-spacing">true</xsl:attribute>
                </style:header-footer-properties>
            </style:header-style>
            <style:footer-style>
                <style:header-footer-properties>
                    <xsl:attribute name="fo:min-height"><xsl:call-template name="ConvertMeasure"><xsl:with-param name="value" select="concat(w:pgMar/@w:footer,'twip')"/></xsl:call-template>cm</xsl:attribute>
                    <xsl:attribute name="fo:margin-top">0.792cm</xsl:attribute>
                    <xsl:attribute name="style:dynamic-spacing">true</xsl:attribute>
                </style:header-footer-properties>
            </style:footer-style>
            -->
            <xsl:if test="not(w:titlePg)">
                <xsl:apply-templates select="w:hdr[@w:type='odd']"/>
                <xsl:apply-templates select="w:hdr[@w:type='even']"/>
                <xsl:apply-templates select="w:ftr[@w:type='odd']"/>
                <xsl:apply-templates select="w:ftr[@w:type='even']"/>
            </xsl:if>
        </style:master-page>
    </xsl:template>
    <xsl:template match="w:hdr">
        <!--
        <xsl:choose>
            <xsl:when test="@w:type = 'odd'">
                <style:header>
                    <xsl:apply-templates select="wx:pBdrGroup | w:p | w:tbl"/>
                </style:header>
            </xsl:when>
            <xsl:when test="@w:type = 'even'">
                <style:header>
                    <xsl:apply-templates select="wx:pBdrGroup | w:p | w:tbl"/>
                </style:header>
            </xsl:when>
        </xsl:choose>
        -->
    </xsl:template>
    <xsl:template match="w:ftr">
     <!--
        <xsl:choose>
            <xsl:when test="@w:type = 'odd'">
                <style:footer>
                    <xsl:apply-templates select="wx:pBdrGroup | w:p | w:tbl"/>
                </style:footer>
            </xsl:when>
            <xsl:when test="@w:type = 'even'">
                <style:footer-left>
                    <xsl:apply-templates select="wx:pBdrGroup | w:p | w:tbl"/>
                </style:footer-left>
            </xsl:when>
        </xsl:choose>
        -->
    </xsl:template>
    <xsl:template match="wx:pBdrGroup">
        <xsl:apply-templates select="w:p | w:tbl"/>
    </xsl:template>
    <!-- xsl:template name="convert-number-format">
        <xsl:param name="number-format"/>
        <xsl:choose>
            <xsl:when test="$number-format = 'decimal' or $number-format = 'decimal-half-width'">
                <xsl:attribute name="style:num-format">1</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'decimal-zero'">
                <xsl:attribute name="style:num-format">1</xsl:attribute>
                <xsl:attribute name="style:num-prefix">0</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'decimal-enclosed-fullstop'">
                <xsl:attribute name="style:num-format">1</xsl:attribute>
                <xsl:attribute name="style:num-prefix">.</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'decimal-enclosed-paren'">
                <xsl:attribute name="style:num-format">1</xsl:attribute>
                <xsl:attribute name="style:num-prefix">(</xsl:attribute>
                <xsl:attribute name="style:num-suffix">)</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'number-in-dash'">
                <xsl:attribute name="style:num-format">1</xsl:attribute>
                <xsl:attribute name="style:num-prefix">-</xsl:attribute>
                <xsl:attribute name="style:num-suffix">-</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'upper-letter'">
                <xsl:attribute name="style:num-format">A</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'lower-letter'">
                <xsl:attribute name="style:num-format">a</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'upper-roman'">
                <xsl:attribute name="style:num-format">I</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'lower-roman'">
                <xsl:attribute name="style:num-format">i</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'decimal-full-width' or $number-format = 'decimal-full-width2'">
                <xsl:attribute name="style:num-format">１, ２, ３, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'decimal-enclosed-circle-chinese' or $number-format = 'decimal-enclosed-circle'">
                <xsl:attribute name="style:num-format">①, ②, ③, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'ideograph-enclosed-circle'">
                <xsl:attribute name="style:num-format">一, 二, 三, ...</xsl:attribute>
                <xsl:attribute name="style:num-prefix">(</xsl:attribute>
                <xsl:attribute name="style:num-suffix">)</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'chinese-counting-thousand' or $number-format = 'ideograph-digital' or $number-format = 'japanese-counting' or $number-format = 'japanese-digital-ten-thousand' or $number-format = 'taiwanese-counting-thousand' or $number-format = 'taiwanese-counting' or $number-format = 'taiwanese-digital' or $number-format = 'chinese-counting' or $number-format = 'korean-digital2' or $number-format = 'chinese-not-impl'">
                <xsl:attribute name="style:num-format">一, 二, 三, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'chinese-legal-simplified'">
                <xsl:attribute name="style:num-format">壹, 贰, 叁, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'ideograph-legal-traditional'">
                <xsl:attribute name="style:num-format">壹, 貳, 參, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'ideograph-traditional'">
                <xsl:attribute name="style:num-format">甲, 乙, 丙, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'ideograph-zodiac'">
                <xsl:attribute name="style:num-format">子, 丑, 寅, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'japanese-legal'">
                <xsl:attribute name="style:num-format">壱, 弐, 参, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'aiueo-full-width'">
                <xsl:attribute name="style:num-format">ア, イ, ウ, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'aiueo'">
                <xsl:attribute name="style:num-format">ｱ, ｲ, ｳ, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'iroha-full-width'">
                <xsl:attribute name="style:num-format">イ, ロ, ハ, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'iroha'">
                <xsl:attribute name="style:num-format">ｲ, ﾛ, ﾊ, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'korean-digital' or $number-format = 'korean-counting' or $number-format = 'korean-legal'">
                <xsl:attribute name="style:num-format">일, 이, 삼, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'chosung'">
                <xsl:attribute name="style:num-format">ㄱ, ㄴ, ㄷ, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'ganada'">
                <xsl:attribute name="style:num-format">가, 나, 다, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'arabic-alpha' or $number-format = 'arabic-abjad'">
                <xsl:attribute name="style:num-format">أ, ب, ت, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'thai-letters' or $number-format = 'thai-numbers' or $number-format = 'thai-counting'">
                <xsl:attribute name="style:num-format">ก, ข, ฃ, ...</xsl:attribute>
            </xsl:when>
            <xsl:when test="$number-format = 'hebrew-1' or $number-format = 'hebrew-2'">
                <xsl:attribute name="style:num-format">א, ב, ג, ...</xsl:attribute>
            </xsl:when>
            < ordinal, cardinal-text, ordinal-text, hex, chicago, bullet, ideograph-zodiac-traditional, 
            vietnamese-counting, russian-lower, russian-upper, hindi-vowels, hindi-consonants, hindi-numbers, hindi-counting >
            <xsl:otherwise>
                <xsl:attribute name="style:num-format">Native Numbering</xsl:attribute>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template -->
</xsl:stylesheet>
