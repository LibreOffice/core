<?xml version="1.0" encoding="utf-8"?>
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
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:office="urn:oasis:names:tc:openoffice:xmlns:office:1.0"
                xmlns:style="urn:oasis:names:tc:openoffice:xmlns:style:1.0"
                xmlns:text="urn:oasis:names:tc:openoffice:xmlns:text:1.0"
                xmlns:table="urn:oasis:names:tc:openoffice:xmlns:table:1.0"
                xmlns:draw="urn:oasis:names:tc:openoffice:xmlns:drawing:1.0"
                xmlns:fo="http://www.w3.org/1999/XSL/Format"
                xmlns:xlink="http://www.w3.org/1999/xlink"
                xmlns:dc="http://purl.org/dc/elements/1.1/"
                xmlns:meta="urn:oasis:names:tc:openoffice:xmlns:meta:1.0"
                xmlns:number="urn:oasis:names:tc:openoffice:xmlns:datastyle:1.0"
                xmlns:pres="urn:oasis:names:tc:openoffice:xmlns:presentation:1.0"
                xmlns:svg="http://www.w3.org/2000/svg"
                xmlns:chart="urn:oasis:names:tc:openoffice:xmlns:chart:1.0"
                xmlns:dr3d="urn:oasis:names:tc:openoffice:xmlns:dr3d:1.0"
                xmlns:math="http://www.w3.org/1998/Math/MathML"
                xmlns:form="urn:oasis:names:tc:openoffice:xmlns:form:1.0"
                xmlns:script="urn:oasis:names:tc:openoffice:xmlns:script:1.0"
                xmlns:ooo="http://openoffice.org/2004/office"
                xmlns:ooow="http://openoffice.org/2004/writer"
                xmlns:oooc="http://openoffice.org/2004/calc"
                xmlns:dom="http://www.w3.org/2001/xml-events"
                xmlns:smil="http://www.w3.org/2001/SMIL20"
                xmlns:anim="urn:oasis:names:tc:openoffice:xmlns:animation:1.0"

                version="1.0">

  <!-- fly in -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-fly-in']">
    <anim:par pres:preset-property="Direction;Accelerate;Decelerate">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- fade in an swivel -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-entrance-fade-in-and-swivel']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- falling in -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-entrance-falling-in']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- Unfold -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-entrance-unfold']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- Unfold -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-exit-unfold']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- turn and grow -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-entrance-turn-and-grow']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- fold -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-fold']">
    <anim:par pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- whip -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-entrance-whip']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- flip -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-entrance-flip']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- swish -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-entrance-swish']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- blinds -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-venetian-blinds']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- box -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-box']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- checkerboard -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-checkerboard']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- circle -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-circle']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- crawl in -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-fly-in-slow']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- diamond -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-diamond']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- peek in -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-peek-in']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- plus -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-plus']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- random bars -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-random-bars']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- split -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-split']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- strips -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-diagonal-squares']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- wheel -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-wheel']">
    <anim:par pres:preset-property="Spokes">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- wipe -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-wipe']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- color typewriter -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-entrance-colored-lettering']">
    <anim:iterate pres:preset-property="Color1;Color2" pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- stretch -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-stretchy']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- zoom -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-zoom']">
    <anim:par pres:preset-property="Zoom">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- swivel -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-entrance-swivel']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

<!-- emphasis -->

  <!-- change fill color -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-fill-color']">
    <anim:par pres:preset-property="FillColor;ColorStyle;Accelerate;Decelerate;AutoReverse">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- change font -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-font']">
    <anim:par pres:preset-property="FontStyle" pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- change font color -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-font-color']">
    <anim:par pres:preset-property="CharColor;ColorStyle;Accelerate;Decelerate;AutoReverse" pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- change font size -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-font-size']">
    <anim:par pres:preset-property="CharHeight" pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- change font style -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-font-style']">
    <anim:par pres:preset-property="CharDecoration" pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- change line color -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-line-color']">
    <anim:par pres:preset-property="LineColor;ColorStyle;Accelerate;Decelerate;AutoReverse">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- grow/shrink -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-grow-and-shrink']">
    <anim:par pres:preset-property="Scale;Accelerate;Decelerate;AutoReverse">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- spin -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-spin']">
    <anim:par pres:preset-property="Rotate;Accelerate;Decelerate;AutoReverse">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- Transparency -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-transparency']">
    <anim:par pres:preset-property="Transparency">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- brush on color -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-emphasis-color-over-by-word']">
    <anim:iterate pres:preset-property="Color" pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- color blend -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-color-blend']">
    <anim:par pres:preset-property="Color">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- color wave -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-emphasis-color-over-by-letter']">
    <anim:iterate pres:preset-property="Color" pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- bold flash -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-bold-flash']">
    <anim:par pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- reveal underline -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-emphasis-reveal-underline']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- shimmer -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-emphasis-shimmer']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- verticales highlight -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-vertical-highlight']">
    <anim:par pres:preset-property="Color">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- flicker -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-flicker']">
    <anim:par pres:preset-property="Color">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- grow with color -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-emphasis-grow-with-color']">
    <anim:iterate pres:preset-property="Color" pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- teeter -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-teeter']">
    <anim:par pres:preset-property="Color">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- blast -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-blast']">
    <anim:par pres:preset-property="Color">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- style emphasis -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-emphasis-style-emphasis']">
    <anim:par pres:preset-property="Color" pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- bold reveal -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-emphasis-bold-reveal']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

    <!-- wave -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-emphasis-wave']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

<!-- exit -->

  <!-- blinds -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-venetian-blinds']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- box -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-box']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- checkerboard -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-checkerboard']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- circle -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-circle']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- crawl out -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-crawl-out']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- diamond -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-diamond']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- fly out -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-fly-out']">
    <anim:par pres:preset-property="Direction;Accelerate;Decelerate">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- peek out -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-peek-out']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- plus -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-plus']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- random bars -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-random-bars']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- split -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-split']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- strips -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-diagonal-squares']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- wheel -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-wheel']">
    <anim:par pres:preset-property="Spokes">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- wipe -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-wipe']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- collapse -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-collapse']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- color typewriter -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-exit-colored-lettering']">
    <anim:iterate pres:preset-property="Color1;Color2" pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- zoom -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-zoom']">
    <anim:par pres:preset-property="Zoom">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- swivel -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-swivel']">
    <anim:par pres:preset-property="Direction">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- fade in an swivel -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-exit-fade-out-and-swivel']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- fade in an swivel -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-exit-turn-and-grow']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- fold -->
  <xsl:template match="anim:par[@pres:preset-id = 'ooo-exit-fold']">
    <anim:par pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:par>
  </xsl:template>

  <!-- whip -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-exit-whip']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- flip -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-exit-flip']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- swish -->
  <xsl:template match="anim:iterate[@pres:preset-id = 'ooo-exit-swish']">
    <anim:iterate pres:text-only="true">
      <xsl:apply-templates select="*|@*|text()"/>
    </anim:iterate>
  </xsl:template>

  <!-- remove pres:group-id -->
  <xsl:template match="@xlink:href" />
  <xsl:template match="@smil:targetElement" />
  <xsl:template match="@pres:group-id" />

  <!-- copy   -->
  <xsl:template match="*|@*|text()">
    <xsl:copy>
      <xsl:apply-templates select="*|@*|text()"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
