
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



#define N_SVGSCRIPT_FRAGMENTS 9

static const char aSVGScript0[] =
"<![CDATA[\n\
\n\
    /** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\
     * - Presentation Engine v4.7 -\n\
     *\n\
     ** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\
     *  This program is free software: you can redistribute it and/or modify\n\
     *  it under the terms of the GNU General Public License as published by\n\
     *  the Free Software Foundation, either version 3 of the License, or\n\
     *  (at your option) any later version.\n\
     *\n\
     *  This program is distributed in the hope that it will be useful,\n\
     *  but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
     *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
     *  GNU General Public License for more details.\n\
     *\n\
     *  You should have received a copy of the GNU General Public License\n\
     *  along with this program.  If not, see http://www.gnu.org/licenses/.\n\
     *\n\
     ** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n\
     *  Some parts of this script are based on the JessyInk project:\n\
     *  http://code.google.com/p/jessyink/\n\
     *\n\
     ** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */\n\
\n\
    window.onload = init;\n\
\n\
\n\
    // ooo elements\n\
    var aOOOElemMetaSlides = 'ooo:meta_slides';\n\
    var aOOOElemMetaSlide = 'ooo:meta_slide';\n\
    var aOOOElemTextField = 'ooo:text_field';\n\
\n\
    // ooo attributes\n\
    var aOOOAttrNumberOfSlides = 'number-of-slides';\n\
    var aOOOAttrNumberingType = 'page-numbering-type';\n\
\n\
    var aOOOAttrSlide = 'slide';\n\
    var aOOOAttrMaster = 'master';\n\
    var aOOOAttrBackgroundVisibility = 'background-visibility';\n\
    var aOOOAttrMasterObjectsVisibility = 'master-objects-visibility';\n\
    var aOOOAttrPageNumberVisibility = 'page-number-visibility';\n\
    var aOOOAttrDateTimeVisibility = 'date-time-visibility';\n\
    var aOOOAttrFooterVisibility = 'footer-visibility';\n\
    var aOOOAttrHeaderVisibility = 'header-visibility';\n\
    var aOOOAttrDateTimeField = 'date-time-field';\n\
    var aOOOAttrFooterField = 'footer-field';\n\
    var aOOOAttrHeaderField = 'header-field';\n\
\n\
    var aOOOAttrDateTimeFormat = 'date-time-format';\n\
\n\
    var aOOOAttrTextAdjust = 'text-adjust';\n\
\n\
    // Placeholder class names\n\
    var aSlideNumberClassName = 'Slide_Number';\n\
    var aDateTimeClassName = 'Date/Time';\n\
    var aFooterClassName = 'Footer';\n\
    var aHeaderClassName = 'Header';\n\
\n\
    // Creating a namespace dictionary.\n\
    var NSS = new Object();\n\
    NSS['svg']='http://www.w3.org/2000/svg';\n\
    NSS['rdf']='http://www.w3.org/1999/02/22-rdf-syntax-ns#';\n\
    NSS['xlink']='http://www.w3.org/1999/xlink';\n\
    NSS['xml']='http://www.w3.org/XML/1998/namespace';\n\
    NSS['ooo'] = 'http://xml.openoffice.org/svg/export';\n\
\n\
    // Presentation modes.\n\
    var SLIDE_MODE = 1;\n\
    var INDEX_MODE = 2;\n\
\n\
    // Mouse handler actions.\n\
    var MOUSE_UP = 1;\n\
    var MOUSE_DOWN = 2;\n\
    var MOUSE_MOVE = 3;\n\
    var MOUSE_WHEEL = 4;\n\
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
    var ENTER_KEY = 13;\n\
    var SPACE_KEY = 32;\n\
    var ESCAPE_KEY = 27;\n\
\n\
    // Visibility Values\n\
    var HIDDEN = 0;\n\
    var VISIBLE = 1;\n\
    var INHERIT = 2;\n\
    var aVisibilityAttributeValue = [ 'hidden', 'visible', 'inherit' ];\n\
    var aVisibilityValue = { 'hidden' : HIDDEN, 'visible' : VISIBLE, 'inherit' : INHERIT };\n\
\n\
    // Parameters\n\
    var ROOT_NODE = document.getElementsByTagNameNS( NSS['svg'], 'svg' )[0];\n\
    var WIDTH = 0;\n\
    var HEIGHT = 0;\n\
    var INDEX_COLUMNS_DEFAULT = 3;\n\
    var INDEX_OFFSET = 0;\n\
\n\
    // Initialization.\n\
    var theMetaDoc;\n\
    var theSlideIndexPage;\n\
    var currentMode = SLIDE_MODE;\n\
    var processingEffect = false;\n\
    var nCurSlide = 0;\n\
\n\
    // Initialize char and key code dictionaries.\n\
    var charCodeDictionary = getDefaultCharCodeDictionary();\n\
    var keyCodeDictionary = getDefaultKeyCodeDictionary();\n\
\n\
    // Initialize mouse handler dictionary.\n\
    var mouseHandlerDictionary = getDefaultMouseHandlerDictionary();\n\
\n\
    /***************************\n\
     ** OOP support functions **\n\
     ***************************/\n\
\n\
    function object( aObject )\n\
    {\n\
        var F = function() {};\n\
        F.prototype = aObject;\n\
        return new F();\n\
    }\n\
\n\
    function extend( aSubType, aSuperType )\n\
    {\n\
        if (!aSuperType || !aSubType) {\n\
            alert('extend failed, verify dependencies');\n\
        }\n\
        var OP = Object.prototype;\n\
        var sp = aSuperType.prototype;\n\
        var rp = object( sp );\n\
        aSubType.prototype = rp;\n\
\n\
        rp.constructor = aSubType;\n\
        aSubType.superclass = sp;\n\
\n\
        // assign constructor property\n\
        if (aSuperType != Object && sp.constructor == OP.constructor) {\n\
            sp.constructor = aSuperType;\n\
        }\n\
\n\
        return aSubType;\n\
    }\n\
\n\
    // ------------------------------------------------------------------------------------------ //\n\
    /**********************************\n\
     ** Helper functions and classes **\n\
     **********************************/\n\
\n\
    function Rectangle( aSVGRectElem )\n\
    {\n\
        var x = parseInt( aSVGRectElem.getAttribute( 'x' ) );\n\
        var y = parseInt( aSVGRectElem.getAttribute( 'y' ) );\n\
        var width = parseInt( aSVGRectElem.getAttribute( 'width' ) );\n\
        var height = parseInt( aSVGRectElem.getAttribute( 'height' ) );\n\
\n\
        this.left = x;\n\
        this.right = x + width;\n\
        this.top = y;\n\
        this.bottom = y + height;\n\
    }\n\
\n\
    function log( message )\n\
    {\n\
        if( typeof console == 'object' )\n\
        {\n\
            console.log( message );\n\
        }\n\
        else if( typeof opera == 'object' )\n\
        {\n\
            opera.postError( message );\n\
        }\n\
        else if( typeof java == 'object' && typeof java.lang == 'object' )\n\
        {\n\
            java.lang.System.out.println( message );\n\
        }\n\
    }\n\
\n\
    function getNSAttribute( sNSPrefix, aElem, sAttrName )\n\
    {\n\
        if( !aElem ) return null;\n\
        if( aElem.hasAttributeNS( NSS[sNSPrefix], sAttrName ) )\n\
        {\n\
            return aElem.getAttributeNS( NSS[sNSPrefix], sAttrName );\n\
        }\n\
        return null;\n\
    }\n\
\n\
    function getOOOAttribute( aElem, sAttrName )\n\
    {\n\
        return getNSAttribute( 'ooo', aElem, sAttrName );\n\
    }\n\
\n\
    function setNSAttribute( sNSPrefix, aElem, sAttrName, aValue )\n\
";

