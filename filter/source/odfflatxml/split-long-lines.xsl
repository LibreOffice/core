<?xml version="1.0" encoding="UTF-8"?>
<!--
Version: MPL 1.1 / GPLv3+ / LGPLv3+

The contents of this file are subject to the Mozilla Public License Version
1.1 (the "License"); you may not use this file except in compliance with
the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Initial Developer of the Original Code is
       Gioele Barabucci <gioele@svario.it>
Portions created by the Initial Developer are Copyright (C) 2010 the
Initial Developer. All Rights Reserved.

Contributor(s): Gioele Barabucci <gioele@svario.it>

Alternatively, the contents of this file may be used under the terms of
either the GNU General Public License Version 3 or later (the "GPLv3+"), or
the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
instead of those above.
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
                version="1.0">
	<xsl:param name="num-max-spaces">5</xsl:param>
	<xsl:param name="sep"><xsl:text xml:space="preserve">&#32;</xsl:text></xsl:param>
	<xsl:param name="newline"><xsl:text xml:space="preserve">&#10;</xsl:text></xsl:param>

	<xsl:template match="text:p/text()">
		<xsl:call-template name="split-text">
			<xsl:with-param name="num-prec-spaces">0</xsl:with-param>
			<xsl:with-param name="text" select="."/>
		</xsl:call-template>
	</xsl:template>

	<xsl:template name="split-text">
		<xsl:param name="num-prec-spaces"/>
		<xsl:param name="text"/>

		<xsl:variable name="contains-space" select="contains($text, $sep)"/>
		<xsl:variable name="text-before">
			<xsl:choose>
				<xsl:when test="$contains-space"><xsl:value-of select="substring-before($text, $sep)"/></xsl:when>
				<xsl:when test="$text = $sep"><xsl:value-of select="$text"/></xsl:when>
				<xsl:otherwise><xsl:value-of select="$text"/></xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name="text-after">
			<xsl:choose>
				<xsl:when test="$contains-space"><xsl:value-of select="substring-after($text, $sep)"/></xsl:when>
				<xsl:otherwise></xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name="finished" select="string-length($text-after) = 0"/>
		<xsl:variable name="is-empty" select="string-length($text-before) = 0"/>

		<xsl:value-of select="$text-before"/>

		<xsl:choose>
			<xsl:when test="$num-prec-spaces = $num-max-spaces">
				<xsl:value-of select="$newline"/>
				<xsl:call-template name="split-text">
					<xsl:with-param name="text" select="$text-after"/>
					<xsl:with-param name="num-prec-spaces">0</xsl:with-param>
				</xsl:call-template>
			</xsl:when>

			<xsl:otherwise>
				<xsl:if test="$contains-space">
					<xsl:value-of select="$sep"/>
				</xsl:if>

				<xsl:if test="not($finished)">
					<xsl:variable name="incr" select="number(not($is-empty))"/>

					<xsl:call-template name="split-text">
						<xsl:with-param name="text" select="$text-after"/>
						<xsl:with-param name="num-prec-spaces" select="$num-prec-spaces + $incr"/>
					</xsl:call-template>
				</xsl:if>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet>
