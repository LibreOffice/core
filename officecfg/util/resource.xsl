<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
<!--***********************************************************************
 *
 *  $RCSfile: resource.xsl,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tpf $ $Date: 2002-08-07 14:57:24 $
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

    <!-- Set correct output format -->
    <xsl:output method="text" indent="no" encoding="ISO-8859-1"/>

    <xsl:param name="locale"/>
    <xsl:param name="fallback-locale">en-US</xsl:param>

    <xsl:namespace-alias stylesheet-prefix="xs" result-prefix="xs"></xsl:namespace-alias>

    <!-- overwrite default rule for text & attribute nodes to be silent -->
    <xsl:template match="text()|@*">
    </xsl:template>

    <!-- match root -->
    <xsl:template match="/">
        <xsl:apply-templates/>
    </xsl:template>

    <!-- match all elements that could possibly contain info elements -->
    <xsl:template match="info/desc[@xml:lang=$locale]|info/label[@xml:lang=$locale]">

        <!-- print TPF's path to current node -->
        <xsl:for-each select="ancestor-or-self::*">
            <xsl:if test="local-name(.)!='component-schema'"><xsl:value-of select="local-name(.)"/>
                <xsl:choose>
                    <xsl:when test="@oor:name">[<xsl:value-of select="@oor:name"/>]</xsl:when>
                    <xsl:when test="@oor:value">[<xsl:value-of select="@oor:value"/>]</xsl:when>
                </xsl:choose>
                <xsl:if test="position()!=last()">.</xsl:if>
            </xsl:if>
        </xsl:for-each>

        <!-- print separator (equals sign) -->
        <xsl:text>=</xsl:text>

        <!-- print info/desc|label element's content -->
        <xsl:value-of select="normalize-space(.)"/>

        <!-- print linefeed -->
        <xsl:text>&#10;</xsl:text>

        <xsl:apply-templates/>

    </xsl:template>

</xsl:transform>