static const char aSVGScript1[] =
"\
    {\n\
        if( !aElem ) return false;\n\
        if( 'setAttributeNS' in aElem )\n\
        {\n\
            aElem.setAttributeNS( NSS[sNSPrefix], sAttrName, aValue );\n\
            return true;\n\
        }\n\
        else\n\
        {\n\
            aElem.setAttribute(sNSPrefix + ':' + sAttrName, aValue );\n\
            return true;\n\
        }\n\
    }\n\
\n\
    function setOOOAttribute( aElem, sAttrName, aValue )\n\
    {\n\
        return setNSAttribute( 'ooo', aElem, sAttrName, aValue );\n\
    }\n\
\n\
    function checkElemAndSetAttribute( aElem, sAttrName, aValue )\n\
    {\n\
        if( aElem )\n\
            aElem.setAttribute( sAttrName, aValue );\n\
    }\n\
\n\
    function getElementsByProperty( node, name )\n\
    {\n\
        var elems = new Array();\n\
\n\
        if( node.getAttribute( name ) )\n\
            elems.push( node );\n\
\n\
        for( var counter = 0; counter < node.childNodes.length; ++counter )\n\
        {\n\
            if( node.childNodes[counter].nodeType == 1 )\n\
                elems = elems.concat( getElementsByProperty( node.childNodes[counter], name ) );\n\
        }\n\
        return elems;\n\
    }\n\
\n\
    function getElementsByClassName( aElem, sClassName )\n\
    {\n\
\n\
        var aElementSet = new Array();\n\
        // not all browsers support the 'getElementsByClassName' method\n\
        if( 'getElementsByClassName' in aElem )\n\
        {\n\
            aElementSet = aElem.getElementsByClassName( sClassName );\n\
        }\n\
        else\n\
        {\n\
            var aElementSetByClassProperty = getElementsByProperty( aElem, 'class' );\n\
            for( var i = 0; i < aElementSetByClassProperty.length; ++i )\n\
            {\n\
                var sAttrClassName = aElementSetByClassProperty[i].getAttribute( 'class' );\n\
                if( sAttrClassName == sClassName )\n\
                {\n\
                    aElementSet.push( aElementSetByClassProperty[i] );\n\
                }\n\
            }\n\
        }\n\
        return aElementSet;\n\
    }\n\
\n\
    function getElementByClassName( aElem, sClassName /*, sTagName */)\n\
    {\n\
        var aElementSet = getElementsByClassName( aElem, sClassName );\n\
        if ( aElementSet.length == 1 )\n\
            return aElementSet[0];\n\
        else\n\
            return null;\n\
    }\n\
\n\
    function getClassAttribute(  aElem )\n\
    {\n\
        if( aElem )\n\
            return aElem.getAttribute( 'class' );\n\
        return '';\n\
    }\n\
\n\
    function initVisibilityProperty( aElement )\n\
    {\n\
        var nVisibility = VISIBLE;\n\
        var sVisibility = aElement.getAttribute( 'visibility' );\n\
        if( sVisibility ) nVisibility = aVisibilityValue[ sVisibility ];\n\
        return nVisibility;\n\
    }\n\
\n\
    function setElementVisibility( aElement, nCurrentVisibility, nNewVisibility )\n\
    {\n\
        if( nCurrentVisibility !=  nNewVisibility )\n\
        {\n\
            checkElemAndSetAttribute( aElement, 'visibility', aVisibilityAttributeValue[nNewVisibility] );\n\
            return nNewVisibility;\n\
        }\n\
        return nCurrentVisibility;\n\
    }\n\
\n\
    function getSafeIndex( nIndex, nMin, nMax )\n\
    {\n\
        if( nIndex < nMin )\n\
            return nMin;\n\
        else if( nIndex > nMax )\n\
            return nMax;\n\
        else\n\
            return nIndex;\n\
    }\n\
\n\
\n\
    // ------------------------------------------------------------------------------------------ //\n\
    /******************\n\
     ** Core Classes **\n\
     ******************/\n\
\n\
    /** Class MetaDocument **\n\
     *  This class provides a pool of properties related to the whole presentation and\n\
     *  it is responsible for initializing the set of MetaSlide objects that handle\n\
     *  the meta information for each slide.\n\
     */\n\
    function MetaDocument( aMetaDocElem )\n\
    {\n\
        this.nNumberOfSlides = parseInt( aMetaDocElem.getAttributeNS( NSS['ooo'], aOOOAttrNumberOfSlides ) );\n\
        assert( typeof this.nNumberOfSlides == 'number' && this.nNumberOfSlides > 0,\n\
                'MetaDocument: number of slides is zero or undefined.' );\n\
        this.startSlideNumber = 0;\n\
        this.sPageNumberingType = aMetaDocElem.getAttributeNS( NSS['ooo'], aOOOAttrNumberingType ) || 'arabic';\n\
        this.aMetaSlideSet = new Array();\n\
        this.aMasterPageSet = new Object();\n\
        this.aTextFieldSet = new Array();\n\
        this.slideNumberField =  new SlideNumberField( this.sPageNumberingType );\n\
\n\
        for( var i = 0; i < this.nNumberOfSlides; ++i )\n\
        {\n\
            var sMetaSlideId = aOOOElemMetaSlide + '_' + i;\n\
            this.aMetaSlideSet.push( new MetaSlide( sMetaSlideId, this ) );\n\
        }\n\
        assert( this.aMetaSlideSet.length == this.nNumberOfSlides,\n\
                'MetaDocument: aMetaSlideSet.length != nNumberOfSlides.' );\n\
        this.aMetaSlideSet[ this.startSlideNumber ].show();\n\
    }\n\
\n\
    MetaDocument.prototype.initPlaceholderShapes = function()\n\
    {\n\
        this.aMetaSlideSet[0].initPlaceholderShapes();\n\
    };\n\
\n\
\n\
    /** Class MetaSlide **\n\
     *  This class is responsible for managing the visibility of all master page shapes\n\
     *  and background related to a given slide element; it performs the creation and\n\
     *  the initialization of each Text Field object.\n\
     */\n\
    function MetaSlide( sMetaSlideId, aMetaDoc )\n\
    {\n\
        this.theDocument = document;\n\
        this.id = sMetaSlideId;\n\
        this.theMetaDoc = aMetaDoc;\n\
        this.element = this.theDocument.getElementById( this.id );\n\
        assert( this.element, 'MetaSlide: meta_slide element <' + this.id + '> not found.' );\n\
        // - Initialize the Slide Element -\n\
        this.slideId = this.element.getAttributeNS( NSS['ooo'], aOOOAttrSlide );\n\
        this.slideElement = this.theDocument.getElementById( this.slideId );\n\
        assert( this.slideElement, 'MetaSlide: slide element <' + this.slideId + '> not found.' );\n\
        // - Initialize the Target Master Page Element -\n\
        this.masterPage = this.initMasterPage();\n\
        // - Initialize Background -\n\
        //this.aBackground                 = getElementByClassName( this.aSlide, 'Background' );\n\
        // - Initialize Visibility Properties -\n\
        this.nAreMasterObjectsVisible     = this.initVisibilityProperty( aOOOAttrMasterObjectsVisibility,  VISIBLE );\n\
        this.nIsBackgroundVisible         = this.initVisibilityProperty( aOOOAttrBackgroundVisibility,     VISIBLE );\n\
        this.nIsPageNumberVisible         = this.initVisibilityProperty( aOOOAttrPageNumberVisibility,     HIDDEN );\n\
        this.nIsDateTimeVisible           = this.initVisibilityProperty( aOOOAttrDateTimeVisibility,       VISIBLE );\n\
        this.nIsFooterVisible             = this.initVisibilityProperty( aOOOAttrFooterVisibility,         VISIBLE );\n\
        this.nIsHeaderVisible             = this.initVisibilityProperty( aOOOAttrHeaderVisibility,         VISIBLE );\n\
        // - Initialize Master Page Text Fields (Placeholders)-\n\
        this.aMPTextFieldSet = new Object();\n\
        this.aMPTextFieldSet[aSlideNumberClassName]   = this.initSlideNumberField();\n\
        this.aMPTextFieldSet[aDateTimeClassName]      = this.initDateTimeField( aOOOAttrDateTimeField );\n\
        this.aMPTextFieldSet[aFooterClassName]        = this.initFixedTextField( aOOOAttrFooterField );\n\
        this.aMPTextFieldSet[aHeaderClassName]        = this.initFixedTextField( aOOOAttrHeaderField );\n\
    }\n\
\n\
    /*** MetaSlide methods ***/\n\
    MetaSlide.prototype =\n\
    {\n\
        /*** public methods ***/\n\
            hide : function()\n\
            {\n\
                checkElemAndSetAttribute( this.slideElement, 'visibility', 'hidden' );\n\
\n\
                this.masterPage.hide();\n\
                this.masterPage.hideBackground();\n\
\n\
                var aFieldSet = this.aMPTextFieldSet;\n\
                var aShapeSet = this.masterPage.aPlaceholderShapeSet;\n\
                if( aFieldSet[aSlideNumberClassName] )         aFieldSet[aSlideNumberClassName].hide( aShapeSet[aSlideNumberClassName] );\n\
                if( aFieldSet[aDateTimeClassName] )            aFieldSet[aDateTimeClassName].hide( aShapeSet[aDateTimeClassName] );\n\
                if( aFieldSet[aFooterClassName] )              aFieldSet[aFooterClassName].hide( aShapeSet[aFooterClassName] );\n\
                if( aFieldSet[aHeaderClassName] )              aFieldSet[aHeaderClassName].hide( aShapeSet[aHeaderClassName] );\n\
            },\n\
";

