<?xml version="1.0" encoding="utf-8" standalone="yes" ?>

<xsl:transform  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
		xmlns:xs="http://www.w3.org/2001/XMLSchema"		
		xmlns:oor="http://openoffice.org/2001/registry"		
		version="1.0">

<xsl:output method="text" indent="no" encoding="ISO-8859-1"/>

<xsl:namespace-alias stylesheet-prefix="xs" result-prefix="xs"></xsl:namespace-alias>

    <!-- match root -->
    <xsl:template match="/">
        <xsl:apply-templates/>
    </xsl:template>

    <!-- match all elements that could possibly contain info/desc elements -->
    <xsl:template match="group|set|node-ref|prop|enumeration|length|minLength|maxLength|minInclusive|maxInclusive|minExclusive|maxExclusive|whiteSpace">

        <xsl:if test="1>string-length(info/desc)">
            <!-- print TPF's path to current node -->
            <xsl:message terminate="yes">

                <!-- print linefeed -->
                <xsl:text>&#10;</xsl:text>
                <xsl:text>ERROR: No info/desc element specified or string length of info/desc element is 0.</xsl:text>
                <xsl:text>&#10;</xsl:text>
                <xsl:text>       There has to be a reasonable description to be specified for any item.</xsl:text>
                <xsl:text>&#10;</xsl:text>

                <xsl:text>       - path: </xsl:text>
                <xsl:for-each select="ancestor-or-self::*"><xsl:text>/</xsl:text>
                    <xsl:value-of select="local-name(.)"/>
                    <xsl:choose>
                        <xsl:when test="@oor:name">[<xsl:value-of select="@oor:name"/>]</xsl:when>
                        <xsl:when test="@oor:value">[<xsl:value-of select="@oor:value"/>]</xsl:when>
                    </xsl:choose>
                </xsl:for-each>

                <xsl:text>&#10;</xsl:text>
                <xsl:text>       - author: </xsl:text>

                <xsl:for-each select="ancestor-or-self::*">
                    <!-- print element name -->
                    <xsl:if test="info/author">
                        <!-- print arrow -->
                        <xsl:text>-></xsl:text>
                        <xsl:value-of select="info/author"/>
                    </xsl:if>
                </xsl:for-each>

            </xsl:message>

        </xsl:if>

        <xsl:apply-templates/>

    </xsl:template>

</xsl:transform>
