/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:dom="http://www.w3.org/2001/xml-events" xmlns:xforms="http://www.w3.org/2002/xforms" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"  
                xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#" xmlns:UML = 'org.omg.xmi.namespace.UML' xml:space="default">
  <xsl:output method="text" />

  <!-- Key all attributes with the same name and same value -->
  <xsl:key name="same-valued-tagged-data"
           match="UML:TaggedValue.dataValue" use="." />

  <xsl:template match="/">
    <out>
      <xsl:text>
/*

   THIS FILE IS GENERATED AUTOMATICALLY! DO NOT EDIT!
   
*/

package analyze;

import java.util.LinkedHashMap;

/**
 *
 * @author hb137859
 */
public class ResourceIdMap extends LinkedHashMap &lt;String, Integer &gt; {

    static ResourceIdMap mInstance = new ResourceIdMap();
    
    /** Creates a new instance of ResourceIdMap */
    protected ResourceIdMap() {
/* Attributes */&#xa;</xsl:text>
      <xsl:for-each select='.//UML:Attribute[@name!="reserved"][count(.//UML:Stereotype[@xmi.idref="noqname"]) = 0]//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="attrid"]'>
        <xsl:choose>
          <xsl:when test='generate-id(UML:TaggedValue.dataValue) != generate-id(key("same-valued-tagged-data", UML:TaggedValue.dataValue)[1])'/>
          <!-- <xsl:when test='.//UML:TaggedValue.dataValue = preceding::*//UML:TaggedValue.dataValue'/>-->
          <xsl:otherwise>
            <xsl:text>put("</xsl:text>
            <xsl:call-template name='idtoqname'>
              <xsl:with-param name='id'><xsl:value-of select='.//UML:TaggedValue.dataValue'/></xsl:with-param>
            </xsl:call-template>
            <xsl:text>", </xsl:text>
            <xsl:value-of select='10000 + position()'/>
            <xsl:text>);&#xa;</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:for-each>
      <xsl:text>

/* Operations */&#xa;</xsl:text>
      <xsl:for-each select='.//UML:Operation[@name!="reserved"][count(.//UML:Stereotype[@xmi.idref="noqname"]) = 0]//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="opid"]'>
        <xsl:choose>
          <xsl:when test='generate-id(UML:TaggedValue.dataValue) != generate-id(key("same-valued-tagged-data", UML:TaggedValue.dataValue)[1])'/>
          <xsl:otherwise>
            <xsl:text>put("</xsl:text>
            <xsl:call-template name='idtoqname'>
              <xsl:with-param name='id'><xsl:value-of select='.//UML:TaggedValue.dataValue'/></xsl:with-param>
            </xsl:call-template>
            <xsl:text>", </xsl:text>
            <xsl:value-of select='20000 + position()'/>
            <xsl:text>); &#xa;</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:for-each>
      <xsl:text>

/* Classes */&#xa;</xsl:text>
      <xsl:for-each select='.//UML:Class[@name!="reserved"]//UML:TaggedValue[.//UML:TagDefinition/@xmi.idref="classid"]'>
        <xsl:choose>
          <xsl:when test='.//UML:Stereotype[@xmi.idref="noqname"]'/>
          <xsl:when test='generate-id(UML:TaggedValue.dataValue) != generate-id(key("same-valued-tagged-data", UML:TaggedValue.dataValue)[1])'/>
          <xsl:otherwise>
            <xsl:text>put("</xsl:text>
            <xsl:call-template name='idtoqname'>
              <xsl:with-param name='id'><xsl:value-of select='.//UML:TaggedValue.dataValue'/></xsl:with-param>
            </xsl:call-template>
            <xsl:text>", </xsl:text>
            <xsl:value-of select='30000 + position()'/>
            <xsl:text>); &#xa;</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:for-each>
      <xsl:text>
   }

   static ResourceIdMap instance() {
       return mInstance;
   }
};
</xsl:text>
</out>
</xsl:template>

  <xsl:template name='idtoqname'>
    <xsl:param name='id'/>rtf:<xsl:value-of select='substring-after($id, ":")'/>
  </xsl:template>

</xsl:stylesheet>