static const char aSVGScript2[] =
"\
\n\
            hideExceptMaster : function()\n\
            {\n\
                checkElemAndSetAttribute( this.slideElement, 'visibility', 'hidden' );\n\
            },\n\
\n\
            show : function()\n\
            {\n\
                checkElemAndSetAttribute( this.slideElement, 'visibility', 'visible' );\n\
\n\
                this.masterPage.setVisibility( this.nAreMasterObjectsVisible );\n\
                this.masterPage.setVisibilityBackground( this.nIsBackgroundVisible );\n\
\n\
\n\
                this.setTextFieldVisibility( aSlideNumberClassName, this.nIsPageNumberVisible );\n\
                this.setTextFieldVisibility( aDateTimeClassName, this.nIsDateTimeVisible );\n\
                this.setTextFieldVisibility( aFooterClassName, this.nIsFooterVisible );\n\
                this.setTextFieldVisibility( aHeaderClassName, this.nIsHeaderVisible );\n\
            },\n\
\n\
        getMasterPageId : function()\n\
        {\n\
            return this.masterPage.id;\n\
        },\n\
\n\
        getMasterPageElement : function()\n\
        {\n\
            return this.masterPage.element;\n\
        },\n\
\n\
        getBackground : function()\n\
        {\n\
            return getElementByClassName( this.slideElement, 'Background' );\n\
        },\n\
\n\
        getMasterPageBackground : function()\n\
        {\n\
            return this.masterPage.background;\n\
        },\n\
\n\
        /*** private methods ***/\n\
        initMasterPage : function()\n\
        {\n\
            var sMasterPageId = this.element.getAttributeNS( NSS['ooo'], aOOOAttrMaster );\n\
            if( !this.theMetaDoc.aMasterPageSet.hasOwnProperty( sMasterPageId ) )\n\
                this.theMetaDoc.aMasterPageSet[ sMasterPageId ] = new MasterPage( sMasterPageId );\n\
            return this.theMetaDoc.aMasterPageSet[ sMasterPageId ];\n\
        },\n\
\n\
        initVisibilityProperty : function( aVisibilityAttribute, nDefaultValue )\n\
        {\n\
            var nVisibility = nDefaultValue;\n\
            var sVisibility = getOOOAttribute( this.element, aVisibilityAttribute );\n\
            if( sVisibility )\n\
                nVisibility = aVisibilityValue[ sVisibility ];\n\
            return nVisibility;\n\
        },\n\
\n\
        initSlideNumberField : function()\n\
        {\n\
            return this.theMetaDoc.slideNumberField;\n\
        },\n\
\n\
        initDateTimeField : function( aOOOAttrDateTimeField )\n\
        {\n\
            var sTextFieldId = getOOOAttribute( this.element, aOOOAttrDateTimeField );\n\
            if( !sTextFieldId )  return null;\n\
\n\
            var nLength = aOOOElemTextField.length + 1;\n\
            var nIndex = parseInt(sTextFieldId.substring( nLength ) );\n\
            if( typeof nIndex != 'number') return null;\n\
\n\
            if( !this.theMetaDoc.aTextFieldSet[ nIndex ] )\n\
            {\n\
                var aTextField;\n\
                var aTextFieldElem = document.getElementById( sTextFieldId );\n\
                var sClassName = getClassAttribute( aTextFieldElem );\n\
                if( sClassName == 'FixedDateTimeField' )\n\
                {\n\
                    aTextField = new FixedTextField( aTextFieldElem );\n\
                }\n\
                else if( sClassName == 'VariableDateTimeField' )\n\
                {\n\
                    aTextField = new VariableDateTimeField( aTextFieldElem );\n\
                }\n\
                else\n\
                {\n\
                    aTextField = null;\n\
                }\n\
                this.theMetaDoc.aTextFieldSet[ nIndex ] = aTextField;\n\
            }\n\
            return this.theMetaDoc.aTextFieldSet[ nIndex ];\n\
        },\n\
\n\
        initFixedTextField : function( aOOOAttribute )\n\
        {\n\
            var sTextFieldId = getOOOAttribute( this.element, aOOOAttribute );\n\
            if( !sTextFieldId ) return null;\n\
\n\
            var nLength = aOOOElemTextField.length + 1;\n\
            var nIndex = parseInt( sTextFieldId.substring( nLength ) );\n\
            if( typeof nIndex != 'number') return null;\n\
\n\
            if( !this.theMetaDoc.aTextFieldSet[ nIndex ] )\n\
            {\n\
                var aTextFieldElem = document.getElementById( sTextFieldId );\n\
                this.theMetaDoc.aTextFieldSet[ nIndex ]\n\
                    = new FixedTextField( aTextFieldElem );\n\
            }\n\
            return this.theMetaDoc.aTextFieldSet[ nIndex ];\n\
        },\n\
\n\
        setTextFieldVisibility : function( sClassName, nVisible )\n\
        {\n\
            var aTextField = this.aMPTextFieldSet[ sClassName ];\n\
            var aPlaceholderShape = this.masterPage.aPlaceholderShapeSet[ sClassName ];\n\
            if( !aTextField ) return;\n\
            aTextField.setVisibility( this.nAreMasterObjectsVisible & nVisible, aPlaceholderShape );\n\
        }\n\
    };\n\
\n\
    /** Class MasterPage **\n\
     *  This class gives access to a master page element, its background and\n\
     *  each placeholder shape present in the master page element.\n\
     */\n\
    function MasterPage( sMasterPageId )\n\
    {\n\
        this.id = sMasterPageId;\n\
        this.element = document.getElementById( this.id );\n\
        assert( this.element, 'MasterPage: master page element <' + this.id + '> not found.' );\n\
        this.background = getElementByClassName( this.element, 'Background' );\n\
        this.backgroundId = this.background.getAttribute( 'id' );\n\
        this.backgroundVisibility = initVisibilityProperty( this.background );\n\
        this.backgroundObjects = getElementByClassName( this.element, 'BackgroundObjects' );\n\
        this.backgroundObjectsId = this.backgroundObjects.getAttribute( 'id' );\n\
        this.backgroundObjectsVisibility = initVisibilityProperty( this.backgroundObjects );\n\
        this.aPlaceholderShapeSet = new Object();\n\
        this.initPlaceholderShapes();\n\
    }\n\
\n\
    /*** MasterPage methods ***/\n\
    MasterPage.prototype =\n\
    {\n\
         /*** public method ***/\n\
        setVisibility : function( nVisibility )\n\
        {\n\
            this.backgroundObjectsVisibility = setElementVisibility( this.backgroundObjects, this.backgroundObjectsVisibility, nVisibility );\n\
        },\n\
\n\
        setVisibilityBackground : function( nVisibility )\n\
        {\n\
            this.backgroundVisibility = setElementVisibility( this.background, this.backgroundVisibility, nVisibility );\n\
        },\n\
\n\
        hide : function()\n\
        {\n\
            this.setVisibility( HIDDEN );\n\
        },\n\
\n\
        show : function()\n\
        {\n\
            this.setVisibility( VISIBLE );\n\
        },\n\
\n\
        hideBackground : function()\n\
        {\n\
            this.setVisibilityBackground( HIDDEN );\n\
        },\n\
\n\
        showBackground : function()\n\
        {\n\
            this.setVisibilityBackground( VISIBLE );\n\
        },\n\
\n\
        /*** private method ***/\n\
        initPlaceholderShapes : function()\n\
        {\n\
            this.aPlaceholderShapeSet[ aSlideNumberClassName ] = new PlaceholderShape( this, aSlideNumberClassName );\n\
            this.aPlaceholderShapeSet[ aDateTimeClassName ] = new PlaceholderShape( this, aDateTimeClassName );\n\
            this.aPlaceholderShapeSet[ aFooterClassName ] = new PlaceholderShape( this, aFooterClassName );\n\
            this.aPlaceholderShapeSet[ aHeaderClassName ] = new PlaceholderShape( this, aHeaderClassName );\n\
        }\n\
    };\n\
\n\
    /** Class PlaceholderShape **\n\
     *  This class manages the visibility and the text content of a placeholder shape.\n\
     */\n\
    function PlaceholderShape( aMasterPage, sClassName )\n\
    {\n\
        this.masterPage = aMasterPage;\n\
        this.className = sClassName;\n\
        this.element = null;\n\
        this.textElement = null;\n\
\n\
        this.init();\n\
    }\n\
\n\
    /* public methods */\n\
    PlaceholderShape.prototype.setTextContent = function( sText )\n\
    {\n\
";

