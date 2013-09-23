<!--
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:dom="http://www.w3.org/2001/xml-events" xmlns:xforms="http://www.w3.org/2002/xforms" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"  
                xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#" xmlns:UML = 'org.omg.xmi.namespace.UML' xml:space="default">
  <xsl:output method="text" />

  <xsl:template match="/">
    <out xml:space="preserve">
      <xsl:text>
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
/*
  THIS FILE IS GENERATED AUTOMATICALLY! DO NOT EDIT!
 */
#ifndef INCLUDED_RESOURCES_HXX
#define INCLUDED_RESOURCES_HXX

#include &lt;vector&gt;
#include &lt;boost/shared_ptr.hpp&gt;

#include &lt;resourcemodel/OutputWithDepth.hxx&gt;

#include &lt;doctok/resourceids.hxx&gt;
#include &lt;doctok/WW8StructBase.hxx&gt;
#include &lt;doctok/WW8ResourceModelImpl.hxx&gt;
#include &lt;doctok/WW8PropertySetImpl.hxx&gt;
#include &lt;doctok/Dff.hxx&gt;

namespace writerfilter {
namespace doctok {

using namespace ::std;&#xa;</xsl:text>
      <xsl:apply-templates select="//UML:Class" mode="class"/>
      <xsl:text>
OUString getDffOptName(sal_uInt32 nPid);
bool isBooleanDffOpt(sal_uInt32 nId);
}}
#endif // INCLUDED_RESOURCES_HXX&#xa;</xsl:text></out>
</xsl:template>
  
  <xsl:template match="UML:Class" mode="class">
    <xsl:choose>
      <xsl:when test='.//UML:Stereotype[@xmi.idref = "ww8resource"]'>
        <xsl:apply-templates select="." mode="ww8resource"/>
      </xsl:when>
      <xsl:when test='.//UML:Stereotype[@xmi.idref = "dffrecord"]'>
        <xsl:apply-templates select="." mode="dffrecord"/>
      </xsl:when>
      <xsl:when test='.//UML:Stereotype[@xmi.idref = "dffopt"]'>
        <xsl:apply-templates select="." mode="dffopt"/>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="UML:Class" mode="ww8resource">
    <xsl:variable name="parentresource">
      <xsl:call-template name='parenttype'>
        <xsl:with-param name='type'><xsl:value-of select='@xmi.id'/></xsl:with-param>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name='size_'><xsl:value-of select='./*/UML:TaggedValue[./*/UML:TagDefinition/@xmi.idref="size"]/UML:TaggedValue.dataValue'/></xsl:variable>
    <xsl:variable name='size'>
      <xsl:choose>
        <xsl:when test='$size_ != ""'><xsl:value-of select='$size_'/></xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="classname">WW8<xsl:value-of select="@name"/></xsl:variable>
    <xsl:variable name="superclass">
      <xsl:choose>
        <xsl:when test='.//UML:Stereotype[@xmi.idref = "ww8sprm"]'>WW8PropertyImpl</xsl:when>
        <xsl:otherwise>WW8StructBase</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="surroundclass">
      <xsl:choose>
        <xsl:when test='.//UML:Stereotype[@xmi.idref = "ww8sprm"]'>WW8PropertyImpl</xsl:when>
        <xsl:otherwise>WW8StructBase</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:text>
