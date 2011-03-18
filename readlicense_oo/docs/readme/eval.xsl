<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="html" />
	<xsl:template match="/Readme">
	<html>
	<head>
	<title></title>
	<style type="text/css">
		h1,h2,h3,h4,h5,p {margin:0px;}
		.mac {color: #000;}
		.wnt {color: #000;}
		.unx {color: #000;}
		.soli {color: #000;}
		.solx {color: #000;}
		.none {color: #000;}
		.all {color: #000;}		
		.section {background-color: #DEDEDE;}
		.comment {text-color: #666; font-size: xx-small;}
		body * {font-size: 9pt}
	</style>
	</head>
	<body>
	<table border='1' style='border-collapse:collapse;' cellpadding='5'>
	<tr>
	<th>attribs</th><th class="wnt">WNT</th><th class="mac">MAC</th><th class="soli">SOLI</th><th class="solx">SOLX</th><th class="unx">UNX</th><th>none</th><th>all</th><th>Content</th>
	</tr>
	<xsl:apply-templates />
	</table>
	</body>
	</html>
	</xsl:template>
	
	<xsl:template match="Section">
  	<tr>
		<th colspan="9" align='left' class="section">SECTION <xsl:value-of select="@id" /></th>
		</tr>
		<xsl:apply-templates />
	</xsl:template>
	
	<xsl:template match="Paragraph">
		<tr>
      <td>
				<xsl:if test="@os">os=<xsl:value-of select="@os"/></xsl:if><xsl:text> </xsl:text><xsl:if test="@gui">gui=<xsl:value-of select="@gui"/></xsl:if>
			</td>
			<th class="wnt"><xsl:if test="@os='WNT' or @gui='WNT'">WNT</xsl:if></th>
			<th class="mac"><xsl:if test="@os='MACOSX'">MAC</xsl:if></th>
			<th class="soli"><xsl:if test="@os='SOLARIS' and @cpuname='INTEL'">SOLI</xsl:if></th>
			<th class="solx"><xsl:if test="@os='SOLARIS' and @cpuname='SPARC'">SOLX</xsl:if></th>
			<th class="unx"><xsl:if test="@os='LINUX' or @gui='UNX'">UNX</xsl:if></th>
			<th class="none"><xsl:if test="@os='none'">NONE</xsl:if></th>
			<th class="all"><xsl:if test="@os='all'">ALL</xsl:if></th>
			<td>
				<xsl:choose>
					<xsl:when test="@xml:lang='x-comment'">
						<span class="comment"><xsl:apply-templates/></span>
					</xsl:when>
					<xsl:otherwise>
						<xsl:if test="not(@style='')">
							<xsl:text disable-output-escaping="yes">&lt;</xsl:text><xsl:value-of select="@style"/><xsl:text disable-output-escaping="yes">&gt;</xsl:text>
						</xsl:if>
						<xsl:apply-templates/>
						<xsl:if test="not(@style='')">
							<xsl:text disable-output-escaping="yes">&lt;/</xsl:text><xsl:value-of select="@style"/><xsl:text disable-output-escaping="yes">&gt;</xsl:text>
						</xsl:if>
					</xsl:otherwise>
				</xsl:choose>
				</td>
		</tr>
	</xsl:template>
	
	<xsl:template match="List">
		<xsl:choose>
		<xsl:when test="@enum='true'">
			<ol>
				<xsl:for-each select="child::*">
				<li><xsl:apply-templates/></li>
				</xsl:for-each>
			</ol>
		</xsl:when>
		<xsl:otherwise>
			<ul>
				<xsl:for-each select="child::*">
				<li><xsl:apply-templates/></li>
				</xsl:for-each>
			</ul>
		</xsl:otherwise>
		</xsl:choose>
		
	</xsl:template>
	
</xsl:stylesheet>

