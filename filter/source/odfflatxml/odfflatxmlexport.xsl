<?xml version='1.0' encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0">
    <xsl:output method="xml" encoding="UTF-8" indent="yes"/>

  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="@office:mimetype[string(.)='application/vnd.oasis.opendocument.drawing']">
      <xsl:attribute name="office:mimetype">application/vnd.oasis.opendocument.graphics</xsl:attribute>
  </xsl:template>
</xsl:stylesheet>