static const char aSVGScript3[] =
"\
        if( !this.textElement )\n\
        {\n\
            log( 'error: PlaceholderShape.setTextContent: text element is not valid in placeholder of type '\n\
                    + this.className + ' that belongs to master slide ' + this.masterPage.id );\n\
            return;\n\
        }\n\
        this.textElement.textContent = sText;\n\
    };\n\
\n\
    PlaceholderShape.prototype.setVisibility = function( nVisibility )\n\
    {\n\
        this.element.setAttribute( 'visibility', aVisibilityAttributeValue[nVisibility] );\n\
    };\n\
\n\
    PlaceholderShape.prototype.show = function()\n\
    {\n\
        this.element.setAttribute( 'visibility', 'visible' );\n\
    };\n\
\n\
    PlaceholderShape.prototype.hide = function()\n\
    {\n\
        this.element.setAttribute( 'visibility', 'hidden' );\n\
    };\n\
\n\
    /* private methods */\n\
    PlaceholderShape.prototype.init = function()\n\
    {\n\
        var aShapeElem = getElementByClassName( this.masterPage.backgroundObjects, this.className );\n\
        if( !aShapeElem ) return;\n\
\n\
        this.element = aShapeElem;\n\
        this.element.setAttribute( 'visibility', 'hidden' );\n\
\n\
        this.textElement = getElementByClassName( this.element , 'PlaceholderText' );\n\
        if( !this.textElement )  return;\n\
\n\
\n\
        var aSVGRectElemSet = this.element.getElementsByTagName( 'rect' );\n\
        if( aSVGRectElemSet.length != 1) return;\n\
\n\
        var aRect = new Rectangle( aSVGRectElemSet[0] );\n\
\n\
        var sTextAdjust = getOOOAttribute( this.element, aOOOAttrTextAdjust ) || 'left';\n\
        var sTextAnchor, sX;\n\
        if( sTextAdjust == 'left' )\n\
        {\n\
            sTextAnchor = 'start';\n\
            sX = String( aRect.left );\n\
        }\n\
        else if( sTextAdjust == 'right' )\n\
        {\n\
            sTextAnchor = 'end';\n\
            sX = String( aRect.right );\n\
        }\n\
        else if( sTextAdjust == 'center' )\n\
        {\n\
            sTextAnchor = 'middle';\n\
            var nMiddle = ( aRect.left + aRect.right ) / 2;\n\
            sX = String( parseInt( String( nMiddle ) ) );\n\
        }\n\
\n\
\n\
        this.textElement.setAttribute( 'text-anchor', sTextAnchor );\n\
        this.textElement.setAttribute( 'x', sX );\n\
    };\n\
\n\
\n\
    // ------------------------------------------------------------------------------------------ //\n\
    /********************************\n\
     ** Text Field Class Hierarchy **\n\
     ********************************/\n\
\n\
    /** Class TextField **\n\
     *  This class is the root abstract class of the hierarchy.\n\
     *  The 'shapeElement' property is the shape element to which\n\
     *  this TextField object provides the text content.\n\
     */\n\
    function TextField( aTextFieldElem )\n\
    {\n\
        this.bIsUpdated = false;\n\
    }\n\
\n\
    /*** TextField public methods ***/\n\
    TextField.prototype.getShapeElement = function()\n\
    {\n\
        return this.shapeElement;\n\
    };\n\
\n\
    TextField.prototype.setVisibility = function( nVisibility, aPlaceholderShape )\n\
    {\n\
        if( !this.bIsUpdated )\n\
        {\n\
            if( nVisibility )\n\
            {\n\
                this.update( aPlaceholderShape );\n\
                this.bIsUpdated = true;\n\
            }\n\
            aPlaceholderShape.setVisibility( nVisibility );\n\
        }\n\
        else if( !nVisibility )\n\
        {\n\
            aPlaceholderShape.hide();\n\
            this.bIsUpdated = false;\n\
        }\n\
    };\n\
\n\
    TextField.prototype.show = function( aPlaceholderShape )\n\
    {\n\
        this.setVisibility( VISIBLE, aPlaceholderShape );\n\
    };\n\
\n\
    TextField.prototype.hide = function( aPlaceholderShape )\n\
    {\n\
        this.setVisibility( HIDDEN, aPlaceholderShape );\n\
    };\n\
\n\
\n\
    /** Class FixedTextField **\n\
     *  This class handles text field with a fixed text.\n\
     *  The text content is provided by the 'text' property.\n\
     */\n\
    function FixedTextField( aTextFieldElem )\n\
    {\n\
        TextField.call( this, aTextFieldElem );\n\
        this.text = aTextFieldElem.textContent;\n\
    }\n\
    extend( FixedTextField, TextField );\n\
\n\
    FixedTextField.prototype.update = function( aPlaceholderShape )\n\
    {\n\
        aPlaceholderShape.setTextContent( this.text );\n\
    };\n\
\n\
\n\
    /** Class VariableDateTimeField **\n\
     *  Provide the text content for the related shape by generating the current\n\
     *  date/time in the format specified by the 'dateTimeFormat' property.\n\
     */\n\
    function VariableDateTimeField( aTextFieldElem )\n\
    {\n\
        VariableDateTimeField.superclass.constructor.call( this, aTextFieldElem );\n\
        this.dateTimeFormat = getOOOAttribute( aTextFieldElem, aOOOAttrDateTimeFormat );\n\
    }\n\
    extend( VariableDateTimeField, TextField );\n\
\n\
    /*** public methods ***/\n\
    VariableDateTimeField.prototype.update = function( aPlaceholderShape )\n\
    {\n\
        var sText = this.createDateTimeText( this.dateTimeFormat );\n\
        aPlaceholderShape.setTextContent( sText );\n\
    };\n\
\n\
    VariableDateTimeField.prototype.createDateTimeText = function( sDateTimeFormat )\n\
    {\n\
        // TODO handle date/time format\n\
        var aDate = Date();\n\
        var sDate = aDate.toLocaleString();\n\
        return sDate;\n\
    };\n\
\n\
    /** Class SlideNumberField **\n\
     *  Provides the text content to the related shape by generating\n\
     *  the current page number in the given page numbering type.\n\
     */\n\
    function SlideNumberField( sPageNumberingType )\n\
    {\n\
        SlideNumberField.superclass.constructor.call( this, null );\n\
        this.pageNumberingType = sPageNumberingType;\n\
    }\n\
    extend( SlideNumberField, TextField );\n\
\n\
    /*** public methods ***/\n\
    SlideNumberField.prototype.getNumberingType = function()\n\
    {\n\
        return this.pageNumberingType;\n\
    };\n\
\n\
    SlideNumberField.prototype.update = function( aPlaceholderShape )\n\
    {\n\
        var sText = this.createSlideNumberText( nCurSlide + 1, this.getNumberingType() );\n\
        aPlaceholderShape.setTextContent( sText );\n\
    };\n\
\n\
    SlideNumberField.prototype.createSlideNumberText = function( nSlideNumber, sNumberingType )\n\
    {\n\
        // TODO handle page numbering type\n\
        return String( nSlideNumber );\n\
    };\n\
\n\
\n\
\n\
    //------------------------------------------------------------------------------------------- //\n\
    /********************************\n\
     ** Slide Index Classes **\n\
     ********************************/\n\
\n\
    /** Class SlideIndePagex **\n\
     *  This class is responsible for handling the slide index page\n\
     */\n\
    function SlideIndexPage()\n\
";

