<?xml version="1.0" encoding="UTF-8"?>
<!--
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="text"/>

    <xsl:template match="/|node()">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="@*|text()|processing-instruction()|comment()"/>

    <xsl:template match="property[@name='icon_name'] | property[@name='icon-name'] | property[@name='pixbuf']">
        <xsl:variable name="inpath" select="normalize-space(.)"/>
        <xsl:variable name="outpath">
            <xsl:choose>
                <xsl:when test="starts-with($inpath,'res/')">
                    <xsl:value-of select="concat('%GLOBALRES%/',substring-after($inpath,'res/'))"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="concat('%MODULE%/',$inpath)"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:value-of select="$outpath"/>
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

</xsl:stylesheet>
