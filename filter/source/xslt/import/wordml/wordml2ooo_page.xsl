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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:style="http://openoffice.org/2000/style" xmlns:wx="http://schemas.microsoft.com/office/word/2003/auxHint" exclude-result-prefixes="w wx">
    <xsl:template match="w:sectPr" mode="page-master">
        <style:page-master>
            <xsl:attribute name="style:name">pm<xsl:number from="/w:wordDocument/w:body" level="any" count="w:sectPr"/></xsl:attribute>
            <style:properties>
                <xsl:attribute name="fo:page-width"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgSz/@w:w,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                <xsl:attribute name="fo:page-height"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgSz/@w:h,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                <xsl:choose>
                    <xsl:when test="/w:wordDocument/w:docPr/w:gutterAtTop">
                        <xsl:attribute name="fo:margin-top"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgMar/@w:top + w:pgMar/@w:gutter,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                        <xsl:attribute name="fo:margin-left"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgMar/@w:left,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                        <xsl:attribute name="fo:margin-right"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgMar/@w:right,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                    </xsl:when>
                    <xsl:when test="w:rtlGutter">
                        <xsl:attribute name="fo:margin-top"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgMar/@w:top,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                        <xsl:attribute name="fo:margin-left"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgMar/@w:left,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                        <xsl:attribute name="fo:margin-right"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgMar/@w:right + w:pgMar/@w:gutter,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:attribute name="fo:margin-top"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgMar/@w:top,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                        <xsl:attribute name="fo:margin-left"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgMar/@w:left + w:pgMar/@w:gutter,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                        <xsl:attribute name="fo:margin-right"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgMar/@w:right,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                    </xsl:otherwise>
                </xsl:choose>
                <xsl:attribute name="fo:margin-bottom"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgMar/@w:bottom,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                <xsl:attribute name="style:footnote-max-height"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgMar/@w:footer,'dxa')"/></xsl:call-template></xsl:attribute>
                <xsl:if test="w:pgSz/@w:orient">
                    <xsl:attribute name="style:print-orientation"><xsl:value-of select="w:pgSz/@w:orient"/></xsl:attribute>
                </xsl:if>
                <xsl:if test="w:cols/@w:num">
                    <!-- create sction property-->
                    <style:columns>
                        <xsl:attribute name="fo:column-count"><xsl:value-of select="w:cols/@w:num"/></xsl:attribute>
                        <xsl:attribute name="fo:column-gap"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:cols/@w:space,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                    </style:columns>
                </xsl:if>
            </style:properties>
        </style:page-master>
    </xsl:template>
    <xsl:template match="w:sectPr" mode="master-page">
        <!-- style:page-master style:style-->
        <style:master-page>
            <xsl:variable name="master-page-name">
                <xsl:number count="w:sectPr" from="/w:wordDocument/w:body" level="any"/>
            </xsl:variable>
            <xsl:attribute name="style:name">Standard<xsl:value-of select="$master-page-name"/></xsl:attribute>
            <xsl:attribute name="style:page-master-name"><xsl:value-of select="concat('pm', $master-page-name)"/></xsl:attribute>
            <!-- Headers and footers-->
            <style:header-style>
                <style:properties>
                    <xsl:attribute name="fo:min-height"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgMar/@w:header,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                    <xsl:attribute name="fo:margin-bottom">0.792cm</xsl:attribute>
                    <xsl:attribute name="style:dynamic-spacing">true</xsl:attribute>
                </style:properties>
            </style:header-style>
            <style:footer-style>
                <style:properties>
                    <xsl:attribute name="fo:min-height"><xsl:call-template name="convert2cm"><xsl:with-param name="value" select="concat(w:pgMar/@w:footer,'dxa')"/></xsl:call-template>cm</xsl:attribute>
                    <xsl:attribute name="fo:margin-top">0.792cm</xsl:attribute>
                    <xsl:attribute name="style:dynamic-spacing">true</xsl:attribute>
                </style:properties>
            </style:footer-style>
            <xsl:if test="not(w:titlePg)">
                <xsl:apply-templates select="w:hdr[@w:type='odd']"/>
                <xsl:apply-templates select="w:hdr[@w:type='even']"/>
                <xsl:apply-templates select="w:ftr[@w:type='odd']"/>
                <xsl:apply-templates select="w:ftr[@w:type='even']"/>
            </xsl:if>
        </style:master-page>
    </xsl:template>
    <xsl:template match="w:hdr">
        <xsl:choose>
            <xsl:when test="@w:type = 'odd'">
                <style:header>
                    <xsl:apply-templates/>
                </style:header>
            </xsl:when>
            <xsl:when test="@w:type = 'even'">
                <style:header-left>
                    <xsl:apply-templates/>
                </style:header-left>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="w:ftr">
        <xsl:choose>
            <xsl:when test="@w:type = 'odd'">
                <style:footer>
                    <xsl:apply-templates/>
                </style:footer>
            </xsl:when>
            <xsl:when test="@w:type = 'even'">
                <style:footer-left>
                    <xsl:apply-templates/>
                </style:footer-left>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
</xsl:stylesheet>
