/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmistat.xsl,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2006-11-01 09:14:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:UML = 'org.omg.xmi.namespace.UML' xml:space="default">
<xsl:output indent="yes"/>
<xsl:template match="/">
  <xsl:for-each select="//UML:Class[@xmi.id]">
    <xsl:value-of select="@xmi.id"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:for-each>
  <xsl:value-of select="count(//UML:Class)"/>
</xsl:template>
</xsl:stylesheet>