<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	
<xsl:template match="/">
    <stage2>
        <xsl:copy-of select="/stage1/analyze"/>
        <xsl:for-each select="/stage1/qname">
            <xsl:if test="not(local-name(preceding-sibling::*[1]) = 'qname')">
                <qnames>
                    <xsl:call-template name="handleqnames"/>
                </qnames>
            </xsl:if>
        </xsl:for-each>
    </stage2>
</xsl:template>

<xsl:template name="handleqnames">
    <xsl:choose>
        <xsl:when test="local-name() = 'qname'">
            <xsl:copy-of select="."/>
            <xsl:for-each select="following-sibling::*[1]">
                <xsl:call-template name="handleqnames"/>
            </xsl:for-each>
        </xsl:when>
        <xsl:when test="local-name() = 'nostatus' or local-name() = 'status'">
            <xsl:copy-of select="."/>
        </xsl:when>
    </xsl:choose>
</xsl:template>
</xsl:stylesheet>