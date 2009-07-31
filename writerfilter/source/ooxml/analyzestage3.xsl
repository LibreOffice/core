<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	
	
<xsl:key name="qnames" match="/stage2/qnames/qname" use="@qname"/>

<xsl:template match="/">
	<stage3 xmlns:rng="http://relaxng.org/ns/structure/1.0">
	<xsl:for-each select="/stage2/analyze">
		<xsl:for-each select="attribute|element">
			<xsl:variable name="qname" select="@qname"/>
			<xsl:copy>
				<xsl:copy-of select="@*"/>
					<xsl:for-each select="key('qnames', @qname)">
						<file>
							<xsl:attribute name="name">
								<xsl:value-of select="@file"/>
							</xsl:attribute>
							<xsl:attribute name="line">
								<xsl:value-of select="@line"/>
							</xsl:attribute>
                            <xsl:for-each select="ancestor::qnames/status|ancestor::qnames/nostatus">
                                <xsl:copy>
                                    <xsl:copy-of select="@*"/>
                                    <xsl:attribute name="qname-count">
                                        <xsl:value-of select="count(ancestor::qnames/qname)"/>
                                    </xsl:attribute>
                                </xsl:copy>
                            </xsl:for-each>
						</file>
					</xsl:for-each>
			</xsl:copy>
		</xsl:for-each>
	</xsl:for-each>
	</stage3>
</xsl:template>

</xsl:stylesheet>	