/**
    Automatically generated class: </xsl:text><xsl:value-of select="@name"/><xsl:text>
    </xsl:text>
    <xsl:for-each select='./UML:ModelElement.stereotype/UML:Stereotype'>
      <xsl:text>
    Stereotype </xsl:text>
      <xsl:value-of select='@xmi.idref'/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
    <xsl:text>
    Parent Resource: </xsl:text>
    <xsl:value-of select="$parentresource"/>
    <xsl:text>
    Size: </xsl:text><xsl:value-of select='$size'/>
    <xsl:text>
*/
class </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>: public </xsl:text>
    <xsl:value-of select='$superclass'/>
    <xsl:if test='$parentresource != ""'>
      <xsl:text>, public writerfilter::Reference&lt; </xsl:text>
      <xsl:value-of select='$parentresource'/>
      <xsl:text>&gt;</xsl:text>
    </xsl:if>
    <xsl:text>
{</xsl:text>
    <xsl:if test='.//UML:Stereotype[@xmi.idref ="withindex"]'>
     <xsl:text>    sal_uInt32 mnIndex;</xsl:text>
    </xsl:if>
    <xsl:if test='.//UML:Stereotype[@xmi.idref ="withmembers"]'>
      <xsl:for-each select='.//UML:Attribute[@name!="reserved"]'>
        <xsl:apply-templates select='.' mode='declmembers'/>
      </xsl:for-each>
    </xsl:if>
    <xsl:if test='.//UML:Stereotype[@xmi.idref ="needsdocument"]'>
    </xsl:if>
    <xsl:if test='.//UML:Stereotype[@xmi.idref ="plcf"]'>
      <xsl:text>
    sal_uInt32 mnPlcfPayloadOffset;
    vector&lt;sal_uInt32&gt; entryOffsets;
    vector&lt;sal_uInt32&gt; payloadOffsets;
    vector&lt;sal_uInt32&gt; payloadIndices;
      </xsl:text>
    </xsl:if>
    <xsl:text>
    public:
    typedef boost::shared_ptr&lt;</xsl:text>
    <xsl:value-of select='$classname'/>
    <xsl:text>&gt; Pointer_t;

    </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>(
        WW8Stream &amp; rStream, 
        sal_uInt32 nOffset = 0, 
        sal_uInt32 nCount = </xsl:text>
    <xsl:value-of select='$size'/>
    <xsl:text>)
    : </xsl:text><xsl:value-of select='$superclass'/>
    <xsl:text>(rStream, nOffset, nCount)</xsl:text>
    <xsl:if test='.//UML:Stereotype[@xmi.idref ="withindex"]'>
      <xsl:text>, mnIndex(0)</xsl:text>
    </xsl:if>
    <xsl:text>
    {
      init();
    }
    </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>(
        WW8StructBase * pParent,
        sal_uInt32 nOffset = 0, 
        sal_uInt32 nCount = </xsl:text>
        <xsl:value-of select='$size'/>
        <xsl:text>)
    : </xsl:text>
    <xsl:value-of select='$superclass'/>
    <xsl:text>(pParent, nOffset, nCount)</xsl:text>
    <xsl:if test='.//UML:Stereotype[@xmi.idref ="withindex"]'>
      <xsl:text>, mnIndex(0)</xsl:text>
    </xsl:if>
    <xsl:text>
    {
      init();
    }&#xa;    </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>( const </xsl:text>
    <xsl:value-of select='$surroundclass'/>
    <xsl:text>&amp; rSurround,
    sal_uInt32 nOffset = 0, sal_uInt32 nCount = </xsl:text>
    <xsl:value-of select='$size'/>)
    <xsl:text>
    : </xsl:text>
    <xsl:value-of select='$superclass'/>
    <xsl:text>(rSurround, nOffset, nCount)</xsl:text>
    <xsl:if test='.//UML:Stereotype[@xmi.idref ="withindex"]'>
      <xsl:text>, mnIndex(0)</xsl:text>
    </xsl:if>
    <xsl:text>
    {
        init();
    }&#xa;
    </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>(
        const Sequence &amp; 
        rSequence, sal_uInt32 nOffset = 0, 
        sal_uInt32 nCount = </xsl:text>
        <xsl:value-of select='$size'/>
        <xsl:text>)
    : </xsl:text>
    <xsl:value-of select='$superclass'/>
    <xsl:text>(rSequence, nOffset, nCount)</xsl:text>
    <xsl:if test='.//UML:Stereotype[@xmi.idref ="withindex"]'>
      <xsl:text>, mnIndex(0)</xsl:text>
    </xsl:if>
    <xsl:text>
    {
      init();
    }

    virtual ~</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>();&#xa;</xsl:text>
    <xsl:if test='.//UML:Stereotype[@xmi.idref ="initimpl"]'>
      <xsl:text>    void initImpl();&#xa;</xsl:text>
    </xsl:if>
    <xsl:if test='.//UML:Stereotype[@xmi.idref ="plcf"]'>
      <xsl:text>    void initPayload();</xsl:text>
    </xsl:if>
    <xsl:text>
    void init();

    /**
        Return type of resource.
    */
    virtual string getType() const
    {
        return "</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>";
    }

    /**
        Return static size.
    */
    static sal_uInt32 getSize() { return </xsl:text>
    <xsl:value-of select='$size'/>
    <xsl:text>; }</xsl:text>
    <xsl:if test="not(.//UML:Stereotype[@xmi.ref='ww8nocalcsize'])">
      <xsl:text>
    /**
        Calculate actual size.
    */  
    sal_uInt32 calcSize(); 
      </xsl:text>
    </xsl:if>
    <xsl:if test='.//UML:Stereotype[@xmi.idref ="plcf"]'>
      <xsl:text>
    /**
        Return offset for entry of PLCF.

        @param nIndex      index of entry
    */
    sal_uInt32 getEntryOffset(sal_uInt32 nIndex)
    {
        return entryOffsets[nIndex];
    }</xsl:text>
    <xsl:if test="not(.//UML:Stereotype[@xmi.idref='ww8nopayloadoffset'])">
      <xsl:text>
    /**
        Calculate offset for payload.
    */
    sal_uInt32 calcPayloadOffset();</xsl:text>
    </xsl:if>
    <xsl:text>
    /**
    Set offset for payloads.

    @param nOffset    the offset to set
    */
    void setPayloadOffset(sal_uInt32 nOffset)
    {
    mnPlcfPayloadOffset = nOffset;
    }

    /**
    Return offset for payload of PLCF.

    @param nIndex      index of entry
    */
    sal_uInt32 getPayloadOffset(sal_uInt32 nIndex)
    {
    return payloadOffsets[nIndex];
    }

    /**
    Return size of payload.
    */
    sal_uInt32 getPayloadSize(sal_uInt32 nIndex) const
    {
    return payloadOffsets[nIndex + 1] - payloadOffsets[nIndex];
    }  


    /**
    Return payload index of an entry.

    */
    sal_uInt32 getPayloadIndex(sal_uInt32 nIndex)
    {
    return payloadIndices[nIndex];
    }
      </xsl:text>
    </xsl:if>
    <xsl:if test='.//UML:Stereotype[@xmi.idref ="withindex"]'>
      <xsl:text>
    /**
        Set index in parent.

        @param    nIndex    the index
    */
    void setIndex(sal_uInt32 nIndex) { mnIndex = nIndex; }
      </xsl:text>
    </xsl:if>
    <xsl:if test='.//UML:Stereotype[@xmi.idref ="ww8sprm"]'>
      <xsl:variable name='propsreftype'>writerfilter::Reference &lt; Properties &gt;::Pointer_t</xsl:variable>      
      <xsl:text>    /**
      Return reference to properties of Sprm.
    */
      </xsl:text>
      <xsl:value-of select='$propsreftype'/>
      <xsl:text> getProps()
    { 
        return </xsl:text>
        <xsl:value-of select='$propsreftype'/>
        <xsl:text> (new </xsl:text>
        <xsl:value-of select='$classname'/>
        <xsl:text>(*this)); 
    }
        </xsl:text>
    </xsl:if>

    <xsl:if test='.//UML:Stereotype[@xmi.idref ="ww8sprmbinary"]'>
      <xsl:text>
        virtual writerfilter::Reference &lt; BinaryObj &gt; ::Pointer_t getBinary();
      </xsl:text>
    </xsl:if>

    <xsl:if test='.//UML:Stereotype[@xmi.idref ="needsdocument"]'>
    </xsl:if>
    <xsl:apply-templates select="." mode="declsProperties"/>
    <xsl:if test='$parentresource="Table"'>
      <xsl:apply-templates select="." mode="declsTable"/>
    </xsl:if>
    <xsl:choose>
      <xsl:when test='$parentresource="Properties"'>
        <xsl:apply-templates select="." mode="resolveProperties"/>
      </xsl:when>
      <xsl:when test='$parentresource="Table"'>
        <xsl:apply-templates select="." mode="resolveTable"/>
      </xsl:when>
    </xsl:choose>
    <xsl:text>
  };
    </xsl:text>
  </xsl:template>
  