static const char aSVGScript4[] =
"\
    {\n\
        this.pageElementId = 'slide_index';\n\
        this.pageBgColor = 'rgb(252,252,252)';\n\
        this.pageElement = this.createPageElement();\n\
        assert( this.pageElement, 'SlideIndexPage: pageElement is not valid' );\n\
        this.indexColumns = INDEX_COLUMNS_DEFAULT;\n\
        this.totalThumbnails = this.indexColumns * this.indexColumns;\n\
        this.selectedSlideIndex = nCurSlide;\n\
\n\
        // set up layout paramers\n\
        this.xSpacingFactor = 600/28000;\n\
        this.ySpacingFactor = 450/21000;\n\
        this.xSpacing = WIDTH * this.xSpacingFactor;\n\
        this.ySpacing = HEIGHT * this.ySpacingFactor;\n\
        this.halfBorderWidthFactor = ( 300/28000 ) * ( this.indexColumns / 3 );\n\
        this.halfBorderWidth = WIDTH * this.halfBorderWidthFactor;\n\
        this.borderWidth = 2 * this.halfBorderWidth;\n\
        // the following formula is used to compute the slide shrinking factor:\n\
        // scaleFactor = ( WIDTH - ( columns + 1 ) * xSpacing ) / ( columns * ( WIDTH + borderWidth ) )\n\
        // indeed we can divide everything by WIDTH:\n\
        this.scaleFactor = ( 1 - ( this.indexColumns + 1 ) * this.xSpacingFactor ) /\n\
                                ( this.indexColumns * ( 1 + 2 * this.halfBorderWidthFactor ) );\n\
\n\
        // We create a Thumbnail Border and Thumbnail MouseArea rectangle template that will be\n\
        // used by every Thumbnail. The Mouse Area rectangle is used in order to trigger the\n\
        // mouseover event properly even when the slide background is hidden.\n\
        this.thumbnailMouseAreaTemplateId = 'thumbnail_mouse_area';\n\
        this.thumbnailMouseAreaTemplateElement = null;\n\
        this.thumbnailBorderTemplateId = 'thumbnail_border';\n\
        this.thumbnailBorderTemplateElement = null;\n\
        this.createTemplateElements();\n\
\n\
        // Now we create the grid of thumbnails\n\
        this.aThumbnailSet = new Array( this.totalThumbnails );\n\
        for( var i = 0; i < this.totalThumbnails; ++i )\n\
        {\n\
            this.aThumbnailSet[i] = new Thumbnail( this, i );\n\
            this.aThumbnailSet[i].updateView();\n\
        }\n\
\n\
        this.curThumbnailIndex = this.selectedSlideIndex % this.totalThumbnails;\n\
        this.aThumbnailSet[ this.curThumbnailIndex ].select();\n\
    }\n\
\n\
\n\
    /* public methods */\n\
    SlideIndexPage.prototype.getTotalThumbnails = function()\n\
    {\n\
        return this.totalThumbnails;\n\
    };\n\
\n\
    SlideIndexPage.prototype.show = function()\n\
    {\n\
        this.pageElement.setAttribute( 'display', 'inherit' );\n\
    };\n\
\n\
    SlideIndexPage.prototype.hide = function()\n\
    {\n\
        this.pageElement.setAttribute( 'display', 'none' );\n\
    };\n\
\n\
    /** setSelection\n\
     *\n\
     * Change the selected thumbnail from the current one to the thumbnail with index nIndex.\n\
     *\n\
     * @param nIndex - the thumbnail index\n\
     */\n\
    SlideIndexPage.prototype.setSelection = function( nIndex )\n\
    {\n\
        nIndex = getSafeIndex( nIndex, 0, this.getTotalThumbnails() - 1 );\n\
        if( this.curThumbnailIndex != nIndex )\n\
        {\n\
            this.aThumbnailSet[ this.curThumbnailIndex ].unselect();\n\
            this.aThumbnailSet[ nIndex ].select();\n\
            this.curThumbnailIndex = nIndex;\n\
        }\n\
        this.selectedSlideIndex = this.aThumbnailSet[ nIndex ].slideIndex;\n\
    };\n\
\n\
    SlideIndexPage.prototype.createPageElement = function()\n\
    {\n\
        var aPageElement = document.createElementNS( NSS['svg'], 'g' );\n\
        aPageElement.setAttribute( 'id', this.pageElementId );\n\
        aPageElement.setAttribute( 'display', 'none' );\n\
\n\
        // the slide index page background\n\
        var sPageBgColor = this.pageBgColor + ';';\n\
        var aRectElement = document.createElementNS( NSS['svg'], 'rect' );\n\
        aRectElement.setAttribute( 'x', 0 );\n\
        aRectElement.setAttribute( 'y', 0 );\n\
        aRectElement.setAttribute( 'width', WIDTH );\n\
        aRectElement.setAttribute( 'height', HEIGHT );\n\
        aRectElement.setAttribute( 'style', 'stroke:none;fill:' + sPageBgColor );\n\
\n\
        aPageElement.appendChild( aRectElement );\n\
        // The index page is appended after all slide elements\n\
        // so when it is displayed it covers them all\n\
        ROOT_NODE.appendChild( aPageElement );\n\
        return( document.getElementById( this.pageElementId ) );\n\
    };\n\
\n\
    SlideIndexPage.prototype.createTemplateElements = function()\n\
    {\n\
        // We define a Rect element as a template of thumbnail border for all slide-thumbnails.\n\
        // The stroke color is defined individually by each thumbnail according to\n\
        // its selection status.\n\
        var aDefsElement = document.createElementNS( NSS['svg'], 'defs' );\n\
        var aRectElement = document.createElementNS( NSS['svg'], 'rect' );\n\
        aRectElement.setAttribute( 'id', this.thumbnailBorderTemplateId );\n\
        aRectElement.setAttribute( 'x', -this.halfBorderWidth );\n\
        aRectElement.setAttribute( 'y', -this.halfBorderWidth );\n\
        aRectElement.setAttribute( 'rx', this.halfBorderWidth );\n\
        aRectElement.setAttribute( 'ry', this.halfBorderWidth );\n\
        aRectElement.setAttribute( 'width', WIDTH + this.halfBorderWidth );\n\
        aRectElement.setAttribute( 'height', HEIGHT + this.halfBorderWidth );\n\
        aRectElement.setAttribute( 'stroke-width', this.borderWidth );\n\
        aRectElement.setAttribute( 'fill', 'none' );\n\
        aDefsElement.appendChild( aRectElement );\n\
\n\
        // We define a Rect element as a template of mouse area for triggering the mouseover event.\n\
        // A copy is used by each thumbnail element.\n\
        aRectElement = document.createElementNS( NSS['svg'], 'rect' );\n\
        aRectElement.setAttribute( 'id', this.thumbnailMouseAreaTemplateId );\n\
        aRectElement.setAttribute( 'x', 0 );\n\
        aRectElement.setAttribute( 'y', 0 );\n\
        aRectElement.setAttribute( 'width', WIDTH );\n\
        aRectElement.setAttribute( 'height', HEIGHT );\n\
        aRectElement.setAttribute( 'fill', this.pageBgColor );\n\
        aDefsElement.appendChild( aRectElement );\n\
\n\
        this.pageElement.appendChild( aDefsElement );\n\
\n\
        this.thumbnailMouseAreaTemplateElement = document.getElementById( this.thumbnailMouseAreaTemplateId );\n\
        this.thumbnailBorderTemplateElement = document.getElementById( this.thumbnailBorderTemplateId );\n\
    };\n\
\n\
    SlideIndexPage.prototype.decreaseNumberOfColumns  = function()\n\
    {\n\
        this.setNumberOfColumns( this.indexColumns - 1 );\n\
    };\n\
\n\
    SlideIndexPage.prototype.increaseNumberOfColumns  = function()\n\
    {\n\
        this.setNumberOfColumns( this.indexColumns + 1 );\n\
    };\n\
\n\
    SlideIndexPage.prototype.resetNumberOfColumns  = function()\n\
    {\n\
        this.setNumberOfColumns( INDEX_COLUMNS_DEFAULT );\n\
    };\n\
\n\
    /** setNumberOfColumns\n\
     *\n\
     * Change the size of the thumbnail grid.\n\
     *\n\
     * @param nNumberOfColumns - the new number of columns/rows of the thumbnail grid\n\
     */\n\
    SlideIndexPage.prototype.setNumberOfColumns  = function( nNumberOfColumns )\n\
    {\n\
        if( this.indexColumns == nNumberOfColumns )  return;\n\
        if( nNumberOfColumns < 2 || nNumberOfColumns > 6 ) return;\n\
\n\
        var suspendHandle = ROOT_NODE.suspendRedraw(500);\n\
\n\
        var nOldTotalThumbnails = this.totalThumbnails;\n\
        this.indexColumns = nNumberOfColumns;\n\
        this.totalThumbnails = nNumberOfColumns * nNumberOfColumns;;\n\
\n\
        this.aThumbnailSet[this.curThumbnailIndex].unselect();\n\
\n\
        // if we decreased the number of used columns we remove the exceding thumbnail elements\n\
        for( var i = this.totalThumbnails; i < nOldTotalThumbnails; ++i )\n\
        {\n\
            this.aThumbnailSet[i].removeElement();\n\
        };\n\
\n\
        // if we increased the number of used columns we create the needed thumbnail objects\n\
        for( var i = nOldTotalThumbnails; i < this.totalThumbnails; ++i )\n\
        {\n\
            this.aThumbnailSet[i] = new Thumbnail( this, i );\n\
        };\n\
\n\
        // we set up layout parameters that depend on the number of columns\n\
        this.halfBorderWidthFactor = ( 300/28000 ) * ( this.indexColumns / 3 );\n\
        this.halfBorderWidth = WIDTH * this.halfBorderWidthFactor;\n\
        this.borderWidth = 2 * this.halfBorderWidth;\n\
        // scaleFactor = ( WIDTH - ( columns + 1 ) * xSpacing ) / ( columns * ( WIDTH + borderWidth ) )\n\
        this.scaleFactor = ( 1 - ( this.indexColumns + 1 ) * this.xSpacingFactor ) /\n\
                                ( this.indexColumns * ( 1 + 2 * this.halfBorderWidthFactor ) );\n\
\n\
        // update the thumbnail border size\n\
        var aRectElement = this.thumbnailBorderTemplateElement;\n\
        aRectElement.setAttribute( 'x', -this.halfBorderWidth );\n\
        aRectElement.setAttribute( 'y', -this.halfBorderWidth );\n\
        aRectElement.setAttribute( 'rx', this.halfBorderWidth );\n\
        aRectElement.setAttribute( 'ry', this.halfBorderWidth );\n\
        aRectElement.setAttribute( 'width', WIDTH + this.halfBorderWidth );\n\
        aRectElement.setAttribute( 'height', HEIGHT + this.halfBorderWidth );\n\
        aRectElement.setAttribute( 'stroke-width', this.borderWidth );\n\
\n\
";

