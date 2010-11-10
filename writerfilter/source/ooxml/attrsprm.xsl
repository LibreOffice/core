<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet 
    version="1.0" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:rng="http://relaxng.org/ns/structure/1.0"
    xmlns:xalan="http://xml.apache.org/xalan"
    exclude-result-prefixes = "xalan"
    xml:indent="true">
  <xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes" omit-xml-declaration="no"/>

  <xsl:include href="factorytools.xsl"/>
  
  <xsl:template match="/">
    <class>
      <xsl:for-each select="//rng:define[@name='CT_FFData']">
	<xsl:call-template name="sprm"/>
	<xsl:call-template name="attribute"/>
      </xsl:for-each>
    </class>
  </xsl:template>

  <xsl:template name="sprminner">
    <xsl:variable name="defname" select="@name"/>
    <xsl:for-each select=".//rng:ref[not(ancestor::rng:element or ancestor::rng:attribute)]">
      <xsl:call-template name="sprminner"/>
    </xsl:for-each>
    <xsl:for-each select=".//rng:element">
      <xsl:for-each select="rng:ref">
	<xsl:variable name="refname" select="@name"/>
	<xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$refname]">
	  <xsl:call-template name="sprminner"/>
	</xsl:for-each>
      </xsl:for-each>      
      <element>
	<xsl:variable name="elementname" select="@name"/>
	<xsl:attribute name="name">
	  <xsl:value-of select="@name"/>
	</xsl:attribute>
	<xsl:for-each select="rng:ref">
	  <xsl:variable name="refname" select="@name"/>
	  <xsl:for-each select="ancestor::namespace/resource[@name=$refname]">
	    <xsl:attribute name="action">
	      <xsl:choose>
		<xsl:when test="@resource='Properties'">
		  <xsl:text>resolve</xsl:text>
		</xsl:when>
		<xsl:otherwise>
		  <xsl:text>set</xsl:text>
		</xsl:otherwise>
	      </xsl:choose>
	    </xsl:attribute>
	  </xsl:for-each>
	  <xsl:for-each select="ancestor::namespace/resource[@name=$defname]">
	    <xsl:for-each select="element[@name=$elementname]">
	      <xsl:attribute name="id">
		<xsl:call-template name="idtoqname">
		  <xsl:with-param name="id">
		    <xsl:value-of select="@tokenid"/>
		  </xsl:with-param>
		</xsl:call-template>
	      </xsl:attribute>
	    </xsl:for-each>
	  </xsl:for-each>
	</xsl:for-each>
      </element>	
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="sprm">
    <sprm>
      <xsl:call-template name="sprminner"/>
    </sprm>
  </xsl:template>

  <xsl:template name="attributeinner">
    <xsl:param name="parent"/>
    <xsl:for-each select=".//rng:ref[not(ancestor::rng:element or ancestor::rng:attribute)]">
      <xsl:variable name="refname" select="@name"/>
      <xsl:comment><xsl:value-of select="$newparent"/></xsl:comment>
      <xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$refname]">
	<xsl:call-template name="attributeinner">
	  <xsl:with-param name="parent" select="$parent"/>
	</xsl:call-template>
      </xsl:for-each>
    </xsl:for-each>
    <xsl:for-each select=".//rng:element">
      <xsl:variable name="newparent">
	<xsl:if test="string-length($parent)">
	  <xsl:value-of select="$parent"/>
	  <xsl:text>:</xsl:text>
	</xsl:if>
	<xsl:value-of select="@name"/>
      </xsl:variable>
      <xsl:for-each select="rng:ref">
	<xsl:variable name="refname" select="@name"/>
	<xsl:for-each select="ancestor::namespace/rng:grammar/rng:define[@name=$refname]">
	  <xsl:call-template name="attributeinner">
	    <xsl:with-param name="parent" select="$newparent"/>
	  </xsl:call-template>
	</xsl:for-each>
      </xsl:for-each>
    </xsl:for-each>
    <xsl:variable name="defname" select="@name"/>
    <xsl:variable name="resource">
      <xsl:for-each select="ancestor::namespace/resource[@name=$defname]">
	<xsl:value-of select="@resource"/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:if test="$resource='Properties'">
      <xsl:for-each select=".//rng:attribute">
	<xsl:variable name="attrname" select="@name"/>
	<attribute>	  
	  <xsl:attribute name="name">
	    <xsl:if test="string-length($parent) > 0">
	      <xsl:value-of select="$parent"/>
	      <xsl:text>:</xsl:text>
	    </xsl:if>
	    <xsl:value-of select="$attrname"/>
	  </xsl:attribute>
	  <xsl:for-each select="ancestor::namespace/resource[@name=$defname]">
	    <xsl:for-each select="attribute[@name=$attrname]">
	      <xsl:attribute name="id">
		<xsl:call-template name="idtoqname">
		  <xsl:with-param name="id">
		    <xsl:value-of select="@tokenid"/>
		  </xsl:with-param>
		</xsl:call-template>
	      </xsl:attribute>
	    </xsl:for-each>
	  </xsl:for-each>
	</attribute>
      </xsl:for-each>
    </xsl:if>
  </xsl:template>

  <xsl:template name="attribute">
    <attribute>
      <xsl:call-template name="attributeinner"/>
    </attribute>
  </xsl:template>

</xsl:stylesheet>