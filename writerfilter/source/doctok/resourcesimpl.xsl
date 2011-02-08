<!--
/*************************************************************************
 *
  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
  
  Copyright 2000, 2010 Oracle and/or its affiliates.
 
  OpenOffice.org - a multi-platform office productivity suite
 
  This file is part of OpenOffice.org.
 
  OpenOffice.org is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 3
  only, as published by the Free Software Foundation.
 
  OpenOffice.org is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License version 3 for more details
  (a copy is included in the LICENSE file that accompanied this code).
 
  You should have received a copy of the GNU Lesser General Public License
  version 3 along with OpenOffice.org.  If not, see
  <http://www.openoffice.org/license.html>
  for a copy of the LGPLv3 License.

 ************************************************************************/

-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:dom="http://www.w3.org/2001/xml-events" xmlns:xforms="http://www.w3.org/2002/xforms" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"  
  xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#" xmlns:UML = 'org.omg.xmi.namespace.UML' xml:space="default">
<xsl:output method="text" />

  <xsl:include href="resourcetools.xsl"/>

<xsl:template match="/">
  <xsl:call-template name="licenseheader"/>
  <xsl:text>
#include &lt;resources.hxx&gt;

#ifndef WW8_OUTPUT_WITH_DEPTH
#include &lt;WW8OutputWithDepth.hxx&gt;
#endif

#ifndef INCLUDED_SPRMIDS_HXX
#include &lt;sprmids.hxx&gt;
#endif

