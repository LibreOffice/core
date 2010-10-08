<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:w="http://schemas.microsoft.com/office/word/2003/wordml" xmlns:wx="http://schemas.microsoft.com/office/word/2003/auxHint" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:aml="http://schemas.microsoft.com/aml/2001/core" xmlns:dt="uuid:C2F41010-65B3-11d1-A29F-00AA00C14882" xmlns:v="urn:schemas-microsoft-com:vml" xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" xmlns:config="urn:oasis:names:tc:opendocument:xmlns:config:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:dom="http://www.w3.org/2001/xml-events" exclude-result-prefixes="w wx aml o dt  v" xmlns:an="urn:flr:annotate">

<an:page-layout-properties
     context-node-input="w:sectPr"
     context-node-output="style:page-layout-properties">
<an:so-supported select="w:pgMar/@w:gutter"/>
<an:so-supported select="w:pgSz/@w:code"/>
</an:page-layout-properties>

<xsl:template name="page-layout-properties">

<xsl:attribute name="fo:margin-top.value">
  <xsl:value-of select="concat('(.(twips2cm(?[',name(w:hdr),'](?(>=($0[',w:pgMar/@w:top,'])($1(|[',w:pgMar/@w:header,'][720])))($1)($0))[',w:pgMar/@w:top,']))[cm])')"/>
</xsl:attribute>
<xsl:attribute name="fo:margin-bottom.value">
  <xsl:value-of select="concat('(.(twips2cm(?[',name(w:ftr),'](|[',w:pgMar/@w:footer,'][720])[',w:pgMar/@w:bottom,']))[cm])')"/>
</xsl:attribute>
<xsl:attribute name="fo:margin-left.value">
  <xsl:value-of select="concat('(.(twips2cm[',w:pgMar/@w:left,'])[cm])')"/>
</xsl:attribute>
<xsl:attribute name="fo:margin-right.value">
  <xsl:value-of select="concat('(.(twips2cm[',w:pgMar/@w:right,'])[cm])')"/>
</xsl:attribute>

<xsl:attribute name="fo:page-width.value">
  <xsl:value-of select="concat('(.(twips2cm[',w:pgSz/@w:w,'])[cm])')"/>
</xsl:attribute>
<xsl:attribute name="fo:page-height.value">
  <xsl:value-of select="concat('(.(twips2cm[',w:pgSz/@w:h,'])[cm])')"/>
</xsl:attribute>
<xsl:attribute name="style:footnote-max-height.value">  
  <xsl:value-of select="'[0cm]'"/>
</xsl:attribute>
<xsl:attribute name="style:print-orientation.value">
  <xsl:value-of select="concat('(|[',w:pgSz/@w:orient,'][portrait])')"/>
</xsl:attribute>
<xsl:apply-templates select="//w:bgPict"/>
<xsl:call-template name="column-properties"/>
</xsl:template>


<an:column-properties
     context-node-input="w:sectPr"
     context-node-output="style:page-layout-properties">
<an:so-supported select="w:cols/@w:sep"/>
</an:column-properties>
<xsl:template name="column-properties">
<style:columns>
<xsl:attribute name="fo:column-count.value">
  <xsl:value-of select="concat('(|[',w:cols/@w:num,'][1])')"/>
</xsl:attribute>

<xsl:if test="not(w:cols/w:col)"> 
<!-- bug in the OASIS spec resp. bug in xmloff  -->
<xsl:attribute name="fo:column-gap.value">
  <xsl:value-of select="concat('(.(twips2cm[',w:cols/@w:space,'])[cm])')"/>
</xsl:attribute>
</xsl:if>

<xsl:for-each select="w:cols/w:col">
  <style:column> 
     <xsl:attribute name="style:rel-width.value">
       <xsl:value-of select="concat('(.[',@w:w,'][*])')"/>
     </xsl:attribute>
     <xsl:attribute name="fo:start-indent.value">
       <xsl:value-of select="'[0cm]'"/>
     </xsl:attribute>
     <xsl:attribute name="fo:end-indent.value">
       <xsl:value-of select="concat('(.(twips2cm(|[',@w:space,'][0]))[cm])')"/>
     </xsl:attribute>
  </style:column> 
</xsl:for-each>
</style:columns>
</xsl:template>

<an:text-properties 
     context-node-input="w:rPr"
     context-node-output="style:text-properties">
</an:text-properties >
<xsl:template name="text-properties">
<xsl:attribute name="fo:font-weight.value">
 <xsl:value-of select="concat('(switch(|[',w:b/@val,'][',local-name(w:b),'])[on][bold][off][normal][b][bold][])')"/>
</xsl:attribute>
<xsl:attribute name="style:font-weight-asian.value">
 <xsl:value-of select="concat('(switch(|[',w:b/@val,'][',local-name(w:b),'])[on][bold][off][normal][b][bold][])')"/>
</xsl:attribute>
<xsl:attribute name="style:font-weight-complex.value">
 <xsl:value-of select="concat('(switch(|[',w:b-cs/@val,'][',local-name(w:b-cs),'])[on][bold][off][normal][b-cs][bold][])')"/>
</xsl:attribute>
<xsl:attribute name="fo:font-style.value">
 <xsl:value-of select="concat('(switch(|[',w:i/@val,'][',local-name(w:i),'])[on][italic][off][normal][i][italic][])')"/>
</xsl:attribute>
<xsl:attribute name="style:font-style-asian.value">
 <xsl:value-of select="concat('(switch(|[',w:i/@val,'][',local-name(w:i),'])[on][italic][off][normal][i][italic][])')"/>
</xsl:attribute>
<xsl:attribute name="style:font-style-complex.value">
 <xsl:value-of select="concat('(switch(|[',w:i-cs/@val,'][',local-name(w:i-cs),'])[on][italic][off][normal][i-cs][italic][])')"/>
</xsl:attribute>
<xsl:attribute name="fo:text-transform.value">
 <xsl:value-of select="concat('(switch(|[',w:caps/@val,'][',local-name(w:caps),'])[on][uppercase][off][normal][caps][uppercase][])')"/>
</xsl:attribute>
<xsl:attribute name="fo:font-variant.value">
 <xsl:value-of select="concat('(switch(|[',w:smallCaps/@val,'][',local-name(w:smallCaps),'])[on][small-caps][off][normal][smallCaps][small-caps][])')"/>
</xsl:attribute>
</xsl:template>

</xsl:stylesheet>