<xsl:template match='UML:Class' mode='declsProperties'>
  <xsl:choose>
    <xsl:when test='.//UML:Stereotype[@xmi.idref ="withmembers"]'>
      <xsl:for-each select='.//UML:Attribute[@name!="reserved"]'>
        <xsl:apply-templates select='.' mode='accessmembers'/>
      </xsl:for-each>
    </xsl:when>
    <xsl:otherwise>
      <xsl:variable name="debug">
        <xsl:choose>
          <xsl:when test='.//UML:Stereotype[@xmi.idref="debug"]'>
            <xsl:text>true</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>false</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:for-each select='.//UML:Attribute[@name!="reserved"]'>
        <xsl:apply-templates select='.' mode='access'>
          <xsl:with-param name="debug">
            <xsl:value-of select="$debug"/>
          </xsl:with-param>
        </xsl:apply-templates>
      </xsl:for-each>
    </xsl:otherwise>
  </xsl:choose>
  <xsl:for-each select='.//UML:Operation[@name!="reserved"]'>
    <xsl:apply-templates select='.' mode='declsOperations'/>
  </xsl:for-each>
</xsl:template>

<xsl:template match="UML:Class" mode="resolveProperties">
  <xsl:if test='.//UML:Stereotype[@xmi.idref = "resolvenoauto"]'>
    void resolveNoAuto(Properties &amp; rHandler);
  </xsl:if>

  void dump(OutputWithDepth &lt; string &gt; &amp; o) const;

  void resolve(Properties &amp; rHandler);