namespace writerfilter {
namespace doctok {

extern WW8OutputWithDepth output;

using namespace ::std;
</xsl:text>
  <xsl:apply-templates select='/XMI/XMI.content/UML:Model' mode="sprmkind"/>
  <xsl:apply-templates select='.//UML:Class' mode='class'/>
  <xsl:apply-templates select='//UML:Model' mode='createsprmprops'/>
  <xsl:apply-templates select='/XMI/XMI.content/UML:Model' 
                       mode='createsprmbinary'/>
  <xsl:apply-templates select='/XMI/XMI.content/UML:Model' 
                       mode='createdffrecord'/>
  <xsl:apply-templates select='/XMI/XMI.content/UML:Model' mode='ww8foptename'/>
  <xsl:apply-templates select='/XMI/XMI.content/UML:Model' 
                       mode='isbooleandffopt'/>
  <xsl:text>
}}
</xsl:text>
</xsl:template>

  <xsl:template match="UML:Class" mode="class">
    <xsl:choose>
      <xsl:when test='.//UML:Stereotype[@xmi.idref = "ww8resource"]'>
        <xsl:apply-templates select="." mode="ww8resource">
          <xsl:with-param name="classprefix">WW8</xsl:with-param>          
          <xsl:with-param name="needsinit">true</xsl:with-param>          
        </xsl:apply-templates>
      </xsl:when>
      <xsl:when test='.//UML:Stereotype[@xmi.idref = "dffrecord"]'>
        <xsl:apply-templates select="." mode="ww8resource">
          <xsl:with-param name="classprefix">Dff</xsl:with-param>
        </xsl:apply-templates>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="UML:Class" mode="ww8resource">
    <xsl:param name="classprefix"/>
    <xsl:param name="needsinit"/>
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
    <xsl:variable name="classname"><xsl:value-of select="$classprefix"/><xsl:value-of select="@name"/></xsl:variable>
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
/*
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
    </xsl:text>
    
    <xsl:value-of select="$classname"/>
    <xsl:text>::~</xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>()
    {
    }
    </xsl:text>
    <xsl:if test="$needsinit='true'">
      <xsl:text>
    void </xsl:text>
      <xsl:value-of select="$classname"/>
      <xsl:text>::init()
    {
      </xsl:text>    
      <xsl:if test='.//UML:Stereotype[@xmi.idref ="withmembers"]'>
        <xsl:for-each select='.//UML:Attribute[@name!="reserved"]'>
          <xsl:apply-templates select='.' mode='initmembers'/>
        </xsl:for-each>
      </xsl:if>
      <xsl:if test='.//UML:Stereotype[@xmi.idref ="initimpl"]'>
        <xsl:text>    initImpl();&#xa;</xsl:text>
      </xsl:if>
      <xsl:text>
    }&#xa;</xsl:text>
    </xsl:if>

    <xsl:choose>
      <xsl:when test='$parentresource="Properties"'>
        <xsl:apply-templates select="." mode="resolveProperties">
          <xsl:with-param name="classname" select="$classname"/>
        </xsl:apply-templates>
      </xsl:when>
      <xsl:when test='$parentresource="Table"'>
        <xsl:apply-templates select="." mode="resolveTable">
          <xsl:with-param name="classname" select="$classname"/>
        </xsl:apply-templates>
      </xsl:when>
    </xsl:choose>

  </xsl:template>

  <xsl:template match="UML:Class" mode="resolveProperties">
    <xsl:param name="classname"/>
    <xsl:variable name="rHandler">
      <xsl:choose>
        <xsl:when test='.//UML:Attribute[@name!="reserved"]'>
          <xsl:if test='.//UML:Stereotype[@xmi.idref != "noresolve"]'>
            <xsl:text>rHandler</xsl:text>
          </xsl:if>
        </xsl:when>
        <xsl:when test='.//UML:Operation[@name!="reserved"]'>
          <xsl:if test='.//UML:Stereotype[@xmi.idref != "noresolve"]'>
            <xsl:text>rHandler</xsl:text>
          </xsl:if>
        </xsl:when>
        <xsl:when test='.//UML:Stereotype[@xmi.idref = "resolvenoauto"]'>
          <xsl:text>rHandler</xsl:text>
        </xsl:when>
      </xsl:choose>
    </xsl:variable>
    <xsl:text>
    void </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:choose>
      <xsl:when test=".//UML:Stereotype[@xmi.idref='ww8resource']">
        <xsl:text>::resolve(Properties &amp; </xsl:text>
        <xsl:value-of select="$rHandler"/>
        <xsl:text>)</xsl:text>
      </xsl:when>
      <xsl:when test=".//UML:Stereotype[@xmi.idref='dffrecord']">
        <xsl:text>::resolveLocal(Properties &amp; </xsl:text>
        <xsl:value-of select="$rHandler"/>
        <xsl:text>)</xsl:text>
      </xsl:when>
    </xsl:choose>
    <xsl:text>
    {</xsl:text>
    <xsl:if test='.//UML:Stereotype[@xmi.idref="debug"]'>
      <xsl:text>
        dump(output);</xsl:text>
    </xsl:if>
    <xsl:text>
        try 
        {</xsl:text>
    <xsl:for-each select='.//UML:Attribute[@name!="reserved"]'>
      <xsl:choose>
        <xsl:when test='.//UML:Stereotype[@xmi.idref="noresolve"]'>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select='.' mode='resolveAttribute'/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
    <xsl:for-each select='.//UML:Operation[@name!="reserved"]'>
      <xsl:choose>
        <xsl:when test='.//UML:Stereotype[@xmi.idref="noresolve"]'>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select='.' mode='resolveOperation'/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>    
    
    <xsl:if test='.//UML:Stereotype[@xmi.idref = "resolvenoauto"]'>
      resolveNoAuto(rHandler);
    </xsl:if>
    <xsl:text>
      } catch (Exception &amp; e) {
         clog &lt;&lt; e.getText() &lt;&lt; endl;
      }
    }

    void 
    </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::dump(OutputWithDepth &lt; string &gt; &amp; o) const
    {
        o.addItem("&lt;dump type='</xsl:text>
        <xsl:value-of select='@name'/>
        <xsl:text>'&gt;");
        
        WW8StructBase::dump(o);</xsl:text>
        <xsl:for-each select='.//UML:Attribute[@name!="reserved"]'>
          <xsl:apply-templates select='.' mode='dumpAttribute'/>
        </xsl:for-each>

        <xsl:text>
        o.addItem("&lt;/dump&gt;");
    }&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match='UML:Attribute' mode='resolveAttribute'>
    <xsl:variable name="attrid">
      <xsl:for-each select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="attrid"]'>
      <xsl:value-of select='UML:TaggedValue.dataValue'/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="qname">
      <xsl:call-template name='idtoqname'>
        <xsl:with-param name='id'>
          <xsl:value-of select='$attrid'/>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test='.//UML:Stereotype/@xmi.idref = "attribute"'>
        <xsl:text>
            {
                WW8Value::Pointer_t pVal = createValue(get_</xsl:text>
              <xsl:value-of select="@name"/>
              <xsl:text>());
                rHandler.attribute(</xsl:text>
                <xsl:value-of select="$qname"/>
              <xsl:text>, *pVal);
            }</xsl:text>
      </xsl:when>
      <xsl:when test='.//UML:Stereotype/@xmi.idref = "array"'>
        <xsl:variable name="elementtype">
          <xsl:value-of select='.//UML:DataType/@xmi.idref'/>
        </xsl:variable>
        <xsl:variable name="parentclass">
          <xsl:call-template name='parenttype'>
            <xsl:with-param name='type'>
              <xsl:value-of select='$elementtype'/>
            </xsl:with-param>
          </xsl:call-template>
        </xsl:variable>
        <xsl:text>
            {
                sal_uInt32 nCount = get_</xsl:text>
              <xsl:value-of select="@name"/>
              <xsl:text>_count();              
                for (sal_uInt32 n = 0; n &lt; nCount; ++n)
                {
                    WW8Value::Pointer_t pVal = createValue(get_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text>(n));
                    rHandler.attribute(</xsl:text>
                    <xsl:value-of select="$qname"/>
                    <xsl:text>, *pVal);
                }
            }</xsl:text>
      </xsl:when>
      <xsl:when test='.//UML:Stereotype/@xmi.idref = "string"'>
        <xsl:text>
            {
                WW8StringValue aVal(get_</xsl:text>
                <xsl:value-of select='@name'/>
                <xsl:text>());
                rHandler.attribute(</xsl:text>
                <xsl:value-of select="$qname"/>
                <xsl:text>, aVal);
            }</xsl:text>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match='UML:Attribute' mode='dumpAttribute'>
    <xsl:variable name="type">
      <xsl:value-of select='.//UML:DataType/@xmi.idref'/>
      </xsl:variable><xsl:variable name="saltype">
      <xsl:call-template name='saltype'>
        <xsl:with-param name='type'>
          <xsl:value-of select='$type'/>
        </xsl:with-param>
        <xsl:with-param name='parenttype'>
          <xsl:apply-templates select="." mode="parentclass"/>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="name" select="@name"/>
    <xsl:for-each select=".//UML:Stereotype">      
      <xsl:choose>
        <xsl:when test='@xmi.idref = "attributeremainder"'/>
        <xsl:when test='@xmi.idref = "array"'>
          <xsl:text>
          {
            sal_uInt32 nCount = get_</xsl:text>
            <xsl:value-of select='$name'/>
            <xsl:text>_count();
            
            for (sal_uInt32 n = 0; n &lt; nCount; ++n)
            {
                writerfilter::dump(o, "</xsl:text>
                <xsl:value-of select='$name'/>
                <xsl:text>", get_</xsl:text>
                <xsl:value-of select="$name"/>
                <xsl:text>(n));
            }
        }</xsl:text>
        </xsl:when>
        <xsl:when test='@xmi.idref = "string"'>
          <xsl:text>
        {
            WW8StringValue aVal(get_</xsl:text>
            <xsl:value-of select='$name'/>
            <xsl:text>());
            
            o.addItem("</xsl:text>
            <xsl:value-of select='$name'/>
            <xsl:text>" &lt;&lt; "=\"" + aVal.toString() + "\"");
        }</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>
        writerfilter::dump(o, "</xsl:text>
        <xsl:value-of select='$name'/>
        <xsl:text>", get_</xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>());</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="resolveOperationAttribute">
    <xsl:variable name="opid">
      <xsl:for-each select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="opid"]'>
        <xsl:value-of select='./UML:TaggedValue.dataValue'/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:text>
          {
              WW8Value::Pointer_t pVal = createValue(get_</xsl:text>
              <xsl:value-of select="@name"/>
              <xsl:text>());
              rHandler.attribute(</xsl:text>
              <xsl:call-template name='idtoqname'>
                <xsl:with-param name='id'><xsl:value-of select='$opid'/></xsl:with-param>
              </xsl:call-template>
              <xsl:text>, *pVal);
          }&#xa;</xsl:text>
  </xsl:template>

  <xsl:template name="resolveOperationArray">
    <xsl:variable name="elementtype">
      <xsl:value-of select='.//UML:Parameter.type/@xmi.idref'/>
    </xsl:variable>
    <xsl:variable name="opid">
      <xsl:for-each select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="opid"]'>
        <xsl:value-of select='./UML:TaggedValue.dataValue'/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="parentclass">
      <xsl:for-each select='/XMI/XMI.content/UML:Model/UML:Namespace.ownedElement/UML:Generalization[UML:Generalization.child/UML:Class/@xmi.idref=$elementtype]'>
        <xsl:value-of select='./UML:Generalization.parent/UML:Class/@xmi.idref'/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:text>
          {
              sal_uInt32 nCount = get_</xsl:text>
              <xsl:value-of select="@name"/>
              <xsl:text>_count();
              
              for (sal_uInt32 n = 0; n &lt; nCount; ++n)
              {
                  WW8Value::Pointer_t pVal = createValue(get_</xsl:text>
                  <xsl:value-of select="@name"/>
                  <xsl:text>(n));
                  rHandler.attribute(</xsl:text>
                  <xsl:call-template name='idtoqname'>
                    <xsl:with-param name='id'><xsl:value-of select='$opid'/></xsl:with-param>
                  </xsl:call-template>
                  <xsl:text>, *pVal);
             }
         }&#xa;</xsl:text>
  </xsl:template>

  <xsl:template name="resolveOperationBinary">
    <xsl:text>
          {
              WW8BinaryObjReference::Pointer_t pRef(get_</xsl:text>
              <xsl:value-of select="@name"/>
              <xsl:text>());
              WW8Sprm aSprm(pRef);
              
              rHandler.sprm(aSprm);
          }&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match='UML:Operation' mode='resolveOperation'>
    <xsl:choose>
      <xsl:when test='.//UML:Stereotype/@xmi.idref = "attribute"'>
        <xsl:call-template name="resolveOperationAttribute"/>
      </xsl:when>
      <xsl:when test='.//UML:Stereotype/@xmi.idref = "array"'>
        <xsl:call-template name="resolveOperationArray"/>
      </xsl:when>
      <xsl:when test='.//UML:Stereotype/@xmi.idref = "binary"'>
        <xsl:call-template name="resolveOperationBinary"/>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="UML:Class" mode="resolveTable">
    <xsl:param name="classname"/>
    <xsl:text>void </xsl:text>
    <xsl:value-of select="$classname"/>
    <xsl:text>::resolve(Table &amp; rHandler)
    {
       sal_uInt32 nCount = getEntryCount();
       writerfilter::Reference&lt; Properties &gt;::Pointer_t pEntry;
       for (sal_uInt32 n = 0; n &lt; nCount; ++n)
       {
           pEntry = getEntry(n);
           
           if (pEntry.get() != 0x0)
           {
               try 
               {
                   rHandler.entry(n, pEntry);
               }
               catch (ExceptionOutOfBounds e)
               {    
                   (void) e;
               }
           }
       }
   }
    </xsl:text>
  </xsl:template>

<xsl:template match='UML:Model' mode='createsprmprops'>
writerfilter::Reference &lt; Properties &gt; ::Pointer_t createSprmProps
(WW8PropertyImpl &amp; rProp)
{
    writerfilter::Reference &lt; Properties &gt; ::Pointer_t pResult;

    switch (rProp.getId())
    {
<xsl:for-each select='.//UML:Class[.//UML:Stereotype/@xmi.idref="ww8sprm" and .//UML:Stereotype/@xmi.idref="ww8resource"]'>
    case <xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="sprmcode"]//UML:TaggedValue.dataValue'/>:
        pResult = writerfilter::Reference &lt; Properties &gt; ::Pointer_t(new WW8<xsl:value-of select='@name'/>(rProp, 0, rProp.getCount()));

        break;
</xsl:for-each>
    default:
        break;
    }

    return pResult;
}
</xsl:template>

<xsl:template match='UML:Model' mode='createsprmbinary'>
<xsl:text>
writerfilter::Reference &lt; BinaryObj &gt; ::Pointer_t createSprmBinary
(WW8PropertyImpl &amp; rProp)
{
    writerfilter::Reference &lt; BinaryObj &gt; ::Pointer_t pResult;

    switch (rProp.getId())
    {&#xa;
    // warnings....
    case 0:
        break;
</xsl:text>
<xsl:for-each select='.//UML:Class[.//UML:Stereotype/@xmi.idref="ww8sprmbinary" and .//UML:Stereotype/@xmi.idref="ww8resource"]'>
    case <xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="sprmcode"]//UML:TaggedValue.dataValue'/>:
        {
            WW8<xsl:value-of select='@name'/>::Pointer_t p(new WW8<xsl:value-of select='@name'/>(rProp, 0, rProp.getCount()));
            
            pResult = p->getBinary();
        }
        
        break;
</xsl:for-each>
<xsl:text>
    default:
           break;
    }

    return pResult;
}
</xsl:text>
</xsl:template>

<!-- returns optname of UML:Class -->
<xsl:template name="optname">
  <xsl:for-each select="./UML:ModelElement.taggedValue/UML:TaggedValue">
    <xsl:if test=".//UML:TagDefinition/@xmi.idref='optname'">
      <xsl:value-of select="./UML:TaggedValue.dataValue"/>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

<xsl:template name="ww8foptenamecase">
  <xsl:text>
      case </xsl:text>
      <xsl:variable name="optname">
        <xsl:call-template name="optname"/>
      </xsl:variable>
      <xsl:call-template name="idtoqname">
        <xsl:with-param name="id" select="$optname"/>
      </xsl:call-template>
      <xsl:text>:
      result = rtl::OUString::createFromAscii("</xsl:text>
      <xsl:value-of select="$optname"/>
      <xsl:text>");
      break;</xsl:text>
</xsl:template>

<xsl:template name="getdffoptname">
  <xsl:text>
rtl::OUString getDffOptName(sal_uInt32 nPid)
{
    rtl::OUString result;

    switch (nPid)
    {&#xa;</xsl:text>
    <xsl:for-each select="./UML:Class[./UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='dffopt']">
      <xsl:call-template name="ww8foptenamecase"/>
    </xsl:for-each>
    <xsl:text>
      default:
         break;
    }

    return result;
}</xsl:text>
</xsl:template>

<xsl:template name="ww8foptegetvalue">
  <xsl:text>
WW8Value::Pointer_t WW8FOPTE::get_value()
{
    WW8Value::Pointer_t pResult;

    switch (get_pid())
    {</xsl:text>
    <xsl:for-each select=".//UML:Class[./UML:ModelElement.stereotype/UML:Stereotype/@xmi.idref='dffopt']">
      <xsl:variable name="type">
        <xsl:for-each select="./UML:ModelElement.taggedValue/UML:TaggedValue[./UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='type']">
          <xsl:value-of select="UML:TaggedValue.dataValue"/>
        </xsl:for-each>
      </xsl:variable>
      <xsl:if test="$type != 'unknown'">
        <xsl:variable name="typetype">
          <xsl:call-template name="typetype">
            <xsl:with-param name="type" select="$type"/>
          </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="valuetype">
          <xsl:call-template name="valuetype">
            <xsl:with-param name="type" select="$type"/>
          </xsl:call-template>
        </xsl:variable>
        <xsl:text>
    case </xsl:text>
    <xsl:for-each select="./UML:ModelElement.taggedValue/UML:TaggedValue[./UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='dffid']">
      <xsl:value-of select="UML:TaggedValue.dataValue"/>
    </xsl:for-each>
    <xsl:variable name="optname">
      <xsl:for-each select="./UML:ModelElement.taggedValue/UML:TaggedValue[./UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='optname']">
        <xsl:value-of select="UML:TaggedValue.dataValue"/>
      </xsl:for-each>
    </xsl:variable>
    <xsl:call-template name="idtoqname">
      <xsl:with-param name="id" select="$optname"/>
    </xsl:call-template>
    <xsl:text>:</xsl:text>
    <xsl:choose>
      <xsl:when test="./UML:ModelElement.taggedValue/UML:TaggedValue[./UML:TaggedValue.type/UML:TagDefinition/@xmi.idref='isbool']">
        <xsl:text>
        pResult = createValue(getU32(0x2) &amp; 1);</xsl:text>
      </xsl:when>
      <xsl:when test="$typetype='complex'">
        <xsl:text>
        pResult = createValue(new </xsl:text>
        <xsl:value-of select="$valuetype"/>
        <xsl:text>(getU32(0x2)));</xsl:text>
      </xsl:when>
      <xsl:when test="$typetype='string'">
        <xsl:text>
        pResult = get_stringValue();&#xa;</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>
        pResult = createValue(getU32(0x2));</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>
        break;</xsl:text>
      </xsl:if>
    </xsl:for-each>
    <xsl:text>
    default:
        pResult = createValue(getU32(0x2));

        break;
    }

    return pResult;
}</xsl:text>
</xsl:template>

<xsl:template match='UML:Model/UML:Namespace.ownedElement' mode='ww8foptename'>
  <xsl:text>
rtl::OUString WW8FOPTE::get_name()
{
    return getDffOptName(get_pid());
}</xsl:text>
<xsl:call-template name="getdffoptname"/>
<xsl:text>

WW8Value::Pointer_t WW8FOPTE::get_stringValue()
{
    WW8Value::Pointer_t pResult;
    DffOPT * pOpt = dynamic_cast &lt; DffOPT * &gt;(mpParent); 
    sal_uInt32 nExtraOffset = pOpt->get_extraoffset(mnIndex);
    sal_uInt32 nExtraLength = pOpt->get_extraoffset(mnIndex + 1) - nExtraOffset;
    pResult = createValue(mpParent->getString(nExtraOffset, nExtraLength / 2));

    return pResult;
}
</xsl:text>
<xsl:call-template name="ww8foptegetvalue"/>
</xsl:template>

<xsl:template match="UML:Model" mode="createdffrecord">
<xsl:text>
DffRecord * createDffRecord
(WW8StructBase * pParent, sal_uInt32 nOffset, sal_uInt32 * pCount)
{
    DffRecord * pResult = NULL;
    DffRecord aTmpRec(pParent, nOffset, 8);
    sal_uInt32 nSize = aTmpRec.calcSize();

    if (pCount != NULL)
        *pCount = nSize;

    switch (aTmpRec.getRecordType())
    {&#xa;</xsl:text>
    <xsl:for-each select='.//UML:Class[.//UML:Stereotype/@xmi.idref="dffrecord"]'>
      <xsl:text>        case </xsl:text>
      <xsl:value-of select=".//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref='dffid']//UML:TaggedValue.dataValue"/>
      <xsl:text>:
             pResult = new Dff</xsl:text>
                 <xsl:value-of select="@name"/>
                 <xsl:text>(pParent, nOffset, nSize);
                 
             break;&#xa;</xsl:text>
    </xsl:for-each>
    <xsl:text>
         default:
             pResult = new DffRecord(pParent, nOffset, nSize);

             break;
    }

    return pResult;
}

DffRecord * createDffRecord
(WW8Stream &amp; rStream, sal_uInt32 nOffset, sal_uInt32 * pCount)
{
    DffRecord * pResult = NULL;
    DffRecord aTmpRec(rStream, nOffset, 8);
    sal_uInt32 nSize = aTmpRec.calcSize();

    if (pCount != NULL)
        *pCount = nSize;

    switch (aTmpRec.getRecordType())
    {&#xa;</xsl:text>
    <xsl:for-each select='.//UML:Class[.//UML:Stereotype/@xmi.idref="dffrecord"]'>
      <xsl:text>        case </xsl:text>
      <xsl:value-of select=".//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref='dffid']//UML:TaggedValue.dataValue"/>
      <xsl:text>:
             pResult = new Dff</xsl:text>
                 <xsl:value-of select="@name"/>
                 <xsl:text>(rStream, nOffset, nSize);
                 
             break;&#xa;</xsl:text>
    </xsl:for-each>
    <xsl:text>
         default:
             pResult = new DffRecord(rStream, nOffset, nSize);

             break;
    }

    return pResult;
}
    </xsl:text>
</xsl:template>

<xsl:template match="UML:Attribute" mode="initmembers">
  <xsl:variable name="stereotype"><xsl:value-of select=".//UML:Stereotype/@xmi.idref"/></xsl:variable>
  <xsl:variable name="offset_"><xsl:value-of select='translate(.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="offset"]/UML:TaggedValue.dataValue, "ABCDEFX", "abcdefx")'/></xsl:variable>
  <xsl:variable name="offset">
    <xsl:if test='$offset_ != "" and not(starts-with($offset_, "0x"))'>0x</xsl:if>
    <xsl:value-of select='$offset_'/>
  </xsl:variable>
  <xsl:variable name="shift"><xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="shift"]/UML:TaggedValue.dataValue'/></xsl:variable>
  <xsl:variable name="mask_"><xsl:value-of select='translate(.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="mask"]/UML:TaggedValue.dataValue, "ABCDEFX", "abcdefx")'/></xsl:variable>
  <xsl:variable name="mask">
    <xsl:if test='$mask_ != "" and not(starts-with($mask_, "0x"))'>0x</xsl:if>
    <xsl:value-of select='$mask_'/>
  </xsl:variable>
  <xsl:variable name="type"><xsl:value-of select='.//UML:DataType/@xmi.idref'/></xsl:variable><xsl:variable name="saltype">
  <xsl:call-template name='saltype'>
    <xsl:with-param name='type'>
      <xsl:value-of select='$type'/>
    </xsl:with-param>
  </xsl:call-template>
</xsl:variable>

/*
<xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="comment"]/UML:TaggedValue.dataValue'/>

Name: <xsl:value-of select='@name'/>(<xsl:value-of select='$stereotype'/>)
Offset: <xsl:value-of select='$offset'/>
Mask: <xsl:value-of select='$mask'/>
Shift:  <xsl:value-of select='$shift'/>
*/