static const char aSVGScript5[] =
"\
        // now we update the displacement on the index page of each thumbnail (old and new)\n\
        for( var i = 0; i < this.totalThumbnails; ++i )\n\
        {\n\
            this.aThumbnailSet[i].updateView();\n\
        }\n\
\n\
        this.curThumbnailIndex = this.selectedSlideIndex % this.totalThumbnails;\n\
        this.aThumbnailSet[this.curThumbnailIndex].select();\n\
\n\
        // needed for forcing the indexSetPageSlide routine to update the INDEX_OFFSET\n\
        INDEX_OFFSET = -1;\n\
        indexSetPageSlide( this.selectedSlideIndex );\n\
\n\
        ROOT_NODE.unsuspendRedraw( suspendHandle );\n\
        ROOT_NODE.forceRedraw();\n\
    };\n\
\n\
\n\
    /** Class Thumbnail **\n\
     *  This class handles a slide thumbnail.\n\
     */\n\
    function Thumbnail( aSlideIndexPage, nIndex )\n\
    {\n\
        this.container = aSlideIndexPage;\n\
        this.index = nIndex;//= getSafeIndex( nIndex, 0, this.container.getTotalThumbnails() );\n\
        this.pageElement = this.container.pageElement;\n\
        this.thumbnailId = 'thumbnail' + this.index;\n\
        this.thumbnailElement = this.createThumbnailElement();\n\
        this.slideElement = getElementByClassName( this.thumbnailElement, 'Slide' );\n\
        this.backgroundElement = getElementByClassName( this.thumbnailElement, 'Background' );\n\
        this.backgroundObjectsElement = getElementByClassName( this.thumbnailElement, 'BackgroundObjects' );\n\
        this.borderElement = getElementByClassName( this.thumbnailElement, 'Border' );\n\
        this.aTransformSet = new Array( 3 );\n\
        this.visibility = VISIBLE;\n\
        this.isSelected = false;\n\
    };\n\
\n\
    /* static const class member */\n\
    Thumbnail.prototype.sNormalBorderColor = 'rgb(216,216,216)';\n\
    Thumbnail.prototype.sSelectionBorderColor = 'rgb(92,92,255)';\n\
\n\
    /* public methods */\n\
    Thumbnail.prototype.removeElement = function()\n\
    {\n\
        if( this.thumbnailElement )\n\
            this.container.pageElement.removeChild( this.thumbnailElement );\n\
    };\n\
\n\
    Thumbnail.prototype.show = function()\n\
    {\n\
        if( this.visibility == HIDDEN )\n\
        {\n\
            this.thumbnailElement.setAttribute( 'display', 'inherit' );\n\
            this.visibility = VISIBLE;\n\
        }\n\
    };\n\
\n\
    Thumbnail.prototype.hide = function()\n\
    {\n\
        if( this.visibility == VISIBLE )\n\
        {\n\
            this.thumbnailElement.setAttribute( 'display', 'none' );\n\
            this.visibility = HIDDEN;\n\
        }\n\
    };\n\
\n\
    Thumbnail.prototype.select = function()\n\
    {\n\
        if( !this.isSelected )\n\
        {\n\
            this.borderElement.setAttribute( 'stroke', this.sSelectionBorderColor );\n\
            this.isSelected = true;\n\
        }\n\
    };\n\
\n\
    Thumbnail.prototype.unselect = function()\n\
    {\n\
        if( this.isSelected )\n\
        {\n\
            this.borderElement.setAttribute( 'stroke', this.sNormalBorderColor );\n\
            this.isSelected = false;\n\
        }\n\
    };\n\
\n\
    /** updateView\n\
     *\n\
     *  This method updates the displacement of the thumbnail on the slide index page,\n\
     *  the value of the row, column coordinates of the thumbnail in the grid, and\n\
     *  the onmouseover property of the thumbnail element.\n\
     *\n\
     */\n\
    Thumbnail.prototype.updateView = function()\n\
    {\n\
        this.column = this.index % this.container.indexColumns;\n\
        this.row = ( this.index - this.column ) / this.container.indexColumns;\n\
        this.halfBorderWidth = this.container.halfBorderWidth;\n\
        this.borderWidth = this.container.borderWidth;\n\
        this.width = ( WIDTH + this.borderWidth ) * this.container.scaleFactor;\n\
        this.height = ( HEIGHT + this.borderWidth ) * this.container.scaleFactor;\n\
        this.aTransformSet[2] = 'translate(' + this.halfBorderWidth + ' ' + this.halfBorderWidth + ')';\n\
        this.aTransformSet[1] = 'scale(' + this.container.scaleFactor + ')';\n\
        var sTransformAttrValue = this.computeTransform();\n\
        this.thumbnailElement.setAttribute( 'transform', sTransformAttrValue );\n\
        this.thumbnailElement.setAttribute( 'onmouseover', 'theSlideIndexPage.aThumbnailSet[' + this.index  + '].onMouseOver()' );\n\
    };\n\
\n\
    /** update\n\
     *\n\
     * This method update the content of the thumbnail view\n\
     *\n\
     * @param nIndex - the index of the slide to be shown in the thumbnail\n\
     */\n\
    Thumbnail.prototype.update = function( nIndex )\n\
    {\n\
       if( this.slideIndex == nIndex )  return;\n\
\n\
       var aMetaSlide = theMetaDoc.aMetaSlideSet[nIndex];\n\
       setNSAttribute( 'xlink', this.slideElement, 'href', '#' + aMetaSlide.slideId );\n\
       if( aMetaSlide.nIsBackgroundVisible )\n\
       {\n\
           setNSAttribute( 'xlink', this.backgroundElement, 'href', '#' + aMetaSlide.masterPage.backgroundId );\n\
           this.backgroundElement.setAttribute( 'visibility', 'inherit' );\n\
       }\n\
       else\n\
       {\n\
           this.backgroundElement.setAttribute( 'visibility', 'hidden' );\n\
       }\n\
       if( aMetaSlide.nAreMasterObjectsVisible )\n\
       {\n\
           setNSAttribute( 'xlink',  this.backgroundObjectsElement, 'href', '#' + aMetaSlide.masterPage.backgroundObjectsId );\n\
           this.backgroundObjectsElement.setAttribute( 'visibility', 'inherit' );\n\
       }\n\
       else\n\
       {\n\
           this.backgroundObjectsElement.setAttribute( 'visibility', 'hidden' );\n\
       }\n\
       this.slideIndex = nIndex;\n\
    };\n\
\n\
    Thumbnail.prototype.clear = function( nIndex )\n\
    {\n\
       setNSAttribute( 'xlink', this.slideElement, 'href', '' );\n\
       setNSAttribute( 'xlink', this.backgroundElement, 'href', '' );\n\
       setNSAttribute( 'xlink', this.backgroundObjectsElement, 'href', '' );\n\
    };\n\
\n\
    /* private methods */\n\
    Thumbnail.prototype.createThumbnailElement = function()\n\
    {\n\
        var aThumbnailElement = document.createElementNS( NSS['svg'], 'g' );\n\
        aThumbnailElement.setAttribute( 'id', this.thumbnailId );\n\
        aThumbnailElement.setAttribute( 'display', 'inherit' );\n\
\n\
        var aMouseAreaElement = document.createElementNS( NSS['svg'], 'use' );\n\
        setNSAttribute( 'xlink', aMouseAreaElement, 'href', '#' + this.container.thumbnailMouseAreaTemplateId );\n\
        aMouseAreaElement.setAttribute( 'class', 'MouseArea' );\n\
        aThumbnailElement.appendChild( aMouseAreaElement );\n\
\n\
        var aBackgroundElement = document.createElementNS( NSS['svg'], 'use' );\n\
        setNSAttribute( 'xlink', aBackgroundElement, 'href', '' );\n\
        aBackgroundElement.setAttribute( 'visibility', 'inherit');\n\
        aBackgroundElement.setAttribute( 'class', 'Background' );\n\
        aThumbnailElement.appendChild( aBackgroundElement );\n\
\n\
        var aBackgroundObjectsElement = document.createElementNS( NSS['svg'], 'use' );\n\
        setNSAttribute( 'xlink', aBackgroundObjectsElement, 'href', '' );\n\
        aBackgroundObjectsElement.setAttribute( 'visibility', 'inherit');\n\
        aBackgroundObjectsElement.setAttribute( 'class', 'BackgroundObjects' );\n\
        aThumbnailElement.appendChild( aBackgroundObjectsElement );\n\
\n\
        var aSlideElement = document.createElementNS( NSS['svg'], 'use' );\n\
        setNSAttribute( 'xlink', aSlideElement, 'href', '' );\n\
        aSlideElement.setAttribute( 'class', 'Slide' );\n\
        aThumbnailElement.appendChild( aSlideElement );\n\
\n\
        var aBorderElement = document.createElementNS( NSS['svg'], 'use' );\n\
        setNSAttribute( 'xlink', aBorderElement, 'href', '#' + this.container.thumbnailBorderTemplateId );\n\
        aBorderElement.setAttribute( 'stroke', this.sNormalBorderColor );\n\
        aBorderElement.setAttribute( 'class', 'Border' );\n\
        aThumbnailElement.appendChild( aBorderElement );\n\
\n\
        this.container.pageElement.appendChild( aThumbnailElement );\n\
        return( document.getElementById( this.thumbnailId ) );\n\
    };\n\
\n\
    Thumbnail.prototype.computeTransform = function()\n\
    {\n\
        var nXSpacing = this.container.xSpacing;\n\
        var nYSpacing = this.container.ySpacing;\n\
\n\
        var nXOffset = nXSpacing + ( this.width + nXSpacing ) * this.column;\n\
        var nYOffset = nYSpacing + ( this.height + nYSpacing ) * this.row;\n\
\n\
        this.aTransformSet[0] = 'translate(' + nXOffset + ' ' + nYOffset + ')';\n\
\n\
        sTransform = this.aTransformSet.join( ' ' );\n\
\n\
        return sTransform;\n\
    };\n\
\n\
";

