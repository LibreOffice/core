<?xml version='1.0' encoding="UTF-8"?>
<!-- =====================================================================

  OpenOffice.org - a multi-platform office productivity suite

  $RCSfile: oovbaconsts.xsl,v $

  $Revision: 1.2 $

  last change: $Author: vg $ $Date: 2007-12-07 11:18:25 $

  The Contents of this file are made available subject to
  the terms of GNU Lesser General Public License Version 2.1.


    GNU Lesser General Public License Version 2.1
    =============================================
    Copyright 2005 by Sun Microsystems, Inc.
    901 San Antonio Road, Palo Alto, CA 94303, USA

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License version 2.1, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
    MA  02111-1307  USA

====================================================================== -->
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:redirect="http://xml.apache.org/xalan/redirect"
                extension-element-prefixes="redirect">
<xsl:output method="text" indent="no" omit-xml-declaration="no" version="1.0" encoding="UTF-8" />

    <!-- OPTIONAL: output directory for the IDL
                   Have to end with path separator -->
    <xsl:param name="IDL_DIRECTORY" />

    <xsl:key name="same-context-elements"
             match="/api/element[@type='constant']"
             use="normalize-space(source/context)" />

    <xsl:template match="/">
        <!-- once for each enumeration -->
        <xsl:for-each select="/api/element[@type='enumeration']">
            <xsl:sort select="normalize-space(source/name)" />

            <redirect:write select="concat($IDL_DIRECTORY, normalize-space(source/name),'.idl')">
                <xsl:call-template name="write-idl-prefix-1" />
                <xsl:call-template name="write-idl-prefix-2" />
                <!-- write IDL content -->
                <xsl:for-each select="key('same-context-elements', normalize-space(source/name))">
                    <xsl:call-template name="write-idl-content" />
                </xsl:for-each>
                <xsl:call-template name="write-idl-suffix" />
            </redirect:write>
        </xsl:for-each>
    </xsl:template>


    <!-- writing the IDL prefix -->
    <xsl:template name="write-idl-prefix-1">
	<xsl:variable name="submod" select="source/@id"/>
	<xsl:variable name="period" select="'.'"/>
	<xsl:variable name="lcletters">abcdefghijklmnopqrstuvwxyz</xsl:variable>
	<xsl:variable name="ucletters">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>
<xsl:text>
        module org { module openoffice { module </xsl:text><xsl:value-of select="translate(substring-before($submod,$period),$ucletters,$lcletters)"/>
    </xsl:template>
    <xsl:template name="write-idl-prefix-2">
<xsl:text> {
            constants </xsl:text><xsl:value-of select="normalize-space(source/name)" /><xsl:text> {</xsl:text>
    </xsl:template>

    <!-- writing the IDL content -->
    <xsl:template name="write-idl-content">
<xsl:text>
                const long </xsl:text><xsl:value-of select="source/name" /><xsl:text> = </xsl:text><xsl:value-of select="source/value" /><xsl:text>;</xsl:text>
    </xsl:template>

    <!-- writing the IDL suffix -->
    <xsl:template name="write-idl-suffix">
<xsl:text>
            };
        }; }; };
</xsl:text>
    </xsl:template>
</xsl:stylesheet>

