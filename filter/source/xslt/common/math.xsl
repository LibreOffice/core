<?xml version="1.0" encoding="UTF-8"?>
<!--

    OpenOffice.org - a multi-platform office productivity suite
 
    $RCSfile: math.xsl,v $
 
    $Revision: 1.6 $
 
    last change: $Author: rt $ $Date: 2005-09-08 22:00:23 $
 
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
 
-->
    <xsl:template name="sin-private">
        <xsl:param name="x" select="0"/>
        <xsl:param name="n" select="0"/>
        <xsl:param name="nx" select="1"/>
        <xsl:param name="sign" select="1"/>
        <xsl:param name="max-n" select="20"/>
        <xsl:param name="sinx" select="0"/>
        <xsl:choose>
            <xsl:when test="not ($max-n &gt;  $n) or $nx = 0 ">
                <xsl:value-of select="$sinx"/>
            </xsl:when>
            <xsl:when test="$n = 0">
                <xsl:call-template name="sin-private">
                    <xsl:with-param name="x" select="$x"/>
                    <xsl:with-param name="n" select="$n + 1"/>
                    <xsl:with-param name="sign" select="$sign *  -1"/>
                    <xsl:with-param name="max-n" select="$max-n"/>
                    <xsl:with-param name="nx" select="$x "/>
                    <xsl:with-param name="sinx" select="$sinx + $x"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="new-nx" select="($nx * $x * $x)  div ( 2  *  $n )  div ( 2  *  $n  + 1)  "/>
                <xsl:call-template name="sin-private">
                    <xsl:with-param name="x" select="$x"/>
                    <xsl:with-param name="n" select="$n + 1"/>
                    <xsl:with-param name="sign" select="$sign *  -1"/>
                    <xsl:with-param name="max-n" select="$max-n"/>
                    <xsl:with-param name="nx" select=" $new-nx "/>
                    <xsl:with-param name="sinx" select="$sinx + $new-nx  * $sign"/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="cos-private">
        <xsl:param name="x" select="0"/>
        <xsl:param name="n" select="0"/>
        <xsl:param name="nx" select="1"/>
        <xsl:param name="sign" select="1"/>
        <xsl:param name="max-n" select="20"/>
        <xsl:param name="cosx" select="0"/>
        <xsl:choose>
            <xsl:when test="not ($max-n &gt;  $n)  or $nx = 0  ">
                <xsl:value-of select="$cosx"/>
            </xsl:when>
            <xsl:when test="$n = 0">
                <xsl:call-template name="cos-private">
                    <xsl:with-param name="x" select="$x"/>
                    <xsl:with-param name="n" select="$n + 1"/>
                    <xsl:with-param name="sign" select="$sign *  -1"/>
                    <xsl:with-param name="max-n" select="$max-n"/>
                    <xsl:with-param name="nx" select=" 1 "/>
                    <xsl:with-param name="cosx" select="1"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="new-nx" select="($nx * $x * $x)  div ( 2  *  $n  -1 )  div ( 2  *  $n )  "/>
                <xsl:call-template name="cos-private">
                    <xsl:with-param name="x" select="$x"/>
                    <xsl:with-param name="n" select="$n + 1"/>
                    <xsl:with-param name="sign" select="$sign *  -1"/>
                    <xsl:with-param name="max-n" select="$max-n"/>
                    <xsl:with-param name="nx" select=" $new-nx "/>
                    <xsl:with-param name="cosx" select="$cosx + $new-nx  * $sign"/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="atan-private">
        <xsl:param name="x" select="0"/>
        <xsl:param name="n" select="0"/>
        <xsl:param name="nx" select="1"/>
        <xsl:param name="sign" select="1"/>
        <xsl:param name="max-n" select="40"/>
        <xsl:param name="arctanx" select="0"/>
        <xsl:choose>
            <xsl:when test="not ($max-n &gt;  $n) or $nx = 0 ">
                <xsl:value-of select="$arctanx"/>
            </xsl:when>
            <xsl:when test="$n = 0">
                <xsl:call-template name="atan-private">
                    <xsl:with-param name="x" select="$x"/>
                    <xsl:with-param name="n" select="$n + 1"/>
                    <xsl:with-param name="sign" select="$sign *  -1"/>
                    <xsl:with-param name="max-n" select="$max-n"/>
                    <xsl:with-param name="nx" select="$x "/>
                    <xsl:with-param name="arctanx" select="$arctanx + $x"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="new-nx" select=" $nx * $x * $x "/>
                <xsl:call-template name="atan-private">
                    <xsl:with-param name="x" select="$x"/>
                    <xsl:with-param name="n" select="$n + 1"/>
                    <xsl:with-param name="sign" select="$sign *  -1"/>
                    <xsl:with-param name="max-n" select="$max-n"/>
                    <xsl:with-param name="nx" select=" $new-nx "/>
                    <xsl:with-param name="arctanx" select="$arctanx + $new-nx div (2 * $n +1)  * $sign"/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="sqrt-private">
        <xsl:param name="x"/>
        <xsl:param name="rounding-factor" select="100"/>
        <xsl:variable name="shift" select="string-length( $rounding-factor)"/>
        <xsl:variable name="power">
            <xsl:call-template name="power">
                <xsl:with-param name="x" select="100"/>
                <xsl:with-param name="y" select="$shift"/>
                <xsl:with-param name="rounding-factor" select="1"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="integer-x" select=" round( $power * $x )"/>
        <xsl:variable name="integer-quotient">
            <xsl:call-template name="integer-sqrt">
                <xsl:with-param name="x" select="$integer-x"/>
                <xsl:with-param name="length" select=" string-length( $integer-x ) "/>
                <xsl:with-param name="curr-pos" select=" 2 -  (round (string-length( $integer-x ) div 2 )  * 2 - string-length( $integer-x ) ) "/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="power-10">
            <xsl:call-template name="power">
                <xsl:with-param name="x" select="10"/>
                <xsl:with-param name="y" select="$shift - 1"/>
                <xsl:with-param name="rounding-factor" select="1"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:value-of select="  round( $integer-quotient div 10) div $power-10 "/>
    </xsl:template>
    <xsl:template name="integer-sqrt">
        <xsl:param name="x"/>
        <xsl:param name="length"/>
        <xsl:param name="curr-pos"/>
        <xsl:param name="last-quotient" select="0"/>
        <xsl:choose>
            <xsl:when test="$curr-pos &gt; $length">
                <xsl:value-of select="$last-quotient"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="curr-x" select="substring( $x, 1,  $curr-pos )"/>
                <xsl:variable name="new-quotient">
                    <xsl:call-template name="get-one-sqrt-digit">
                        <xsl:with-param name="x" select="$curr-x"/>
                        <xsl:with-param name="last-quotient" select="$last-quotient"/>
                        <xsl:with-param name="n" select="5"/>
                        <xsl:with-param name="direct" select="0"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="integer-sqrt">
                    <xsl:with-param name="x" select="$x"/>
                    <xsl:with-param name="length" select="$length"/>
                    <xsl:with-param name="curr-pos" select="$curr-pos + 2"/>
                    <xsl:with-param name="last-quotient" select="number($new-quotient)"/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="get-one-sqrt-digit">
        <xsl:param name="x"/>
        <xsl:param name="last-quotient"/>
        <xsl:param name="n"/>
        <xsl:param name="direct" select="1"/>
        <xsl:variable name="quotient" select=" concat( $last-quotient, $n) "/>
        <xsl:variable name="accumulate" select="$quotient * $quotient "/>
        <xsl:choose>
            <xsl:when test="$accumulate  =  $x">
                <xsl:value-of select="concat($last-quotient , $n  )"/>
            </xsl:when>
            <xsl:when test="$direct = 0 and $accumulate  &lt;  $x">
                <xsl:call-template name="get-one-sqrt-digit">
                    <xsl:with-param name="x" select="$x"/>
                    <xsl:with-param name="last-quotient" select="$last-quotient"/>
                    <xsl:with-param name="n" select="$n + 1"/>
                    <xsl:with-param name="direct" select="1"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="$direct = 0 and $accumulate  &gt;  $x">
                <xsl:call-template name="get-one-sqrt-digit">
                    <xsl:with-param name="x" select="$x"/>
                    <xsl:with-param name="last-quotient" select="$last-quotient"/>
                    <xsl:with-param name="n" select="$n - 1"/>
                    <xsl:with-param name="direct" select="-1"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test=" $accumulate * $direct  &lt;  $x * $direct  ">
                <xsl:call-template name="get-one-sqrt-digit">
                    <xsl:with-param name="x" select="$x"/>
                    <xsl:with-param name="last-quotient" select="$last-quotient"/>
                    <xsl:with-param name="n" select="$n+ $direct"/>
                    <xsl:with-param name="direct" select="$direct"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="not($n &lt; 9)  or $n = -1">
                <xsl:value-of select="concat($last-quotient , $n - $direct) "/>
            </xsl:when>
            <xsl:when test="$direct = 1">
                <xsl:value-of select="concat($last-quotient , $n - 1) "/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="concat($last-quotient , $n) "/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="convert2redian">
        <xsl:param name="x" select="'0'"/>
        <xsl:param name="rounding-factor" select="100"/>
        <xsl:choose>
            <xsl:when test="contains($x,'deg')">
                <xsl:value-of select="round($rounding-factor * number(substring-before($x, 'deg') div 180 * $pi)) div $rounding-factor"/>
            </xsl:when>
            <xsl:when test="contains($x,'fd')">
                <xsl:value-of select="round($rounding-factor * number(substring-before($x, 'fd') div 180 div 65536 * $pi)) div $rounding-factor"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="round($rounding-factor * number($x) div 180 * $pi) div $rounding-factor"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="convert2degree">
        <xsl:param name="x" select="'0'"/>
        <xsl:param name="rounding-factor" select="100"/>
        <xsl:choose>
            <xsl:when test="contains($x,'deg')">
                <xsl:value-of select="round($rounding-factor * substring-before($x,'deg')) div $rounding-factor"/>
            </xsl:when>
            <xsl:when test="contains($x,'fd')">
                <xsl:value-of select="round($rounding-factor * number(substring-before($x, 'fd')) div 65536 ) div $rounding-factor"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="round($rounding-factor * number($x) * 180 div $pi) div $rounding-factor"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="convert2fd">
        <xsl:param name="x" select="'0'"/>
        <xsl:param name="rounding-factor" select="100"/>
        <xsl:choose>
            <xsl:when test="contains($x,'deg')">
                <xsl:value-of select="round($rounding-factor * number(substring-before($x, 'deg') * 65535)) div $rounding-factor"/>
            </xsl:when>
            <xsl:when test="contains($x,'fd')">
                <xsl:value-of select="round($rounding-factor * number(substring-before($x, 'fd'))) div $rounding-factor"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="round($rounding-factor * number($x) * 180 div $pi * 65535) div $rounding-factor"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

</xsl:stylesheet>
