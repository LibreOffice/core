<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" 
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
                xmlns:rng="http://relaxng.org/ns/structure/1.0">

  <xsl:output method="xml" indent="yes"/>

  <xsl:template match="/">
    <todo>
      <xsl:for-each select="/stage3">
        <xsl:for-each select="attribute|element">
          <xsl:choose>
            <xsl:when test="@supported = 'no'"/>            
            <xsl:when test="@qname and @resource='Properties' and not(file)">
              <xsl:copy-of select="."/>
            </xsl:when>
            <xsl:when test="@qname and file/status[number(@done) &lt; 100 and number(@planned) &gt; 0]">
              <xsl:copy>
                <xsl:copy-of select="@*"/>
                <xsl:copy-of select="file[status[number(@done) &lt; 100 and number(@planned) &gt; 0]]"/>
              </xsl:copy>
            </xsl:when>
          </xsl:choose>
        </xsl:for-each>
      </xsl:for-each>
    </todo>
  </xsl:template>

</xsl:stylesheet>