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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:wx="http://schemas.microsoft.com/office/word/2003/auxHint" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:aml="http://schemas.microsoft.com/aml/2001/core" xmlns:office="http://openoffice.org/2000/office" xmlns:style="http://openoffice.org/2000/style" xmlns:text="http://openoffice.org/2000/text" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="http://openoffice.org/2000/meta" xmlns:config="http://openoffice.org/2001/config" exclude-result-prefixes="w wx aml o" office:version="1.0">
    <xsl:output method="xml" indent="no" encoding="UTF-8" version="1.0"/>
    <xsl:include href="../../common/measure_conversion.xsl"/>
    <xsl:include href="../common/ms2ooo_docpr.xsl"/>
    <xsl:include href="wordml2ooo_text.xsl"/>
    <xsl:include href="wordml2ooo_settings.xsl"/>
    <xsl:include href="wordml2ooo_table.xsl"/>
    <xsl:include href="wordml2ooo_page.xsl"/>
    <xsl:include href="wordml2ooo_list.xsl"/>
    <xsl:include href="wordml2ooo_draw.xsl"/>
    <xsl:key name="paragraph-style" match="w:style[@w:type = 'paragraph']" use="@w:styleId"/>
    <xsl:key name="heading-style" match="w:style[@w:type = 'paragraph' and w:pPr/w:outlineLvl]" use="@w:styleId"/>
    <xsl:template match="/">
        <xsl:apply-templates select="w:wordDocument"/>
    </xsl:template>
    <xsl:template match="w:wordDocument">
        <office:document office:class="text" office:version="1.0">
            <xsl:apply-templates select="o:DocumentProperties"/>
            <xsl:apply-templates select="w:docPr"/>
            <xsl:apply-templates select="w:fonts"/>
            <xsl:apply-templates select="w:styles"/>
            <xsl:apply-templates select="w:body"/>
        </office:document>
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
        <office:styles>
            <xsl:call-template name="create-default-paragraph-styles"/>
            <xsl:call-template name="create-default-text-styles"/>
            <xsl:call-template name="create-default-frame-style"/>
            <!-- StarWriter has no default style family 'list'. glu -->
            <xsl:if test="w:style[@w:type = 'paragraph' and w:pPr/w:outlineLvl and w:pPr/w:listPr]">
                <xsl:call-template name="create-outline-style"/>
            </xsl:if>
            <xsl:apply-templates select="w:style[@w:type='list']" mode="list"/>
            <xsl:apply-templates select="w:style[@w:type!='list']"/>
        </office:styles>
        <office:automatic-styles>
            <xsl:apply-templates select="/w:wordDocument/w:body//w:p" mode="style"/>
            <xsl:apply-templates select="/w:wordDocument/w:body//w:rPr[not(parent::w:pPr)]" mode="style"/>
            <xsl:apply-templates select="/w:wordDocument/w:body//w:tblPr" mode="style"/>
            <xsl:apply-templates select="/w:wordDocument/w:body//w:gridCol" mode="style"/>
            <xsl:apply-templates select="/w:wordDocument/w:body//w:trHeight" mode="style"/>
            <xsl:apply-templates select="/w:wordDocument/w:body//w:tcPr" mode="style"/>
            <xsl:apply-templates select="/w:wordDocument/w:body//w:listPr" mode="style"/>
            <xsl:apply-templates select="/w:wordDocument/w:body//w:sectPr" mode="page-master"/>
        </office:automatic-styles>
        <office:master-styles>
            <xsl:apply-templates select="/w:wordDocument/w:body//w:sectPr" mode="master-page"/>
        </office:master-styles>
    </xsl:template>
    <xsl:template match="w:style">
        <style:style>
            <xsl:attribute name="style:name"><xsl:value-of select="@w:styleId"/></xsl:attribute>
            <xsl:if test="w:basedOn">
                <xsl:attribute name="style:parent-style-name"><xsl:value-of select="w:basedOn/@w:val"/></xsl:attribute>
            </xsl:if>
            <xsl:if test="w:next">
                <xsl:attribute name="style:next-style-name"><xsl:value-of select="w:next/@w:val"/></xsl:attribute>
            </xsl:if>
            <xsl:choose>
                <xsl:when test="@w:type = 'character'">
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
                <xsl:apply-templates select="w:pPr"/>
                <xsl:apply-templates select="w:rPr"/>
            </xsl:element>
        </style:style>
    </xsl:template>
    <xsl:template match="w:body">
        <xsl:element name="office:body">
            <xsl:apply-templates/>
        </xsl:element>
    </xsl:template>
    <xsl:template name="create-default-frame-style">
        <!--add for default frame style -->
        <style:style style:name="Frame" style:family="graphics">
            <style:properties text:anchor-type="paragraph" svg:x="0inch" svg:y="0inch" style:wrap="parallel" style:number-wrapped-paragraphs="no-limit" style:wrap-contour="false" style:vertical-pos="top" style:vertical-rel="paragraph-content" style:horizontal-pos="center" style:horizontal-rel="paragraph-content"/>
        </style:style>
    </xsl:template>
</xsl:stylesheet>
