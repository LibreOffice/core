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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:wx="http://schemas.microsoft.com/office/word/2003/auxHint" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:draw="http://openoffice.org/2000/drawing" xmlns:text="http://openoffice.org/2000/text" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:office="http://openoffice.org/2000/office" xmlns:svg="http://www.w3.org/2000/svg" exclude-result-prefixes="w wx o v">
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
                <xsl:attribute name="svg:width"><xsl:value-of select="(number($widthval) div 1440) * 2.54"/>cm</xsl:attribute>
                <xsl:attribute name="svg:height"><xsl:value-of select="(number($heightval) div 1440) * 2.54"/>cm</xsl:attribute>
                <xsl:attribute name="svg:x"><xsl:value-of select="(number($topval + $topmar) div 1440) * 2.54"/>cm</xsl:attribute>
                <xsl:attribute name="svg:y"><xsl:value-of select="(number($leftval + $leftmar) div 1440) * 2.54"/>cm</xsl:attribute>
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
                <xsl:attribute name="svg:width"><xsl:value-of select="(number($widthval) div 1440) * 2.54"/>cm</xsl:attribute>
                <xsl:attribute name="svg:height"><xsl:value-of select="(number($heightval) div 1440) * 2.54"/>cm</xsl:attribute>
                <xsl:attribute name="svg:x"><xsl:value-of select="(number($topval + $topmar) div 1440) * 2.54"/>cm</xsl:attribute>
                <xsl:attribute name="svg:y"><xsl:value-of select="(number($leftval + $leftmar) div 1440) * 2.54"/>cm</xsl:attribute>
            </xsl:if>
        </xsl:element>
    </xsl:template>
</xsl:stylesheet>
