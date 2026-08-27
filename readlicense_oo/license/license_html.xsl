<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:x="http://www.w3.org/1999/xhtml" version="1.0">

  <xsl:output method="xml"/>

  <xsl:param name="build_type" />
  <xsl:param name="no_mpl_subset" />
  <xsl:param name="os" />
  <xsl:param name="themes" />

  <xsl:template match="x:div">
    <!-- Pad with spaces on both sides so that the class matches a whole word. -->
    <xsl:variable name="word" select="concat(' ',@class,' ')"/>
    <xsl:choose>
      <xsl:when test="(
                not(contains(concat(' ',$build_type,' '),$word)) and
                not(contains(concat(' ',$no_mpl_subset,' '),$word)) and
                not(contains(concat(' ',$os,' '),$word)) and
                not(contains(concat(' ',$themes,' '),$word)) and @class)">
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
