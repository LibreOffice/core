<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
<!--***********************************************************************
 *
 *  $RCSfile: data_val.xsl,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 13:48:15 $
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

<!-- ************************************************************************************** -->
<!-- * Transformation from New Format XCS & XCU to schema description					*** -->
<!-- ************************************************************************************** -->
<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
				xmlns:xsi="http://www.w3.org/1999/XMLSchema-instance"
				xmlns:xs="http://www.w3.org/2001/XMLSchema"
				xmlns:oor="http://openoffice.org/2001/registry"
				xmlns:install="http://openoffice.org/2004/installation"
				xmlns:filehelper="http://www.jclark.com/xt/java/org.openoffice.configuration.FileHelper"			
				extension-element-prefixes="filehelper">

<!-- Get the correct format -->
<xsl:output method="xml" indent="yes" />

<xsl:param name="xcs"/>
<xsl:param name="schemaRoot">.</xsl:param>

<xsl:variable name="schemaRootURL"><xsl:value-of select="filehelper:makeAbs($schemaRoot)"/></xsl:variable>
<xsl:variable name="schemaURL"><xsl:value-of select="filehelper:makeAbs($xcs)"/></xsl:variable>

<!-- ************************************** -->
<!-- * oor:component-data							*** -->
<!-- ************************************** -->
	<xsl:template match="/oor:component-data">		
        <xsl:variable name="component-schema" select="document($schemaURL)/oor:component-schema"/>
		<xsl:for-each select="node|prop">
			<xsl:apply-templates select=".">				
				<xsl:with-param name="context" select="$component-schema/component/*[@oor:name = current()/@oor:name]"/>							
				<xsl:with-param name="component-schema" select="$component-schema"/>							
			</xsl:apply-templates>
		</xsl:for-each>				
	</xsl:template>

<!-- ****************************************** -->
<!-- * handle template references           *** -->
<!-- ****************************************** -->
	<xsl:template name="resolve-template">
		<xsl:param name = "node-type"/>
		<xsl:param name = "schema-type"/>
		<xsl:param name = "component-schema"/>
		<xsl:variable name = "path">
			<xsl:call-template name="collectPath"/>
		</xsl:variable>
		
		<xsl:if test="not ($component-schema)">
			<xsl:message terminate="yes">ERROR: Template '<xsl:value-of select="$node-type"/>', 
                                         referenced from node '<xsl:value-of select="$path"/>' 
                                         does not exist in schema!
            </xsl:message>
		</xsl:if>

        <xsl:choose>
            <xsl:when test="$schema-type='node-ref'">
                <xsl:apply-templates select=".">				
                    <xsl:with-param name="context" select="$component-schema/templates/*[@oor:name = $node-type]"/>																					
                    <xsl:with-param name="component-schema" select="$component-schema"/>							
                </xsl:apply-templates>
            </xsl:when>
            <xsl:when test="$schema-type='set'">
                <xsl:for-each select="node|prop">						
                    <xsl:apply-templates select=".">				
                        <xsl:with-param name="context" select="$component-schema/templates/*[@oor:name = $node-type]"/>																					
                        <xsl:with-param name="component-schema" select="$component-schema"/>							
                    </xsl:apply-templates>
                </xsl:for-each>
            </xsl:when>
            <xsl:otherwise>
                <xsl:message terminate="true">ERROR: The schema element for node <xsl:value-of select="$path"/>
                                              is a <xsl:value-of select="$schema-type"/> and should not have a node-type.
                </xsl:message>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
<!-- ****************************************** -->
<!-- * node									*** -->
<!-- ****************************************** -->
	<xsl:template match="node">
		<xsl:param name = "context"/>
		<xsl:param name = "component-schema"/>
		<xsl:variable name = "path">
			<xsl:call-template name="collectPath"/>
		</xsl:variable>
		
		<xsl:if test="not ($context)">
			<xsl:message terminate="yes">ERROR: Node '<xsl:value-of select="$path"/>' does not exist in schema!</xsl:message>
		</xsl:if>

        <xsl:call-template name="checkModule"/>

		<xsl:choose>
			<!-- look for matching templates in other components -->
			<xsl:when test="$context/@oor:node-type and $context/@oor:component">
				<xsl:variable name="fileURL">
					<xsl:call-template name="composeFileURL">
						<xsl:with-param name="componentName"><xsl:value-of select="$context/@oor:component"/></xsl:with-param>
					</xsl:call-template>
				</xsl:variable>

                <xsl:call-template name="resolve-template">
                    <xsl:with-param name="node-type" select="$context/@oor:node-type"/>
                    <xsl:with-param name="schema-type" select="local-name($context)"/>
                    <xsl:with-param name="component-schema" select="document($fileURL)/oor:component-schema"/>
                </xsl:call-template>
			</xsl:when>
			<!-- look for matching templates within the same component -->
			<xsl:when test="$context/@oor:node-type">
                <xsl:call-template name="resolve-template">
                    <xsl:with-param name="node-type" select="$context/@oor:node-type"/>
                    <xsl:with-param name="schema-type" select="local-name($context)"/>
                    <xsl:with-param name="component-schema" select="$component-schema"/>
                </xsl:call-template>
			</xsl:when>
			<!-- is the node extensible ? -->
			<xsl:when test="$context/@oor:extensible='true'">
				<xsl:for-each select="node|prop">						
					<xsl:apply-templates select="." mode="extensible">				
					    <xsl:with-param name="context" select="$context/*[@oor:name = current()/@oor:name]"/>																					
                        <xsl:with-param name="component-schema" select="$component-schema"/>							
				    </xsl:apply-templates>
				</xsl:for-each>
			</xsl:when>
			<xsl:otherwise>	
				<xsl:for-each select="node|prop">						
					<xsl:apply-templates select=".">				
						<xsl:with-param name="context" select="$context/*[@oor:name = current()/@oor:name]"/>																					
                        <xsl:with-param name="component-schema" select="$component-schema"/>							
					</xsl:apply-templates>
				</xsl:for-each>
			</xsl:otherwise>
		</xsl:choose>	
	</xsl:template>

