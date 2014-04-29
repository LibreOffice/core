<?xml version="1.0" encoding="UTF-8"?>
<!--
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
-->
<!-- =================================================

This template is a skeleton for single level TOC pages 
with Frames : it generates the main index.html which contains
the frameset.

Do not overwrite this ! copy it and complete the missing
code.

I use the @ character wherever there is a missing code, so
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
	
	<xsl:include href="../layout.xsl"/>
    

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

		<!--
			@ Add frameset here...
			
			  the following noframes tag is
			  naturally optional.
			-->
		
		<noframes/>
		
		<!-- the body tag has no influence here,
			we add it for good style.
			-->
			
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