</xsl:template>

<xsl:template match="UML:Class" mode="declsTable">
  sal_uInt32 getEntryCount();
  writerfilter::Reference&lt; Properties &gt;::Pointer_t getEntry(sal_uInt32 nIndex);
</xsl:template>

<xsl:template match="UML:Class" mode="resolveTable">
  void resolve(Table &amp; rHandler);
</xsl:template>

<xsl:template match="UML:Attribute" mode="access">
  <xsl:param name="debug"/>
  <xsl:variable name="stereotype"><xsl:value-of select=".//UML:Stereotype[1]/@xmi.idref"/></xsl:variable>
  <xsl:variable name="offset_"><xsl:value-of select='translate(.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="offset"]/UML:TaggedValue.dataValue, "ABCDEFX", "abcdefx")'/></xsl:variable>
  <xsl:variable name="offset">
    <xsl:value-of select='$offset_'/>
  </xsl:variable>
  <xsl:variable name="shift"><xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="shift"]/UML:TaggedValue.dataValue'/></xsl:variable>
  <xsl:variable name="mask_"><xsl:value-of select='translate(.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="mask"]/UML:TaggedValue.dataValue, "ABCDEFX", "abcdefx")'/></xsl:variable>
  <xsl:variable name="mask">
    <xsl:if test='$mask_ != "" and not(starts-with($mask_, "0x"))'>0x</xsl:if>
    <xsl:value-of select='$mask_'/>
  </xsl:variable>
  <xsl:variable name='type'>
    <xsl:value-of select="UML:StructuralFeature.type/UML:DataType/@xmi.idref"/>
  </xsl:variable>
  <xsl:variable name="completetype">
    <xsl:text>WW8</xsl:text>
    <xsl:value-of select="$type"/>
  </xsl:variable>
  <xsl:variable name="saltype">
    <xsl:apply-templates select="." mode="returntype"/>
  </xsl:variable>
  <xsl:variable name='typetype'>
    <xsl:call-template name='typetype'>
      <xsl:with-param name='type'><xsl:value-of select="$type"/></xsl:with-param>
    </xsl:call-template>
  </xsl:variable>
  
    /**
        <xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="comment"]/UML:TaggedValue.dataValue'/>
        
        &lt;table&gt;
        &lt;tr&gt;
        &lt;th&gt;
        Name
        &lt;/th&gt;
        &lt;td&gt;
        <xsl:value-of select='@name'/>(<xsl:value-of select='$stereotype'/>)
        &lt;/td&gt;
        &lt;/tr&gt;
        &lt;tr&gt;
        &lt;th&gt;
        Offset
        &lt;/th&gt;
        &lt;td&gt;
        <xsl:value-of select='$offset'/>
        &lt;/td&gt;
        &lt;/tr&gt;
        &lt;tr&gt;
        &lt;th&gt;
        Mask
        &lt;/th&gt;
        &lt;td&gt;
        <xsl:value-of select='$mask'/>
        &lt;/td&gt;
        &lt;/tr&gt;
        &lt;tr&gt;
        &lt;th&gt;
        Shift
        &lt;/th&gt;
        &lt;td&gt;
        <xsl:value-of select='$shift'/>
        &lt;/td&gt;
        &lt;/tr&gt;
        &lt;/table&gt;
        */
        
