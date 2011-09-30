<?xml version="1.0" encoding="UTF-8"?>
<!--*************************************************************************
 *
  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
  
  Copyright 2000, 2010 Oracle and/or its affiliates.
 
  OpenOffice.org - a multi-platform office productivity suite
 
  This file is part of OpenOffice.org.
 
  OpenOffice.org is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 3
  only, as published by the Free Software Foundation.
 
  OpenOffice.org is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License version 3 for more details
  (a copy is included in the LICENSE file that accompanied this code).
 
  You should have received a copy of the GNU Lesser General Public License
  version 3 along with OpenOffice.org.  If not, see
  <http://www.openoffice.org/license.html>
  for a copy of the LGPLv3 License.

 ************************************************************************-->
<!-- =================================================

This template is a skeleton for single level TOC pages 
Do not overwrite this ! copy it and complete the missing
code.

I use the @ character whereever there is a missing code, so 
you can use a simple find to navigate and find the
places...

====================================================== -->

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
			      <xsl:call-template name="body"/>
		   </html>
	 </xsl:template>
	 
	 
	 <!-- =============================
	         Document properties
	         
	 This section contains templates which
	 give the document properties...
	               
	 ================================== -->
	 
	 <!-- this tempaltes gives the
	 relative href of the document. To use
	 with the <a href="..."> attribute-->
	
	
	 <xsl:template match="document" mode="href">
	   <xsl:value-of select="concat(../@directory,'/')"/>
	   <xsl:if test="@dir">
	     <xsl:value-of select="concat(@dir,'/')"/>
	   </xsl:if>
	   <xsl:value-of select="@fn"/>
	 </xsl:template>
	 
	 
	<xsl:template match="document/@title">
	 	<xsl:param name="target" select="''"/>
		
	 	<span class="doctitle">
	 		<a>
	 		<xsl:attribute name="href"> 
				<xsl:apply-templates select=".." mode="href"/>
			</xsl:attribute>
			
			<xsl:if test=" $target != ''">
				<xsl:attribute name="target">
					<xsl:value-of select="$target"/>
				</xsl:attribute>
			</xsl:if>
			
			<xsl:value-of select="."/>
			</a>
	 	</span>
	 	<br/>
	 </xsl:template>
	

	 <xsl:template match="document/@description">
	 	<span class="docdescription">
	 		<xsl:value-of select="."/>
	 	</span>
	 	<br/>
	 </xsl:template>
	 

	 <xsl:template match="document/@author">
	 	<span class="docauthor">
	 		<xsl:value-of select="."/>
	 	</span>
	 	<br/>
	 </xsl:template>
	 

	 <xsl:template match="document/@create-date">
	 	<span class="doccreationdate">
	 		<xsl:value-of select="."/>
	 	</span>
	 	<br/>
	 </xsl:template>
	 

	 <xsl:template match="document/@update-date">
	 	<span class="doclastchangedate">
	 		<xsl:value-of select="."/>
	 	</span>
	 	<br/>
	 </xsl:template>
	 

	 <xsl:template match="document/@filename">
	 	<span class="docfilename">
	 		<xsl:value-of select="."/>
	 	</span>
	 	<br/>
	 </xsl:template>


	 <xsl:template match="document/@format">
	 	<span class="docfileformatinfo">
	 		<xsl:value-of select="."/>
	 	</span>
	 	<br/>
	 </xsl:template>


	 <xsl:template match="document/@pages">
	 	<span class="docnumberofpages">
	 		<xsl:value-of select="."/>
	 	</span>
	 	<br/>
	 </xsl:template>
	 

	 <xsl:template match="document/@size">
	 	<span class="docsizeinkb">
	 		<xsl:value-of select="."/>
	 	</span>
	 	<br/>
	 </xsl:template>
	 
	 <xsl:template match="document/@icon">
	   <img src="images/{.}"/>
	 	<br/>
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
				<meta HTTP-EQUIV="CONTENT-TYPE" CONTENT="text/html; charset=UTF-8"/>
				<meta name="description" content="{/session/general-info/@description}"/>
				<meta name="keywords" content="{/session/general-info/@keywords}"/>
				<meta name="author" content="{/session/general-info/@author}"/>
				<meta name="email" content="{/session/general-info/@email}"/>
				<meta name="copyright" content="{/session/general-info/@copyright}"/>
				<!-- create date?
				     update date?
				     fav icon?
				     -->
		     <link href="style.css" rel="stylesheet" type="text/css"/>

		</head>
	 </xsl:template>
	     
</xsl:stylesheet>
