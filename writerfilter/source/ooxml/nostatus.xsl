<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text"/>

<xsl:template match="/">
	<xsl:for-each select="/stage3">
		<xsl:for-each select="attribute|element">
			<xsl:for-each select="file[nostatus]">
				<xsl:value-of select="@name"/>
				<xsl:text>:</xsl:text>
				<xsl:value-of select="@line"/>
				<xsl:text>&#xa;</xsl:text>
			</xsl:for-each>
		</xsl:for-each>
	</xsl:for-each>
</xsl:template>

</xsl:stylesheet>