<xsl:choose>
  <xsl:when test='$stereotype = "attribute"'>
    <xsl:if test='././/UML:Stereotype[@xmi.idref="virtual"]'>
      <xsl:text>virtual </xsl:text>
    </xsl:if>
    <xsl:value-of select='$saltype'/> get_<xsl:value-of select='@name'/>() const
    {
    <xsl:if test='$debug="true"'>
      <xsl:text>
        clog &lt;&lt; "&lt;called&gt;</xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text>&lt;/called&gt;" 
        &lt;&lt; endl;&#xa;
      </xsl:text>
    </xsl:if>
    <xsl:choose>
      <xsl:when test='././/UML:Stereotype[@xmi.idref="attributeremainder"]'>
        <xsl:text>return writerfilter::Reference &lt; BinaryObj &gt;::Pointer_t(new WW8BinaryObjReference(getRemainder(</xsl:text>
        <xsl:value-of select="$offset"/>
        <xsl:text>)));&#xa;</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:choose>
          <xsl:when test='$typetype = "complex"'>
            <xsl:text>        return </xsl:text>
            <xsl:value-of select="$saltype"/>
            <xsl:text>(new </xsl:text>
            <xsl:value-of select="$completetype"/>
            <xsl:text>(*this, </xsl:text>
            <xsl:value-of select="$offset"/>
            <xsl:text>));&#xa;</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>        return (get</xsl:text>
            <xsl:value-of select="$type"/>
            <xsl:text>(</xsl:text>
            <xsl:value-of select="$offset"/>
            <xsl:text>)</xsl:text>
            <xsl:if test="string-length($mask)>0">
              <xsl:text>&amp; </xsl:text>
              <xsl:value-of select="$mask"/>
            </xsl:if>
            <xsl:text>)</xsl:text>
            <xsl:if test="$shift>0">
              <xsl:text>&gt;&gt; </xsl:text>
              <xsl:value-of select="$shift"/>
            </xsl:if>
            <xsl:text>;</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>
    }
  </xsl:when>
  <xsl:when test='$stereotype = "array"'>
    <xsl:text>sal_uInt32 get_</xsl:text>
    <xsl:value-of select='@name'/>
    <xsl:text>_count() const </xsl:text>
    <xsl:choose>
      <xsl:when test='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref = "arraycount"]'>
        <xsl:text>{ return </xsl:text>
        <xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref = "arraycount"]/UML:TaggedValue.dataValue'/>
        <xsl:text>; }&#xa;</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>;&#xa;</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:value-of select='$saltype'/>
    <xsl:text> get_</xsl:text>
    <xsl:value-of select='@name'/>
    <xsl:text>(sal_uInt32 nIndex) const {</xsl:text>
    <xsl:choose>
      <xsl:when test="$typetype='complex'">
        <xsl:text> return </xsl:text>
        <xsl:value-of select="$saltype"/>
        <xsl:text>( new </xsl:text>
        <xsl:value-of select="$completetype"/>
        <xsl:text>(static_cast&lt;const WW8StructBase &gt;(*this), </xsl:text>
        <xsl:value-of select="$offset"/>
        <xsl:text> + nIndex * </xsl:text>
        <xsl:value-of select="$completetype"/>
        <xsl:text>::getSize()));</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text> return get</xsl:text>
        <xsl:value-of select='$type'/>
        <xsl:text>(</xsl:text><xsl:value-of select='$offset'/>
        <xsl:text> + nIndex * sizeof(</xsl:text>
        <xsl:value-of select='$saltype'/>
        <xsl:text>))</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>; }&#xa;</xsl:text>
  </xsl:when>
  <xsl:when test='$stereotype = "string"'>
    OUString get_<xsl:value-of select='@name'/>() const { return getString(<xsl:value-of select='$offset'/>); }
  </xsl:when>
