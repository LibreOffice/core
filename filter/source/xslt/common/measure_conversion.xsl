<?xml version="1.0" encoding="UTF-8"?>
<!--

   $RCSfile: measure_conversion.xsl,v $

   $Revision: 1.2 $

   last change: $Author: obo $ $Date: 2004-04-27 13:14:00 $

   The Contents of this file are made available subject to the terms of
   either of the following licenses

          - GNU Lesser General Public License Version 2.1
          - Sun Industry Standards Source License Version 1.1

   Sun Microsystems Inc., October, 2000

   GNU Lesser General Public License Version 2.1
   =============================================
   Copyright 2000 by Sun Microsystems, Inc.
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


   Sun Industry Standards Source License Version 1.1
   =================================================
   The contents of this file are subject to the Sun Industry Standards
   Source License Version 1.1 (the "License"); You may not use this file
   except in compliance with the License. You may obtain a copy of the
   License at http://www.openoffice.org/license.html.

   Software provided under this License is provided on an "AS IS" basis,
   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
   See the License for the specific provisions governing your rights and
   obligations concerning the Software.

   The Initial Developer of the Original Code is: Sun Microsystems, Inc.

   Copyright © 2002 by Sun Microsystems, Inc.

   All Rights Reserved.

   Contributor(s): _______________________________________

-->
<!--
    For further documentation and updates visit http://xml.openoffice.org/sx2ml
