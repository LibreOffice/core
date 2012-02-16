<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- <xsl:output method="text" doctype-public="-//W3C//DTD HTML 3.2//EN" omit-xml-declaration="yes"/> --> 
  	
  <!-- inputvariable declaration -->
	<xsl:param name="os1"/>
	<xsl:param name="gui1"/>
	<xsl:param name="cp1"/>
	<xsl:param name="com1"/>
	<xsl:param name="lang1"/>
	<xsl:param name="type"/>
	<xsl:param name="file"/>
	<xsl:strip-space elements="*"/>
	
	<xsl:param name="platform">
		<xsl:if test="$os1='LINUX'">
			<xsl:value-of select="'LINUX'"/>
		</xsl:if>
		<xsl:if test="$os1='WNT'">
			<xsl:value-of select="'WIN'"/>
		</xsl:if>
		<xsl:if test="$os1='SOLARIS'">
			<xsl:if test="$cp1='SPARC'">
				<xsl:value-of select="'SOLSPARC'"/>
			</xsl:if>
			<xsl:if test="$cp1='INTEL'">
				<xsl:value-of select="'SOLX86'"/>
			</xsl:if>
		</xsl:if>
		<xsl:if test="$os1='MACOSX'">
			<xsl:value-of select="'MAC'"/>
		</xsl:if>
	</xsl:param>
	
	<xsl:param name="lf">
		<xsl:choose>
			<xsl:when test="$platform='WIN'"><xsl:text>&#xD;&#xA;</xsl:text></xsl:when>
			<xsl:otherwise><xsl:text>&#xA;</xsl:text></xsl:otherwise>
		</xsl:choose>
	</xsl:param>
	
	<xsl:template match="/">
		<xsl:choose>
			<xsl:when test="$type='html'">
			
				<xsl:document method="html" href="{$file}" doctype-public="-//W3C//DTD HTML 3.2//EN">
					<xsl:apply-templates mode="html"/>
				</xsl:document>
			
			</xsl:when>
			<xsl:when test="$type='text'">
				
				<xsl:document method="text" href="{$file}">
					<xsl:call-template name="textout" />
				</xsl:document>
			
			</xsl:when>
		</xsl:choose>
	</xsl:template>
	
	<xsl:template match="*" mode="html">
		<xsl:choose>
			<xsl:when test="(contains(@class,$platform) or not(@class) or (name(.)='p'))">
				<xsl:if test="(@xml:lang=$lang1 or not(@xml:lang))"> <!-- check for correct language -->
					<xsl:element name="{name(.)}">
						<xsl:if test="(name(.)='a')">
							<xsl:attribute name="href">
								<xsl:value-of select="@href"/>
							</xsl:attribute>
						</xsl:if>
						<xsl:if test="(name(.)='p') and (@class)">
							<xsl:attribute name="class">
								<xsl:value-of select="@class"/>
							</xsl:attribute>
						</xsl:if>
						<xsl:if test="name(.)='div'">
							<xsl:attribute name="id">
								<xsl:value-of select="@id"/>
							</xsl:attribute>
						</xsl:if>
						<xsl:apply-templates mode="html"/>
					</xsl:element>
				</xsl:if>
			</xsl:when>
			<xsl:otherwise>
			
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<xsl:template name="textout">
		<xsl:apply-templates />
	</xsl:template>
	
	<xsl:template match="*">
		<xsl:if test="(@xml:lang=$lang1 or not(@xml:lang))"> <!-- check for correct language -->
		<xsl:choose>
			<xsl:when test="name(.)='html'">
				<xsl:apply-templates/>
			</xsl:when>
			<xsl:when test="name(.)='body'">
				<xsl:apply-templates/>
			</xsl:when>
			<xsl:when test="name(.)='hr'">
				<xsl:text>
			
----------------------------------------------------------------------------------------------------------

</xsl:text>
			</xsl:when>
			<xsl:when test="name(.)='div'">
				<xsl:if test="(contains(@class,$platform) or not(@class))">
					<xsl:apply-templates/>
				</xsl:if>
			</xsl:when>
			<xsl:when test="name(.)='a'">
				<xsl:apply-templates/>
			</xsl:when>
			<xsl:when test="name(.)='tt'">
				<xsl:apply-templates/>
			</xsl:when>
			<xsl:when test="name(.)='li'">
				<xsl:value-of select="$lf"/>* <xsl:apply-templates/>
			</xsl:when>
			<xsl:when test="name(.)='ul'">
				<xsl:value-of select="$lf"/>
				<xsl:apply-templates/>
			</xsl:when>
			<xsl:when test="name(.)='ol'">
				<xsl:value-of select="$lf"/>
				<xsl:apply-templates/>
			</xsl:when>
			<xsl:when test="name(.)='p'">
				<xsl:if test="(not(name(..)='li') and (count(a) = 0))">
					<xsl:value-of select="$lf"/>
					<xsl:value-of select="$lf"/>
				</xsl:if>
				<xsl:apply-templates/>
			</xsl:when>
			<xsl:when test="name(.)='h1'">
				<xsl:value-of select="$lf"/>
				<xsl:text>======================================================================</xsl:text>
				<xsl:value-of select="$lf"/>
				<xsl:apply-templates/>
				<xsl:value-of select="$lf"/>
				<xsl:text>======================================================================</xsl:text>
				<xsl:value-of select="$lf"/>
			</xsl:when>
			<xsl:when test="name(.)='h2'">
				<xsl:value-of select="$lf"/>
				<xsl:value-of select="$lf"/>
				<xsl:text>----------------------------------------------------------------------</xsl:text>
				<xsl:value-of select="$lf"/>
				<xsl:apply-templates/>
				<xsl:value-of select="$lf"/>
				<xsl:text>----------------------------------------------------------------------</xsl:text>
			</xsl:when>
			<xsl:when test="name(.)='h3'">
				<xsl:value-of select="$lf"/>
				<xsl:value-of select="$lf"/>
				<xsl:apply-templates/>
				<xsl:value-of select="$lf"/>
				<xsl:text>----------------------------------------------------------------------</xsl:text>
			</xsl:when>
		</xsl:choose>
	</xsl:if>
	</xsl:template>

</xsl:stylesheet>
