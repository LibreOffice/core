<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
<!--***********************************************************************
 *
 *  $RCSfile: schema_trim.xsl,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 13:48:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
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
