/*************************************************************************
 *
 *  $RCSfile: svgscript.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 17:57:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

static const char aSVGScript1[] =
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

static const char aSVGScript2[] =
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

/*
<![CDATA[
    var nCurSlide = 0;
    var nSlides = 0;
    var aSlides = new Object();
    var aMasters = new Object();
    var aMasterVisibilities;

    function onClick( aEvt )
    {
        var nOffset = 0;

        if( aEvt.getButton() == 0 )
            nOffset = 1;
        else if( aEvt.getButton() == 2 )
            nOffset = -1;

        if( 0 != nOffset )
            switchSlide( aEvt, nOffset );
    }

    function onKeyPress( aEvt )
    {
        var nCode = String.fromCharCode( aEvt.getCharCode() );
        var nOffset = 0;

        if( ( ' ' == nCode ) ||
            ( ( !aEvt.getCtrlKey() && !aEvt.getAltKey() && !aEvt.getMetaKey() && !aEvt.getShiftKey() ) &&
              ( aEvt.getKeyCode() == aEvt.DOM_VK_PAGE_DOWN() ||
                aEvt.getKeyCode() == aEvt.DOM_VK_PAGE_RIGHT() ) ) )
        {
            nOffset = 1;
        }
        else if( ( !aEvt.getCtrlKey() && !aEvt.getAltKey() && !aEvt.getMetaKey() && !aEvt.getShiftKey() ) &&
                 ( aEvt.getKeyCode() == aEvt.DOM_VK_PAGE_UP() ||
                   aEvt.getKeyCode() == aEvt.DOM_VK_LEFT() ) )
        {
            nOffset = -1
        }

        if( 0 != nOffset )
            switchSlide( aEvt, nOffset );
    }

    function switchSlide( aEvt, nOffset )
    {
        var nNextSlide = nCurSlide + nOffset;

        if( nNextSlide < 0 && nSlides > 0 )
            nNextSlide = nSlides - 1;
        else if( nNextSlide >= nSlides )
            nNextSlide = 0;

        aSlides[ nCurSlide ].setAttributeNS( null, "visibility", "hidden" );
        aSlides[ nNextSlide ].setAttributeNS( null, "visibility", "visible" );

        var aCurMaster = aMasters[ nCurSlide ];
        var aCurMasterVisibility = aMasterVisibilities[ nCurSlide ];

        var aNextMaster = aMasters[ nNextSlide ];
        var aNextMasterVisibility = aMasterVisibilities[ nNextSlide ];

        if( ( aCurMaster != aNextMaster ) || ( aCurMasterVisibility != aNextMasterVisibility ) )
        {
            if( aCurMaster != aNextMaster )
                aCurMaster.setAttributeNS( null, "visibility", "hidden" );

            aNextMaster.setAttributeNS( null, "visibility", aNextMasterVisibility );
        }

        nCurSlide = nNextSlide;
    }

    function init()
    {
        nSlides = document.getElementById( "meta_slides" ).getAttributeNS( null, "numberOfSlides" );

        for( i = 0; i < nSlides; i++ )
        {
            var aSlide = document.getElementById( "meta_slide" + i );

            aSlides[ i ] = document.getElementById( aSlide.getAttributeNS( null, "slide" ) );
            aMasters[ i ] = document.getElementById( aSlide.getAttributeNS( null, "master" ) );
            aMasterVisibilities[ i ] = aSlide.getAttributeNS( null, "master-visibility" );
        }
    }

    init();
]]*/
