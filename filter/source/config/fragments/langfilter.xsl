<?xml version='1.0' encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:oor="http://openoffice.org/2001/registry">
<xsl:output method="xml" indent="yes" omit-xml-declaration="no" version="1.0" encoding="UTF-8"/>
<xsl:param name="lang"/>
<xsl:template match="*[.//*[@xml:lang=$lang]] | @*">
    <xsl:copy>
        <xsl:apply-templates select="*|@*"/>
    </xsl:copy>
</xsl:template>

<xsl:template match="value[@xml:lang=$lang] | text()[../@xml:lang=$lang]">
    <xsl:copy>
        <xsl:apply-templates select="@*|text()"/>
    </xsl:copy>
</xsl:template>
 
<xsl:template match="text()"/>


</xsl:stylesheet>

