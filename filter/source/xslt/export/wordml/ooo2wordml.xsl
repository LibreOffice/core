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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:wx="http://schemas.microsoft.com/office/word/2003/auxHint" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:w10="urn:schemas-microsoft-com:office:word" xmlns:SL="http://schemas.microsoft.com/schemaLibrary/2003/core" xmlns:aml="http://schemas.microsoft.com/aml/2001/core" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:dt="uuid:C2F41010-65B3-11d1-A29F-00AA00C14882" xmlns:office="http://openoffice.org/2000/office" xmlns:table="http://openoffice.org/2000/table" xmlns:style="http://openoffice.org/2000/style" xmlns:text="http://openoffice.org/2000/text" xmlns:draw="http://openoffice.org/2000/drawing" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:number="http://openoffice.org/2000/datastyle" xmlns:meta="http://openoffice.org/2000/meta" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:svg="http://www.w3.org/2000/svg" xmlns:chart="http://openoffice.org/2000/chart" xmlns:dr3d="http://openoffice.org/2000/dr3d" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="http://openoffice.org/2000/form" xmlns:script="http://openoffice.org/2000/script" xmlns:config="http://openoffice.org/2001/config" exclude-result-prefixes="office table style text draw number meta dc svg chart dr3d math form script config" office:version="1.0">
    <xsl:output method="xml" indent="no" encoding="UTF-8" version="1.0" standalone="yes"/>
    <xsl:key name="style" match="style:style" use="@style:name"/>
    <xsl:key name="master-page" match="style:master-page" use="@style:name"/>
    <xsl:key name="page-master" match="style:page-master" use="@style:name"/>
    <xsl:key name="slave-style" match="style:style[string-length(normalize-space(@style:master-page-name)) &gt; 0]" use="@style:name"/>
    <xsl:template match="office:document">
        <xsl:processing-instruction name="mso-application">progid="Word.Document"</xsl:processing-instruction>
        <w:wordDocument xml:space="preserve">
            <xsl:apply-templates select="office:meta"/>
            <xsl:apply-templates select="office:font-decls"/>
            <w:styles>
                <xsl:apply-templates select="office:styles"/>
                <xsl:apply-templates select="office:automatic-styles"/>
            </w:styles>
            <xsl:apply-templates select="office:settings"/>
            <xsl:apply-templates select="office:body"/>
        </w:wordDocument>
    </xsl:template>
    <xsl:template match="office:meta">
        <o:DocumentProperties>
            <o:Title>
                <xsl:value-of select="dc:title"/>
            </o:Title>
            <o:Subject>
                <xsl:value-of select="dc:subject"/>
            </o:Subject>
            <o:Keywords>
                <xsl:for-each select="meta:keywords/meta:keyword">
                    <xsl:value-of select="."/>
                    <xsl:if test="position()!=last()">, </xsl:if>
                </xsl:for-each>
            </o:Keywords>
            <o:Description>
                <xsl:value-of select="dc:description"/>
            </o:Description>
            <o:Category>
                <xsl:value-of select="meta:user-defined[@meta:name = 'Category']"/>
            </o:Category>
            <o:Author>
                <xsl:value-of select="meta:initial-creator"/>
            </o:Author>
            <o:LastAuthor>
                <xsl:value-of select="dc:creator"/>
            </o:LastAuthor>
            <o:Manager>
                <xsl:value-of select="meta:user-defined[@meta:name = 'Manager']"/>
            </o:Manager>
            <o:Company>
                <xsl:value-of select="meta:user-defined[@meta:name = 'Company']"/>
            </o:Company>
            <o:HyperlinkBase>
                <xsl:value-of select="meta:user-defined[@meta:name = 'HyperlinkBase']"/>
            </o:HyperlinkBase>
            <o:Revision>
                <xsl:value-of select="meta:editing-cycles"/>
            </o:Revision>
            <!-- PresentationFormat, Guid, AppName, Version -->
            <o:TotalTime>
                <xsl:if test="meta:editing-duration">
                    <xsl:variable name="hours">
                        <xsl:choose>
                            <xsl:when test="contains(meta:editing-duration, 'H')">
                                <xsl:value-of select="substring-before( substring-after( meta:editing-duration, 'PT'), 'H')"/>
                            </xsl:when>
                            <xsl:otherwise>0</xsl:otherwise>
                        </xsl:choose>
                    </xsl:variable>
                    <xsl:variable name="minutes">
                        <xsl:choose>
                            <xsl:when test="contains(meta:editing-duration, 'M') and contains(meta:editing-duration, 'H')">
                                <xsl:value-of select="substring-before( substring-after( meta:editing-duration, 'H'), 'M')"/>
                            </xsl:when>
                            <xsl:when test="contains(meta:editing-duration, 'M')">
                                <xsl:value-of select="substring-before( substring-after( meta:editing-duration, 'PT'), 'M')"/>
                            </xsl:when>
                            <xsl:otherwise>0</xsl:otherwise>
                        </xsl:choose>
                    </xsl:variable>
                    <xsl:value-of select="$hours * 60 + $minutes"/>
                </xsl:if>
            </o:TotalTime>
            <o:LastPrinted>
                <xsl:if test="meta:print-date">
                    <xsl:value-of select="concat( meta:print-date, 'Z')"/>
                </xsl:if>
            </o:LastPrinted>
            <o:Created>
                <xsl:if test="meta:creation-date">
                    <xsl:value-of select="concat( meta:creation-date, 'Z')"/>
                </xsl:if>
            </o:Created>
            <o:LastSaved>
                <xsl:if test="dc:date">
                    <xsl:value-of select="concat( dc:date, 'Z')"/>
                </xsl:if>
            </o:LastSaved>
            <o:Pages>
                <xsl:value-of select="meta:document-statistic/@meta:page-count"/>
            </o:Pages>
            <o:Words>
                <xsl:value-of select="meta:document-statistic/@meta:word-count"/>
            </o:Words>
            <o:Characters>
                <xsl:value-of select="meta:document-statistic/@meta:character-count"/>
            </o:Characters>
            <!-- CharactersWithSpaces, Bytes, Lines -->
            <o:Paragraphs>
                <xsl:value-of select="meta:document-statistic/@meta:paragraph-count"/>
            </o:Paragraphs>
        </o:DocumentProperties>
        <o:CustomDocumentProperties>
            <o:Editor dt:dt="string">
                <xsl:value-of select="meta:generator"/>
            </o:Editor>
            <o:Language dt:dt="string">
                <xsl:value-of select="dc:language"/>
            </o:Language>
            <xsl:for-each select="meta:user-defined">
                <!-- transfer strings to XML QName, necessary to convert several characters -->
                <xsl:element name="{concat( 'o:', translate(@meta:name,'.,| ~`!@#$%^&amp;&lt;&gt;*()+=[]{};:&quot;/\?','_'))}">
                    <xsl:attribute name="dt:dt">string</xsl:attribute>
                    <xsl:value-of select="."/>
                </xsl:element>
            </xsl:for-each>
        </o:CustomDocumentProperties>
    </xsl:template>
    <xsl:template match="office:font-decls">
        <!-- get default font from default paragraph properties -->
        <w:fonts>
            <xsl:variable name="default-paragraph-properties" select="/office:document/office:styles/style:default-style[@style:family = 'paragraph']/style:properties"/>
            <w:defaultFonts w:ascii="{$default-paragraph-properties/@style:font-name}" w:h-ansi="{$default-paragraph-properties/@style:font-name}" w:fareast="{$default-paragraph-properties/@style:font-name-asian}" w:cs="{$default-paragraph-properties/@style:font-name-complex}"/>
            <xsl:for-each select="style:font-decl">
                <w:font w:name="{@style:name}">
                    <xsl:if test="@style:font-charset = 'x-symbol'">
                        <w:charset w:val="02"/>
                    </xsl:if>
                    <xsl:choose>
                        <xsl:when test="@style:font-family-generic = 'swiss'">
                            <w:family w:val="Swiss"/>
                        </xsl:when>
                        <xsl:when test="@style:font-family-generic = 'modern'">
                            <w:family w:val="Modern"/>
                        </xsl:when>
                        <xsl:when test="@style:font-family-generic = 'roman'">
                            <w:family w:val="Roman"/>
                        </xsl:when>
                        <xsl:when test="@style:font-family-generic = 'script'">
                            <w:family w:val="Script"/>
                        </xsl:when>
                        <xsl:when test="@style:font-family-generic = 'decorative'">
                            <w:family w:val="Decorative"/>
                        </xsl:when>
                        <xsl:when test="@style:font-family-generic = 'system'">
                            <w:family w:val="System"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <w:family w:val="System"/>
                        </xsl:otherwise>
                    </xsl:choose>
                    <w:pitch w:val="{@style:font-pitch}"/>
                </w:font>
            </xsl:for-each>
        </w:fonts>
    </xsl:template>
    <xsl:template match="office:styles | office:automatic-styles">
        <xsl:for-each select="*[(name()='style:style' or name()='style:default-style') and (@style:family= 'paragraph' or @style:family= 'text' or @style:family='table')]">
            <xsl:variable name="style-name">
                <xsl:choose>
                    <xsl:when test="name() = 'style:default-style'">
                        <xsl:value-of select="concat('default-', @style:family, '-style')"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="@style:name"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:variable>
            <w:style w:styleId="{$style-name}">
                <xsl:choose>
                    <xsl:when test="@style:family = 'paragraph'">
                        <xsl:attribute name="w:type">paragraph</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="@style:family = 'text'">
                        <xsl:attribute name="w:type">character</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="@style:family = 'table'">
                        <xsl:attribute name="w:type">table</xsl:attribute>
                    </xsl:when>
                </xsl:choose>
                <xsl:if test="name() = 'style:default-style'">
                    <xsl:attribute name="w:default">on</xsl:attribute>
                </xsl:if>
                <xsl:choose>
                    <xsl:when test="@style:parent-style-name">
                        <w:basedOn w:val="{@style:parent-style-name}"/>
                    </xsl:when>
                    <xsl:when test="name() = 'style:style' and @style:family= 'paragraph'">
                        <w:basedOn w:val="{concat('default-', @style:family, '-style')}"/>
                    </xsl:when>
                </xsl:choose>
                <w:name w:val="{$style-name}"/>
                <xsl:if test="parent::office:automatic-styles">
                    <w:hidden w:val="on"/>
                </xsl:if>
                <xsl:if test="@style:next-style-name">
                    <w:next w:val="{@style:next-style-name}"/>
                </xsl:if>
                <xsl:choose>
                    <xsl:when test="@style:family = 'paragraph'">
                        <xsl:apply-templates select="style:properties" mode="paragraph"/>
                    </xsl:when>
                    <xsl:when test="@style:family = 'table'">
                        <w:tblPr>
                            <xsl:apply-templates select="style:properties" mode="table"/>
                        </w:tblPr>
                    </xsl:when>
                </xsl:choose>
                <xsl:apply-templates select="style:properties" mode="character"/>
            </w:style>
        </xsl:for-each>
    </xsl:template>
    <xsl:template match="style:master-page">
        <xsl:apply-templates select="key( 'page-master', @style:page-master-name)"/>
        <xsl:if test="style:header">
            <w:hdr w:type="odd">
                <xsl:apply-templates select="style:header//text:p"/>
            </w:hdr>
        </xsl:if>
        <xsl:if test="style:header-left">
            <w:hdr w:type="even">
                <xsl:apply-templates select="style:header-left//text:p"/>
            </w:hdr>
        </xsl:if>
        <xsl:if test="style:footer">
            <w:ftr w:type="odd">
                <xsl:apply-templates select="style:footer//text:p"/>
            </w:ftr>
        </xsl:if>
        <xsl:if test="style:footer-left">
            <w:ftr w:type="even">
                <xsl:apply-templates select="style:footer-left//text:p"/>
            </w:ftr>
        </xsl:if>
    </xsl:template>
    <xsl:template match="style:page-master">
        <xsl:choose>
            <xsl:when test="@style:page-usage = 'left'">
                <w:type w:val="even-page"/>
            </xsl:when>
            <xsl:when test="@style:page-usage = 'right'">
                <w:type w:val="odd-page"/>
            </xsl:when>
            <xsl:when test="@style:page-usage = 'all'">
                <w:type w:val="next-page"/>
            </xsl:when>
            <!-- for mirrored, and default -->
            <xsl:otherwise>
                <w:type w:val="next-page"/>
            </xsl:otherwise>
        </xsl:choose>
        <w:pgSz>
            <xsl:if test="style:properties/@fo:page-width">
                <xsl:attribute name="w:w"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="style:properties/@fo:page-width"/></xsl:call-template></xsl:attribute>
            </xsl:if>
            <xsl:if test="style:properties/@fo:page-height">
                <xsl:attribute name="w:h"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="style:properties/@fo:page-height"/></xsl:call-template></xsl:attribute>
            </xsl:if>
            <xsl:if test="style:properties/@style:print-orientation">
                <xsl:attribute name="w:orient"><xsl:value-of select="style:properties/@style:print-orientation"/></xsl:attribute>
            </xsl:if>
        </w:pgSz>
        <w:pgMar>
            <xsl:if test="style:properties/@fo:margin-top">
                <xsl:variable name="top-margin">
                    <xsl:call-template name="convert2twip">
                        <xsl:with-param name="input_length" select="style:properties/@fo:margin-top"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:attribute name="w:top"><xsl:value-of select="$top-margin"/></xsl:attribute>
                <xsl:if test="style:header-style/style:properties/@fo:min-height">
                    <xsl:variable name="header-height">
                        <xsl:call-template name="convert2twip">
                            <xsl:with-param name="input_length" select="style:header-style/style:properties/@fo:min-height"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:attribute name="w:header"><xsl:value-of select="$top-margin - $header-height"/></xsl:attribute>
                </xsl:if>
            </xsl:if>
            <xsl:if test="style:properties/@fo:margin-bottom">
                <xsl:variable name="bottom-margin">
                    <xsl:call-template name="convert2twip">
                        <xsl:with-param name="input_length" select="style:properties/@fo:margin-bottom"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:attribute name="w:bottom"><xsl:value-of select="$bottom-margin"/></xsl:attribute>
                <xsl:if test="style:footer-style/style:properties/@fo:min-height">
                    <xsl:variable name="footer-height">
                        <xsl:call-template name="convert2twip">
                            <xsl:with-param name="input_length" select="style:footer-style/style:properties/@fo:min-height"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:attribute name="w:footer"><xsl:value-of select="$bottom-margin - $footer-height"/></xsl:attribute>
                </xsl:if>
            </xsl:if>
            <xsl:if test="style:properties/@fo:margin-left">
                <xsl:attribute name="w:left"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="style:properties/@fo:margin-left"/></xsl:call-template></xsl:attribute>
            </xsl:if>
            <xsl:if test="style:properties/@fo:margin-right">
                <xsl:attribute name="w:right"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="style:properties/@fo:margin-right"/></xsl:call-template></xsl:attribute>
            </xsl:if>
        </w:pgMar>
        <xsl:variable name="border-top" select="style:properties/@fo:border-top | style:properties/@fo:border"/>
        <xsl:variable name="border-bottom" select="style:properties/@fo:border-bottom | style:properties/@fo:border"/>
        <xsl:variable name="border-left" select="style:properties/@fo:border-left | style:properties/@fo:border"/>
        <xsl:variable name="border-right" select="style:properties/@fo:border-right | style:properties/@fo:border"/>
        <xsl:variable name="border-line-width-top" select="style:properties/@style:border-line-width-top | style:properties/@style:border-line-width "/>
        <xsl:variable name="border-line-width-bottom" select="style:properties/@style:border-line-width-bottom | style:properties/@style:border-line-width"/>
        <xsl:variable name="border-line-width-left" select="style:properties/@style:border-line-width-left | style:properties/@style:border-line-width"/>
        <xsl:variable name="border-line-width-right" select="style:properties/@style:border-line-width-right | style:properties/@style:border-line-width"/>
        <xsl:variable name="padding-top" select="style:properties/@fo:padding-top | style:properties/@fo:padding"/>
        <xsl:variable name="padding-bottom" select="style:properties/@fo:padding-bottom | style:properties/@fo:padding"/>
        <xsl:variable name="padding-left" select="style:properties/@fo:padding-left | style:properties/@fo:padding"/>
        <xsl:variable name="padding-right" select="style:properties/@fo:padding-right | style:properties/@fo:padding"/>
        <w:pgBorders w:offset-from="text">
            <xsl:if test="$border-top">
                <xsl:element name="w:top">
                    <xsl:call-template name="get-border">
                        <xsl:with-param name="so-border" select="$border-top"/>
                        <xsl:with-param name="so-border-line-width" select="$border-line-width-top"/>
                        <xsl:with-param name="so-border-position" select=" 'top' "/>
                    </xsl:call-template>
                    <xsl:attribute name="w:space"><xsl:call-template name="convert2pt"><xsl:with-param name="input_length" select="$padding-top"/></xsl:call-template></xsl:attribute>
                    <xsl:if test="style:properties/@style:shadow!='none'">
                        <xsl:attribute name="w:shadow">on</xsl:attribute>
                    </xsl:if>
                </xsl:element>
            </xsl:if>
            <xsl:if test="$border-bottom">
                <xsl:element name="w:bottom">
                    <xsl:call-template name="get-border">
                        <xsl:with-param name="so-border" select="$border-bottom"/>
                        <xsl:with-param name="so-border-line-width" select="$border-line-width-bottom"/>
                        <xsl:with-param name="so-border-position" select=" 'bottom' "/>
                    </xsl:call-template>
                    <xsl:attribute name="w:space"><xsl:call-template name="convert2pt"><xsl:with-param name="input_length" select="$padding-bottom"/></xsl:call-template></xsl:attribute>
                    <xsl:if test="style:properties/@style:shadow!='none'">
                        <xsl:attribute name="w:shadow">on</xsl:attribute>
                    </xsl:if>
                </xsl:element>
            </xsl:if>
            <xsl:if test="$border-left">
                <xsl:element name="w:left">
                    <xsl:call-template name="get-border">
                        <xsl:with-param name="so-border" select="$border-left"/>
                        <xsl:with-param name="so-border-line-width" select="$border-line-width-left"/>
                        <xsl:with-param name="so-border-position" select=" 'left' "/>
                    </xsl:call-template>
                    <xsl:attribute name="w:space"><xsl:call-template name="convert2pt"><xsl:with-param name="input_length" select="$padding-left"/></xsl:call-template></xsl:attribute>
                    <xsl:if test="style:properties/@style:shadow!='none'">
                        <xsl:attribute name="w:shadow">on</xsl:attribute>
                    </xsl:if>
                </xsl:element>
            </xsl:if>
            <xsl:if test="$border-right">
                <xsl:element name="w:right">
                    <xsl:call-template name="get-border">
                        <xsl:with-param name="so-border" select="$border-right"/>
                        <xsl:with-param name="so-border-line-width" select="$border-line-width-right"/>
                        <xsl:with-param name="so-border-position" select=" 'right' "/>
                    </xsl:call-template>
                    <xsl:attribute name="w:space"><xsl:call-template name="convert2pt"><xsl:with-param name="input_length" select="$padding-right"/></xsl:call-template></xsl:attribute>
                    <xsl:if test="style:properties/@style:shadow!='none'">
                        <xsl:attribute name="w:shadow">on</xsl:attribute>
                    </xsl:if>
                </xsl:element>
            </xsl:if>
        </w:pgBorders>
        <xsl:apply-templates select="/office:document/office:styles/text:linenumbering-configuration"/>
    </xsl:template>
    <xsl:template match="text:linenumbering-configuration">
        <xsl:if test="not(@text:number-lines = 'false')">
            <xsl:element name="w:lnNumType">
                <xsl:if test="@text:increment">
                    <xsl:attribute name="w:count-by"><xsl:value-of select="@text:increment"/></xsl:attribute>
                </xsl:if>
                <xsl:if test="@text:offset">
                    <xsl:attribute name="w:distance"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="@text:offset"/></xsl:call-template></xsl:attribute>
                </xsl:if>
                <xsl:attribute name="w:restart">continuous</xsl:attribute>
            </xsl:element>
        </xsl:if>
    </xsl:template>
    <xsl:template match="style:style" mode="section">
        <xsl:param name="master-page"/>
        <xsl:variable name="page-width">
            <xsl:call-template name="convert2twip">
                <xsl:with-param name="input_length" select="$master-page/style:properties/@fo:page-width"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="margin-left">
            <xsl:call-template name="convert2twip">
                <xsl:with-param name="input_length" select="$master-page/style:properties/@fo:margin-left"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="margin-right">
            <xsl:call-template name="convert2twip">
                <xsl:with-param name="input_length" select="$master-page/style:properties/@fo:margin-right"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="valid-width">
            <xsl:value-of select="$page-width - $margin-left - $margin-right"/>
        </xsl:variable>
        <w:type w:val="continuous"/>
        <xsl:apply-templates select="style:properties/style:columns">
            <xsl:with-param name="page-width" select="$valid-width"/>
        </xsl:apply-templates>
    </xsl:template>
    <xsl:template match="style:columns">
        <xsl:param name="page-width"/>
        <w:cols w:num="{@fo:column-count}">
            <xsl:if test="@fo:column-gap">
                <xsl:attribute name="w:space"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="@fo:column-gap"/></xsl:call-template></xsl:attribute>
            </xsl:if>
            <xsl:if test="style:column-sep">
                <xsl:attribute name="w:sep">on</xsl:attribute>
            </xsl:if>
            <xsl:choose>
                <xsl:when test="not(style:column)">
                    <xsl:attribute name="w:equalWidth">on</xsl:attribute>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:variable name="column-relative-width">
                        <xsl:call-template name="get-sum-column-width">
                            <xsl:with-param name="current-column" select="style:column[1]"/>
                            <xsl:with-param name="current-width" select="'0'"/>
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:for-each select="style:column">
                        <xsl:element name="w:col">
                            <xsl:attribute name="w:w"><xsl:value-of select="floor(substring-before(@style:rel-width,'*') * $page-width div $column-relative-width)"/></xsl:attribute>
                            <xsl:if test="@fo:margin-right">
                                <xsl:variable name="margin-right">
                                    <xsl:call-template name="convert2twip">
                                        <xsl:with-param name="input_length" select="@fo:margin-right"/>
                                    </xsl:call-template>
                                </xsl:variable>
                                <xsl:choose>
                                    <xsl:when test="following-sibling::style:column">
                                        <xsl:variable name="margin-left">
                                            <xsl:call-template name="convert2twip">
                                                <xsl:with-param name="input_length" select="@fo:margin-left"/>
                                            </xsl:call-template>
                                        </xsl:variable>
                                        <xsl:attribute name="w:space"><xsl:value-of select="$margin-right + $margin-left"/></xsl:attribute>
                                    </xsl:when>
                                    <xsl:otherwise>
                                        <xsl:attribute name="w:space"><xsl:value-of select="$margin-right"/></xsl:attribute>
                                    </xsl:otherwise>
                                </xsl:choose>
                            </xsl:if>
                        </xsl:element>
                    </xsl:for-each>
                </xsl:otherwise>
            </xsl:choose>
        </w:cols>
    </xsl:template>
    <xsl:template name="get-sum-column-width">
        <xsl:param name="current-column"/>
        <xsl:param name="current-width"/>
        <xsl:variable name="new-width" select="$current-width + substring-before($current-column/@style:rel-width,'*')"/>
        <xsl:choose>
            <xsl:when test="$current-column/following-sibling::style:column">
                <xsl:call-template name="get-sum-column-width">
                    <xsl:with-param name="current-column" select="$current-column/following-sibling::style:column[1]"/>
                    <xsl:with-param name="current-width" select="$new-width"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$new-width"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="style:properties" mode="paragraph">
        <w:pPr>
            <xsl:if test="@fo:break-before = 'page'">
                <w:pageBreakBefore w:val="on"/>
            </xsl:if>
            <xsl:if test="contains(@style:writing-mode, 'rl')">
                <w:bidi w:val="on"/>
            </xsl:if>
            <xsl:choose>
                <xsl:when test="@fo:text-align = 'start'">
                    <xsl:choose>
                        <xsl:when test="contains(@style:writing-mode, 'rl')">
                            <w:jc w:val="right"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <w:jc w:val="left"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:when>
                <xsl:when test="@fo:text-align = 'end'">
                    <xsl:choose>
                        <xsl:when test="contains(@style:writing-mode, 'rl')">
                            <w:jc w:val="left"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <w:jc w:val="right"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:when>
                <xsl:when test="@fo:text-align = 'justify'">
                    <w:jc w:val="both"/>
                </xsl:when>
                <xsl:when test="@fo:text-align = 'center'">
                    <w:jc w:val="center"/>
                </xsl:when>
            </xsl:choose>
            <w:spacing>
                <xsl:choose>
                    <xsl:when test="@fo:line-height">
                        <xsl:choose>
                            <xsl:when test="contains(@fo:line-height, '%')">
                                <xsl:attribute name="w:line-rule">auto</xsl:attribute>
                                <xsl:attribute name="w:line"><xsl:value-of select="round(substring-before(@fo:line-height, '%') div 100 * 240)"/></xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="w:line-rule">exact</xsl:attribute>
                                <xsl:attribute name="w:line"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="@fo:line-height"/></xsl:call-template></xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:when test="@style:line-height-at-least">
                        <xsl:attribute name="w:line-rule">at-least</xsl:attribute>
                        <xsl:attribute name="w:line"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="@style:line-height-at-least"/></xsl:call-template></xsl:attribute>
                    </xsl:when>
                </xsl:choose>
                <xsl:if test="@fo:margin-top">
                    <xsl:attribute name="w:before"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="@fo:margin-top"/></xsl:call-template></xsl:attribute>
                </xsl:if>
                <xsl:if test="@fo:margin-bottom">
                    <xsl:attribute name="w:after"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="@fo:margin-bottom"/></xsl:call-template></xsl:attribute>
                </xsl:if>
            </w:spacing>
            <w:ind>
                <xsl:if test="@fo:margin-left">
                    <xsl:attribute name="w:left"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="@fo:margin-left"/></xsl:call-template></xsl:attribute>
                </xsl:if>
                <xsl:if test="@fo:margin-right">
                    <xsl:attribute name="w:right"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="@fo:margin-right"/></xsl:call-template></xsl:attribute>
                </xsl:if>
                <xsl:if test="@fo:text-indent">
                    <xsl:attribute name="w:first-line"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="@fo:text-indent"/></xsl:call-template></xsl:attribute>
                </xsl:if>
            </w:ind>
            <xsl:if test="@style:auto-text-indent = 'false'">
                <w:adjustRightInd w:val="off"/>
            </xsl:if>
            <xsl:if test="contains(@fo:background-color, '#')">
                <w:shd w:val="clear" w:color="auto" w:fill="{substring-after(@fo:background-color, '#')}"/>
            </xsl:if>
            <xsl:if test="@fo:keep-with-next='true'">
                <w:keepNext/>
            </xsl:if>
            <xsl:choose>
                <xsl:when test="@fo:widows | @fo:orphans">
                    <w:widowControl w:val="on"/>
                </xsl:when>
                <xsl:otherwise>
                    <w:widowControl w:val="off"/>
                </xsl:otherwise>
            </xsl:choose>
            <xsl:if test="@style:break-inside = 'avoid'">
                <w:keepLines/>
            </xsl:if>
            <xsl:if test="@fo:hyphenate = 'false'">
                <w:suppressAutoHyphens/>
            </xsl:if>
            <xsl:if test="style:tab-stops">
                <w:tabs>
                    <xsl:for-each select="style:tab-stops/style:tab-stop">
                        <w:tab>
                            <xsl:choose>
                                <xsl:when test="@style:type='char'">
                                    <xsl:attribute name="w:val">decimal</xsl:attribute>
                                </xsl:when>
                                <xsl:when test="@style:type">
                                    <xsl:attribute name="w:val"><xsl:value-of select="@style:type"/></xsl:attribute>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:attribute name="w:val">left</xsl:attribute>
                                </xsl:otherwise>
                            </xsl:choose>
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
                                    <xsl:otherwise>
                                        <xsl:attribute name="w:leader">dot</xsl:attribute>
                                    </xsl:otherwise>
                                </xsl:choose>
                            </xsl:if>
                            <xsl:if test="@style:position">
                                <xsl:attribute name="w:pos"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="@style:position"/></xsl:call-template></xsl:attribute>
                            </xsl:if>
                        </w:tab>
                    </xsl:for-each>
                </w:tabs>
            </xsl:if>
            <xsl:variable name="border-top" select="@fo:border-top | @fo:border"/>
            <xsl:variable name="border-bottom" select="@fo:border-bottom | @fo:border"/>
            <xsl:variable name="border-left" select="@fo:border-left | @fo:border"/>
            <xsl:variable name="border-right" select="@fo:border-right | @fo:border"/>
            <xsl:variable name="border-line-width-top" select="@style:border-line-width-top | @style:border-line-width "/>
            <xsl:variable name="border-line-width-bottom" select="@style:border-line-width-bottom | @style:border-line-width"/>
            <xsl:variable name="border-line-width-left" select="@style:border-line-width-left | @style:border-line-width"/>
            <xsl:variable name="border-line-width-right" select="@style:border-line-width-right | @style:border-line-width"/>
            <xsl:variable name="padding-top" select="@fo:padding-top | @fo:padding"/>
            <xsl:variable name="padding-bottom" select="@fo:padding-bottom | @fo:padding"/>
            <xsl:variable name="padding-left" select="@fo:padding-left | @fo:padding"/>
            <xsl:variable name="padding-right" select="@fo:padding-right | @fo:padding"/>
            <w:pBdr>
                <xsl:if test="$border-top">
                    <xsl:element name="w:top">
                        <xsl:call-template name="get-border">
                            <xsl:with-param name="so-border" select="$border-top"/>
                            <xsl:with-param name="so-border-line-width" select="$border-line-width-top"/>
                            <xsl:with-param name="so-border-position" select=" 'top' "/>
                        </xsl:call-template>
                        <xsl:attribute name="w:space"><xsl:call-template name="convert2pt"><xsl:with-param name="input_length" select="$padding-top"/></xsl:call-template></xsl:attribute>
                        <xsl:if test="@style:shadow!='none'">
                            <xsl:attribute name="w:shadow">on</xsl:attribute>
                        </xsl:if>
                    </xsl:element>
                </xsl:if>
                <xsl:if test="$border-bottom">
                    <xsl:element name="w:bottom">
                        <xsl:call-template name="get-border">
                            <xsl:with-param name="so-border" select="$border-bottom"/>
                            <xsl:with-param name="so-border-line-width" select="$border-line-width-bottom"/>
                            <xsl:with-param name="so-border-position" select=" 'bottom' "/>
                        </xsl:call-template>
                        <xsl:attribute name="w:space"><xsl:call-template name="convert2pt"><xsl:with-param name="input_length" select="$padding-bottom"/></xsl:call-template></xsl:attribute>
                        <xsl:if test="@style:shadow!='none'">
                            <xsl:attribute name="w:shadow">on</xsl:attribute>
                        </xsl:if>
                    </xsl:element>
                </xsl:if>
                <xsl:if test="$border-left">
                    <xsl:element name="w:left">
                        <xsl:call-template name="get-border">
                            <xsl:with-param name="so-border" select="$border-left"/>
                            <xsl:with-param name="so-border-line-width" select="$border-line-width-left"/>
                            <xsl:with-param name="so-border-position" select=" 'left' "/>
                        </xsl:call-template>
                        <xsl:attribute name="w:space"><xsl:call-template name="convert2pt"><xsl:with-param name="input_length" select="$padding-left"/></xsl:call-template></xsl:attribute>
                        <xsl:if test="@style:shadow!='none'">
                            <xsl:attribute name="w:shadow">on</xsl:attribute>
                        </xsl:if>
                    </xsl:element>
                </xsl:if>
                <xsl:if test="$border-right">
                    <xsl:element name="w:right">
                        <xsl:call-template name="get-border">
                            <xsl:with-param name="so-border" select="$border-right"/>
                            <xsl:with-param name="so-border-line-width" select="$border-line-width-right"/>
                            <xsl:with-param name="so-border-position" select=" 'right' "/>
                        </xsl:call-template>
                        <xsl:attribute name="w:space"><xsl:call-template name="convert2pt"><xsl:with-param name="input_length" select="$padding-right"/></xsl:call-template></xsl:attribute>
                        <xsl:if test="@style:shadow!='none'">
                            <xsl:attribute name="w:shadow">on</xsl:attribute>
                        </xsl:if>
                    </xsl:element>
                </xsl:if>
            </w:pBdr>
        </w:pPr>
    </xsl:template>
    <xsl:template match="style:properties" mode="character">
        <w:rPr>
            <xsl:if test="@style:font-name">
                <w:rFonts w:ascii="{@style:font-name}" w:h-ansi="{@style:font-name}" w:fareast="{@style:font-name-asian}" w:cs="{@style:font-name-complex}"/>
            </xsl:if>
            <xsl:if test="@fo:font-size">
                <w:sz w:val="{substring-before(@fo:font-size,'pt') * 2}"/>
            </xsl:if>
            <xsl:if test="@fo:font-size-complex">
                <w:sz-cs w:val="{substring-before(@fo:font-size-complex, 'pt') * 2}"/>
            </xsl:if>
            <xsl:if test="@fo:font-style = 'italic' or @fo:font-style-asian = 'italic'">
                <w:i/>
            </xsl:if>
            <xsl:if test="@fo:font-style-complex = 'italic'">
                <w:i-cs/>
            </xsl:if>
            <xsl:if test="@fo:font-weight = 'bold' or @fo:font-weight-asian = 'bold'">
                <w:b/>
            </xsl:if>
            <xsl:if test="@fo:font-weight-complex = 'bold'">
                <w:b-cs/>
            </xsl:if>
            <xsl:if test="@style:text-underline">
                <w:u>
                    <xsl:choose>
                        <xsl:when test="@style:text-underline = 'single' and @fo:score-spaces = 'false'">
                            <xsl:attribute name="w:val">words</xsl:attribute>
                        </xsl:when>
                        <xsl:when test="@style:text-underline = 'bold'">
                            <xsl:attribute name="w:val">thick</xsl:attribute>
                        </xsl:when>
                        <xsl:when test="@style:text-underline = 'bold-dotted'">
                            <xsl:attribute name="w:val">dotted-heavy</xsl:attribute>
                        </xsl:when>
                        <xsl:when test="@style:text-underline = 'bold-dash'">
                            <xsl:attribute name="w:val">dashed-heavy</xsl:attribute>
                        </xsl:when>
                        <xsl:when test="@style:text-underline = 'long-dash'">
                            <xsl:attribute name="w:val">dash-long</xsl:attribute>
                        </xsl:when>
                        <xsl:when test="@style:text-underline = 'dash-long-heavy'">
                            <xsl:attribute name="w:val">bold-long-dash</xsl:attribute>
                        </xsl:when>
                        <xsl:when test="@style:text-underline = 'bold-dot-dash'">
                            <xsl:attribute name="w:val">dash-dot-heavy</xsl:attribute>
                        </xsl:when>
                        <xsl:when test="@style:text-underline = 'bold-dot-dot-dash'">
                            <xsl:attribute name="w:val">dash-dot-dot-heavy</xsl:attribute>
                        </xsl:when>
                        <xsl:when test="@style:text-underline = 'bold-wave'">
                            <xsl:attribute name="w:val">wavy-heavy</xsl:attribute>
                        </xsl:when>
                        <xsl:when test="@style:text-underline = 'double-wave'">
                            <xsl:attribute name="w:val">wavy-double</xsl:attribute>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:attribute name="w:val"><xsl:value-of select="@style:text-underline"/></xsl:attribute>
                        </xsl:otherwise>
                    </xsl:choose>
                    <xsl:if test="contains(@style:text-underline-color,'#')">
                        <xsl:attribute name="w:color"><xsl:value-of select="substring-after(@style:text-underline-color,'#')"/></xsl:attribute>
                    </xsl:if>
                </w:u>
            </xsl:if>
            <xsl:if test="@style:text-shadow">
                <w:shadow/>
            </xsl:if>
            <xsl:choose>
                <xsl:when test="@style:text-crossing-out = 'double-line'">
                    <w:dstrike/>
                </xsl:when>
                <xsl:when test="@style:text-crossing-out = 'single-line'">
                    <w:strike/>
                </xsl:when>
            </xsl:choose>
            <xsl:if test="@fo:color">
                <w:color>
                    <xsl:choose>
                        <xsl:when test="@fo:color != '#000000'">
                            <xsl:attribute name="w:val"><xsl:value-of select="substring-after(@fo:color,'#')"/></xsl:attribute>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:attribute name="w:val">auto</xsl:attribute>
                        </xsl:otherwise>
                    </xsl:choose>
                </w:color>
            </xsl:if>
            <xsl:if test="@fo:font-variant = 'small-caps'">
                <w:smallCaps/>
            </xsl:if>
            <xsl:if test="@fo:text-transform = 'uppercase'">
                <w:caps/>
            </xsl:if>
            <xsl:if test="@style:font-relief = 'engraved'">
                <w:imprint/>
            </xsl:if>
            <xsl:if test="@style:font-relief = 'embossed'">
                <w:emboss/>
            </xsl:if>
            <xsl:if test="@style:text-outline = 'true'">
                <w:outline/>
            </xsl:if>
            <xsl:if test="contains(@style:text-scale,'%')">
                <w:w w:val="{substring-before(@style:text-scale,'%')}"/>
            </xsl:if>
            <xsl:if test="@style:text-emphasize">
                <w:em>
                    <xsl:choose>
                        <xsl:when test="contains(@style:text-emphasize, 'accent')">
                            <xsl:attribute name="w:val">comma</xsl:attribute>
                        </xsl:when>
                        <xsl:when test="contains(@style:text-emphasize, 'disc')">
                            <xsl:attribute name="w:val">under-dot</xsl:attribute>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:attribute name="w:val"><xsl:value-of select="substring-before(@style:text-emphasize, ' ')"/></xsl:attribute>
                        </xsl:otherwise>
                    </xsl:choose>
                </w:em>
            </xsl:if>
            <xsl:if test="@fo:letter-spacing != 'normal'">
                <w:spacing>
                    <xsl:attribute name="w:val"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="@fo:letter-spacing"/></xsl:call-template></xsl:attribute>
                </w:spacing>
            </xsl:if>
            <xsl:if test="@style:text-blinking = 'true'">
                <w:effect w:val="blink-background"/>
            </xsl:if>
            <xsl:if test="@fo:language | @fo:language-asian | @fo:language-complex">
                <w:lang>
                    <xsl:if test="@fo:language and @fo:country">
                        <xsl:attribute name="w:val"><xsl:value-of select="concat(@fo:language, '-', @fo:country)"/></xsl:attribute>
                    </xsl:if>
                    <xsl:if test="@fo:language-asian and @fo:country-asian">
                        <xsl:attribute name="w:fareast"><xsl:value-of select="concat(@fo:language-asian, '-', @fo:country-asian)"/></xsl:attribute>
                    </xsl:if>
                    <xsl:if test="@fo:language-complex and @fo:language-complex">
                        <xsl:attribute name="w:bidi"><xsl:value-of select="concat(@fo:language-complex, '-', @fo:language-complex)"/></xsl:attribute>
                    </xsl:if>
                </w:lang>
            </xsl:if>
            <xsl:if test="@style:text-position">
                <xsl:variable name="position" select="substring-before(@style:text-position, ' ')"/>
                <w:vertAlign>
                    <xsl:choose>
                        <xsl:when test="contains($position, 'super') or not(contains($position, '-'))">
                            <xsl:attribute name="w:val">superscript</xsl:attribute>
                        </xsl:when>
                        <xsl:when test="contains($position, 'sub') or contains($position, '-')">
                            <xsl:attribute name="w:val">subscript</xsl:attribute>
                        </xsl:when>
                    </xsl:choose>
                </w:vertAlign>
                <xsl:if test="contains($position,'%')">
                    <w:position w:val="{substring-before($position, '%')}"/>
                </xsl:if>
            </xsl:if>
            <xsl:if test="@text:display = 'true'">
                <w:vanish/>
            </xsl:if>
            <xsl:if test="contains(@style:text-background-color, '#')">
                <w:shd w:val="clear" w:color="auto" w:fill="{substring-after(@style:text-background-color, '#')}"/>
            </xsl:if>
        </w:rPr>
    </xsl:template>
    <xsl:template match="style:properties" mode="table">
        <xsl:param name="within-body"/>
        <xsl:if test="$within-body = 'yes'">
            <w:tblW>
                <xsl:choose>
                    <xsl:when test="@style:rel-width">
                        <xsl:attribute name="w:w"><xsl:call-template name="convert2pct"><xsl:with-param name="input_length" select="@style:width"/></xsl:call-template></xsl:attribute>
                        <xsl:attribute name="w:type">pct</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="@style:width">
                        <xsl:attribute name="w:w"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="@style:width"/></xsl:call-template></xsl:attribute>
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
                    <xsl:attribute name="w:w"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="@fo:margin-left"/></xsl:call-template></xsl:attribute>
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
    <xsl:template match="office:settings">
        <w:docPr>
            <xsl:variable name="view-settings" select="config:config-item-set[@config:name = 'view-settings']"/>
            <xsl:choose>
                <xsl:when test="$view-settings/config:config-item[@config:name = 'InBrowseMode'] = 'true'">
                    <w:view w:val="outline"/>
                </xsl:when>
                <xsl:otherwise>
                    <w:view w:val="print"/>
                </xsl:otherwise>
            </xsl:choose>
            <xsl:variable name="views" select="$view-settings/config:config-item-map-indexed[@config:name = 'Views']"/>
            <w:zoom w:percent="{$views/config:config-item-map-entry/config:config-item[@config:name = 'ZoomFactor']}">
                <xsl:variable name="zoom-type" select="$views/config:config-item-map-entry/config:config-item[@config:name = 'ZoomType']"/>
                <xsl:choose>
                    <xsl:when test="$zoom-type = '3'">
                        <xsl:attribute name="w:val">best-fit</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$zoom-type = '2'">
                        <xsl:attribute name="w:val">full-page</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="$zoom-type = '1'">
                        <xsl:attribute name="w:val">text-fit</xsl:attribute>
                    </xsl:when>
                </xsl:choose>
            </w:zoom>
            <w:defaultTabStop>
                <xsl:attribute name="w:val"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="/office:document/office:styles/style:default-style[@style:family='paragraph']/style:properties/@style:tab-stop-distance"/></xsl:call-template></xsl:attribute>
            </w:defaultTabStop>
            <xsl:if test="../office:master-styles/style:master-page/style:header-left">
                <w:evenAndOddHeaders/>
            </xsl:if>
        </w:docPr>
    </xsl:template>
    <xsl:template match="office:body">
        <w:body>
            <xsl:apply-templates select=".//*[(name() = 'text:p' and not(ancestor::table:table) and not(ancestor::office:annotation)) or name()= 'text:h' or name()= 'table:table']"/>
            <xsl:variable name="paragraph-heading" select=".//*[name() = 'text:p' or name() = 'text:h']"/>
            <xsl:variable name="page" select="$paragraph-heading[key( 'slave-style', @text:style-name)]"/>
            <w:sectPr>
                <xsl:choose>
                    <xsl:when test="count($page) &gt; 0">
                        <xsl:apply-templates select="key('master-page', key( 'slave-style', $page[last()]/@text:style-name)/@style:master-page-name)"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:apply-templates select="/office:document/office:master-styles/style:master-page[last()]"/>
                    </xsl:otherwise>
                </xsl:choose>
                <xsl:if test="$paragraph-heading[last()]/ancestor::text:section">
                    <xsl:apply-templates select="key('style',$paragraph-heading[last()]/ancestor::text:section[1]/@text:style-name)" mode="section"/>
                </xsl:if>
            </w:sectPr>
        </w:body>
    </xsl:template>
    <xsl:template match="text:p | text:h">
        <w:p>
            <w:pPr>
                <xsl:if test="@text:style-name">
                    <w:pStyle w:val="{@text:style-name}"/>
                </xsl:if>
                <xsl:if test="@text:level">
                    <w:outlineLvl w:val="{@text:level - 1}"/>
                </xsl:if>
                <xsl:variable name="following-paragraph-heading" select="following::*[(name()= 'text:p' or name()= 'text:h')]"/>
                <xsl:variable name="following-section" select="following::text:section[1]"/>
                <xsl:variable name="ancestor-section" select="ancestor::text:section"/>
                <!-- if the following neighbour paragraph/heading are slave of one master style, or new section starts,
				 then a new page will start -->
                <xsl:variable name="next-is-new-page" select="boolean(key( 'slave-style', $following-paragraph-heading[1]/@text:style-name))"/>
                <xsl:variable name="next-is-new-section">
                    <xsl:if test="$following-section and generate-id($following-section/descendant::*[name()= 'text:p' or name()= 'text:h' and position() =1]) = generate-id($following-paragraph-heading[1])">
                        <xsl:value-of select="'yes'"/>
                    </xsl:if>
                </xsl:variable>
                <xsl:variable name="next-is-section-end">
                    <xsl:if test="$ancestor-section and generate-id($ancestor-section[1]/following::*[name()= 'text:p' or name()= 'text:h' and position() =1]) = generate-id($following-paragraph-heading[1])">
                        <xsl:value-of select="'yes'"/>
                    </xsl:if>
                </xsl:variable>
                <xsl:if test="ancestor::office:body and ($next-is-new-page or $next-is-new-section = 'yes' or $next-is-section-end = 'yes')">
                    <w:sectPr>
                        <xsl:choose>
                            <xsl:when test="key( 'slave-style', @text:style-name)">
                                <xsl:apply-templates select="key('master-page', key( 'slave-style', @text:style-name)[1]/@style:master-page-name)"/>
                                <xsl:if test="$ancestor-section">
                                    <xsl:apply-templates select="key('style',$ancestor-section[1]/@text:style-name)" mode="section">
                                        <xsl:with-param name="master-page" select="key( 'page-master', key('master-page', key( 'slave-style', @text:style-name)[1]/@style:master-page-name)/@style:page-master-name)"/>
                                    </xsl:apply-templates>
                                </xsl:if>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:variable name="preceding-style" select="preceding::*[(name()= 'text:p' or name()= 'text:h') and key( 'slave-style', @text:style-name)]"/>
                                <xsl:apply-templates select="key('master-page', key( 'slave-style', $preceding-style[1]/@text:style-name)[1]/@style:master-page-name)"/>
                                <xsl:if test="$ancestor-section">
                                    <xsl:apply-templates select="key('style',$ancestor-section[1]/@text:style-name)" mode="section">
                                        <xsl:with-param name="master-page" select="key( 'page-master', key('master-page', key( 'slave-style', $preceding-style[1]/@text:style-name)[1]/@style:master-page-name)/@style:page-master-name)"/>
                                    </xsl:apply-templates>
                                </xsl:if>
                            </xsl:otherwise>
                        </xsl:choose>
                    </w:sectPr>
                </xsl:if>
            </w:pPr>
            <!-- get break column from style -->
            <xsl:variable name="style" select="key('style', @text:style-name)/style:properties"/>
            <xsl:if test="$style/@fo:break-before = 'column'">
                <w:r>
                    <w:br w:type="column"/>
                </w:r>
            </xsl:if>
            <xsl:apply-templates select="text() | .//text:span/text() | .//text:hidden-text | .//text:line-break | .//text:tab-stop | .//text:s"/>
            <xsl:if test="$style/@fo:break-after">
                <w:r>
                    <w:br w:type="{$style/@fo:break-after}"/>
                </w:r>
            </xsl:if>
        </w:p>
    </xsl:template>
    <xsl:template match="text()">
        <xsl:if test="string-length(normalize-space(.)) &gt; 0">
            <w:r>
                <xsl:if test="parent::text:span">
                    <w:rPr>
                        <w:rStyle w:val="{parent::text:span/@text:style-name}"/>
                    </w:rPr>
                </xsl:if>
                <w:t>
                    <xsl:value-of select="."/>
                </w:t>
            </w:r>
        </xsl:if>
    </xsl:template>
    <xsl:template match="text:hidden-text">
        <w:r>
            <w:rPr>
                <xsl:if test="parent::text:span">
                    <w:rStyle w:val="{parent::text:span/@text:style-name}"/>
                </xsl:if>
                <w:vanish/>
            </w:rPr>
            <w:t>
                <xsl:value-of select="@text:string-value"/>
            </w:t>
        </w:r>
    </xsl:template>
    <xsl:template match="text:line-break">
        <w:r>
            <xsl:if test="parent::text:span">
                <w:rPr>
                    <w:rStyle w:val="{parent::text:span/@text:style-name}"/>
                </w:rPr>
            </xsl:if>
            <w:br w:type="text-wrapping" w:clear="all"/>
        </w:r>
    </xsl:template>
    <xsl:template match="text:tab-stop">
        <w:r>
            <xsl:if test="parent::text:span">
                <w:rPr>
                    <w:rStyle w:val="{parent::text:span/@text:style-name}"/>
                </w:rPr>
            </xsl:if>
            <w:tab/>
        </w:r>
    </xsl:template>
    <xsl:template match="text:s">
        <w:r>
            <xsl:if test="parent::text:span">
                <w:rPr>
                    <w:rStyle w:val="{parent::text:span/@text:style-name}"/>
                </w:rPr>
            </xsl:if>
            <w:t>
                <xsl:if test="@text:c">
                    <xsl:call-template name="add-space">
                        <xsl:with-param name="number" select="@text:c"/>
                    </xsl:call-template>
                </xsl:if>
                <xsl:text> </xsl:text>
            </w:t>
        </w:r>
    </xsl:template>
    <xsl:template name="add-space">
        <xsl:param name="number"/>
        <xsl:if test="$number &gt; 1">
            <xsl:call-template name="add-space">
                <xsl:with-param name="number" select="$number - 1"/>
            </xsl:call-template>
            <xsl:text> </xsl:text>
        </xsl:if>
    </xsl:template>
    <xsl:template match="table:table">
        <w:tbl>
            <w:tblPr>
                <w:tblStyle w:val="{@table:style-name}"/>
                <xsl:apply-templates select="key('style', @table:style-name)/style:properties" mode="table">
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
        <xsl:variable name="column-width" select="key('style', @table:style-name)/style:properties/@style:column-width"/>
        <xsl:variable name="column-width-in-twip">
            <xsl:call-template name="convert2twip">
                <xsl:with-param name="input_length" select="$column-width"/>
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
            <xsl:if test="parent::table:table-header-rows">
                <xsl:attribute name="w:tblHeader">on</xsl:attribute>
            </xsl:if>
            <xsl:variable name="row-height" select="key('style', @table:style-name)/style:properties/@style:row-height"/>
            <xsl:if test="$row-height">
                <w:trHeight>
                    <xsl:attribute name="w:val"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="$row-height"/></xsl:call-template></xsl:attribute>
                </w:trHeight>
            </xsl:if>
            <xsl:apply-templates select="table:table-cell"/>
        </xsl:element>
    </xsl:template>
    <xsl:template match="table:table-cell">
        <xsl:element name="w:tc">
            <xsl:element name="w:tcPr">
                <!-- set w:type to auto that makes the cell width auto fit the content. Gary Yang -->
                <w:tcW w:w="0" w:type="auto"/>
                <xsl:if test="@table:number-columns-spanned">
                    <w:gridSpan w:val="{@table:number-columns-spanned}"/>
                </xsl:if>
                <xsl:variable name="cell-style-properties" select="key('style', @table:style-name)/style:properties"/>
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
                            <xsl:attribute name="w:w"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="$cell-style-properties/@fo:padding-top"/></xsl:call-template></xsl:attribute>
                        </w:top>
                    </xsl:if>
                    <xsl:if test="$cell-style-properties/@fo:padding-bottom">
                        <w:bottom w:type="dxa">
                            <xsl:attribute name="w:w"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="$cell-style-properties/@fo:padding-bottom"/></xsl:call-template></xsl:attribute>
                        </w:bottom>
                    </xsl:if>
                    <xsl:if test="$cell-style-properties/@fo:padding-left">
                        <w:left w:type="dxa">
                            <xsl:attribute name="w:w"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="$cell-style-properties/@fo:padding-left"/></xsl:call-template></xsl:attribute>
                        </w:left>
                    </xsl:if>
                    <xsl:if test="$cell-style-properties/@fo:padding-right">
                        <w:right w:type="dxa">
                            <xsl:attribute name="w:w"><xsl:call-template name="convert2twip"><xsl:with-param name="input_length" select="$cell-style-properties/@fo:padding-right"/></xsl:call-template></xsl:attribute>
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
            <xsl:apply-templates select="text:p"/>
        </xsl:element>
    </xsl:template>
    <!-- multiple usage: get size, type, color of table-cell, paragraph, and page borders. -->
    <xsl:template name="get-border-size">
        <xsl:param name="border"/>
        <xsl:param name="border-line-width"/>
        <xsl:choose>
            <xsl:when test="$border = 'none' or $border = 'hidden'">
                <xsl:text>none;0</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="border-inner-line-value">
                    <xsl:call-template name="convert2cm">
                        <xsl:with-param name="input_length" select="$border-line-width"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="border-value">
                    <xsl:call-template name="convert2cm">
                        <xsl:with-param name="input_length" select="$border"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="border-style">
                    <xsl:choose>
                        <xsl:when test="contains($border,'solid')">solid</xsl:when>
                        <xsl:when test="contains($border,'double')">double</xsl:when>
                        <xsl:otherwise>none</xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <!-- MS word and SO wirter borders Mapping
				MS word Borders                         SO borders
				w:val="single" w:sz="0"             		0.05pt(0.002cm);solid
				w:val="single" w:sz="12"             		1.00pt(0.035cm);solid
				w:val="single" w:sz="18"  					2.50pt(0.088cm);solid
				w:val="single" w:sz="36" 						4.00pt(0.141cm);solid
				w:val="single" w:sz="48"  						5.00pt(0.176cm);solid
				w:val="double" w:sz="2"  						1.10pt(0.039cm);double
				w:val="double" w:sz="6"  						2.60pt(0.092cm);double
				w:val="thin-thick-small-gap" w:sz="12"        		  3.00pt(0.105cm);double
				w:val="thin-thick-large-gap" w:sz="18"   			 3.55pt(0.125cm);double
				w:val="thick-thin-medium-gap" w:sz="24" 		4.50pt(0.158cm);double
				w:val="thin-thick-medium-gap" w:sz="24"		 5.05pt(0.178cm);double
				w:val="thin-thick-small-gap" w:sz="24"			6.00pt(0.211cm);double
				w:val="thin-thick-medium-gap" w:sz="36			" 6.55pt(0.231cm);double
				w:val="double" w:sz="18" 						7.50pt(0.264cm);double
				w:val="thin-thick-medium-gap" w:sz="48" 		9.00pt(0.317cm);double;style:border-line-width="0.088cm 0.088cm 0.141cm"
				w:val="double" w:sz="24"						9.00pt(0.317cm);double;style:border-line-width="0.141cm 0.088cm 0.088cm" 
				we adjust the criteria by adding about 1/2 range of this current criteria and next criteria.  Gary. Yang -->
                <xsl:variable name="microsoft-border-style-size">
                    <xsl:choose>
                        <xsl:when test=" $border-style = 'solid'">
                            <xsl:choose>
                                <xsl:when test="$border-value &lt;= 0.018">single;0</xsl:when>
                                <xsl:when test="$border-value &lt;= 0.055">single;12</xsl:when>
                                <xsl:when test="$border-value &lt;= 0.110">single;18</xsl:when>
                                <xsl:when test="$border-value &lt;= 0.155">single;36</xsl:when>
                                <xsl:when test="$border-value &lt;= 0.198">single;48</xsl:when>
                                <xsl:otherwise>single;48</xsl:otherwise>
                            </xsl:choose>
                        </xsl:when>
                        <xsl:when test="$border-style = 'double'">
                            <xsl:choose>
                                <xsl:when test="$border-value &lt; 0.064">double;2</xsl:when>
                                <xsl:when test="$border-value &lt; 0.098">double;6</xsl:when>
                                <xsl:when test="$border-value &lt; 0.115">thin-thick-small-gap;12</xsl:when>
                                <xsl:when test="$border-value &lt; 0.135">thin-thick-large-gap;18</xsl:when>
                                <xsl:when test="$border-value &lt; 0.168">thick-thin-medium-gap;24</xsl:when>
                                <xsl:when test="$border-value &lt; 0.190">thin-thick-medium-gap;24</xsl:when>
                                <xsl:when test="$border-value &lt; 0.221">thin-thick-small-gap;24</xsl:when>
                                <xsl:when test="$border-value &lt; 0.241">thin-thick-medium-gap;36</xsl:when>
                                <xsl:when test="$border-value &lt; 0.300">double;18</xsl:when>
                                <xsl:when test="$border-value &lt; 0.430">
                                    <xsl:if test="$border-inner-line-value &lt; 0.10">thin-thick-medium-gap;48</xsl:if>
                                    <xsl:if test="$border-inner-line-value &gt; 0.10">double;24</xsl:if>
                                </xsl:when>
                                <xsl:otherwise>double;24</xsl:otherwise>
                            </xsl:choose>
                        </xsl:when>
                        <xsl:otherwise>none;0</xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
                <xsl:value-of select="$microsoft-border-style-size"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <!-- pct in MS Office 2003 XML means 1/50 %, i.s. 1/5000 -->
    <xsl:template name="convert2pct">
        <xsl:param name="input_length"/>
        <xsl:choose>
            <xsl:when test="contains($input_length, '%')">
                <xsl:value-of select="substring-before($input_length, '%') * 50"/>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
    <!-- twip in MS Office 2003 XML means 1/1440 inch, often marked by 'dxa', 'ftsdxa', and 'nil' sometimes. -->
    <xsl:template name="convert2twip">
        <xsl:param name="input_length"/>
        <xsl:choose>
            <xsl:when test="contains($input_length, 'cm')">
                <xsl:value-of select="round( substring-before($input_length, 'cm') * 1440 div 2.54)"/>
            </xsl:when>
            <xsl:when test="contains($input_length, 'mm')">
                <xsl:value-of select="round( substring-before($input_length, 'mm') * 1440 div 25.4)"/>
            </xsl:when>
            <xsl:when test="contains($input_length, 'inch')">
                <xsl:value-of select="round( substring-before($input_length, 'inch') * 1440)"/>
            </xsl:when>
            <xsl:when test="contains($input_length, 'pt')">
                <xsl:value-of select="round( substring-before($input_length, 'pt') * 1440 div 72)"/>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="convert2cm">
        <xsl:param name="input_length"/>
        <xsl:choose>
            <xsl:when test="contains($input_length, 'cm')">
                <xsl:value-of select="substring-before($input_length, 'cm')"/>
            </xsl:when>
            <xsl:when test="contains($input_length, 'mm')">
                <xsl:value-of select="substring-before($input_length, 'mm') div 10"/>
            </xsl:when>
            <xsl:when test="contains($input_length, 'inch')">
                <xsl:value-of select="substring-before($input_length, 'inch') * 2.54"/>
            </xsl:when>
            <xsl:when test="contains($input_length, 'pt')">
                <xsl:value-of select="substring-before($input_length, 'pt') div 72 * 2.54"/>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
    <!-- pt in OpenOffice.org/StarOffice means 1/72 inch -->
    <xsl:template name="convert2pt">
        <xsl:param name="input_length"/>
        <xsl:choose>
            <xsl:when test="contains($input_length, 'cm')">
                <xsl:value-of select="round( substring-before($input_length, 'cm') div 2.54 * 72)"/>
            </xsl:when>
            <xsl:when test="contains($input_length, 'mm')">
                <xsl:value-of select="round( substring-before($input_length, 'mm') div 25.4 * 72)"/>
            </xsl:when>
            <xsl:when test="contains($input_length, 'inch')">
                <xsl:value-of select="round( substring-before($input_length, 'inch') * 72)"/>
            </xsl:when>
            <xsl:when test="contains($input_length, 'pt')">
                <xsl:value-of select="substring-before($input_length, 'pt')"/>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
    <!-- get bottom and right border style, size, color  -->
    <xsl:template name="get-border">
        <xsl:param name="so-border"/>
        <xsl:param name="so-border-line-width"/>
        <xsl:param name="so-border-position"/>
        <xsl:variable name="ms-style-width">
            <xsl:call-template name="get-border-size">
                <xsl:with-param name="border" select="$so-border"/>
                <xsl:with-param name="border-line-width" select="$so-border-line-width"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:choose>
            <xsl:when test="$so-border-position = 'bottom' or $so-border-position = 'right'">
                <!-- if border style is bottom or right border we need to change the thin-thick to thick-thin; Vice Versa -->
                <xsl:choose>
                    <xsl:when test="substring-before($ms-style-width, '-')='thin'">
                        <xsl:attribute name="w:val"><xsl:value-of select="concat( 'thick-thin', substring-after(substring-before($ms-style-width, ';'), 'k' ))"/></xsl:attribute>
                    </xsl:when>
                    <xsl:when test="substring-before($ms-style-width, '-')='thick'">
                        <xsl:attribute name="w:val"><xsl:value-of select="concat( 'thin-thick', substring-after(substring-before($ms-style-width, ';'), 'n' ))"/></xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="w:val"><xsl:value-of select="substring-before($ms-style-width, ';')"/></xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
                <xsl:attribute name="w:sz"><xsl:value-of select="substring-after($ms-style-width,';')"/></xsl:attribute>
            </xsl:when>
            <xsl:when test="$so-border-position = 'top' or $so-border-position = 'left'">
                <xsl:attribute name="w:val"><xsl:value-of select="substring-before($ms-style-width,';')"/></xsl:attribute>
                <xsl:attribute name="w:sz"><xsl:value-of select="substring-after($ms-style-width,';')"/></xsl:attribute>
            </xsl:when>
        </xsl:choose>
        <!--get border color -->
        <xsl:choose>
            <xsl:when test="contains($so-border,'#')">
                <xsl:attribute name="w:color"><xsl:value-of select="substring-after($so-border, '#')"/></xsl:attribute>
            </xsl:when>
            <xsl:otherwise>
                <xsl:attribute name="w:color">auto</xsl:attribute>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
</xsl:stylesheet>
