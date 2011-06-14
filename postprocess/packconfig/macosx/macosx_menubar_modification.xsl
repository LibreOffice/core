<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version='1.0' 
  xmlns:menu="http://openoffice.org/2001/menu" 
  xmlns:xsl='http://www.w3.org/1999/XSL/Transform' >
  
  <!-- identity template, does reproduce every IN node on the output -->
  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>

  <!-- filtering template : removes the concerned nodes -->
  <!-- removes the separator just before the expected item -->
  <xsl:template match="menu:menuseparator[following-sibling::menu:menuitem[1]/@menu:id='.uno:Quit']"/>
  <!-- suppression of the Quit item -->
  <xsl:template match="menu:menuitem[@menu:id='.uno:Quit']"/>

  <xsl:template match="menu:menuseparator[following-sibling::menu:menuitem[1]/@menu:id='.uno:About']"/>
  <!-- suppression of the About item -->
  <xsl:template match="menu:menuitem[@menu:id='.uno:About']"/>

  <!-- suppression of the OptionsTreeDialog item -->
  <xsl:template match="menu:menuitem[@menu:id='.uno:OptionsTreeDialog']"/>

</xsl:stylesheet>
