<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xt="http://www.jclark.com/xt"
                extension-element-prefixes="xt">
                
  <xsl:output method = "HTML" doctype-public = "-//W3C//DTD HTML 3.2//EN" /> 
  	
  <!-- inputvariable declaration -->
  <xsl:param name="os1"/>
  <xsl:param name="gui1"/>
  <xsl:param name="cp1" />
  <xsl:param name="com1"/>
  <xsl:param name="lang1"/>
  <xsl:param name="type"/>
  <xsl:param name="file"/>
  <xsl:strip-space elements="*"/>

<xsl:template match="/Readme">
  <!-- creates the HTML-output-->
  <xsl:if test="$type='html'">
   <!-- the outputname for htlm-files-->
    <xt:document method="html" href="{$file}">
                  <html>
	<!-- HTML utf-8 encoding enable -->
  		<head>
  			<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  		</head>
  		<body>
  			<xsl:apply-templates/>
  		</body>
  	</html>
  	
  	</xt:document>
  </xsl:if>
  <!-- creates the Textoutput -->
  <xsl:if test="$type='text'">
    <!-- the outputfilename for Textfiles -->
    <xt:document method="text" href="{$file}">
    	<xsl:apply-templates/>
    </xt:document>
  </xsl:if>
</xsl:template>

<xsl:template match="Section">
  		<xsl:apply-templates/>
</xsl:template>


<xsl:template match="Paragraph">
	<!-- match the given parameters one of the xmlattributes?  -->
	<xsl:if test="@os=$os1 or @os='all'">
		<xsl:if test="@cpuname=$cp1 or @cpuname='all'">
			<xsl:if test="@gui=$gui1 or @gui='all'">
				<xsl:if test="@com=$com1 or  @com='all'">
					<!--html-output -->
					<xsl:if test="$type='html'">
						<xsl:element name="{@style}">
							<xsl:apply-templates/>
						</xsl:element>
					</xsl:if>
					<!--text-output -->
					<xsl:if test="$type='text'">
						<xsl:if test="@style='h1' or @style='h2' or  @style='h3' or  @style='H1' or @style='H2' or  @style='H3'">
							<xsl:choose>
								<xsl:when test='$os1="WNT"'>
									<xsl:text>&#xD;&#xA;------------------------------------------------------------&#xD;&#xA;</xsl:text>
								</xsl:when>
								<xsl:otherwise>
									<xsl:text>&#xA;------------------------------------------------------------&#xA;</xsl:text>
								</xsl:otherwise>
							</xsl:choose>								
						</xsl:if>
						<!-- enter an carriage return line feed -->
						<xsl:if test="@style='p' or @style='P'">
							<xsl:choose>
								<xsl:when test='$os1="WNT"'>
									<xsl:text>&#xD;&#xA;</xsl:text>
								</xsl:when>
								<xsl:otherwise>
									<xsl:text>&#xA;</xsl:text>
								</xsl:otherwise>
							</xsl:choose>								
						</xsl:if>
						<xsl:apply-templates/>
						<xsl:if test="@style='h1' or @style='h2' or  @style='h3' or  @style='hr' or  @style='H1' or @style='H2' or  @style='H3' or  @style='HR'">
							<xsl:choose>
								<xsl:when test='$os1="WNT"'>
									<xsl:text>------------------------------------------------------------&#xD;&#xA;</xsl:text>
								</xsl:when>
								<xsl:otherwise>
									<xsl:text>------------------------------------------------------------&#xA;</xsl:text>
								</xsl:otherwise>
							</xsl:choose>								
						</xsl:if>
					</xsl:if>
				</xsl:if>
			</xsl:if>
		</xsl:if>
	</xsl:if> 
</xsl:template>

<xsl:template match="List">
	<!-- creates the listoutput, the html-tags will be ignored if textoutput is choosen -->
	<xsl:choose>
	<xsl:when test="@Enum='false'">
		<ul type="circle">
			<xsl:apply-templates/>
		</ul>
	</xsl:when>
	<xsl:when test="@Enum='true'">
		<ol>
			<xsl:apply-templates/>
		</ol>
	</xsl:when>
	</xsl:choose>
</xsl:template>

