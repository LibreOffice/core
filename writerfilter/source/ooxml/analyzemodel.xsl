<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:rng="http://relaxng.org/ns/structure/1.0">

<xsl:include href="factorytools.xsl"/>
<xsl:output method="xml"/>

<xsl:template match="/">
	<analyze>
		<xsl:for-each select="/model/namespace[not(@todo='ignore')]">
			<xsl:call-template name="analyzegrammar"/>
		</xsl:for-each>
	</analyze>
</xsl:template>

<xsl:template name="analyzegrammar">
	<xsl:variable name="nsname" select="@name"/>
	<xsl:for-each select="rng:grammar/rng:define">
		<xsl:variable name="defname" select="@name"/>
		<xsl:for-each select=".//rng:attribute|.//rng:element">
			<xsl:choose>
				<xsl:when test="local-name()='element'">					
					<element>
						<xsl:call-template name="defineattrs">
							<xsl:with-param name="nsname" select="$nsname"/>
							<xsl:with-param name="defname" select="$defname"/>
						</xsl:call-template>
					</element>
				</xsl:when>
				<xsl:when test="local-name()='attribute'">
					<attribute>
						<xsl:call-template name="defineattrs">
							<xsl:with-param name="nsname" select="$nsname"/>
							<xsl:with-param name="defname" select="$defname"/>
						</xsl:call-template>
					</attribute>
				</xsl:when>
			</xsl:choose>
		</xsl:for-each>
	</xsl:for-each>
</xsl:template>

<xsl:template name="defineattrs">
	<xsl:param name="nsname"/>
	<xsl:param name="defname"/>

	<xsl:variable name="localname" select="local-name()"/>
	<xsl:variable name="name" select="@name"/>

	<xsl:attribute name="id">
		<xsl:value-of select="$nsname"/>
		<xsl:text>:</xsl:text>
		<xsl:value-of select="$defname"/>
		<xsl:text>:</xsl:text>
		<xsl:value-of select="@name"/>
	</xsl:attribute>	
	<xsl:for-each select="ancestor::namespace/resource[@name=$defname]">
		<xsl:attribute name="resource"><xsl:value-of select="@resource"/></xsl:attribute>
		<xsl:choose>
			<xsl:when test="$localname='attribute'">
				<xsl:for-each select="attribute[@name=$name and @tokenid]">
					<xsl:attribute name="tokenid"><xsl:value-of select="@tokenid"/></xsl:attribute>
					<xsl:attribute name="qname">
						<xsl:call-template name="idtoqname">
							<xsl:with-param name="id" select="@tokenid"/>
						</xsl:call-template>
					</xsl:attribute>
                    <xsl:attribute name="namespace">
                        <xsl:value-of select="$nsname"/>
                    </xsl:attribute>
                    <xsl:attribute name="define">
                        <xsl:value-of select="$defname"/>
                    </xsl:attribute>
                    <xsl:attribute name="name">
                        <xsl:value-of select="@name"/>
                    </xsl:attribute>
				</xsl:for-each>
			</xsl:when>
			<xsl:when test="$localname='element'">
				<xsl:for-each select="element[@name=$name and @tokenid]">
					<xsl:attribute name="tokenid"><xsl:value-of select="@tokenid"/></xsl:attribute>
					<xsl:attribute name="qname">
						<xsl:call-template name="idtoqname">
							<xsl:with-param name="id" select="@tokenid"/>
						</xsl:call-template>
					</xsl:attribute>
                    <xsl:attribute name="namespace">
                        <xsl:value-of select="$nsname"/>
                    </xsl:attribute>
                    <xsl:attribute name="define">
                        <xsl:value-of select="$defname"/>
                    </xsl:attribute>
                    <xsl:attribute name="name">
                        <xsl:value-of select="@name"/>
                    </xsl:attribute>
				</xsl:for-each>
			</xsl:when>
		</xsl:choose>
	</xsl:for-each>
</xsl:template>
</xsl:stylesheet>