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
