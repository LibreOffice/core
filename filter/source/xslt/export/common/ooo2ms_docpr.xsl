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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:aml="http://schemas.microsoft.com/aml/2001/core" xmlns:wx="http://schemas.microsoft.com/office/word/2003/auxHint" xmlns:office="http://openoffice.org/2000/office" xmlns:table="http://openoffice.org/2000/table" xmlns:style="http://openoffice.org/2000/style" xmlns:text="http://openoffice.org/2000/text" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:dt="uuid:C2F41010-65B3-11d1-A29F-00AA00C14882" xmlns:dc="http://purl.org/dc/elements/1.1/"   xmlns:config="http://openoffice.org/2001/config"  xmlns:draw="http://openoffice.org/2000/drawing" xmlns:svg="http://www.w3.org/2000/svg"  xmlns:v="urn:schemas-microsoft-com:vml"  xmlns:w10="urn:schemas-microsoft-com:office:word"  xmlns:fo="http://www.w3.org/1999/XSL/Format"  xmlns:xlink="http://www.w3.org/1999/xlink"   xmlns:meta="http://openoffice.org/2000/meta" exclude-result-prefixes="office table style text draw svg  fo dc config xlink meta">
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
</xsl:stylesheet>
