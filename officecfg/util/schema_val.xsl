<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
<!--***********************************************************************
 *
 *  $RCSfile: schema_val.xsl,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 13:50:33 $
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

<!-- Parameter -->
<xsl:param name="root">../registry/schema</xsl:param>
<xsl:param name="pathSeparator">/</xsl:param>
<xsl:param name="file"/>
<xsl:variable name="componentName"><xsl:value-of select="translate($file,$pathSeparator,'.')"/></xsl:variable>

	<xsl:template match = "/">
		<xsl:message terminate="no">CHECKING CONSISTENCY ...</xsl:message>
		<xsl:apply-templates/>
	</xsl:template>
	
<!-- make sure that missing features are not invoked -->
	<xsl:template match = "item">
		<xsl:message terminate="yes">ERROR: multiple template types for sets are NOT supported!</xsl:message>
	</xsl:template>

	<xsl:template match = "set[@oor:extensible='true']">
		<xsl:message terminate="yes">ERROR: extensible sets are currently NOT supported!</xsl:message>
	</xsl:template>

	<xsl:template match = "group[@oor:extensible='true']">
		<xsl:if test="count(child::prop) or count(child::set) or count(child::group) or count(child::node-ref)">
			<xsl:message terminate="yes">ERROR: extensible groups with children are currently NOT supported!</xsl:message>
		</xsl:if>
	</xsl:template>

<!-- check if properties of type 'any' do not have a value -->
	<xsl:template match="prop[@oor:type='oor:any']">
		<xsl:if test="count(value)"> 
			<xsl:message terminate="yes">ERROR: Properties of type 'oor:any' MUST NOT have a value!</xsl:message>
		</xsl:if>
	</xsl:template>

<!-- inhibit (explicit) NIL values -->
	<xsl:template match="value[@xsi:nil]">
		<xsl:message terminate="yes">ERROR: xsi:nil is not allowed in schemas !</xsl:message>
	</xsl:template>

<!-- validate for correct node references -->
	<xsl:template match="@oor:node-type">
		<xsl:choose>
			<xsl:when test="../@oor:component">
				<xsl:variable name ="file">
					<xsl:call-template name="locateFile"><xsl:with-param name="componentName" select="../@oor:component"/></xsl:call-template>
				</xsl:variable>
				<xsl:if test="not(document($file)/oor:component-schema/templates/*[@oor:name=current()])">
					<xsl:message terminate="yes">ERROR: node-type '<xsl:value-of select="current()"/>' not found!</xsl:message>
				</xsl:if>				
			</xsl:when>
			<xsl:when test="not(/oor:component-schema/templates/*[@oor:name=current()])">
				<xsl:message terminate="yes">ERROR: node-type '<xsl:value-of select="current()"/>' not found!</xsl:message>
			</xsl:when>
		</xsl:choose>		
	</xsl:template>

<!-- validate if file name matches component-name -->
	<xsl:template match="oor:component-schema">
		<xsl:variable name ="fullName"><xsl:value-of select="@oor:package"/>.<xsl:value-of select="@oor:name"/></xsl:variable>
		<xsl:if test="$fullName != $componentName">
			<xsl:message terminate="yes">ERROR: Component name '<xsl:value-of select="$fullName"/>' does not match with file name!</xsl:message>
		</xsl:if>			
		<xsl:apply-templates select="*|@*"/>
	</xsl:template>


<!-- locate a component file -->
	<xsl:template name="locateFile">
		<xsl:param name="componentName"/>
		<xsl:variable name ="file"><xsl:value-of select="$root"/>/<xsl:value-of select="translate($componentName,'.','/')"/>.xcs</xsl:variable>
		<xsl:if	test="not( document($file) )">
			<xsl:message terminate ="yes">**Error: unable to locate document '<xsl:value-of select="translate($componentName,'.','/')"/>.xcd'</xsl:message>
		</xsl:if>
		<xsl:value-of select="$file"/>
	</xsl:template>

</xsl:transform>