<xsl:choose>
  <xsl:when test='$stereotype = "attribute"'>
    m<xsl:value-of select='@name'/> =  (get<xsl:value-of select="$type"/>(<xsl:value-of select="$offset"/>)<xsl:if test="string-length($mask)>0"> &amp; <xsl:value-of select="$mask"/></xsl:if>)<xsl:if test="$shift>0"> &gt;&gt; <xsl:value-of select="$shift"/></xsl:if>;
  </xsl:when>
  <xsl:when test='$stereotype = "array"'>
    <xsl:text>
      {
    sal_uInt32 nCount = </xsl:text>
    <xsl:value-of select='.//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref = "arraycount"]/UML:TaggedValue.dataValue'/>
    <xsl:text>;
    m</xsl:text>
    <xsl:value-of select='@name'/>
    <xsl:text> = new </xsl:text><xsl:value-of select='$saltype'/><xsl:text>[nCount];

    for (sal_uInt32 n = 0; n &lt; nCount; ++n)
    m</xsl:text><xsl:value-of select='@name'/><xsl:text>[n] = get</xsl:text>
    <xsl:value-of select='$type'/>
    <xsl:text>(</xsl:text>
    <xsl:value-of select='$offset'/>
    <xsl:text> + nIndex * sizeof(</xsl:text>
    <xsl:value-of select='$saltype'/>
    <xsl:text>));
    }
    </xsl:text>
  </xsl:when>
  <xsl:when test='$stereotype = "string"'>
    m<xsl:value-of select='@name'/> = getString(<xsl:value-of select='$offset'/>)
  </xsl:when>
</xsl:choose>
</xsl:template>

<xsl:template match="UML:Model" mode="isbooleandffopt">
<xsl:text>
bool isBooleanDffOpt(sal_uInt32 nId)
{
    bool bResult = false;

    switch (nId)
    {&#xa;</xsl:text>
    <xsl:for-each select='.//UML:Class[.//UML:Stereotype/@xmi.idref="dffopt"]'>
      <xsl:if test=".//UML:TagDefinition[@xmi.idref='isbool']">
        <xsl:text>    case </xsl:text>
        <xsl:variable name="optname">
          <xsl:value-of select=".//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref='optname']//UML:TaggedValue.dataValue"/>
        </xsl:variable>
        <xsl:call-template name="idtoqname">
          <xsl:with-param name="id" select="$optname"/>
        </xsl:call-template>
        <xsl:text>:&#xa;</xsl:text>
      </xsl:if>
    </xsl:for-each>
    <xsl:text>
      bResult = true;
      break;

    default:
        break;
    }
    
    return bResult;
}
</xsl:text>
</xsl:template>

</xsl:stylesheet>

