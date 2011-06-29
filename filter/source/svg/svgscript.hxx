/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

static const char aSVGScriptA[] =
"<![CDATA[\n\
    var nCurSlide = 0;\n\
    var nSlides = 0;\n\
    var aSlides = new Object();\n\
    var aMasters = new Object();\n\
    var aMasterVisibilities = new Object();\n\
\n\
    function onClick( aEvt )\n\
    {\n\
        var nOffset = 0;\n\
\n\
        if( aEvt.getButton() == 0 )  \n\
            nOffset = 1;\n\
        else if( aEvt.getButton() == 2 ) \n\
            nOffset = -1;\n\
\n\
        if( 0 != nOffset )\n\
            switchSlide( aEvt, nOffset );\n\
    }\n\
\n\
    function onKeyPress( aEvt ) \n\
    {\n\
        var nCode = String.fromCharCode( aEvt.getCharCode() );\n\
        var nOffset = 0;\n\
\n\
        if( ( ' ' == nCode ) || \n\
            ( ( !aEvt.getCtrlKey() && !aEvt.getAltKey() && !aEvt.getMetaKey() && !aEvt.getShiftKey() ) && \n\
              ( aEvt.getKeyCode() == aEvt.DOM_VK_PAGE_DOWN() ||\n\
                aEvt.getKeyCode() == aEvt.DOM_VK_PAGE_RIGHT() ) ) )\n\
        {\n\
            nOffset = 1;\n\
        }\n\
        else if( ( !aEvt.getCtrlKey() && !aEvt.getAltKey() && !aEvt.getMetaKey() && !aEvt.getShiftKey() ) && \n\
                 ( aEvt.getKeyCode() == aEvt.DOM_VK_PAGE_UP() ||\n\
                   aEvt.getKeyCode() == aEvt.DOM_VK_LEFT() ) )\n\
        {\n\
            nOffset = -1\n\
        }\n\
\n\
        if( 0 != nOffset )\n\
            switchSlide( aEvt, nOffset );\n\
    }\n\
\n\
";

static const char aSVGScriptB[] =
"   function switchSlide( aEvt, nOffset ) \n\
    {\n\
        var nNextSlide = nCurSlide + nOffset;\n\
\n\
        if( nNextSlide < 0 && nSlides > 0 )\n\
            nNextSlide = nSlides - 1;\n\
        else if( nNextSlide >= nSlides ) \n\
            nNextSlide = 0;\n\
\n\
        aSlides[ nCurSlide ].setAttributeNS( null, \"visibility\", \"hidden\" );\n\
        aSlides[ nNextSlide ].setAttributeNS( null, \"visibility\", \"visible\" );\n\
\n\
        var aCurMaster = aMasters[ nCurSlide ];\n\
        var aCurMasterVisibility = aMasterVisibilities[ nCurSlide ];\n\
        \n\
        var aNextMaster = aMasters[ nNextSlide ];\n\
        var aNextMasterVisibility = aMasterVisibilities[ nNextSlide ];\n\
\n\
        if( ( aCurMaster != aNextMaster ) || ( aCurMasterVisibility != aNextMasterVisibility ) ) \n\
        {\n\
            if( aCurMaster != aNextMaster )\n\
                aCurMaster.setAttributeNS( null, \"visibility\", \"hidden\" );\n\
            \n\
            aNextMaster.setAttributeNS( null, \"visibility\", aNextMasterVisibility );\n\
        }\n\
\n\
        nCurSlide = nNextSlide; \n\
    }\n\
\n\
    function init() \n\
    {\n\
        nSlides = document.getElementById( \"meta_slides\" ).getAttributeNS( null, \"numberOfSlides\" );\n\
\n\
        for( i = 0; i < nSlides; i++ )\n\
        {\n\
            var aSlide = document.getElementById( \"meta_slide\" + i );\n\
            \n\
            aSlides[ i ] = document.getElementById( aSlide.getAttributeNS( null, \"slide\" ) );\n\
            aMasters[ i ] = document.getElementById( aSlide.getAttributeNS( null, \"master\" ) );\n\
            aMasterVisibilities[ i ] = aSlide.getAttributeNS( null, \"master-visibility\" );\n\
        }\n\
    }\n\
\n\
    init();\n\
]]>";

