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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:office="http://openoffice.org/2000/office" xmlns:meta="http://openoffice.org/2000/meta" xmlns:dt="uuid:C2F41010-65B3-11d1-A29F-00AA00C14882" xmlns:dc="http://purl.org/dc/elements/1.1/" exclude-result-prefixes="o dt">
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
</xsl:stylesheet>
