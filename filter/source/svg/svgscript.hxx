 /*************************************************************************
 *
 *  $RCSfile: svgscript.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2002-08-21 06:03:21 $
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

static const char aSVGScript1[] = "\n\
<![CDATA[\n\
    var nCurSlide = 0;\n\
    var nSlides = 0;\n\
    var aSlideDescriptors = new Object();\n\
\n\
    function SlideDescriptor( aSlide,\n\
                              aMaster,\n\
                              aMasterBackground,\n\
                              aMasterBackgroundVisibility,\n\
                              aMasterObjects,\n\
                              aMasterObjectsVisibility )\n\
    {\n\
        this.aSlide = aSlide;\n\
        this.aMaster = aMaster;\n\
        this.aMasterBackground = aMasterBackground;\n\
        this.aMasterBackgroundVisibility = aMasterBackgroundVisibility;\n\
        this.aMasterObjects = aMasterObjects;\n\
        this.aMasterObjectsVisibility = aMasterObjectsVisibility;\n\
    }\n\
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
    function onKeyPress( aEvt )\n\
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
            nOffset = -1;\n\
        }\n\
\n\
        if( 0 != nOffset )\n\
            switchSlide( aEvt, nOffset );\n\
    }\n\
";

static const char aSVGScript2[] = "\n\
    function switchSlide( aEvt, nOffset )\n\
    {\n\
        var nNxtSlide = nCurSlide + nOffset;\n\
\n\
        if( nNxtSlide < 0 && nSlides > 0 )\n\
            nNxtSlide = nSlides - 1;\n\
        else if( nNxtSlide >= nSlides ) \n\
            nNxtSlide = 0;\n\
\n\
        var aCurDescriptor = aSlideDescriptors[ nCurSlide ];\n\
        var aNxtDescriptor = aSlideDescriptors[ nNxtSlide ];\n\
        var aVisibility = \"visibility\";\n\
\n\
        aCurDescriptor.aSlide.setAttributeNS( null, aVisibility, \"hidden\" );\n\
        aNxtDescriptor.aSlide.setAttributeNS( null, aVisibility, \"visible\" );\n\
\n\
        if( ( 0 == nOffset ) || \n\
            ( aCurDescriptor.aMaster != aNxtDescriptor.aMaster ) ||\n\
            ( aCurDescriptor.aMasterBackgroundVisibility != aNxtDescriptor.aMasterBackgroundVisibility ) ||\n\
            ( aCurDescriptor.aMasterObjectsVisibility != aNxtDescriptor.aMasterObjectsVisibility ) )\n\
\n\
        {\n\
            aCurDescriptor.aMaster.setAttributeNS( null, aVisibility, \"hidden\" );\n\
            aCurDescriptor.aMasterBackground.setAttributeNS( null, aVisibility, \"hidden\" );\n\
            aCurDescriptor.aMasterObjects.setAttributeNS( null, aVisibility, \"hidden\" );\n\
\n\
            aNxtDescriptor.aMaster.setAttributeNS( null, aVisibility, \"visible\" );\n\
            aNxtDescriptor.aMasterBackground.setAttributeNS( null, aVisibility, aNxtDescriptor.aMasterBackgroundVisibility );\n\
            aNxtDescriptor.aMasterObjects.setAttributeNS( null, aVisibility, aNxtDescriptor.aMasterObjectsVisibility );\n\
        }\n\
\n\
        nCurSlide = nNxtSlide; \n\
    }\n\
";

static const char aSVGScript3[] = "\n\
    function onLoad() \n\
    {\n\
        nSlides = document.getElementById( \"meta_slides\" ).getAttributeNS( null, \"numberOfSlides\" );\n\
\n\
        for( i = 0; i < nSlides; i++ )\n\
        {\n\
            var aSlide = document.getElementById( \"meta_slide\" + i );\n\
            var aSlideName = aSlide.getAttributeNS( null, \"slide\" );\n\
            var aMasterName = aSlide.getAttributeNS( null, \"master\" );\n\
\n\
            aSlideDescriptors[ i ] = new SlideDescriptor( \n\
                document.getElementById( aSlideName ),\n\
                document.getElementById( aMasterName ),\n\
                document.getElementById( aMasterName + \"_background\" ),\n\
                aSlide.getAttributeNS( null, \"master-background-visibility\" ),\n\
                document.getElementById( aMasterName + \"_objects\" ),\n\
                aSlide.getAttributeNS( null, \"master-objects-visibility\" ) );\n\
        }\n\
\n\
        switchSlide( 0, 0 );\n\
    }\n\
]]>\n\
";

/*
<![CDATA[
    var nCurSlide = 0;
    var nSlides = 0;
    var aSlideDescriptors = new Object();

    function SlideDescriptor( aSlide,
                              aMaster,
                              aMasterBackground,
                              aMasterBackgroundVisibility,
                              aMasterObjects,
                              aMasterObjectsVisibility )
    {
        this.aSlide = aSlide;
        this.aMaster = aMaster;
        this.aMasterBackground = aMasterBackground;
        this.aMasterBackgroundVisibility = aMasterBackgroundVisibility;
        this.aMasterObjects = aMasterObjects;
        this.aMasterObjectsVisibility = aMasterObjectsVisibility;
    }

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
            nOffset = -1;
        }

        if( 0 != nOffset )
            switchSlide( aEvt, nOffset );
    }

    function switchSlide( aEvt, nOffset )
    {
        var nNxtSlide = nCurSlide + nOffset;

        if( nNxtSlide < 0 && nSlides > 0 )
            nNxtSlide = nSlides - 1;
        else if( nNxtSlide >= nSlides )
            nNxtSlide = 0;

        var aCurDescriptor = aSlideDescriptors[ nCurSlide ];
        var aNxtDescriptor = aSlideDescriptors[ nNxtSlide ];

        aCurDescriptor.aSlide.setAttributeNS( null, "visibility", "hidden" );
        aNxtDescriptor.aSlide.setAttributeNS( null, "visibility", "visible" );

        if( ( 0 == nOffset ) ||
            ( aCurDescriptor.aMaster != aNxtDescriptor.aMaster ) ||
            ( aCurDescriptor.aMasterBackgroundVisibility != aNxtDescriptor.aMasterBackgroundVisibility ) ||
            ( aCurDescriptor.aMasterObjectsVisibility != aNxtDescriptor.aMasterObjectsVisibility ) )

        {
            aCurDescriptor.aMaster.setAttributeNS( null, "visibility", "hidden" );
            aCurDescriptor.aMasterBackground.setAttributeNS( null, "visibility", "hidden" );
            aCurDescriptor.aMasterObjects.setAttributeNS( null, "visibility", "hidden" );

            aNxtDescriptor.aMaster.setAttributeNS( null, "visibility", "visible" );
            aNxtDescriptor.aMasterBackground.setAttributeNS( null, "visibility", aNxtDescriptor.aMasterBackgroundVisibility );
            aNxtDescriptor.aMasterObjects.setAttributeNS( null, "visibility", aNxtDescriptor.aMasterObjectsVisibility );
        }

        nCurSlide = nNxtSlide;
    }

    function onLoad()
    {
        nSlides = document.getElementById( "meta_slides" ).getAttributeNS( null, "numberOfSlides" );

        for( i = 0; i < nSlides; i++ )
        {
            var aSlide = document.getElementById( "meta_slide" + i );
            var aSlideName = aSlide.getAttributeNS( null, "slide" );
            var aMasterName = aSlide.getAttributeNS( null, "master" );

            aSlideDescriptors[ i ] = new SlideDescriptor(
                document.getElementById( aSlideName ),
                document.getElementById( aMasterName ),
                document.getElementById( aMasterName + "_background" ),
                aSlide.getAttributeNS( null, "master-background-visibility" ),
                document.getElementById( aMasterName + "_objects" ),
                aSlide.getAttributeNS( null, "master-objects-visibility" ) );
        }

        switchSlide( 0, 0 );
    }
]]>
*/