static const char aSVGScript6[] =
"\
    Thumbnail.prototype.onMouseOver = function()\n\
    {\n\
        if( ( currentMode == INDEX_MODE ) && ( this.container.curThumbnailIndex !=  this.index ) )\n\
        {\n\
            this.container.setSelection( this.index );\n\
        }\n\
    };\n\
\n\
\n\
\n\
\n\
    // ------------------------------------------------------------------------------------------ //\n\
    /** Initialization function.\n\
     *  The whole presentation is set-up in this function.\n\
     */\n\
    function init()\n\
    {\n\
\n\
        var VIEWBOX = ROOT_NODE.getAttribute('viewBox');\n\
\n\
        if( VIEWBOX )\n\
        {\n\
            WIDTH = ROOT_NODE.viewBox.animVal.width;\n\
            HEIGHT = ROOT_NODE.viewBox.animVal.height;\n\
        }\n\
\n\
        var aMetaDocElem = document.getElementById( aOOOElemMetaSlides );\n\
        assert( aMetaDocElem, 'init: meta document element not found' );\n\
        theMetaDoc =  new MetaDocument( aMetaDocElem );\n\
        theSlideIndexPage = new SlideIndexPage();\n\
\n\
\n\
    }\n\
\n\
    function presentationEngineStop()\n\
    {\n\
        alert( 'We are sorry! An unexpected error occurred.\\nThe presentation engine will be stopped' );\n\
        document.onkeydown = null;\n\
        document.onkeypress = null;\n\
        document.onclick = null;\n\
        window.onmousewheel = null;\n\
    }\n\
\n\
    function assert( condition, message )\n\
    {\n\
       if (!condition)\n\
       {\n\
           presentationEngineStop();\n\
           if (typeof console == 'object')\n\
               console.trace();\n\
           throw new Error( message );\n\
       }\n\
    }\n\
\n\
    /** Event handler for key press.\n\
     *\n\
     *  @param aEvt the event\n\
     */\n\
    function onKeyDown( aEvt )\n\
    {\n\
        if ( !aEvt )\n\
            aEvt = window.event;\n\
\n\
        code = aEvt.keyCode || aEvt.charCode;\n\
\n\
        if ( !processingEffect && keyCodeDictionary[currentMode] && keyCodeDictionary[currentMode][code] )\n\
            return keyCodeDictionary[currentMode][code]();\n\
        else\n\
            document.onkeypress = onKeyPress;\n\
    }\n\
    // Set event handler for key down.\n\
    document.onkeydown = onKeyDown;\n\
\n\
    /** Event handler for key press.\n\
     *\n\
     *  @param aEvt the event\n\
     */\n\
    function onKeyPress( aEvt )\n\
    {\n\
        document.onkeypress = null;\n\
\n\
        if ( !aEvt )\n\
            aEvt = window.event;\n\
\n\
        str = String.fromCharCode( aEvt.keyCode || aEvt.charCode );\n\
\n\
        if ( !processingEffect && charCodeDictionary[currentMode] && charCodeDictionary[currentMode][str] )\n\
            return charCodeDictionary[currentMode][str]();\n\
    }\n\
\n\
    /** Function to supply the default key code dictionary.\n\
     *\n\
     * @returns default key code dictionary\n\
     */\n\
    function getDefaultKeyCodeDictionary()\n\
    {\n\
        var keyCodeDict = new Object();\n\
\n\
        keyCodeDict[SLIDE_MODE] = new Object();\n\
        keyCodeDict[INDEX_MODE] = new Object();\n\
\n\
        keyCodeDict[SLIDE_MODE][LEFT_KEY] = function() { return dispatchEffects(-1); };\n\
        keyCodeDict[SLIDE_MODE][RIGHT_KEY] = function() { return dispatchEffects(1); };\n\
        keyCodeDict[SLIDE_MODE][UP_KEY] = function() { return skipEffects(-1); };\n\
        keyCodeDict[SLIDE_MODE][DOWN_KEY] = function() { return skipEffects(1); };\n\
        keyCodeDict[SLIDE_MODE][PAGE_UP_KEY] = function() { return dispatchEffects(-1); };\n\
        keyCodeDict[SLIDE_MODE][PAGE_DOWN_KEY] = function() { return dispatchEffects(1); };\n\
        keyCodeDict[SLIDE_MODE][HOME_KEY] = function() { return slideSetActiveSlide(0); };\n\
        keyCodeDict[SLIDE_MODE][END_KEY] = function() { return slideSetActiveSlide(theMetaDoc.nNumberOfSlides - 1); };\n\
        keyCodeDict[SLIDE_MODE][SPACE_KEY] = function() { return dispatchEffects(1); };\n\
\n\
        keyCodeDict[INDEX_MODE][LEFT_KEY] = function() { return indexSetPageSlide( theSlideIndexPage.selectedSlideIndex - 1 ); };\n\
        keyCodeDict[INDEX_MODE][RIGHT_KEY] = function() { return indexSetPageSlide( theSlideIndexPage.selectedSlideIndex + 1 ); };\n\
        keyCodeDict[INDEX_MODE][UP_KEY] = function() { return indexSetPageSlide( theSlideIndexPage.selectedSlideIndex - theSlideIndexPage.indexColumns ); };\n\
        keyCodeDict[INDEX_MODE][DOWN_KEY] = function() { return indexSetPageSlide( theSlideIndexPage.selectedSlideIndex + theSlideIndexPage.indexColumns ); };\n\
        keyCodeDict[INDEX_MODE][PAGE_UP_KEY] = function() { return indexSetPageSlide( theSlideIndexPage.selectedSlideIndex - theSlideIndexPage.getTotalThumbnails() ); };\n\
        keyCodeDict[INDEX_MODE][PAGE_DOWN_KEY] = function() { return indexSetPageSlide( theSlideIndexPage.selectedSlideIndex + theSlideIndexPage.getTotalThumbnails() ); };\n\
        keyCodeDict[INDEX_MODE][HOME_KEY] = function() { return indexSetPageSlide( 0 ); };\n\
        keyCodeDict[INDEX_MODE][END_KEY] = function() { return indexSetPageSlide( theMetaDoc.nNumberOfSlides - 1 ); };\n\
        keyCodeDict[INDEX_MODE][ENTER_KEY] = function() { return toggleSlideIndex(); };\n\
        keyCodeDict[INDEX_MODE][SPACE_KEY] = function() { return toggleSlideIndex(); };\n\
        keyCodeDict[INDEX_MODE][ESCAPE_KEY] = function() { return abandonIndexMode(); };\n\
\n\
        return keyCodeDict;\n\
    }\n\
\n\
    /** Function to supply the default char code dictionary.\n\
     *\n\
     * @returns default char code dictionary\n\
     */\n\
    function getDefaultCharCodeDictionary()\n\
    {\n\
        var charCodeDict = new Object();\n\
\n\
        charCodeDict[SLIDE_MODE] = new Object();\n\
        charCodeDict[INDEX_MODE] = new Object();\n\
\n\
        charCodeDict[SLIDE_MODE]['i'] = function () { return toggleSlideIndex(); };\n\
\n\
        charCodeDict[INDEX_MODE]['i'] = function () { return toggleSlideIndex(); };\n\
        charCodeDict[INDEX_MODE]['-'] = function () { return theSlideIndexPage.decreaseNumberOfColumns(); };\n\
        charCodeDict[INDEX_MODE]['='] = function () { return theSlideIndexPage.increaseNumberOfColumns(); };\n\
        charCodeDict[INDEX_MODE]['+'] = function () { return theSlideIndexPage.increaseNumberOfColumns(); };\n\
        charCodeDict[INDEX_MODE]['0'] = function () { return theSlideIndexPage.resetNumberOfColumns(); };\n\
\n\
        return charCodeDict;\n\
    }\n\
\n\
\n\
    function slideOnMouseDown( aEvt )\n\
    {\n\
        if (!aEvt)\n\
            aEvt = window.event;\n\
\n\
        var nOffset = 0;\n\
\n\
        if( aEvt.button == 0 )\n\
            nOffset = 1;\n\
        else if( aEvt.button == 2 )\n\
            nOffset = -1;\n\
\n\
        if( 0 != nOffset )\n\
            switchSlide( nOffset );\n\
    }\n\
\n\
    /** Event handler for mouse wheel events in slide mode.\n\
     *  based on http://adomas.org/javascript-mouse-wheel/\n\
     *\n\
     *  @param aEvt the event\n\
     */\n\
    function slideOnMouseWheel(aEvt)\n\
    {\n\
        var delta = 0;\n\
\n\
        if (!aEvt)\n\
            aEvt = window.event;\n\
\n\
        if (aEvt.wheelDelta)\n\
        { // IE Opera\n\
            delta = aEvt.wheelDelta/120;\n\
        }\n\
        else if (aEvt.detail)\n\
        { // MOZ\n\
            delta = -aEvt.detail/3;\n\
        }\n\
\n\
        if (delta > 0)\n\
            skipEffects(-1);\n\
        else if (delta < 0)\n\
            skipEffects(1);\n\
\n\
        if (aEvt.preventDefault)\n\
            aEvt.preventDefault();\n\
\n\
        aEvt.returnValue = false;\n\
    }\n\
\n\
    // Mozilla\n\
    if( window.addEventListener )\n\
    {\n\
";

