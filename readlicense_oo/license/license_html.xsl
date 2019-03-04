<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:x="http://www.w3.org/1999/xhtml" version="1.0">

  <xsl:output method="xml"/>

  <xsl:param name="build_type" />
  <xsl:param name="no_mpl_subset" />
  <xsl:param name="os" />
  <xsl:param name="themes" />

  <xsl:template match="x:div">
    <xsl:choose>
      <xsl:when test="(
                not(contains($build_type,@class)) and
                not(contains($no_mpl_subset,@class)) and
                not(contains($os,@class)) and
                not(contains($themes,@class)) and @class)">
      <!-- do not write out license text for these externals -->
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="node()"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@* | node()">
    <xsl:copy>
      <xsl:apply-templates select="@* | node()"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