</xsl:choose>
</xsl:template>

<xsl:template match='UML:Attribute' mode='accessmembers'>
  <xsl:text>    </xsl:text>
  <xsl:call-template name='saltype'>
    <xsl:with-param name='type'>
      <xsl:value-of select='.//UML:DataType/@xmi.idref'/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text> get_</xsl:text>
  <xsl:value-of select='@name'/>
  <xsl:text>() const { return m</xsl:text>
  <xsl:value-of select='@name'/>
  <xsl:text>; }
  </xsl:text>
</xsl:template>


<xsl:template match="UML:Operation" mode="declsOperations">
  <xsl:choose>
    <xsl:when test='.//UML:Stereotype/@xmi.idref = "array"'>
      <xsl:variable name="elementtype">
        <xsl:value-of select='.//UML:Parameter.type/UML:Class/@xmi.idref'/>
      </xsl:variable>
      <xsl:variable name="parentclass">
        <xsl:call-template name='parenttype'>
          <xsl:with-param name='type'>
            <xsl:value-of select='$elementtype'/>
          </xsl:with-param>
        </xsl:call-template>
      </xsl:variable>
      <xsl:variable name='saltype'>
        <xsl:call-template name='saltype'>
          <xsl:with-param name='type'>
            <xsl:value-of select='$elementtype'/>
          </xsl:with-param>
          <xsl:with-param name='parenttype'>
            <xsl:value-of select='$parentclass'/>
          </xsl:with-param>
        </xsl:call-template>
      </xsl:variable>
      /**
      Array: <xsl:value-of select="@name"/>
      ElementType: <xsl:value-of select="$elementtype"/>(<xsl:value-of select="$parentclass"/>)
      */

      sal_uInt32 get_<xsl:value-of select="@name"/>_count();

      <xsl:text>    </xsl:text><xsl:value-of select='$saltype'/> get_<xsl:value-of select="@name"/>(sal_uInt32 pos);
    </xsl:when>
    <xsl:when test='.//UML:Stereotype/@xmi.idref = "attribute" or .//UML:Stereotype/@xmi.idref = "binary"'>
      <xsl:variable name="returntype_">
        <xsl:value-of select='.//UML:Parameter.type/UML:Class/@xmi.idref'/>
      </xsl:variable>
      <xsl:variable name="returntype">
        <xsl:call-template name='saltype'>
          <xsl:with-param name='type'>
            <xsl:value-of select='$returntype_'/>
          </xsl:with-param>
          <xsl:with-param name='parenttype'>
            <xsl:call-template name='parenttype'>
              <xsl:with-param name='type'>
                <xsl:value-of select='$returntype_'/>
              </xsl:with-param>
            </xsl:call-template>
          </xsl:with-param>
        </xsl:call-template>
      </xsl:variable>
      /**
      Operation: <xsl:value-of select="@name"/>
      */

      <xsl:text>    </xsl:text>
      <xsl:value-of select='$returntype'/>
      <xsl:text> get_</xsl:text>
      <xsl:value-of select='@name'/>
      <xsl:text>();&#xa;</xsl:text>
    </xsl:when>
  </xsl:choose>
