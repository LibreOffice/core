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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:wx="http://schemas.microsoft.com/office/word/2003/auxHint" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:aml="http://schemas.microsoft.com/aml/2001/core" xmlns:dt="uuid:C2F41010-65B3-11d1-A29F-00AA00C14882" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:dom="http://www.w3.org/2001/xml-events" exclude-result-prefixes="w wx aml o dt  v">
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
</xsl:stylesheet>