static const char aSVGScript1[] =
"<![CDATA[\n\
\n\
    window.onload = init;\n\
        \n\
    // Keycodes.\n\
    var LEFT_KEY = 37;          // cursor left keycode\n\
    var UP_KEY = 38;            // cursor up keycode\n\
    var RIGHT_KEY = 39;         // cursor right keycode\n\
    var DOWN_KEY = 40;          // cursor down keycode\n\
    var PAGE_UP_KEY = 33;       // page up keycode\n\
    var PAGE_DOWN_KEY = 34;     // page down keycode\n\
    var HOME_KEY = 36;          // home keycode\n\
    var END_KEY = 35;           // end keycode\n\
    var ENTER_KEY = 13;         \n\
    var SPACE_KEY = 32;\n\
    var ESCAPE_KEY = 27;\n\
    \n\
    \n\
    var nCurSlide = 0;\n\
    var nSlides = 0;\n\
    var aSlides = new Array();\n\
    var aMasters = new Array();\n\
    var aMasterVisibilities = new Array();\n\
\n\
\n\
\n\
\n\
";


static const char aSVGScript2[] =
"   function onClick( aEvt )\n\
    {\n\
        if (!aEvt)\n\
            aEvt = window.event;\n\
    \n\
        var nOffset = 0;\n\
    \n\
        if( aEvt.button == 0 )  \n\
            nOffset = 1;\n\
        else if( aEvt.button == 2 ) \n\
            nOffset = -1;\n\
    \n\
        if( 0 != nOffset )\n\
            switchSlide( aEvt, nOffset );\n\
    }\n\
    document.onclick = onClick;\n\
    \n\
    \n\
";

static const char aSVGScript3[] =
"   function onKeyPress( aEvt )\n\
    {\n\
        if (!aEvt)\n\
            aEvt = window.event;\n\
    \n\
        var nCode = aEvt.keyCode || aEvt.charCode;\n\
        var nOffset = 0;\n\
        if( nCode == SPACE_KEY || nCode == PAGE_DOWN_KEY || nCode == RIGHT_KEY )\n\
        {\n\
            nOffset = 1;\n\
        }\n\
        else if( nCode == PAGE_UP_KEY ||nCode == LEFT_KEY )\n\
        {\n\
            nOffset = -1;\n\
        }\n\
    \n\
        if( 0 != nOffset )\n\
            switchSlide( aEvt, nOffset );\n\
    }\n\
    document.onkeypress = onKeyPress;   \n\
    \n\
";

static const char aSVGScript4[] =
"   function switchSlide( aEvt, nOffset ) \n\
    {\n\
        var nNextSlide = nCurSlide + nOffset;\n\
\n\
        if( nNextSlide < 0 && nSlides > 0 )\n\
            nNextSlide = nSlides - 1;\n\
        else if( nNextSlide >= nSlides ) \n\
            nNextSlide = 0;\n\
\n\
        aSlides[ nCurSlide ].setAttribute( \"visibility\", \"hidden\" );\n\
        aSlides[ nNextSlide ].setAttribute( \"visibility\", \"visible\" );\n\
\n\
        var aCurMaster = aMasters[ nCurSlide ];\n\
        var aCurMasterVisibility = aMasterVisibilities[ nCurSlide ];\n\
        \n\
        var aNextMaster = aMasters[ nNextSlide ];\n\
        var aNextMasterVisibility = aMasterVisibilities[ nNextSlide ];\n\
\n\
        if( ( aCurMaster != aNextMaster ) || ( aCurMasterVisibility != aNextMasterVisibility ) ) \n\
        {\n\
            if( aCurMaster != aNextMaster )\n\
                aCurMaster.setAttribute(\"visibility\", \"hidden\" );\n\
            \n\
            aNextMaster.setAttribute( \"visibility\", aNextMasterVisibility );\n\
        }\n\
\n\
        nCurSlide = nNextSlide; \n\
    }\n\
\n\
";

static const char aSVGScript5[] =
"   function init() \n\
    {\n\
        nSlides = document.getElementById( \"meta_slides\" ).getAttribute( \"ooo:numberOfSlides\" );\n\
\n\
        for( i = 0; i < nSlides; i++ )\n\
        {\n\
            var aMetaSlide = document.getElementById( \"meta_slide\" + i );\n\
            if( aMetaSlide )\n\
            {\n\
                aSlides.push( document.getElementById( aMetaSlide.getAttribute( \"ooo:slide\" ) ) );\n\
                aMasters.push( document.getElementById( aMetaSlide.getAttribute( \"ooo:master\" ) ) );\n\
                aMasterVisibilities.push( aMetaSlide.getAttribute( \"ooo:master-visibility\" ) );\n\
            }\n\
        }\n\
    }\n\
 \n\
]]>";



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