</xsl:template>

<xsl:template match="UML:Attribute" mode='declmembers'>
  <xsl:text>    </xsl:text>
  <xsl:call-template name='saltype'>
    <xsl:with-param name='type'>
      <xsl:value-of select='.//UML:DataType/@xmi.idref'/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:text> m</xsl:text>
  <xsl:value-of select="@name"/>
  <xsl:text>;
  </xsl:text>
</xsl:template>

<xsl:template name='idtoqname'>
  <xsl:param name='id'/>NS_<xsl:value-of select='substring-before($id, ":")'/>::LN_<xsl:value-of select='substring-after($id, ":")'/>
</xsl:template>

<xsl:template name='saltype'>
  <xsl:param name='type'/>
  <xsl:param name='parenttype'/>
  <xsl:choose>
    <xsl:when test="$type='U8'">sal_uInt8</xsl:when>
    <xsl:when test="$type='S8'">sal_Int8</xsl:when>
    <xsl:when test="$type='U16'">sal_uInt16</xsl:when>
    <xsl:when test="$type='S16'">sal_Int16</xsl:when>
    <xsl:when test="$type='U32'">sal_uInt32</xsl:when>
    <xsl:when test="$type='S32'">sal_Int32</xsl:when>
    <xsl:when test="$type='String'">OUString</xsl:when>
    <xsl:when test="$type='Value'">WW8Value::Pointer_t</xsl:when>
    <xsl:when test="$type='Binary'">WW8BinaryObjReference::Pointer_t</xsl:when>
    <xsl:when test="$parenttype=''">
      <xsl:value-of select="$type"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>writerfilter::Reference &lt; </xsl:text>
      <xsl:value-of select='$parenttype'/>
      <xsl:text> &gt;::Pointer_t</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name='typetype'>
  <xsl:param name='type'/><xsl:choose>
  <xsl:when test="$type='U8'">simple</xsl:when>
  <xsl:when test="$type='S8'">simple</xsl:when>
  <xsl:when test="$type='U16'">simple</xsl:when>
  <xsl:when test="$type='S16'">simple</xsl:when>
  <xsl:when test="$type='U32'">simple</xsl:when>
  <xsl:when test="$type='S32'">simple</xsl:when>
  <xsl:when test="$type='String'">string</xsl:when>
  <xsl:when test="$type='Value'">value</xsl:when>
  <xsl:otherwise>complex</xsl:otherwise>
</xsl:choose>
</xsl:template>

<xsl:template match="UML:Attribute" mode="valuetype">
  <xsl:variable name='type'>
    <xsl:value-of select='.//UML:DataType/@xmi.idref'/>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test='$type="U8"'>WW8IntValue</xsl:when>
    <xsl:when test='$type="S8"'>WW8IntValue</xsl:when>
    <xsl:when test='$type="U16"'>WW8IntValue</xsl:when>
    <xsl:when test='$type="S16"'>WW8IntValue</xsl:when>
    <xsl:when test='$type="U32"'>WW8IntValue</xsl:when>
    <xsl:when test='$type="S32"'>WW8IntValue</xsl:when>
    <xsl:otherwise>WW8PropertiesValue</xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="UML:Class" mode="dffrecord">
  <xsl:variable name="classname">
  <xsl:text>Dff</xsl:text><xsl:value-of select="@name"/>
  </xsl:variable>
  <xsl:text>