static const char aSVGScript7[] =
"\
        window.addEventListener( 'DOMMouseScroll', function( aEvt ) { return mouseHandlerDispatch( aEvt, MOUSE_WHEEL ); }, false );\n\
    }\n\
\n\
    // Opera Safari OK - may not work in IE\n\
    window.onmousewheel = function( aEvt ) { return mouseHandlerDispatch( aEvt, MOUSE_WHEEL ); };\n\
\n\
    /** Function to handle all mouse events.\n\
    *\n\
    *  @param  aEvt    event\n\
    *  @param  anAction  type of event (e.g. mouse up, mouse wheel)\n\
    */\n\
    function mouseHandlerDispatch( aEvt, anAction )\n\
    {\n\
        if( !aEvt )\n\
            aEvt = window.event;\n\
\n\
        var retVal = true;\n\
\n\
        if ( mouseHandlerDictionary[currentMode] && mouseHandlerDictionary[currentMode][anAction] )\n\
        {\n\
            var subRetVal = mouseHandlerDictionary[currentMode][anAction]( aEvt );\n\
\n\
            if( subRetVal != null && subRetVal != undefined )\n\
                retVal = subRetVal;\n\
        }\n\
\n\
        if( aEvt.preventDefault && !retVal )\n\
            aEvt.preventDefault();\n\
\n\
        aEvt.returnValue = retVal;\n\
\n\
        return retVal;\n\
    }\n\
\n\
    // Set mouse event handler.\n\
    document.onmousedown = function( aEvt ) { return mouseHandlerDispatch( aEvt, MOUSE_DOWN ); };\n\
    //document.onmousemove = function( aEvt ) { return mouseHandlerDispatch( aEvt, MOUSE_MOVE ); };\n\
\n\
    /** Function to supply the default mouse handler dictionary.\n\
    *\n\
    * @returns default mouse handler dictionary\n\
    */\n\
    function getDefaultMouseHandlerDictionary()\n\
    {\n\
        var mouseHandlerDict = new Object();\n\
\n\
        mouseHandlerDict[SLIDE_MODE] = new Object();\n\
        mouseHandlerDict[INDEX_MODE] = new Object();\n\
\n\
\n\
        mouseHandlerDict[SLIDE_MODE][MOUSE_DOWN] = function( aEvt ) { return slideOnMouseDown( aEvt ); };\n\
        mouseHandlerDict[SLIDE_MODE][MOUSE_WHEEL] = function( aEvt ) { return slideOnMouseWheel( aEvt ); };\n\
\n\
        mouseHandlerDict[INDEX_MODE][MOUSE_DOWN] = function( aEvt ) { return toggleSlideIndex(); };\n\
\n\
        return mouseHandlerDict;\n\
    }\n\
\n\
    /** Function to dispatch the next effect, if there is none left, change the slide.\n\
     *\n\
     *  @param dir direction of the change (1 = forwards, -1 = backwards)\n\
     */\n\
    function dispatchEffects(dir)\n\
    {\n\
        // TODO to be implemented\n\
        switchSlide(dir);\n\
    }\n\
\n\
    /** Function to skip effects and directly either put the slide into start or end state or change slides.\n\
     *\n\
     *  @param dir direction of the change (1 = forwards, -1 = backwards)\n\
     */\n\
    function skipEffects(dir)\n\
    {\n\
        // TODO to be implemented\n\
        switchSlide(dir);\n\
    }\n\
\n\
    /** Function to change between slides.\n\
     *\n\
     *  @param nOffset direction (1 = forwards, -1 = backwards)\n\
     */\n\
    function switchSlide( nOffset )\n\
    {\n\
        var nNextSlide = nCurSlide + nOffset;\n\
        slideSetActiveSlide( nNextSlide );\n\
    }\n\
\n\
    /** Function to display the index sheet.\n\
    *\n\
    *  @param offsetNumber offset number\n\
    */\n\
   function displayIndex( offsetNumber )\n\
   {\n\
       var aMetaSlideSet = theMetaDoc.aMetaSlideSet;\n\
       offsetNumber = getSafeIndex( offsetNumber, 0, aMetaSlideSet.length - 1 );\n\
\n\
       var nTotalThumbnails = theSlideIndexPage.getTotalThumbnails();\n\
       var nEnd = Math.min( offsetNumber + nTotalThumbnails, aMetaSlideSet.length);\n\
\n\
       var aThumbnailSet = theSlideIndexPage.aThumbnailSet;\n\
       var j = 0;\n\
       for( var i = offsetNumber; i < nEnd; ++i, ++j )\n\
       {\n\
           aThumbnailSet[j].update( i );\n\
           aThumbnailSet[j].show();\n\
       }\n\
       for( ; j < nTotalThumbnails; ++j )\n\
       {\n\
           aThumbnailSet[j].hide();\n\
       }\n\
\n\
       //do we need to save the current offset?\n\
       if (INDEX_OFFSET != offsetNumber)\n\
           INDEX_OFFSET = offsetNumber;\n\
   }\n\
\n\
    /** Function to set the active slide in the slide view.\n\
     *\n\
     *  @param nNewSlide index of the active slide\n\
     */\n\
    function slideSetActiveSlide( nNewSlide )\n\
    {\n\
        var aMetaDoc = theMetaDoc;\n\
        var nSlides = aMetaDoc.nNumberOfSlides;\n\
        if( nNewSlide < 0 && nSlides > 0 )\n\
            nNewSlide = nSlides - 1;\n\
        else if( nNewSlide >= nSlides )\n\
            nNewSlide = 0;\n\
\n\
        if( nNewSlide == nCurSlide ) return;\n\
        var nOldSlide = nCurSlide;\n\
        nCurSlide = nNewSlide;\n\
\n\
        var oldMetaSlide = aMetaDoc.aMetaSlideSet[nOldSlide];\n\
        var newMetaSlide = aMetaDoc.aMetaSlideSet[nNewSlide];\n\
\n\
        oldMetaSlide.hide();\n\
        newMetaSlide.show();\n\
    }\n\
\n\
    /** Function to set the page and active slide in index view.\n\
    *\n\
    *  @param nIndex index of the active slide\n\
    *\n\
    *  NOTE: To force a redraw,\n\
    *  set INDEX_OFFSET to -1 before calling indexSetPageSlide().\n\
    *\n\
    *  This is necessary for zooming (otherwise the index might not\n\
    *  get redrawn) and when switching to index mode.\n\
    *\n\
    *  INDEX_OFFSET = -1\n\
    *  indexSetPageSlide(activeSlide);\n\
    */\n\
   function indexSetPageSlide( nIndex )\n\
   {\n\
       var aMetaSlideSet = theMetaDoc.aMetaSlideSet;\n\
       nIndex = getSafeIndex( nIndex, 0, aMetaSlideSet.length - 1 );\n\
\n\
       //calculate the offset\n\
       var nSelectedThumbnailIndex = nIndex % theSlideIndexPage.getTotalThumbnails();\n\
       var offset = nIndex - nSelectedThumbnailIndex;\n\
\n\
       if( offset < 0 )\n\
           offset = 0;\n\
\n\
       //if different from kept offset, then record and change the page\n\
       if( offset != INDEX_OFFSET )\n\
       {\n\
           INDEX_OFFSET = offset;\n\
           displayIndex( INDEX_OFFSET );\n\
       }\n\
\n\
       //set the selected thumbnail and the current slide\n\
       theSlideIndexPage.setSelection( nSelectedThumbnailIndex );\n\
   }\n\
\n\
    /** Function to toggle between index and slide mode.\n\
    */\n\
    function toggleSlideIndex()\n\
    {\n\
        var suspendHandle = ROOT_NODE.suspendRedraw(500);\n\
        var aMetaSlideSet = theMetaDoc.aMetaSlideSet;\n\
\n\
        if (currentMode == SLIDE_MODE)\n\
        {\n\
            aMetaSlideSet[nCurSlide].hide();\n\
            for( var counter = 0; counter < aMetaSlideSet.length; ++counter )\n\
            {\n\
                checkElemAndSetAttribute( aMetaSlideSet[counter].slideElement, 'visibility', 'inherit' );\n\
                aMetaSlideSet[counter].masterPage.setVisibilityBackground( INHERIT );\n\
                aMetaSlideSet[counter].masterPage.setVisibility( INHERIT );\n\
            }\n\
            INDEX_OFFSET = -1;\n\
            indexSetPageSlide( nCurSlide );\n\
            theSlideIndexPage.show();\n\
            currentMode = INDEX_MODE;\n\
        }\n\
        else if (currentMode == INDEX_MODE)\n\
        {\n\
";

static const char aSVGScript8[] =
"\
            theSlideIndexPage.hide();\n\
            nCurSlide = theSlideIndexPage.selectedSlideIndex;\n\
\n\
            for( var counter = 0; counter < aMetaSlideSet.length; ++counter )\n\
            {\n\
                var aMetaSlide = aMetaSlideSet[counter];\n\
                aMetaSlide.slideElement.setAttribute( 'visibility', 'hidden' );\n\
                aMetaSlide.masterPage.setVisibilityBackground( HIDDEN );\n\
                aMetaSlide.masterPage.setVisibility( HIDDEN );\n\
            }\n\
\n\
            aMetaSlideSet[nCurSlide].show();\n\
            //activeEffect = 0;\n\
\n\
            currentMode = SLIDE_MODE;\n\
            //setSlideToState(nCurSlide, STATE_START);\n\
        }\n\
\n\
        ROOT_NODE.unsuspendRedraw(suspendHandle);\n\
        ROOT_NODE.forceRedraw();\n\
    }\n\
\n\
    /** Function that exit from the index mode without changing the shown slide\n\
     *\n\
     */\n\
    function abandonIndexMode()\n\
    {\n\
        theSlideIndexPage.selectedSlideIndex = nCurSlide;\n\
        toggleSlideIndex();\n\
    }\n\
]]>";



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