-->
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">


    <!-- ***** MEASUREMENT CONVERSIONS *****

     * 1 centimeter = 10 mm

     * 1 inch = 25.4 mm
        While the English have already seen the light (read: the metric system), the US
        remains loyal to this medieval system.

     * 1 didot point = 0.376065 mm
            The didot system originated in France but was used in most of Europe

     * 1 pica point = 0.35146 mm
            The Pica points system was developed in England and is used in Great-Britain and the US.

     * 1 PostScript point = 0.35277138 mm
            When Adobe created PostScript, they added their own system of points.
            There are exactly 72 PostScript points in 1 inch.

     * 1 pixel = 0.26458333.. mm   (by 96 dpi)
            Most pictures have the 96 dpi resolution, but the dpi variable may vary by stylesheet parameter
    -->

    <!-- OPTIONAL: DPI (dots per inch) the standard resolution of given pictures (necessary for the conversion of 'cm' into 'pixel')-->
    <!-- Although many pictures have a 96 dpi resolution, a higher resoltion give better results for common browsers -->
    <xsl:param name="dpi" select="111" />

    <!-- changing measure to mm -->
    <xsl:template name="convert2mm">
        <xsl:param name="value" />

        <xsl:param name="centimeter-in-mm"          select="10" />
        <xsl:param name="inch-in-mm"                select="25.4" />
        <xsl:param name="didot-point-in-mm"         select="0.376065" />
        <xsl:param name="pica-point-in-mm"          select="0.35146" />

        <xsl:choose>
            <xsl:when test="contains($value, 'cm')">
                <xsl:value-of select="round(number(substring-before($value,'cm' )) * $centimeter-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'in')">
                <xsl:value-of select="round(number(substring-before($value,'in' )) * $inch-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'dpt')">
                <xsl:value-of select="round(number(substring-before($value,'dpt')) * $didot-point-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'ppt')">
                <xsl:value-of select="round(number(substring-before($value,'ppt')) * $pica-point-in-mm)" />
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$value" />
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>



    <!-- changing measure to cm -->
    <xsl:template name="convert2cm">
        <xsl:param name="value" />

        <xsl:param name="centimeter-in-mm"          select="10" />
        <xsl:param name="inch-in-mm"                select="25.4" />
        <xsl:param name="didot-point-in-mm"         select="0.376065" />
        <xsl:param name="pica-point-in-mm"          select="0.35146" />

        <xsl:choose>
            <xsl:when test="contains($value, 'mm')">
                <xsl:value-of select="number(substring-before($value, 'mm') div $centimeter-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'in')">
                <xsl:value-of select="round(number(substring-before($value, 'in')) div $centimeter-in-mm * $inch-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'dpt')">
                <xsl:value-of select="round(number(substring-before($value,'dpt')) div $centimeter-in-mm * $didot-point-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'ppt')">
                <xsl:value-of select="round(number(substring-before($value,'ppt')) div $centimeter-in-mm * $pica-point-in-mm)" />
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$value" />
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>



    <!-- changing measure to inch (cp. section comment) -->
    <xsl:template name="convert2inch">
        <xsl:param name="value" />

        <xsl:param name="centimeter-in-mm"          select="10" />
        <xsl:param name="inch-in-mm"                select="25.4" />
        <xsl:param name="didot-point-in-mm"         select="0.376065" />
        <xsl:param name="pica-point-in-mm"          select="0.35146" />

        <xsl:choose>
            <xsl:when test="contains($value, 'mm')">
                <xsl:value-of select="round(number(substring-before($value, 'mm')) div $inch-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'cm')">
                <xsl:value-of select="round(number(substring-before($value, 'cm')) div $inch-in-mm * $centimeter-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'dpt')">
                <xsl:value-of select="round(number(substring-before($value,'dpt')) div $inch-in-mm * $didot-point-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'ppt')">
                <xsl:value-of select="round(number(substring-before($value,'ppt')) div $inch-in-mm * $pica-point-in-mm)" />
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$value" />
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>



    <!-- changing measure to dpt (cp. section comment) -->
    <xsl:template name="convert2dpt">
        <xsl:param name="value" />

        <xsl:param name="centimeter-in-mm"          select="10" />
        <xsl:param name="inch-in-mm"                select="25.4" />
        <xsl:param name="didot-point-in-mm"         select="0.376065" />
        <xsl:param name="pica-point-in-mm"          select="0.35146" />

        <xsl:choose>
            <xsl:when test="contains($value, 'mm')">
                <xsl:value-of select="round(number(substring-before($value, 'mm')) div $didot-point-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'cm')">
                <xsl:value-of select="round(number(substring-before($value, 'cm')) div $didot-point-in-mm * $centimeter-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'in')">
                <xsl:value-of select="round(number(substring-before($value, 'in')) div $didot-point-in-mm * $inch-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'ppt')">
                <xsl:value-of select="round(number(substring-before($value,'ppt')) div $didot-point-in-mm * $pica-point-in-mm)" />
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$value" />
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>



    <!-- changing measure to ppt (cp. section comment) -->
    <xsl:template name="convert2ppt">
        <xsl:param name="value" />

        <xsl:param name="centimeter-in-mm"          select="10" />
        <xsl:param name="inch-in-mm"                select="25.4" />
        <xsl:param name="didot-point-in-mm"         select="0.376065" />
        <xsl:param name="pica-point-in-mm"          select="0.35146" />

        <xsl:choose>
            <xsl:when test="contains($value, 'mm')">
                <xsl:value-of select="round(number(substring-before($value, 'mm')) div $pica-point-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'cm')">
                <xsl:value-of select="round(number(substring-before($value, 'cm')) div $pica-point-in-mm * $centimeter-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'in')">
                <xsl:value-of select="round(number(substring-before($value, 'in')) div $pica-point-in-mm * $inch-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'dpt')">
                <xsl:value-of select="round(number(substring-before($value,'dpt')) div $pica-point-in-mm * $didot-point-in-mm)" />
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$value" />
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>



    <!-- changing measure to pixel by via parameter provided dpi (dots per inch) standard factor (cp. section comment) -->
    <xsl:template name="convert2pixel">
        <xsl:param name="value" />

        <xsl:param name="centimeter-in-mm"          select="10" />
        <xsl:param name="inch-in-mm"                select="25.4" />
        <xsl:param name="didot-point-in-mm"         select="0.376065" />
        <xsl:param name="pica-point-in-mm"          select="0.35146" />
        <xsl:param name="pixel-in-mm"               select="$inch-in-mm div $dpi" />

        <xsl:choose>
            <xsl:when test="contains($value, 'mm')">
                <xsl:value-of select="round(number(substring-before($value, 'mm')) div $pixel-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'cm')">
                <xsl:value-of select="round(number(substring-before($value, 'cm')) div $pixel-in-mm * $centimeter-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'in')">
                <xsl:value-of select="round(number(substring-before($value, 'in')) div $pixel-in-mm * $inch-in-mm)" />
            </xsl:when>
            <xsl:when test="contains($value, 'dpt')">
                <xsl:value-of select="round(number(substring-before($value,'dpt')) div $pixel-in-mm * $didot-point-in-mm)" />
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$value" />
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

</xsl:stylesheet>
