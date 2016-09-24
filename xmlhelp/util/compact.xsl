<?xml version="1.0" encoding="UTF-8"?>
<!--
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
-->

<!-- Remove unwanted attributes or/and nodes -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="xml" encoding="UTF-8"/>

    <xsl:strip-space elements="*"/> 
    <xsl:preserve-space elements="paragraph"/> 

    <!-- Copy everything -->
    <xsl:template match="@*|node()|text()">
        <xsl:copy>
            <xsl:apply-templates select="@*[normalize-space()]|node()|text()"/>
        </xsl:copy>
    </xsl:template>

    <!-- To remove attributes or nodes, 
         simply write a matching template that doesn't do anything. 
         Therefore, it is removed -->
    <xsl:template match="image/@localize">
        <xsl:copy>
            <xsl:apply-templates select="@*[normalize-space()]|node()|text()"/>
        </xsl:copy>
    </xsl:template>
    <xsl:template match="@localize"/>
    <xsl:template match="@xml-lang"/>
    <xsl:template match="alt"/>
    <xsl:template match="bookmark_value"/>
    <xsl:template match="comment()"/>       <!-- Remove all XML comments -->
    <xsl:template match="comment"/>
    <xsl:template match="history"/>
    <xsl:template match="image/@id"/>
    <xsl:template match="image/@width"/>
    <xsl:template match="image/@height"/>
    <xsl:template match="link/@name"/>
    <xsl:template match="paragraph/@id"/>
    <xsl:template match="paragraph/@l10n"/>
    <xsl:template match="paragraph/@oldref"/>
    <xsl:template match="table/@id"/>
    <xsl:template match="title/@id"/>
    <xsl:template match="topic/@id"/>
    <xsl:template match="topic/@indexer"/>
    <xsl:template match="topic/@status"/>

</xsl:stylesheet>