<!-- ****************************************** -->
<!-- * prop									*** -->
<!-- ****************************************** -->
	<xsl:template match="prop">
		<xsl:param name = "context"/>
		<xsl:variable name = "path">
			<xsl:call-template name="collectPath"/>
		</xsl:variable>
		

		<xsl:if test="not ($context)">
			<xsl:message terminate="yes">ERROR: Property '<xsl:value-of select="$path"/>' does not exist in schema !</xsl:message>
		</xsl:if>

		<xsl:if test="@oor:op">
			<xsl:message terminate="yes">ERROR: Property '<xsl:value-of select="$path"/>' has unexpected operation '<xsl:value-of select="@oor:op"/>' !</xsl:message>
		</xsl:if>

        <xsl:call-template name="checkModule"/>
		
	</xsl:template>

<!-- ****************************************** -->
<!-- * node (mode:extensible) - not supported * -->
<!-- ****************************************** -->
	<xsl:template match="node" mode="extensible">
		<xsl:variable name = "path">
			<xsl:call-template name="collectPath"/>
		</xsl:variable>

        <xsl:message terminate="yes">ERROR: Node '<xsl:value-of select="$path"/>' is within an extensible node!</xsl:message>
    </xsl:template>
<!-- ****************************************** -->
<!-- * prop (mode:extensible)				*** -->
<!-- ****************************************** -->
	<xsl:template match="prop" mode="extensible">
		<xsl:param name = "context"/>
		<xsl:variable name = "path">
			<xsl:call-template name="collectPath"/>
		</xsl:variable>

        <xsl:choose>
            <xsl:when test="not(@oor:op)">
		        <xsl:if test="not ($context)">
			        <xsl:message terminate="yes">ERROR: Property '<xsl:value-of select="$path"/>' does not exist in schema!</xsl:message>
		        </xsl:if>
		    </xsl:when>		
            <xsl:when test="@oor:op='replace'">
		        <xsl:if test="not (@oor:type)">
			        <xsl:message terminate="yes">ERROR: Property '<xsl:value-of select="$path"/>' does not have a type!</xsl:message>
		        </xsl:if>		
		    </xsl:when>		
		    <xsl:otherwise>		
			    <xsl:message terminate="yes">ERROR: Property '<xsl:value-of select="$path"/>' has unexpected operation '<xsl:value-of select="@oor:op"/>'!</xsl:message>
		    </xsl:otherwise>		
        </xsl:choose>

        <xsl:call-template name="checkModule"/>
	</xsl:template>


<!-- ************************************* -->
<!-- * checkModule					   *** -->
<!-- ************************************* -->
	<xsl:template name="checkModule">		
		<xsl:if test="@install:module">
            <xsl:if test="ancestor::*[@install:module]">
                <xsl:message terminate="yes">ERROR: Nested modules are not supported.  Found module '<xsl:value-of select="@install:module"/>' within module '<xsl:value-of select="ancestor::*/@install:module"/>'!
                </xsl:message>
            </xsl:if> 
		</xsl:if> 
	</xsl:template>

<!-- ************************************* -->
<!-- * collectPath					   *** -->
<!-- ************************************* -->
	<xsl:template name="collectPath">		
		<xsl:for-each select="ancestor-or-self::node()[@oor:name]">
			<xsl:text>/</xsl:text><xsl:value-of select="@oor:name"/>
		</xsl:for-each> 
	</xsl:template>


<!-- ****************************** -->
<!-- * composeFileURL			*** -->
<!-- ****************************** -->
	<xsl:template name="composeFileURL">
		<xsl:param name="componentName"/>
		<xsl:variable name="fileURL">
			<xsl:value-of select="$schemaRootURL"/>/<xsl:value-of select="translate($componentName,'.','/')"/>.xcs
		</xsl:variable>		
		<xsl:value-of select="$fileURL"/>
	</xsl:template>


</xsl:transform>

