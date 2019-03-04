<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:x="http://www.w3.org/1999/xhtml" version="1.0">

  <xsl:output method="text" omit-xml-declaration="yes" indent="no"/>

  <xsl:param name="build_type" />
  <xsl:param name="no_mpl_subset" />
  <xsl:param name="os" />
  <xsl:param name="themes" />

  <xsl:strip-space elements="*"/>

  <xsl:template match="/">
    <xsl:apply-templates select="x:html/x:body"/>
  </xsl:template>

  <xsl:template match="x:body">
    <xsl:apply-templates />
  </xsl:template>

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
        <xsl:apply-templates />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="x:p|x:li">
    <xsl:value-of select="concat(.,'&#10;')"/>
    <xsl:text>&#xa;</xsl:text>
    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match="x:h1">
    <xsl:text># </xsl:text>
    <xsl:value-of select="concat(.,'&#10;')"/>
    <xsl:text>&#xA;</xsl:text>
  </xsl:template>

  <xsl:template match="x:h2">
    <xsl:text>## </xsl:text>
    <xsl:value-of select="concat(.,'&#10;')"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="x:h3">
    <xsl:text>### </xsl:text>
    <xsl:value-of select="concat(.,'&#10;')"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="text()|@*"></xsl:template>

</xsl:stylesheet>
