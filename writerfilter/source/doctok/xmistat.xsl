<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:UML = 'org.omg.xmi.namespace.UML' xml:space="default">
<xsl:output indent="yes"/>
<xsl:template match="/">
  <xsl:for-each select="//UML:Class[@xmi.id]">
    <xsl:value-of select="@xmi.id"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:for-each>
  <xsl:value-of select="count(//UML:Class)"/>
</xsl:template>
</xsl:stylesheet>