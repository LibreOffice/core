<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
<!--***********************************************************************
 *
 *  $RCSfile: alllang.xsl,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dg $ $Date: 2002-05-06 18:45:06 $
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

<xsl:transform 	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
		xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
		xmlns:xs="http://www.w3.org/2001/XMLSchema"
		xmlns:oor="http://openoffice.org/2001/registry">
		
<xsl:output method="xml" indent="yes"/>

<!--************************** PARAMETER ******************************** -->

<!-- locales, delimeter is colon -->
<xsl:param name="locale"/>

<!--************************** TEMPLATES ******************************** -->

<!-- ensure that at least root is available -->
	<xsl:template match="/oor:node">		
		<xsl:copy>
			<xsl:apply-templates select = "@*" />
			<xsl:apply-templates/>					
		</xsl:copy>		
	</xsl:template>

<!-- process all elements -->
	<xsl:template match="*">		
		<xsl:variable name="hasValue"><xsl:call-template name="hasValue"/></xsl:variable>
		<xsl:if test="$hasValue = 'true'">
			<xsl:copy>
				<xsl:apply-templates select = "@*" />
				<xsl:apply-templates/>					
			</xsl:copy>
		</xsl:if>		
	</xsl:template>

<!-- process values dependent on the locales -->
	<xsl:template match="value">		
		<xsl:choose>
			<!-- locale independent value -->
			<xsl:when test="not (string-length($locale))">				
				<xsl:choose>				
					<xsl:when test="not (@xml:lang)">
						<xsl:copy>
							<xsl:apply-templates select = "@*" />
							<xsl:value-of select="."/>
						</xsl:copy>
					</xsl:when>
					<!-- if no locale independent value available, take en-US as default -->
					<xsl:when test="lang('en-US') and not (../value[not (@xml:lang)])">
						<xsl:copy>
							<xsl:apply-templates select = "@xsi:nil|@oor:separator" />
							<xsl:value-of select="."/>
						</xsl:copy>
					</xsl:when>
				</xsl:choose>
			</xsl:when>
			<!-- test for matching locale -->
			<xsl:when test="@xml:lang = $locale">
				<xsl:copy>
					<xsl:apply-templates select = "@*" />
					<xsl:value-of select="."/>
				</xsl:copy>
			</xsl:when>
		</xsl:choose>
	</xsl:template>

<!-- copy of all attributes of current node -->
	<xsl:template match = "@*">
		<xsl:copy/>
	</xsl:template>

<!-- suppress the location of the schema -->
	<xsl:template match = "@xsi:schemaLocation"/>

<!-- determine if locale specific value is within the tree -->
	<xsl:template name="hasValue">		
		<xsl:choose>
			<xsl:when test="not (string-length($locale))">
				<xsl:value-of select="true()"/>
			</xsl:when>
			<xsl:when test="count(descendant::value[@xml:lang != ''])">
				<xsl:value-of select="true()"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="false()"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

</xsl:transform>
