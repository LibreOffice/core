<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
<!--***********************************************************************
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

 ************************************************************************ -->

<xsl:transform  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
		xmlns:xs="http://www.w3.org/2001/XMLSchema"		
		xmlns:oor="http://openoffice.org/2001/registry"		
		version="1.0">

<!-- Get the correct format -->
	<xsl:output method="xml" indent="yes"/>
	<xsl:namespace-alias stylesheet-prefix="xs" result-prefix="xs"></xsl:namespace-alias>


<!-- Remove all comments from the schema files -->
	<xsl:template match="*|@*">
	  <xsl:copy>
		<xsl:apply-templates select="*|@*"/>
	  </xsl:copy>
	</xsl:template>

<!-- suppress the location of the schema -->
	<xsl:template match = "@xsi:schemaLocation"/>

<!-- suppress the constraints of the schema
	<xsl:template match = "constraints"/>  -->

<!-- suppress all documentation items
	<xsl:template match = "info"/> -->

<!-- suppress constraints for deprecated items -->
	<xsl:template match = "constraints[../info/deprecated]"/>

<!-- suppress all documentation for deprecated items -->
	<xsl:template match = "desc[../deprecated]"/>
	<xsl:template match = "label[../deprecated]"/>

<!-- copy all other documentation with content -->
	<xsl:template match="desc|label">
		<xsl:copy>
			<xsl:apply-templates select="@*"/>
			<xsl:value-of select="."/>
		</xsl:copy>		
	</xsl:template>

<!-- suppress all author items -->
	<xsl:template match = "author"/>

<!-- suppress values, which are marked as nil -->
	<xsl:template match="value[@xsi:nil='true']" />

<!-- copy all other values with content -->
	<xsl:template match="value">
		<xsl:copy>
			<xsl:apply-templates select="*|@*"/>
			<xsl:value-of select="."/>
		</xsl:copy>		
	</xsl:template>

</xsl:transform>
