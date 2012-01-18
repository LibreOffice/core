/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/

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