/** 
    Automaticallay generated class:
  </xsl:text>
  <xsl:value-of select="$classname"/>
  <xsl:text>&#xa;
*/&#xa;
class </xsl:text>
<xsl:value-of select="$classname"/>
<xsl:text> : public DffRecord
{
public:
    typedef boost::shared_ptr &lt; </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text> &gt; Pointer_t;&#xa;
    </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>(WW8StructBase * pParent, sal_uInt32 nOffset, sal_uInt32 nCount)&#xa;
    : DffRecord(pParent, nOffset, nCount) {}&#xa;
    </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>(WW8Stream &amp; rStream, sal_uInt32 nOffset, sal_uInt32 nCount)&#xa;
    : DffRecord(rStream, nOffset, nCount) {}&#xa;
    </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>(DffRecord &amp; rRecord)&#xa;
    : DffRecord(rRecord) {}
    virtual ~</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>();

    virtual void resolveLocal(Properties &amp; props);</xsl:text>
    <xsl:if test='.//UML:Stereotype[@xmi.idref = "resolvenoauto"]'>
      void resolveNoAuto(Properties &amp; rHandler);
    </xsl:if>
    <xsl:text>
    virtual void dump(OutputWithDepth &lt; string &gt; &amp; o) const;

    virtual DffRecord * clone() const { return new </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>(*this); }&#xa;
    virtual string getType() const 
    {
        return "</xsl:text>
        <xsl:value-of select="$classname"/>
        <xsl:text>";
    }

    virtual string getName() const { return ""; }
    </xsl:text>
    <xsl:apply-templates select="." mode="declsProperties"/>
    <xsl:text>
};
</xsl:text>
</xsl:template>


<xsl:template match="UML:Class" mode="dffopt">
<!--
  <xsl:variable name="classname">
  <xsl:text>DffOpt</xsl:text><xsl:value-of select="@name"/>
  </xsl:variable>
  <xsl:text>
/** 
    Automaticallay generated class:
  </xsl:text>
  <xsl:value-of select="$classname"/>
  <xsl:text>&#xa;
*/&#xa;
class </xsl:text>
<xsl:value-of select="$classname"/>
<xsl:text> : public WW8FOPTE
{
    </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>(WW8FOPTE &amp; rOpt) : WW8FOPTE(rOpt) {}

    virtual string getName() const
    {
       return "</xsl:text>
       <xsl:value-of select="@name"/>
       <xsl:text>";
     }&#xa;</xsl:text>

     <xsl:text>
};
</xsl:text> 
-->
</xsl:template>

<xsl:template name="parenttype">
  <xsl:param name='type'/>
  <xsl:for-each select='/XMI/XMI.content/UML:Model/UML:Namespace.ownedElement/UML:Generalization[UML:Generalization.child/UML:Class/@xmi.idref=$type]'>
    <xsl:value-of select='./UML:Generalization.parent/UML:Class/@xmi.idref'/>
  </xsl:for-each>
</xsl:template>

<xsl:template match="UML:Attribute" mode="returntype">
  <xsl:variable name='type'>
    <xsl:value-of select="UML:StructuralFeature.type/UML:DataType/@xmi.idref"/>
  </xsl:variable>
  <xsl:call-template name='saltype'>
    <xsl:with-param name='type'>
      <xsl:value-of select="$type"/>
    </xsl:with-param>
    <xsl:with-param name='parenttype'>
      <xsl:call-template name='parenttype'>
        <xsl:with-param name='type'><xsl:value-of select='$type'/></xsl:with-param>
      </xsl:call-template>
    </xsl:with-param>
  </xsl:call-template>
</xsl:template>

</xsl:stylesheet>

