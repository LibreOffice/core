<?xml version="1.0" encoding="UTF-8"?>
<!--
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 * (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
-->

<!-- Generate a .hxx file with type-safe C++ abstractions (based on
     comphelper/configuration.hxx) for all the <prop> and <set> elements in an
     .xcs file.

     Takes up to three parameters ns1, ns2, ns3 that represent the .xcs file's
     nesting within the org/openoffice hierarchy (e.g., for
     officecfg/registry/schema/org/openoffice/Office/Common.xcs, ns1 would be
     "Office", ns2 would be "Common", and ns3 would remain unset.

     TODO: Does not yet handle <node-ref> indirections (as, e.g., used in
     officecfg/registry/schema/org/openoffice/Office/Common.xcs as
     "ApplicationControlLayout").  Also, the available abstractions can still be
     improved (e.g., for extensible groups or for set element types).
-->

<xsl:stylesheet
    version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
     xmlns:oor="http://openoffice.org/2001/registry">
  <xsl:param name="ns1"/>
  <xsl:param name="ns2"/>
  <xsl:param name="ns3"/>
  <xsl:output method="text"/>

  <xsl:template match="/oor:component-schema">
    <xsl:apply-templates select="component">
      <xsl:with-param name="path">
        <xsl:text>/</xsl:text>
        <xsl:value-of select="@oor:package"/>
        <xsl:text>.</xsl:text>
        <xsl:value-of select="@oor:name"/>
      </xsl:with-param>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="component">
    <xsl:param name="path"/>
    <xsl:text>#ifndef INCLUDED_OFFICECFG_</xsl:text>
    <xsl:value-of
        select="translate($ns1, 'abcdefghijklmnopqrstuvwxyz-', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ_')"/>
    <xsl:if test="$ns2">
      <xsl:text>_</xsl:text>
      <xsl:value-of
          select="translate($ns2, 'abcdefghijklmnopqrstuvwxyz-', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ_')"/>
      <xsl:if test="$ns3">
        <xsl:text>_</xsl:text>
        <xsl:value-of
            select="translate($ns3, 'abcdefghijklmnopqrstuvwxyz-', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ_')"/>
      </xsl:if>
    </xsl:if>
    <xsl:text>_HXX&#xA;</xsl:text>
    <xsl:text>#define INCLUDED_OFFICECFG_</xsl:text>
    <xsl:value-of
        select="translate($ns1, 'abcdefghijklmnopqrstuvwxyz-', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ_')"/>
    <xsl:if test="$ns2">
      <xsl:text>_</xsl:text>
      <xsl:value-of
          select="translate($ns2, 'abcdefghijklmnopqrstuvwxyz-', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ_')"/>
      <xsl:if test="$ns3">
        <xsl:text>_</xsl:text>
        <xsl:value-of
            select="translate($ns3, 'abcdefghijklmnopqrstuvwxyz-', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ_')"/>
      </xsl:if>
    </xsl:if>
    <xsl:text>_HXX&#xA;</xsl:text>
    <xsl:text>&#xA;</xsl:text>
    <xsl:text>#include "sal/config.h"&#xA;</xsl:text>
    <xsl:text>&#xA;</xsl:text>
    <xsl:if test=".//prop or .//set">
      <xsl:if
          test=".//prop[count(@oor:nillable) = 0 or @oor:nillable = 'true']">
        <xsl:text>#include "boost/optional.hpp"&#xA;</xsl:text>
      </xsl:if>
      <xsl:if test=".//prop/@oor:type = 'oor:any'">
        <xsl:text>#include "com/sun/star/uno/Any.hxx"&#xA;</xsl:text>
      </xsl:if>
      <xsl:if test=".//prop/@oor:type = 'xs:hexBinary' or .//prop/@oor:type = 'oor:boolean-list' or .//prop/@oor:type = 'oor:short-list' or .//prop/@oor:type = 'oor:int-list' or .//prop/@oor:type = 'oor:long-list' or .//prop/@oor:type = 'oor:double-list' or .//prop/@oor:type = 'oor:string-list'">
        <xsl:text>#include "com/sun/star/uno/Sequence.hxx"&#xA;</xsl:text>
      </xsl:if>
      <xsl:text>#include "rtl/ustring.h"&#xA;</xsl:text>
      <xsl:text>#include "rtl/ustring.hxx"&#xA;</xsl:text>
      <xsl:if test=".//prop/@oor:type = 'xs:short' or .//prop/@oor:type = 'xs:int' or .//prop/@oor:type = 'xs:long' or .//prop/@oor:type = 'xs:hexBinary'">
        <xsl:text>#include "sal/types.h"&#xA;</xsl:text>
      </xsl:if>
      <xsl:text>#include "comphelper/configuration.hxx"&#xA;</xsl:text>
      <xsl:text>&#xA;</xsl:text>
      <xsl:text>namespace officecfg { namespace </xsl:text>
      <xsl:value-of select="translate($ns1, '-', '_')"/>
      <xsl:if test="$ns2">
        <xsl:text> { namespace </xsl:text>
        <xsl:value-of select="translate($ns2, '-', '_')"/>
        <xsl:if test="$ns3">
          <xsl:text> { namespace </xsl:text>
          <xsl:value-of select="translate($ns3, '-', '_')"/>
        </xsl:if>
      </xsl:if>
      <xsl:text> {&#xA;</xsl:text>
      <xsl:text>&#xA;</xsl:text>
      <xsl:apply-templates select="group|set|prop">
        <xsl:with-param name="path" select="$path"/>
      </xsl:apply-templates>
      <xsl:if test="$ns2">
        <xsl:text>} </xsl:text>
        <xsl:if test="$ns3">
          <xsl:text>} </xsl:text>
        </xsl:if>
      </xsl:if>
      <xsl:text>} }&#xA;</xsl:text>
      <xsl:text>&#xA;</xsl:text>
    </xsl:if>
    <xsl:text>#endif&#xA;</xsl:text>
  </xsl:template>

  <xsl:template match="group">
    <xsl:param name="path"/>
    <xsl:if test=".//prop or .//set">
      <xsl:variable name="name" select="translate(@oor:name, '-.', '__')"/>
      <xsl:text>struct </xsl:text>
      <xsl:value-of select="$name"/>
      <xsl:text>: public comphelper::ConfigurationGroup&lt; </xsl:text>
      <xsl:value-of select="$name"/>
      <xsl:text>&gt; {&#xA;</xsl:text>
      <xsl:text>    static rtl::OUString path() { return rtl::OUString(<!--
      -->RTL_CONSTASCII_USTRINGPARAM("</xsl:text>
      <xsl:value-of select="$path"/>
      <xsl:text>/</xsl:text>
      <xsl:value-of select="@oor:name"/>
      <xsl:text>")); }&#xA;</xsl:text>
      <xsl:text>&#xA;</xsl:text>
      <xsl:apply-templates select="group|set|prop">
        <xsl:with-param name="path">
          <xsl:value-of select="$path"/>
          <xsl:text>/</xsl:text>
          <xsl:value-of select="@oor:name"/>
        </xsl:with-param>
      </xsl:apply-templates>
      <xsl:text>private:&#xA;</xsl:text>
      <xsl:text>    </xsl:text>
      <xsl:value-of select="$name"/>
      <xsl:text>(); // not defined&#xA;</xsl:text>
      <xsl:text>    ~</xsl:text>
      <xsl:value-of select="$name"/>
      <xsl:text>(); // not defined&#xA;</xsl:text>
      <xsl:text>};&#xA;</xsl:text>
      <xsl:text>&#xA;</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="set">
    <xsl:param name="path"/>
    <xsl:variable name="name" select="translate(@oor:name, '-.', '__')"/>
    <xsl:text>struct </xsl:text>
    <xsl:value-of select="$name"/>
    <xsl:text>: public comphelper::ConfigurationSet&lt; </xsl:text>
    <xsl:value-of select="$name"/>
    <xsl:text>&gt; {&#xA;</xsl:text>
    <xsl:text>    static rtl::OUString path() { return rtl::OUString(<!--
    -->RTL_CONSTASCII_USTRINGPARAM("</xsl:text>
    <xsl:value-of select="$path"/>
    <xsl:text>/</xsl:text>
    <xsl:value-of select="@oor:name"/>
    <xsl:text>")); }&#xA;</xsl:text>
    <xsl:text>private:&#xA;</xsl:text>
    <xsl:text>    </xsl:text>
    <xsl:value-of select="$name"/>
    <xsl:text>(); // not defined&#xA;</xsl:text>
    <xsl:text>    ~</xsl:text>
    <xsl:value-of select="$name"/>
    <xsl:text>(); // not defined&#xA;</xsl:text>
    <xsl:text>};&#xA;</xsl:text>
    <xsl:text>&#xA;</xsl:text>
  </xsl:template>

  <xsl:template match="prop">
    <xsl:param name="path"/>
    <xsl:variable name="name" select="translate(@oor:name, '-.', '__')"/>
    <xsl:text>struct </xsl:text>
    <xsl:value-of select="$name"/>
    <xsl:text>: public comphelper::</xsl:text>
    <xsl:choose>
      <xsl:when test="@oor:localized = 'true'">
        <xsl:text>ConfigurationLocalizedProperty</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>ConfigurationProperty</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>&lt;</xsl:text>
    <xsl:value-of select="$name"/>
    <xsl:text>, </xsl:text>
    <xsl:if test="not(@oor:nillable = 'false')">
      <xsl:text>boost::optional&lt;</xsl:text>
    </xsl:if>
    <xsl:choose>
      <xsl:when test="@oor:type='oor:any'">
        <xsl:text>com::sun::star::uno::Any</xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='xs:boolean'">
        <xsl:text>bool</xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='xs:short'">
        <xsl:text>sal_Int16</xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='xs:int'">
        <xsl:text>sal_Int32</xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='xs:long'">
        <xsl:text>sal_Int64</xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='xs:double'">
        <xsl:text>double</xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='xs:string'">
        <xsl:text>rtl::OUString</xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='xs:hexBinary'">
        <xsl:text>com::sun::star::uno::Sequence&lt;sal_Int8&gt; </xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='oor:boolean-list'">
        <xsl:text>com::sun::star::uno::Sequence&lt;bool&gt; </xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='oor:short-list'">
        <xsl:text>com::sun::star::uno::Sequence&lt;sal_Int16&gt; </xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='oor:int-list'">
        <xsl:text>com::sun::star::uno::Sequence&lt;sal_Int32&gt; </xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='oor:long-list'">
        <xsl:text>com::sun::star::uno::Sequence&lt;sal_Int64&gt; </xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='oor:double-list'">
        <xsl:text>com::sun::star::uno::Sequence&lt;double&gt; </xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='oor:string-list'">
        <xsl:text>com::sun::star::uno::Sequence&lt;rtl::OUString&gt; </xsl:text>
      </xsl:when>
      <xsl:when test="@oor:type='oor:hexBinary-list'">
        <xsl:text>com::sun::star::uno::Sequence&lt;<!--
        -->com::sun::star::uno::Sequence&lt;sal_Int8&gt; &gt; </xsl:text>
      </xsl:when>
    </xsl:choose>
    <xsl:if test="not(@oor:nillable = 'false')">
      <xsl:text>&gt; </xsl:text>
    </xsl:if>
    <xsl:text>&gt; {&#xA;</xsl:text>
    <xsl:text>    static rtl::OUString path() { return rtl::OUString(<!--
    -->RTL_CONSTASCII_USTRINGPARAM("</xsl:text>
    <xsl:value-of select="$path"/>
    <xsl:text>/</xsl:text>
    <xsl:value-of select="@oor:name"/>
    <xsl:text>")); }&#xA;</xsl:text>
    <xsl:text>private:&#xA;</xsl:text>
    <xsl:text>    </xsl:text>
    <xsl:value-of select="$name"/>
    <xsl:text>(); // not defined&#xA;</xsl:text>
    <xsl:text>    ~</xsl:text>
    <xsl:value-of select="$name"/>
    <xsl:text>(); // not defined&#xA;</xsl:text>
    <xsl:text>};&#xA;</xsl:text>
    <xsl:text>&#xA;</xsl:text>
  </xsl:template>
</xsl:stylesheet>
