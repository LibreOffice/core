/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8inc.xsl,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:51:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xhtml="http://www.w3.org/1999/xhtml">
<xsl:output method="text" encoding="iso-8859-1"/>

<xsl:template match="/">
<xsl:apply-templates select="//xhtml:body/xhtml:table"/>
</xsl:template>

<xsl:template match="xhtml:table">
<xsl:apply-templates select="xhtml:tbody/xhtml:tr[2]"/>
</xsl:template>

<xsl:template name="handleTR">
<xsl:param name="offset"/>
<xsl:param name="shift"/>
<xsl:variable name="name"><xsl:value-of select="xhtml:td[3]"/></xsl:variable>
<xsl:variable name="type"><xsl:value-of select="xhtml:td[4]"/></xsl:variable>
<xsl:variable name="saltype">
<xsl:choose>
  <xsl:when test="$type='U8'">sal_uInt8</xsl:when>
  <xsl:when test="$type='S8'">sal_Int8</xsl:when>
  <xsl:when test="$type='U16'">sal_uInt16</xsl:when>
  <xsl:when test="$type='S16'">sal_Int16</xsl:when>
  <xsl:when test="$type='U32'">sal_uInt32</xsl:when>
  <xsl:when test="$type='S32'">sal_Int32</xsl:when>
  <xsl:otherwise>void *</xsl:otherwise>
</xsl:choose>
</xsl:variable>
<xsl:variable name="bits"><xsl:value-of select="xhtml:td[5]"/></xsl:variable>
<xsl:variable name="mask"><xsl:value-of select="xhtml:td[6]"/></xsl:variable>
<xsl:variable name="comment"><xsl:value-of select="xhtml:td[7]"/></xsl:variable>
/** 
<xsl:value-of select="$comment"/>

offset : <xsl:value-of select="$offset"/>
name : <xsl:value-of select="$name"/>
type   : <xsl:value-of select="$type"/>
shift  : <xsl:value-of select="concat($shift, '&#xa;')"/>
*/

<xsl:value-of select="$saltype"/> get_<xsl:value-of select="$name"/>() const<xsl:choose>
<xsl:when test="$saltype='void *'">;
</xsl:when>
<xsl:otherwise> { return (get<xsl:value-of select="$type"/>(0x<xsl:value-of select="$offset"/>)<xsl:if test="string-length($mask)>0"> &amp; 0x<xsl:value-of select="translate($mask, 'ABCDEF', 'abcdef')"/></xsl:if>)<xsl:if test="$shift>0"> &gt;&gt; <xsl:value-of select="$shift"/></xsl:if>; }
</xsl:otherwise>
</xsl:choose>
</xsl:template>

<xsl:template match="xhtml:tr[./xhtml:td[position()=1]/text()]">
<xsl:variable name="offset"><xsl:value-of select="xhtml:td[2]"/></xsl:variable>
<xsl:call-template name="handleTR">
<xsl:with-param name="offset" select="$offset"/>
<xsl:with-param name="shift">0</xsl:with-param>
</xsl:call-template>
<xsl:variable name="shift1"><xsl:value-of select="substring-after(xhtml:td[5], ':')"/></xsl:variable>
<xsl:apply-templates select="following-sibling::xhtml:tr[position()=1]">
<xsl:with-param name="offset" select="$offset"/>
<xsl:with-param name="shift" select="$shift1"/>
</xsl:apply-templates>
</xsl:template>

<xsl:template match="xhtml:tr[not(./xhtml:td[position()=1]/text())]">
<xsl:param name="offset"/>
<xsl:param name="shift"/>
<xsl:call-template name="handleTR">
<xsl:with-param name="offset" select="$offset"/>
<xsl:with-param name="shift" select="$shift"/>
</xsl:call-template>
<xsl:variable name="shift1"><xsl:value-of select="$shift+substring-after(xhtml:td[5], ':')"/></xsl:variable>
<xsl:apply-templates select="following-sibling::xhtml:tr[position()=1]">
<xsl:with-param name="offset" select="$offset"/>
<xsl:with-param name="shift" select="$shift1"/>
</xsl:apply-templates>
</xsl:template>

<xsl:template match="*">
<xsl:copy-of select="."/>
</xsl:template>

</xsl:stylesheet>
