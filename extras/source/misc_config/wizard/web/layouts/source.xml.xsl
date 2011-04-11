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
<!-- =================================
  
  This templates creates a source.xml file
  which is identicall to the source xml tree
  used for the transformation.
  This is may be usefull for deveopement/debuging
  of layouts.
  
  ==================================== -->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:redirect="http://xml.apache.org/xalan/redirect" 
  extension-element-prefixes="redirect">

   <xsl:output method="xml"/>
    
 
   <xsl:template  match="/">
	 	    <xsl:apply-templates mode="copy"/>
   </xsl:template>
	 
   <xsl:template match="@*|node()" mode="copy">
      <xsl:copy>
	     <xsl:apply-templates select="@*|node()" mode="copy"/>
	   </xsl:copy>
   </xsl:template>    
    
</xsl:stylesheet>
