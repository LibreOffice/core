<?xml version="1.0" encoding="UTF-8"?>
<!--
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output indent="yes" method="xml"/>

<!-- SPECIFY YOUR FILE SYSTEM ROOT PATH TO THE HELP FILES -->
<xsl:param name="fsroot" select="'file:///handbuch/WORKBENCH/helpcontent2/source/'"/>

<!--
######################################################
All others
######################################################
-->
<xsl:template match="/">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="*|@*|comment()|processing-instruction()|text()">
  <xsl:copy>
    <xsl:apply-templates select="*|@*|comment()|processing-instruction()|text()"/>
  </xsl:copy>
</xsl:template>

<xsl:template match="*|@*|comment()|processing-instruction()|text()" mode="embedded">
  <xsl:copy>
    <xsl:apply-templates select="*|@*|comment()|processing-instruction()|text()" mode="embedded"/>
  </xsl:copy>
</xsl:template>

<xsl:template match="bookmark" mode="embedded" />
<xsl:template match="ahelp" mode="embedded">
	<xsl:apply-templates mode="embedded"/>
</xsl:template>

<xsl:template match="paragraph[@role='heading']">
    <title>
        <xsl:apply-templates/>
    </title>
</xsl:template>

<xsl:template match="paragraph[@role=*]">
    <paragraph>
        <xsl:apply-templates/>
    </paragraph>
</xsl:template>

<xsl:template match="sort">
    <xsl:apply-templates/>
</xsl:template>


<!-- 
###################################################### 
EMBED
###################################################### 
-->
<xsl:template match="embed">

		<xsl:variable name="href"><xsl:value-of select="substring-before(concat($fsroot,@href),'#')"/></xsl:variable>
		<xsl:variable name="anchor"><xsl:value-of select="substring-after(@href,'#')"/></xsl:variable>
		<xsl:variable name="doc" select="document($href)"/>
		<xsl:apply-templates select="$doc//section[@id=$anchor]" mode="embedded"/>
		<xsl:if test="not($doc//section[@id=$anchor])"> <!-- fallback for embeds that actually should be embedvars -->
			<paragraph role="paragraph"><xsl:apply-templates select="$doc//variable[@id=$anchor]" mode="embedded"/></paragraph>
		</xsl:if>
</xsl:template>

<!-- 
###################################################### 
EMBEDVAR
###################################################### 
-->
<xsl:template match="embedvar">
	<xsl:if test="not(@href='text/shared/00/00000004.xhp#wie')"> <!-- special treatment if howtoget links -->
		<xsl:variable name="href"><xsl:value-of select="substring-before(concat($fsroot,@href),'#')"/></xsl:variable>
		<xsl:variable name="anchor"><xsl:value-of select="substring-after(@href,'#')"/></xsl:variable>
		<xsl:variable name="doc" select="document($href)"/>
		<xsl:apply-templates select="$doc//variable[@id=$anchor]" mode="embedded"/>
	</xsl:if>
	
	<!-- FPE: embedvars, that point to "text/shared/00/00000004.xml#wie" will only be resolved in the main_transform -->
	
</xsl:template>
</xsl:stylesheet>
