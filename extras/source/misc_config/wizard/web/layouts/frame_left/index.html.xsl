<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0" 
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns="http://www.w3.org/1999/xhtml">

	<xsl:output method               = "html"
                media-type           = "text/html"
                indent               = "yes"
                doctype-public       = "-//W3C//DTD HTML 4.0 Transitional//EN"
                omit-xml-declaration = "yes"
                standalone           = "yes" />
    

	 <!-- =============================
	               ROOT
	 ================================== -->


    <xsl:template  match="/">
      	<html>
				   <xsl:call-template name="head"/>
			      <xsl:call-template name="frameset"/>
		   </html>
	 </xsl:template>
	 
	 
	 <!-- =============================
	               HTML FRAMES
	 ================================== -->
	 
	
	<xsl:template name="frameset">
		<frameset rows="*" cols="284,*" framespacing="0" frameborder="NO" border="0">
		  	<frame src="tocframe.html" name="tocframe" scrolling="Auto" noresize=""/>
		  	<frame src="mainframe.html" name="mainframe"/>
		</frameset>
		<noframes/>
		<body>
		</body>
	</xsl:template>
	

	 <!-- =============================
	               HTML HEAD
	               
    this section should not be changed
	 ================================== -->
	 
	 <xsl:template name="head">
	 	<head>
				<title>
				 	<xsl:value-of select="/session/general-info/@title"/>
				</title>
				<!-- <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"> -->
				<meta name="description" content="{/session/general-info/@description}"/>
				<meta name="keywords" content="{/session/general-info/@keywords}"/>
				<meta name="author" content="{/session/general-info/@author}"/>
				<meta name="email" content="{/session/general-info/@email}"/>
				<meta name="copyright" content="{/session/general-info/@copyright}"/>
				<!-- create date?
				     update date?
				     fav icon?
				     -->
  			  <link REL="shortcut icon" href="images/favicon.ico" type="image/ico"/>
		     <link href="style.css" rel="stylesheet" type="text/css"/>

		</head>
	 </xsl:template>
	
</xsl:stylesheet>