<xsl:template match="List//Text">
	<!-- create the list -->
	<xsl:if test="$type='html'">
		<xsl:if test="@xml:lang=$lang1">
			<li><xsl:apply-templates/></li>
		</xsl:if>
	</xsl:if>
	<xsl:if test="$type='text'">
		<xsl:if test="@xml:lang=$lang1">
			<xsl:text>- </xsl:text>
			<xsl:apply-templates/>
			<xsl:choose>
				<xsl:when test='$os1="WNT"'>
					<xsl:text>&#xD;&#xA;</xsl:text>
				</xsl:when>
				<xsl:otherwise>
					<xsl:text>&#xA;</xsl:text>
				</xsl:otherwise>
			</xsl:choose>								
		</xsl:if>
	</xsl:if>
</xsl:template>

<!-- the textoutput of normal paragraph nodes -->
<!-- these template will be used if an attribute xml:lang exists -->
<xsl:template match="Section/Paragraph/Text[@xml:lang]">
	<xsl:if test="@xml:lang=$lang1">
		<!-- make the output with the given template -->
		<xsl:call-template name="out"/>
	</xsl:if>
</xsl:template>

<!-- these template will be used if no attribute xml:lang exists -->
<xsl:template match="Section/Paragraph/Text[not (@xml:lang)]">
	<xsl:call-template name="out"/>
</xsl:template>

<!-- creates the outputtemplate out -->
<xsl:template name="out">
	<!--  creates output with an extra CR/LF -->
	<xsl:if test="@Wrap='true'">
		<xsl:if test="$type='html'">
			<xsl:element name="br"/>
			<xsl:call-template name="prelinked"/>
		</xsl:if>
		<xsl:if test="$type='text'">
			<xsl:choose>
				<xsl:when test='$os1="WNT"'>
					<xsl:text>&#xD;&#xA;</xsl:text>
				</xsl:when>
				<xsl:otherwise>
					<xsl:text>&#xA;</xsl:text>
				</xsl:otherwise>
			</xsl:choose>								
			<xsl:call-template name="linked"/>
		</xsl:if>
	</xsl:if>

	<!-- here without the extra CR/LF-->
	<xsl:if test="@Wrap='false'">
		<xsl:if test="$type='html'">
			<xsl:call-template name="prelinked"/>
		</xsl:if>
		<xsl:if test="$type='text'">
			<xsl:call-template name="linked"/>
		</xsl:if>
	</xsl:if>
</xsl:template>

<xsl:template name="prelinked">
	<xsl:if test=" @style='b ' or @style='B ' or @style='i ' or @style='I ' or @style='u ' or @style='U '">
		<xsl:element name="{@style}">
			<xsl:call-template name="linked"/>
		</xsl:element>
	</xsl:if>
	<xsl:if test="@style='none'">
		<xsl:call-template name="linked"/>
	</xsl:if>
</xsl:template>

<xsl:template name="linked">
	<xsl:if test="$type='html'">
		<!--this template creates a link if the url attribute is true -->
		<xsl:if test="@url='false'">
			<xsl:apply-templates/>
		</xsl:if>
		<xsl:if test="@url='true'">
	      		<xsl:if test="@path='none'">
				<a><xsl:attribute name="href">	<xsl:apply-templates/></xsl:attribute><xsl:apply-templates/></a>	
	      		</xsl:if>
	      		<xsl:if test="@path='file'">
				<a><xsl:attribute name="href"><xsl:text>file://localhost/</xsl:text><xsl:apply-templates/></xsl:attribute><xsl:apply-templates/></a>	
	      		</xsl:if>
	      		<xsl:if test="@path='url'">
				<a><xsl:attribute name="href"><xsl:text>http://</xsl:text><xsl:apply-templates/></xsl:attribute><xsl:apply-templates/></a>	
	      		</xsl:if>
	      	</xsl:if>
	      	<!-- this code is needed for the line before an link, if not an <p> will be added-->
	      	<xsl:if test="@url='ahead'">
	      		<xsl:apply-templates/>
	      	</xsl:if>
	</xsl:if>
	<xsl:if test="$type='text'">
		<xsl:if test="@url='false'">
			<xsl:apply-templates/>
			<xsl:choose>
				<xsl:when test='$os1="WNT"'>
					<xsl:text>&#xD;&#xA;</xsl:text>
				</xsl:when>
				<xsl:otherwise>
					<xsl:text>&#xA;</xsl:text>
				</xsl:otherwise>
			</xsl:choose>								
		</xsl:if>
		<xsl:if test="@url='true' or @url='ahead'">
			<xsl:apply-templates/>
		</xsl:if>
	</xsl:if>
</xsl:template>

</xsl:stylesheet>
