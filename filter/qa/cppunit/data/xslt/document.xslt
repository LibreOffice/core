<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:exsl="http://exslt.org/common"
                extension-element-prefixes="exsl">

<xsl:param name="targetBaseURL"/>

<xsl:template match="/">
   <exsl:document href="{concat($targetBaseURL, 'xslt-write-test.txt')}" method="text">
      <xsl:text>written</xsl:text>
   </exsl:document>
   <xsl:copy-of select="/"/>
</xsl:template>

</xsl:stylesheet>
