<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:rng="http://relaxng.org/ns/structure/1.0">

<xsl:output method="text"/>
    
<xsl:template match="/">
    <xsl:text>Namespace,Define,Name,Done,Planned,qname-count&#xa;</xsl:text>
    <xsl:for-each select="/todo/attribute|/todo/element">
        <xsl:for-each select=".//status">
                <xsl:for-each select="ancestor::attribute|ancestor::element">
                    <xsl:value-of select="@namespace"/>
                    <xsl:text>,</xsl:text>
                    <xsl:value-of select="@define"/>
                    <xsl:text>,</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text>,</xsl:text>
                </xsl:for-each>
                <xsl:value-of select="@done"/>
                <xsl:text>,</xsl:text>
                <xsl:value-of select="@planned"/>
                <xsl:text>,</xsl:text>
                <xsl:value-of select="@qname-count"/>
                <xsl:text>&#xa;</xsl:text>
        </xsl:for-each>
        <xsl:if test="not(.//status)">
                <xsl:value-of select="@namespace"/>
                <xsl:text>,</xsl:text>
                <xsl:value-of select="@define"/>
                <xsl:text>,</xsl:text>
                <xsl:value-of select="@name"/>
                <xsl:text>,0,0.5,1&#xa;</xsl:text>
        </xsl:if>
    </xsl:for-each>
</xsl:template>

</xsl:stylesheet>