
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



#define N_SVGSCRIPT_FRAGMENTS 37

static const char aSVGScript0[] =
"<![CDATA[\n\
\n\
\n\
    /** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \n\
     * - Presentation Engine v5.0 -\n\
     * \n\
     ** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \n\
     *  This program is free software: you can redistribute it and/or modify \n\
     *  it under the terms of the GNU General Public License as published by \n\
     *  the Free Software Foundation, either version 3 of the License, or    \n\
     *  (at your option) any later version.                                  \n\
     *                                                                         \n\
     *  This program is distributed in the hope that it will be useful,      \n\
     *  but WITHOUT ANY WARRANTY; without even the implied warranty of       \n\
     *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        \n\
     *  GNU General Public License for more details.                         \n\
     *                                                                           \n\
     *  You should have received a copy of the GNU General Public License    \n\
     *  along with this program.  If not, see http://www.gnu.org/licenses/.  \n\
     * \n\
     ** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * \n\
     *  Some parts of this script are based on the JessyInk project:\n\
     *  http://code.google.com/p/jessyink/\n\
     * \n\
     ** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */\n\
\n\
\n\
\n\
    window.onload = init;\n\
\n\
\n\
    var aOOOElemMetaSlides = 'ooo:meta_slides';\n\
    var aOOOElemMetaSlide = 'ooo:meta_slide';\n\
    var aOOOElemTextField = 'ooo:text_field';\n\
\n\
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
    var aSlideNumberClassName = 'Slide_Number';\n\
    var aDateTimeClassName = 'Date/Time';\n\
    var aFooterClassName = 'Footer';\n\
    var aHeaderClassName = 'Header';\n\
\n\
    var NSS = new Object();\n\
    NSS['svg']='http://www.w3.org/2000/svg';\n\
    NSS['rdf']='http://www.w3.org/1999/02/22-rdf-syntax-ns#';\n\
    NSS['xlink']='http://www.w3.org/1999/xlink';\n\
    NSS['xml']='http://www.w3.org/XML/1998/namespace';\n\
    NSS['ooo'] = 'http://xml.openoffice.org/svg/export';\n\
\n\
    var SLIDE_MODE = 1;\n\
    var INDEX_MODE = 2;\n\
\n\
    var MOUSE_UP = 1;\n\
    var MOUSE_DOWN = 2;\n\
    var MOUSE_MOVE = 3;\n\
    var MOUSE_WHEEL = 4;\n\
\n\
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
    var HIDDEN = 0;\n\
    var VISIBLE = 1;\n\
    var INHERIT = 2;\n\
    var aVisibilityAttributeValue = [ 'hidden', 'visible', 'inherit' ];\n\
    var aVisibilityValue = { 'hidden' : HIDDEN, 'visible' : VISIBLE, 'inherit' : INHERIT };\n\
\n\
    var ROOT_NODE = document.getElementsByTagNameNS( NSS['svg'], 'svg' )[0];\n\
    var WIDTH = 0;\n\
    var HEIGHT = 0;\n\
    var INDEX_COLUMNS_DEFAULT = 3;\n\
    var INDEX_OFFSET = 0;\n\
\n\
    var theMetaDoc;\n\
    var theSlideIndexPage;\n\
    var currentMode = SLIDE_MODE;\n\
    var processingEffect = false;\n\
    var nCurSlide = 0;\n\
\n\
    var charCodeDictionary = getDefaultCharCodeDictionary();\n\
    var keyCodeDictionary = getDefaultKeyCodeDictionary();\n\
\n\
    var mouseHandlerDictionary = getDefaultMouseHandlerDictionary();\n\
\n\
\n\
    function object( aObject )\n\
    {\n\
        var F = function() {};\n\
        F.prototype = aObject;\n\
        return new F();\n\
    }\n\
\n\
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
        if (aSuperType != Object && sp.constructor == OP.constructor) {\n\
            sp.constructor = aSuperType;\n\
        }\n\
\n\
        return aSubType;\n\
    }\n\
\n\
\n\
    function instantiate( TemplateClass, BaseType )\n\
    {\n\
        if( !TemplateClass.instanceSet )\n\
            TemplateClass.instanceSet = new Array();\n\
\n\
        var nSize = TemplateClass.instanceSet.length;\n\
\n\
        for( var i = 0; i < nSize; ++i )\n\
        {\n\
            if( TemplateClass.instanceSet[i].base === BaseType )\n\
                return TemplateClass.instanceSet[i].instance;\n\
        }\n\
\n\
        TemplateClass.instanceSet[ nSize ] = new Object();\n\
        TemplateClass.instanceSet[ nSize ].base = BaseType;\n\
        TemplateClass.instanceSet[ nSize ].instance = TemplateClass( BaseType );\n\
\n\
        return TemplateClass.instanceSet[ nSize ].instance;\n\
    };\n\
\n\
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
";

static const char aSVGScript1[] =
"\
    {\n\
        return getNSAttribute( 'ooo', aElem, sAttrName );\n\
    }\n\
\n\
    function setNSAttribute( sNSPrefix, aElem, sAttrName, aValue )\n\
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
\n\
\n\
    function DebugPrinter()\n\
    {\n\
        this.bEnabled = false;\n\
    }\n\
\n\
\n\
    DebugPrinter.prototype.on = function()\n\
    {\n\
        this.bEnabled = true;\n\
    };\n\
\n\
    DebugPrinter.prototype.off = function()\n\
    {\n\
        this.bEnabled = false;\n\
    };\n\
\n\
    DebugPrinter.prototype.isEnabled = function()\n\
    {\n\
        return this.bEnabled;\n\
    };\n\
\n\
    DebugPrinter.prototype.print = function( sMessage, nTime )\n\
    {\n\
        if( this.isEnabled() )\n\
        {\n\
            sInfo = 'DBG: ' + sMessage;\n\
            if( nTime )\n\
                sInfo += ' (at: ' + String( nTime / 1000 ) + 's)';\n\
            log( sInfo );\n\
        }\n\
    };\n\
\n\
\n\
    var NAVDBG = new DebugPrinter();\n\
    NAVDBG.off();\n\
\n\
    var ANIMDBG = new DebugPrinter();\n\
    ANIMDBG.off();\n\
\n\
    var aRegisterEventDebugPrinter = new DebugPrinter();\n\
    aRegisterEventDebugPrinter.off();\n\
\n\
    var aTimerEventQueueDebugPrinter = new DebugPrinter();\n\
    aTimerEventQueueDebugPrinter.off();\n\
\n\
    var aEventMultiplexerDebugPrinter = new DebugPrinter();\n\
    aEventMultiplexerDebugPrinter.off();\n\
\n\
    var aNextEffectEventArrayDebugPrinter = new DebugPrinter();\n\
    aNextEffectEventArrayDebugPrinter.off();\n\
\n\
    var aActivityQueueDebugPrinter = new DebugPrinter();\n\
    aActivityQueueDebugPrinter.off();\n\
\n\
    var aAnimatedElementDebugPrinter = new DebugPrinter();\n\
    aAnimatedElementDebugPrinter.off();\n\
\n\
\n\
\n\
\n\
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
        this.aSlideAnimationsMap = new Object();\n\
        this.initSlideAnimationsMap();\n\
\n\
\n\
        for( var i = 0; i < this.nNumberOfSlides; ++i )\n\
        {\n\
            var sMetaSlideId = aOOOElemMetaSlide + '_' + i;\n\
            this.aMetaSlideSet.push( new MetaSlide( sMetaSlideId, this ) );\n\
        }\n\
        assert( this.aMetaSlideSet.length == this.nNumberOfSlides,\n\
                'MetaDocument: aMetaSlideSet.length != nNumberOfSlides.' );\n\
        this.aMetaSlideSet[ this.startSlideNumber ].show();\n\
\n\
";

static const char aSVGScript2[] =
"\
    }\n\
\n\
    MetaDocument.prototype.initPlaceholderShapes = function()\n\
    {\n\
        this.aMetaSlideSet[0].initPlaceholderShapes();\n\
    };\n\
\n\
    MetaDocument.prototype.initSlideAnimationsMap = function()\n\
    {\n\
        var aAnimationsSection = document.getElementById( 'presentation-animations' );\n\
        if( aAnimationsSection )\n\
        {\n\
            var aAnimationsDefSet = aAnimationsSection.getElementsByTagName( 'defs' );\n\
\n\
            for( var i = 0; i < aAnimationsDefSet.length; ++i )\n\
            {\n\
                var sSlideId = aAnimationsDefSet[i].getAttributeNS( NSS['ooo'], aOOOAttrSlide );\n\
                var aChildSet = getElementChildren( aAnimationsDefSet[i] );\n\
                if( sSlideId && ( aChildSet.length == 1 ) )\n\
                {\n\
                    this.aSlideAnimationsMap[ sSlideId ] = aChildSet[0];\n\
                }\n\
            }\n\
        }\n\
    };\n\
\n\
\n\
\n\
    function MetaSlide( sMetaSlideId, aMetaDoc )\n\
    {\n\
        this.theDocument = document;\n\
        this.id = sMetaSlideId;\n\
        this.theMetaDoc = aMetaDoc;\n\
        this.element = this.theDocument.getElementById( this.id );\n\
        assert( this.element, 'MetaSlide: meta_slide element <' + this.id + '> not found.' );\n\
        this.slideId = this.element.getAttributeNS( NSS['ooo'], aOOOAttrSlide );\n\
        this.slideElement = this.theDocument.getElementById( this.slideId );\n\
        assert( this.slideElement, 'MetaSlide: slide element <' + this.slideId + '> not found.' );\n\
        this.masterPage = this.initMasterPage();\n\
        this.nAreMasterObjectsVisible     = this.initVisibilityProperty( aOOOAttrMasterObjectsVisibility,  VISIBLE );\n\
        this.nIsBackgroundVisible         = this.initVisibilityProperty( aOOOAttrBackgroundVisibility,     VISIBLE );\n\
        this.nIsPageNumberVisible         = this.initVisibilityProperty( aOOOAttrPageNumberVisibility,     HIDDEN );\n\
        this.nIsDateTimeVisible           = this.initVisibilityProperty( aOOOAttrDateTimeVisibility,       VISIBLE );\n\
        this.nIsFooterVisible             = this.initVisibilityProperty( aOOOAttrFooterVisibility,         VISIBLE );\n\
        this.nIsHeaderVisible             = this.initVisibilityProperty( aOOOAttrHeaderVisibility,         VISIBLE );\n\
        this.aMPTextFieldSet = new Object();\n\
        this.aMPTextFieldSet[aSlideNumberClassName]   = this.initSlideNumberField();\n\
        this.aMPTextFieldSet[aDateTimeClassName]      = this.initDateTimeField( aOOOAttrDateTimeField );\n\
        this.aMPTextFieldSet[aFooterClassName]        = this.initFixedTextField( aOOOAttrFooterField );\n\
        this.aMPTextFieldSet[aHeaderClassName]        = this.initFixedTextField( aOOOAttrHeaderField );\n\
\n\
        this.aSlideAnimationsHandler = new SlideAnimations( aSlideShow.getContext() );\n\
        this.aSlideAnimationsHandler.importAnimations( this.getSlideAnimationsRoot() );\n\
        this.aSlideAnimationsHandler.parseElements();\n\
        if( false && this.aSlideAnimationsHandler.aRootNode )\n\
            log( this.aSlideAnimationsHandler.aRootNode.info( true ) );\n\
    }\n\
\n\
    MetaSlide.prototype =\n\
    {\n\
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
        },\n\
\n\
        getSlideAnimationsRoot : function()\n\
        {\n\
            return this.theMetaDoc.aSlideAnimationsMap[ this.slideId ];\n\
        }\n\
\n\
    };\n\
\n\
";

static const char aSVGScript3[] =
"\
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
    MasterPage.prototype =\n\
    {\n\
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
        initPlaceholderShapes : function()\n\
        {\n\
            this.aPlaceholderShapeSet[ aSlideNumberClassName ] = new PlaceholderShape( this, aSlideNumberClassName );\n\
            this.aPlaceholderShapeSet[ aDateTimeClassName ] = new PlaceholderShape( this, aDateTimeClassName );\n\
            this.aPlaceholderShapeSet[ aFooterClassName ] = new PlaceholderShape( this, aFooterClassName );\n\
            this.aPlaceholderShapeSet[ aHeaderClassName ] = new PlaceholderShape( this, aHeaderClassName );\n\
        }\n\
    };\n\
\n\
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
    PlaceholderShape.prototype.setTextContent = function( sText )\n\
    {\n\
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
\n\
    function TextField( aTextFieldElem )\n\
    {\n\
        this.bIsUpdated = false;\n\
    }\n\
\n\
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
    function VariableDateTimeField( aTextFieldElem )\n\
    {\n\
        VariableDateTimeField.superclass.constructor.call( this, aTextFieldElem );\n\
        this.dateTimeFormat = getOOOAttribute( aTextFieldElem, aOOOAttrDateTimeFormat );\n\
    }\n\
    extend( VariableDateTimeField, TextField );\n\
\n\
    VariableDateTimeField.prototype.update = function( aPlaceholderShape )\n\
    {\n\
        var sText = this.createDateTimeText( this.dateTimeFormat );\n\
        aPlaceholderShape.setTextContent( sText );\n\
    };\n\
\n\
";

static const char aSVGScript4[] =
"\
    VariableDateTimeField.prototype.createDateTimeText = function( sDateTimeFormat )\n\
    {\n\
        var aDate = Date();\n\
        var sDate = aDate.toLocaleString();\n\
        return sDate;\n\
    };\n\
\n\
    function SlideNumberField( sPageNumberingType )\n\
    {\n\
        SlideNumberField.superclass.constructor.call( this, null );\n\
        this.pageNumberingType = sPageNumberingType;\n\
    }\n\
    extend( SlideNumberField, TextField );\n\
\n\
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
        return String( nSlideNumber );\n\
    };\n\
\n\
\n\
\n\
\n\
    function SlideIndexPage()\n\
    {\n\
        this.pageElementId = 'slide_index';\n\
        this.pageBgColor = 'rgb(252,252,252)';\n\
        this.pageElement = this.createPageElement();\n\
        assert( this.pageElement, 'SlideIndexPage: pageElement is not valid' );\n\
        this.indexColumns = INDEX_COLUMNS_DEFAULT;\n\
        this.totalThumbnails = this.indexColumns * this.indexColumns;\n\
        this.selectedSlideIndex = nCurSlide;\n\
\n\
        this.xSpacingFactor = 600/28000;\n\
        this.ySpacingFactor = 450/21000;\n\
        this.xSpacing = WIDTH * this.xSpacingFactor;\n\
        this.ySpacing = HEIGHT * this.ySpacingFactor;\n\
        this.halfBorderWidthFactor = ( 300/28000 ) * ( this.indexColumns / 3 );\n\
        this.halfBorderWidth = WIDTH * this.halfBorderWidthFactor;\n\
        this.borderWidth = 2 * this.halfBorderWidth;\n\
        this.scaleFactor = ( 1 - ( this.indexColumns + 1 ) * this.xSpacingFactor ) /\n\
                                ( this.indexColumns * ( 1 + 2 * this.halfBorderWidthFactor ) );\n\
\n\
        this.thumbnailMouseAreaTemplateId = 'thumbnail_mouse_area';\n\
        this.thumbnailMouseAreaTemplateElement = null;\n\
        this.thumbnailBorderTemplateId = 'thumbnail_border';\n\
        this.thumbnailBorderTemplateElement = null;\n\
        this.createTemplateElements();\n\
\n\
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
        var sPageBgColor = this.pageBgColor + ';';\n\
        var aRectElement = document.createElementNS( NSS['svg'], 'rect' );\n\
        aRectElement.setAttribute( 'x', 0 );\n\
        aRectElement.setAttribute( 'y', 0 );\n\
        aRectElement.setAttribute( 'width', WIDTH );\n\
        aRectElement.setAttribute( 'height', HEIGHT );\n\
        aRectElement.setAttribute( 'style', 'stroke:none;fill:' + sPageBgColor );\n\
\n\
        aPageElement.appendChild( aRectElement );\n\
        ROOT_NODE.appendChild( aPageElement );\n\
        return( document.getElementById( this.pageElementId ) );\n\
    };\n\
\n\
    SlideIndexPage.prototype.createTemplateElements = function()\n\
    {\n\
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
        for( var i = this.totalThumbnails; i < nOldTotalThumbnails; ++i )\n\
        {\n\
            this.aThumbnailSet[i].removeElement();\n\
        };\n\
\n\
        for( var i = nOldTotalThumbnails; i < this.totalThumbnails; ++i )\n\
        {\n\
            this.aThumbnailSet[i] = new Thumbnail( this, i );\n\
        };\n\
\n\
        this.halfBorderWidthFactor = ( 300/28000 ) * ( this.indexColumns / 3 );\n\
        this.halfBorderWidth = WIDTH * this.halfBorderWidthFactor;\n\
        this.borderWidth = 2 * this.halfBorderWidth;\n\
        this.scaleFactor = ( 1 - ( this.indexColumns + 1 ) * this.xSpacingFactor ) /\n\
                                ( this.indexColumns * ( 1 + 2 * this.halfBorderWidthFactor ) );\n\
\n\
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
        for( var i = 0; i < this.totalThumbnails; ++i )\n\
        {\n\
            this.aThumbnailSet[i].updateView();\n\
        }\n\
\n\
        this.curThumbnailIndex = this.selectedSlideIndex % this.totalThumbnails;\n\
        this.aThumbnailSet[this.curThumbnailIndex].select();\n\
\n\
        INDEX_OFFSET = -1;\n\
        indexSetPageSlide( this.selectedSlideIndex );\n\
\n\
        ROOT_NODE.unsuspendRedraw( suspendHandle );\n\
        ROOT_NODE.forceRedraw();\n\
    };\n\
\n\
\n\
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
    Thumbnail.prototype.sNormalBorderColor = 'rgb(216,216,216)';\n\
    Thumbnail.prototype.sSelectionBorderColor = 'rgb(92,92,255)';\n\
\n\
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
    function init()\n\
    {\n\
        var VIEWBOX = ROOT_NODE.getAttribute('viewBox');\n\
\n\
        if( VIEWBOX )\n\
        {\n\
            WIDTH = ROOT_NODE.viewBox.animVal.width;\n\
            HEIGHT = ROOT_NODE.viewBox.animVal.height;\n\
        }\n\
\n\
";

static const char aSVGScript6[] =
"\
        var aMetaDocElem = document.getElementById( aOOOElemMetaSlides );\n\
        assert( aMetaDocElem, 'init: meta document element not found' );\n\
        aSlideShow = new SlideShow();\n\
        theMetaDoc =  new MetaDocument( aMetaDocElem );\n\
        theSlideIndexPage = new SlideIndexPage();\n\
\n\
\n\
\n\
\n\
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
    document.onkeydown = onKeyDown;\n\
\n\
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
        keyCodeDict[SLIDE_MODE][PAGE_UP_KEY] = function() { return switchSlide( -1, true ); };\n\
        keyCodeDict[SLIDE_MODE][PAGE_DOWN_KEY] = function() { return switchSlide( 1, true ); };\n\
        keyCodeDict[SLIDE_MODE][HOME_KEY] = function() { return aSlideShow.displaySlide( 0, true ); };\n\
        keyCodeDict[SLIDE_MODE][END_KEY] = function() { return aSlideShow.displaySlide( theMetaDoc.nNumberOfSlides - 1, true ); };\n\
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
            dispatchEffects( nOffset );\n\
    }\n\
\n\
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
    if( window.addEventListener )\n\
    {\n\
        window.addEventListener( 'DOMMouseScroll', function( aEvt ) { return mouseHandlerDispatch( aEvt, MOUSE_WHEEL ); }, false );\n\
    }\n\
\n\
    window.onmousewheel = function( aEvt ) { return mouseHandlerDispatch( aEvt, MOUSE_WHEEL ); };\n\
\n\
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
    document.onmousedown = function( aEvt ) { return mouseHandlerDispatch( aEvt, MOUSE_DOWN ); };\n\
\n\
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
";

static const char aSVGScript7[] =
"\
    }\n\
\n\
    function dispatchEffects(dir)\n\
    {\n\
\n\
        if( dir == 1 )\n\
        {\n\
            var bRet = aSlideShow.nextEffect();\n\
\n\
            if( !bRet )\n\
            {\n\
                switchSlide( 1 );\n\
            }\n\
        }\n\
        else\n\
        {\n\
            switchSlide( dir );\n\
        }\n\
    }\n\
\n\
    function skipEffects(dir)\n\
    {\n\
        switchSlide(dir);\n\
    }\n\
\n\
    function switchSlide( nOffset, bSkipTransition )\n\
    {\n\
        var nNextSlide = nCurSlide + nOffset;\n\
        aSlideShow.displaySlide( nNextSlide, bSkipTransition );\n\
    }\n\
\n\
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
       if (INDEX_OFFSET != offsetNumber)\n\
           INDEX_OFFSET = offsetNumber;\n\
   }\n\
\n\
    function slideSetActiveSlide( nNewSlide  )\n\
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
   function indexSetPageSlide( nIndex )\n\
   {\n\
       var aMetaSlideSet = theMetaDoc.aMetaSlideSet;\n\
       nIndex = getSafeIndex( nIndex, 0, aMetaSlideSet.length - 1 );\n\
\n\
       var nSelectedThumbnailIndex = nIndex % theSlideIndexPage.getTotalThumbnails();\n\
       var offset = nIndex - nSelectedThumbnailIndex;\n\
\n\
       if( offset < 0 )\n\
           offset = 0;\n\
\n\
       if( offset != INDEX_OFFSET )\n\
       {\n\
           INDEX_OFFSET = offset;\n\
           displayIndex( INDEX_OFFSET );\n\
       }\n\
\n\
       theSlideIndexPage.setSelection( nSelectedThumbnailIndex );\n\
   }\n\
\n\
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
            theSlideIndexPage.hide();\n\
            var nNewSlide = theSlideIndexPage.selectedSlideIndex;\n\
\n\
            for( var counter = 0; counter < aMetaSlideSet.length; ++counter )\n\
            {\n\
                var aMetaSlide = aMetaSlideSet[counter];\n\
                aMetaSlide.slideElement.setAttribute( 'visibility', 'hidden' );\n\
                aMetaSlide.masterPage.setVisibilityBackground( HIDDEN );\n\
                aMetaSlide.masterPage.setVisibility( HIDDEN );\n\
            }\n\
\n\
            aSlideShow.displaySlide( nNewSlide, true );\n\
\n\
            currentMode = SLIDE_MODE;\n\
        }\n\
\n\
        ROOT_NODE.unsuspendRedraw(suspendHandle);\n\
        ROOT_NODE.forceRedraw();\n\
    }\n\
\n\
    function abandonIndexMode()\n\
    {\n\
        theSlideIndexPage.selectedSlideIndex = nCurSlide;\n\
        toggleSlideIndex();\n\
    }\n\
\n\
\n\
\n\
\n\
\n\
\n\
function AA() {}\n\
function ANIMATIONS_ANIMATIONS_ANIMATIONS_() {}\n\
function ZZ() {}\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
    var CURR_UNIQUE_ID = 0;\n\
\n\
    function getUniqueId()\n\
    {\n\
        ++CURR_UNIQUE_ID;\n\
        return CURR_UNIQUE_ID;\n\
    }\n\
\n\
    function mem_fn( sMethodName )\n\
    {\n\
        return  function( aObject )\n\
        {\n\
            var aMethod = aObject[ sMethodName ];\n\
            if( aMethod )\n\
                aMethod.call( aObject );\n\
            else\n\
                log( 'method sMethodName not found' );\n\
        };\n\
    }\n\
\n\
    function bind( aObject, aMethod )\n\
    {\n\
        return  function()\n\
                {\n\
                    return aMethod.call( aObject, arguments[0] );\n\
                };\n\
    }\n\
\n\
    function getCurrentSystemTime()\n\
    {\n\
        return ( new Date() ).getTime();\n\
    }\n\
\n\
    function getSlideAnimationsRoot( sSlideId )\n\
    {\n\
        return theMetaDoc.aSlideAnimationsMap[ sSlideId ];\n\
    }\n\
\n\
    function getElementChildren( aElement )\n\
    {\n\
        var aChildrenArray = new Array();\n\
";

static const char aSVGScript8[] =
"\
\n\
        var nSize = aElement.childNodes.length;\n\
\n\
        for( var i = 0; i < nSize; ++i )\n\
        {\n\
            if( aElement.childNodes[i].nodeType == 1 )\n\
                aChildrenArray.push( aElement.childNodes[i] );\n\
        }\n\
\n\
        return aChildrenArray;\n\
    }\n\
\n\
    function removeWhiteSpaces( str )\n\
    {\n\
        if( !str )\n\
            return '';\n\
\n\
        var re = / */;\n\
        var aSplittedString = str.split( re );\n\
        return aSplittedString.join('');\n\
    }\n\
\n\
    function clamp( nValue, nMinimum, nMaximum )\n\
    {\n\
        if( nValue < nMinimum )\n\
        {\n\
            return nMinimum;\n\
        }\n\
        else if( nValue > nMaximum )\n\
        {\n\
            return nMaximum;\n\
        }\n\
        else\n\
        {\n\
            return nValue;\n\
        }\n\
    }\n\
\n\
    function makeMatrixString( a, b, c, d, e, f )\n\
    {\n\
        var s = 'matrix(';\n\
        s += a + ', ';\n\
        s += b + ', ';\n\
        s += c + ', ';\n\
        s += d + ', ';\n\
        s += e + ', ';\n\
        s += f + ')';\n\
\n\
        return s;\n\
    }\n\
\n\
    function matrixToString( aSVGMatrix )\n\
    {\n\
        return makeMatrixString( aSVGMatrix.a, aSVGMatrix.b, aSVGMatrix.c,\n\
                                 aSVGMatrix.d, aSVGMatrix.e, aSVGMatrix.f );\n\
    }\n\
\n\
\n\
\n\
\n\
    function numberParser( sValue )\n\
    {\n\
        if( sValue === '.' )\n\
            return undefined;\n\
        var reFloatNumber = /^[+-]?[0-9]*[.]?[0-9]*$/;\n\
\n\
        if( reFloatNumber.test( sValue ) )\n\
            return parseFloat( sValue );\n\
        else\n\
            return undefined;\n\
    }\n\
\n\
    function booleanParser( sValue )\n\
    {\n\
        sValue = sValue.toLowerCase();\n\
        if( sValue === 'true' )\n\
            return true;\n\
        else if( sValue === 'false' )\n\
            return false;\n\
        else\n\
            return undefined;\n\
    }\n\
\n\
    function colorParser( sValue )\n\
    {\n\
\n\
        function hsl( nHue, nSaturation, nLuminance )\n\
        {\n\
            return new HSLColor( nHue, nSaturation / 100, nLuminance / 100 );\n\
        }\n\
\n\
        function rgb( nRed, nGreen, nBlue )\n\
        {\n\
            return new RGBColor( nRed / 255, nGreen / 255, nBlue / 255 );\n\
        }\n\
\n\
        function prgb( nRed, nGreen, nBlue )\n\
        {\n\
            return new RGBColor( nRed / 100, nGreen / 100, nBlue / 100 );\n\
        }\n\
\n\
        var sCommaPattern = ' *[,] *';\n\
        var sIntegerPattern = '[+-]?[0-9]+';\n\
        var sHexDigitPattern = '[0-9A-Fa-f]';\n\
\n\
        var sHexColorPattern = '#(' + sHexDigitPattern + '{2})('\n\
                                    + sHexDigitPattern + '{2})('\n\
                                    + sHexDigitPattern + '{2})';\n\
\n\
        var sRGBIntegerPattern = 'rgb[(] *' + sIntegerPattern + sCommaPattern\n\
                                          + sIntegerPattern + sCommaPattern\n\
                                          + sIntegerPattern + ' *[)]';\n\
\n\
        var sRGBPercentPattern = 'rgb[(] *' + sIntegerPattern + '%' + sCommaPattern\n\
                                            + sIntegerPattern + '%' + sCommaPattern\n\
                                            + sIntegerPattern + '%' + ' *[)]';\n\
\n\
        var sHSLPercentPattern = 'hsl[(] *' + sIntegerPattern + sCommaPattern\n\
                                            + sIntegerPattern + '%' + sCommaPattern\n\
                                            + sIntegerPattern + '%' + ' *[)]';\n\
\n\
        var reHexColor = RegExp( sHexColorPattern );\n\
        var reRGBInteger = RegExp( sRGBIntegerPattern );\n\
        var reRGBPercent = RegExp( sRGBPercentPattern );\n\
        var reHSLPercent = RegExp( sHSLPercentPattern );\n\
\n\
        if( reHexColor.test( sValue ) )\n\
        {\n\
            var aRGBTriple = reHexColor.exec( sValue );\n\
\n\
            var nRed    = parseInt( aRGBTriple[1], 16 ) / 255;\n\
            var nGreen  = parseInt( aRGBTriple[2], 16 ) / 255;\n\
            var nBlue   = parseInt( aRGBTriple[3], 16 ) / 255;\n\
\n\
            return new RGBColor( nRed, nGreen, nBlue );\n\
        }\n\
        else if( reHSLPercent.test( sValue ) )\n\
        {\n\
            sValue = sValue.replace( '%', '' ).replace( '%', '' );\n\
            return eval( sValue );\n\
        }\n\
        else if( reRGBInteger.test( sValue ) )\n\
        {\n\
            return eval( sValue );\n\
        }\n\
        else if( reRGBPercent.test( sValue ) )\n\
        {\n\
            sValue = 'p' + sValue.replace( '%', '' ).replace( '%', '' ).replace( '%', '' );\n\
            return eval( sValue );\n\
        }\n\
        else\n\
        {\n\
            return null;\n\
        }\n\
    }\n\
\n\
\n\
\n\
\n\
    function RGBColor( nRed, nGreen, nBlue )\n\
    {\n\
        this.eColorSpace = COLOR_SPACE_RGB;\n\
        this.nRed = nRed;\n\
        this.nGreen = nGreen;\n\
        this.nBlue = nBlue;\n\
    }\n\
\n\
\n\
    RGBColor.prototype.clone = function()\n\
    {\n\
        return new RGBColor( this.nRed, this.nGreen, this.nBlue );\n\
    };\n\
\n\
    RGBColor.prototype.add = function( aRGBColor )\n\
    {\n\
        this.nRed += aRGBColor.nRed;\n\
        this.nGreen += aRGBColor.nGreen;\n\
        this.nBlue += aRGBColor.nBlue;\n\
        return this;\n\
    };\n\
\n\
    RGBColor.prototype.scale = function( aT )\n\
    {\n\
        this.nRed *= aT;\n\
        this.nGreen *= aT;\n\
        this.nBlue *= aT;\n\
        return this;\n\
    };\n\
\n\
    RGBColor.clamp = function( aRGBColor )\n\
    {\n\
        var aClampedRGBColor = new RGBColor( 0, 0, 0 );\n\
\n\
        aClampedRGBColor.nRed   = clamp( aRGBColor.nRed, 0.0, 1.0 );\n\
        aClampedRGBColor.nGreen = clamp( aRGBColor.nGreen, 0.0, 1.0 );\n\
        aClampedRGBColor.nBlue  = clamp( aRGBColor.nBlue, 0.0, 1.0 );\n\
\n\
        return aClampedRGBColor;\n\
    };\n\
\n\
";

static const char aSVGScript9[] =
"\
    RGBColor.prototype.convertToHSL = function()\n\
    {\n\
        var nRed   = clamp( this.nRed, 0.0, 1.0 );\n\
        var nGreen = clamp( this.nGreen, 0.0, 1.0 );\n\
        var nBlue  = clamp( this.nBlue, 0.0, 1.0 );\n\
\n\
        var nMax = Math.max( nRed, nGreen, nBlue );\n\
        var nMin = Math.min( nRed, nGreen, nBlue );\n\
        var nDelta = nMax - nMin;\n\
\n\
        var nLuminance  = ( nMax + nMin ) / 2.0;\n\
        var nSaturation = 0.0;\n\
        var nHue = 0.0;\n\
        if( nDelta !== 0 )\n\
        {\n\
            nSaturation = ( nLuminance > 0.5 ) ?\n\
                                ( nDelta / ( 2.0 - nMax - nMin) ) :\n\
                                ( nDelta / ( nMax + nMin ) );\n\
\n\
            if( nRed == nMax )\n\
                nHue = ( nGreen - nBlue ) / nDelta;\n\
            else if( nGreen == nMax )\n\
                nHue = 2.0 + ( nBlue - nRed ) / nDelta;\n\
            else if( nBlue == nMax )\n\
                nHue = 4.0 + ( nRed - nGreen ) / nDelta;\n\
\n\
            nHue *= 60.0;\n\
\n\
            if( nHue < 0.0 )\n\
                nHue += 360.0;\n\
        }\n\
\n\
        return new HSLColor( nHue, nSaturation, nLuminance );\n\
\n\
    };\n\
\n\
    RGBColor.prototype.toString = function( bClamped )\n\
    {\n\
        var aRGBColor;\n\
        if( bClamped )\n\
        {\n\
            aRGBColor = RGBColor.clamp( this );\n\
        }\n\
        else\n\
        {\n\
            aRGBColor = this;\n\
        }\n\
\n\
        var nRed = Math.round( aRGBColor.nRed * 255 );\n\
        var nGreen = Math.round( aRGBColor.nGreen * 255 );\n\
        var nBlue = Math.round( aRGBColor.nBlue * 255 );\n\
\n\
        return ( 'rgb(' + nRed + ',' + nGreen + ',' + nBlue + ')' );\n\
    };\n\
\n\
    RGBColor.interpolate = function( aStartRGB , aEndRGB, nT )\n\
    {\n\
        var aResult = aStartRGB.clone();\n\
        var aTEndRGB = aEndRGB.clone();\n\
        aResult.scale( 1.0 - nT );\n\
        aTEndRGB.scale( nT );\n\
        aResult.add( aTEndRGB );\n\
\n\
        return aResult;\n\
    };\n\
\n\
\n\
\n\
    function HSLColor( nHue, nSaturation, nLuminance )\n\
    {\n\
        this.eColorSpace = COLOR_SPACE_HSL;\n\
        this.nHue = nHue;\n\
        this.nSaturation = nSaturation;\n\
        this.nLuminance = nLuminance;\n\
\n\
        this.normalizeHue();\n\
    }\n\
\n\
\n\
    HSLColor.prototype.clone = function()\n\
    {\n\
        return new HSLColor( this.nHue, this.nSaturation, this.nLuminance );\n\
    };\n\
\n\
    HSLColor.prototype.add = function( aHSLColor )\n\
    {\n\
        this.nHue += aHSLColor.nHue;\n\
        this.nSaturation += aHSLColor.nSaturation;\n\
        this.nLuminance += aHSLColor.nLuminance;\n\
        this.normalizeHue();\n\
        return this;\n\
    };\n\
\n\
    HSLColor.prototype.scale = function( aT )\n\
    {\n\
        this.nHue *= aT;\n\
        this.nSaturation *= aT;\n\
        this.nLuminance *= aT;\n\
        this.normalizeHue();\n\
        return this;\n\
    };\n\
\n\
    HSLColor.clamp = function( aHSLColor )\n\
    {\n\
        var aClampedHSLColor = new HSLColor( 0, 0, 0 );\n\
\n\
        aClampedHSLColor.nHue = aHSLColor.nHue % 360;\n\
        if( aClampedHSLColor.nHue < 0 )\n\
            aClampedHSLColor.nHue += 360;\n\
        aClampedHSLColor.nSaturation = clamp( aHSLColor.nSaturation, 0.0, 1.0 );\n\
        aClampedHSLColor.nLuminance = clamp( aHSLColor.nLuminance, 0.0, 1.0 );\n\
    };\n\
\n\
    HSLColor.prototype.normalizeHue = function()\n\
    {\n\
        this.nHue = this.nHue % 360;\n\
        if( this.nHue < 0 ) this.nHue += 360;\n\
    };\n\
\n\
    HSLColor.prototype.toString = function()\n\
    {\n\
        return 'hsl(' + this.nHue.toFixed( 3 ) + ','\n\
                      + this.nSaturation.toFixed( 3 ) + ','\n\
                      + this.nLuminance.toFixed( 3 ) + ')';\n\
    };\n\
\n\
    HSLColor.prototype.convertToRGB = function()\n\
    {\n\
\n\
        var nHue = this.nHue % 360;\n\
        if( nHue < 0 ) nHue += 360;\n\
        var nSaturation =  clamp( this.nSaturation, 0.0, 1.0 );\n\
        var nLuminance = clamp( this.nLuminance, 0.0, 1.0 );\n\
\n\
\n\
        if( nSaturation === 0 )\n\
        {\n\
            return new RGBColor( nLuminance, nLuminance, nLuminance );\n\
        }\n\
\n\
        var nVal1 = ( nLuminance <= 0.5 ) ?\n\
                            ( nLuminance * (1.0 + nSaturation) ) :\n\
                            ( nLuminance + nSaturation - nLuminance * nSaturation );\n\
\n\
        var nVal2 = 2.0 * nLuminance - nVal1;\n\
\n\
        var nRed    = HSLColor.hsl2rgbHelper( nVal2, nVal1, nHue + 120 );\n\
        var nGreen  = HSLColor.hsl2rgbHelper( nVal2, nVal1, nHue );\n\
        var nBlue   = HSLColor.hsl2rgbHelper( nVal2, nVal1, nHue - 120 );\n\
\n\
        return new RGBColor( nRed, nGreen, nBlue );\n\
    };\n\
\n\
    HSLColor.hsl2rgbHelper = function( nValue1, nValue2, nHue )\n\
    {\n\
        nHue = nHue % 360;\n\
        if( nHue < 0 )\n\
            nHue += 360;\n\
\n\
        if( nHue < 60.0 )\n\
            return nValue1 + ( nValue2 - nValue1 ) * nHue / 60.0;\n\
        else if( nHue < 180.0 )\n\
            return nValue2;\n\
        else if( nHue < 240.0 )\n\
            return ( nValue1 + ( nValue2 - nValue1 ) * ( 240.0 - nHue ) / 60.0 );\n\
        else\n\
            return nValue1;\n\
    };\n\
\n\
    HSLColor.interpolate = function( aFrom, aTo, nT, bCCW )\n\
    {\n\
        var nS = 1.0 - nT;\n\
\n\
        var nHue = 0.0;\n\
        if( aFrom.nHue <= aTo.nHue && !bCCW )\n\
        {\n\
            nHue = nS * (aFrom.nHue + 360.0) + nT * aTo.nHue;\n\
        }\n\
        else if( aFrom.nHue > aTo.nHue && bCCW )\n\
        {\n\
            nHue = nS * aFrom.nHue + nT * (aTo.nHue + 360.0);\n\
        }\n\
        else\n\
        {\n\
            nHue = nS * aFrom.nHue + nT * aTo.nHue;\n\
        }\n\
\n\
        var nSaturation = nS * aFrom.nSaturation + nT * aTo.nSaturation;\n\
        var nLuminance = nS * aFrom.nLuminance + nT * aTo.nLuminance;\n\
\n\
        return new HSLColor( nHue, nSaturation, nLuminance );\n\
    };\n\
\n\
\n\
\n\
\n\
\n\
\n\
    var ANIMATION_NODE_CUSTOM               = 0;\n\
    var ANIMATION_NODE_PAR                  = 1;\n\
";

static const char aSVGScript10[] =
"\
    var ANIMATION_NODE_SEQ                  = 2;\n\
    var ANIMATION_NODE_ITERATE              = 3;\n\
    var ANIMATION_NODE_ANIMATE              = 4;\n\
    var ANIMATION_NODE_SET                  = 5;\n\
    var ANIMATION_NODE_ANIMATEMOTION        = 6;\n\
    var ANIMATION_NODE_ANIMATECOLOR         = 7;\n\
    var ANIMATION_NODE_ANIMATETRANSFORM     = 8;\n\
    var ANIMATION_NODE_TRANSITIONFILTER     = 9;\n\
    var ANIMATION_NODE_AUDIO                = 10;\n\
    var ANIMATION_NODE_COMMAND              = 11;\n\
\n\
    aAnimationNodeTypeInMap = {\n\
                'par'               : ANIMATION_NODE_PAR,\n\
                'seq'               : ANIMATION_NODE_SEQ,\n\
                'iterate'           : ANIMATION_NODE_ITERATE,\n\
                'animate'           : ANIMATION_NODE_ANIMATE,\n\
                'set'               : ANIMATION_NODE_SET,\n\
                'animatemotion'     : ANIMATION_NODE_ANIMATEMOTION,\n\
                'animatecolor'      : ANIMATION_NODE_ANIMATECOLOR,\n\
                'animatetransform'  : ANIMATION_NODE_ANIMATETRANSFORM,\n\
                'transitionfilter'  : ANIMATION_NODE_TRANSITIONFILTER\n\
    };\n\
\n\
\n\
\n\
    function getAnimationElementType( aElement )\n\
    {\n\
        var sName = aElement.localName.toLowerCase();\n\
\n\
        if( sName && aAnimationNodeTypeInMap[ sName ] )\n\
            return aAnimationNodeTypeInMap[ sName ];\n\
        else\n\
            return ANIMATION_NODE_CUSTOM;\n\
    }\n\
\n\
\n\
\n\
\n\
    var INVALID_NODE                = 0;\n\
    var UNRESOLVED_NODE             = 1;\n\
    var RESOLVED_NODE               = 2;\n\
    var ACTIVE_NODE                 = 4;\n\
    var FROZEN_NODE                 = 8;\n\
    var ENDED_NODE                  = 16;\n\
\n\
    function getNodeStateName( eNodeState )\n\
    {\n\
        switch( eNodeState )\n\
        {\n\
            case INVALID_NODE:\n\
                return 'INVALID';\n\
            case UNRESOLVED_NODE:\n\
                return 'UNRESOLVED';\n\
            case RESOLVED_NODE:\n\
                return 'RESOLVED';\n\
            case ACTIVE_NODE:\n\
                return 'ACTIVE';\n\
            case FROZEN_NODE:\n\
                return 'FROZEN';\n\
            case ENDED_NODE:\n\
                return 'ENDED';\n\
            default:\n\
                return 'UNKNOWN';\n\
        }\n\
    }\n\
\n\
\n\
    IMPRESS_DEFAULT_NODE                    = 0;\n\
    IMPRESS_ON_CLICK_NODE                   = 1;\n\
    IMPRESS_WITH_PREVIOUS_NODE              = 2;\n\
    IMPRESS_AFTER_PREVIOUS_NODE             = 3;\n\
    IMPRESS_MAIN_SEQUENCE_NODE              = 4;\n\
    IMPRESS_TIMING_ROOT_NODE                = 5;\n\
    IMPRESS_INTERACTIVE_SEQUENCE_NODE       = 6;\n\
\n\
    aImpressNodeTypeInMap = {\n\
            'on-click'                  : IMPRESS_ON_CLICK_NODE,\n\
            'with-previous'             : IMPRESS_WITH_PREVIOUS_NODE,\n\
            'after-previous'            : IMPRESS_AFTER_PREVIOUS_NODE,\n\
            'main-sequence'             : IMPRESS_MAIN_SEQUENCE_NODE,\n\
            'timing-root'               : IMPRESS_TIMING_ROOT_NODE,\n\
            'interactive-sequence'      : IMPRESS_INTERACTIVE_SEQUENCE_NODE\n\
    };\n\
\n\
    aImpressNodeTypeOutMap = [ 'default', 'on-click', 'with-previous', 'after-previous',\n\
                                'main-sequence', 'timing-root', 'interactive-sequence' ];\n\
\n\
\n\
    aPresetClassInMap = {};\n\
\n\
\n\
    aPresetIdInMap = {};\n\
\n\
\n\
    RESTART_MODE_DEFAULT            = 0;\n\
    RESTART_MODE_INHERIT            = 0;\n\
    RESTART_MODE_ALWAYS             = 1;\n\
    RESTART_MODE_WHEN_NOT_ACTIVE    = 2;\n\
    RESTART_MODE_NEVER              = 3;\n\
\n\
    aRestartModeInMap = {\n\
            'inherit'       : RESTART_MODE_DEFAULT,\n\
            'always'        : RESTART_MODE_ALWAYS,\n\
            'whenNotActive' : RESTART_MODE_WHEN_NOT_ACTIVE,\n\
            'never'         : RESTART_MODE_NEVER\n\
    };\n\
\n\
    aRestartModeOutMap = [ 'inherit','always', 'whenNotActive', 'never' ];\n\
\n\
\n\
    var FILL_MODE_DEFAULT           = 0;\n\
    var FILL_MODE_INHERIT           = 0;\n\
    var FILL_MODE_REMOVE            = 1;\n\
    var FILL_MODE_FREEZE            = 2;\n\
    var FILL_MODE_HOLD              = 3;\n\
    var FILL_MODE_TRANSITION        = 4;\n\
    var FILL_MODE_AUTO              = 5;\n\
\n\
    aFillModeInMap = {\n\
            'inherit'       : FILL_MODE_DEFAULT,\n\
            'remove'        : FILL_MODE_REMOVE,\n\
            'freeze'        : FILL_MODE_FREEZE,\n\
            'hold'          : FILL_MODE_HOLD,\n\
            'transition'    : FILL_MODE_TRANSITION,\n\
            'auto'          : FILL_MODE_AUTO\n\
    };\n\
\n\
    aFillModeOutMap = [ 'inherit', 'remove', 'freeze', 'hold', 'transition', 'auto' ];\n\
\n\
\n\
    var ADDITIVE_MODE_BASE          = 0;\n\
    var ADDITIVE_MODE_SUM           = 1;\n\
    var ADDITIVE_MODE_REPLACE       = 2;\n\
    var ADDITIVE_MODE_MULTIPLY      = 3;\n\
    var ADDITIVE_MODE_NONE          = 4;\n\
\n\
    aAddittiveModeInMap = {\n\
            'base'          : ADDITIVE_MODE_BASE,\n\
            'sum'           : ADDITIVE_MODE_SUM,\n\
            'replace'       : ADDITIVE_MODE_REPLACE,\n\
            'multiply'      : ADDITIVE_MODE_MULTIPLY,\n\
            'none'          : ADDITIVE_MODE_NONE\n\
    };\n\
\n\
    aAddittiveModeOutMap = [ 'base', 'sum', 'replace', 'multiply', 'none' ];\n\
\n\
\n\
    var ACCUMULATE_MODE_NONE        = 0;\n\
    var ACCUMULATE_MODE_SUM         = 1;\n\
\n\
    aAccumulateModeOutMap = [ 'none', 'sum' ];\n\
\n\
    var CALC_MODE_DISCRETE          = 0;\n\
    var CALC_MODE_LINEAR            = 1;\n\
    var CALC_MODE_PACED             = 2;\n\
    var CALC_MODE_SPLINE            = 3;\n\
\n\
    aCalcModeInMap = {\n\
            'discrete'      : CALC_MODE_DISCRETE,\n\
            'linear'        : CALC_MODE_LINEAR,\n\
            'paced'         : CALC_MODE_PACED,\n\
            'spline'        : CALC_MODE_SPLINE\n\
    };\n\
\n\
    aCalcModeOutMap = [ 'discrete', 'linear', 'paced', 'spline' ];\n\
\n\
\n\
    var COLOR_SPACE_RGB = 0;\n\
    var COLOR_SPACE_HSL = 1;\n\
\n\
    aColorSpaceInMap = { 'rgb': COLOR_SPACE_RGB, 'hsl': COLOR_SPACE_HSL };\n\
\n\
    aColorSpaceOutMap = [ 'rgb', 'hsl' ];\n\
\n\
\n\
    var CLOCKWISE               = 0;\n\
    var COUNTERCLOCKWISE        = 1;\n\
\n\
    aClockDirectionInMap = { 'clockwise': CLOCKWISE, 'counterclockwise': COUNTERCLOCKWISE };\n\
\n\
    aClockDirectionOutMap = [ 'clockwise', 'counterclockwise' ];\n\
\n\
\n\
    UNKNOWN_PROPERTY        = 0;\n\
    NUMBER_PROPERTY         = 1;\n\
    ENUM_PROPERTY           = 2;\n\
    COLOR_PROPERTY          = 3;\n\
    STRING_PROPERTY         = 4;\n\
    BOOL_PROPERTY           = 5;\n\
\n\
    aValueTypeOutMap = [ 'unknown', 'number', 'enum', 'color', 'string', 'boolean' ];\n\
\n\
\n\
    var aAttributeMap =\n\
    {\n\
            'height':           {   'type':         NUMBER_PROPERTY,\n\
                                    'get':          'getHeight',\n\
                                    'set':          'setHeight',\n\
                                    'getmod':       'makeScaler( 1/nHeight )',\n\
                                    'setmod':       'makeScaler( nHeight)'          },\n\
";

static const char aSVGScript11[] =
"\
\n\
            'opacity':          {   'type':         NUMBER_PROPERTY,\n\
                                    'get':          'getOpacity',\n\
                                    'set':          'setOpacity'                    },\n\
\n\
            'width':            {   'type':         NUMBER_PROPERTY,\n\
                                    'get':          'getWidth',\n\
                                    'set':          'setWidth',\n\
                                    'getmod':       'makeScaler( 1/nWidth )',\n\
                                    'setmod':       'makeScaler( nWidth)'           },\n\
\n\
            'x':                {   'type':         NUMBER_PROPERTY,\n\
                                    'get':          'getX',\n\
                                    'set':          'setX',\n\
                                    'getmod':       'makeScaler( 1/nWidth )',\n\
                                    'setmod':       'makeScaler( nWidth)'           },\n\
\n\
            'y':                {   'type':         NUMBER_PROPERTY,\n\
                                    'get':          'getY',\n\
                                    'set':          'setY',\n\
                                    'getmod':       'makeScaler( 1/nHeight )',\n\
                                    'setmod':       'makeScaler( nHeight)'          },\n\
\n\
            'fill':             {   'type':         ENUM_PROPERTY,\n\
                                    'get':          'getFillStyle',\n\
                                    'set':          'setFillStyle'                  },\n\
\n\
            'stroke':           {   'type':         ENUM_PROPERTY,\n\
                                    'get':          'getStrokeStyle',\n\
                                    'set':          'setStrokeStyle'                },\n\
\n\
            'visibility':       {   'type':         ENUM_PROPERTY,\n\
                                    'get':          'getVisibility',\n\
                                    'set':          'setVisibility'                 },\n\
\n\
            'fill-color':       {   'type':         COLOR_PROPERTY,\n\
                                    'get':          'getFillColor',\n\
                                    'set':          'setFillColor'                  },\n\
\n\
            'stroke-color':     {   'type':         COLOR_PROPERTY,\n\
                                    'get':          'getStrokeColor',\n\
                                    'set':          'setStrokeColor'                },\n\
\n\
            'color':            {   'type':         COLOR_PROPERTY,\n\
                                    'get':          'getFontColor',\n\
                                    'set':          'setFontColor'                  },\n\
\n\
    };\n\
\n\
\n\
    BARWIPE_TRANSITION          = 1;\n\
    FADE_TRANSITION             = 2; // 37\n\
\n\
    aTransitionTypeInMap = {\n\
                'barWipe'           : BARWIPE_TRANSITION,\n\
                'fade'              : FADE_TRANSITION\n\
    };\n\
\n\
    aTransitionTypeOutMap = [ '', 'barWipe', 'fade' ];\n\
\n\
\n\
    DEFAULT_TRANS_SUBTYPE               = 0;\n\
    LEFTTORIGHT_TRANS_SUBTYPE           = 1;\n\
    TOPTOBOTTOM_TRANS_SUBTYPE           = 2;\n\
    CROSSFADE_TRANS_SUBTYPE             = 3; // 101\n\
\n\
    aTransitionSubtypeInMap = {\n\
                'leftToRight'       : LEFTTORIGHT_TRANS_SUBTYPE,\n\
                'topToBottom'       : TOPTOBOTTOM_TRANS_SUBTYPE,\n\
                'crossfade'         : CROSSFADE_TRANS_SUBTYPE\n\
    };\n\
\n\
    aTransitionSubtypeOutMap = [ 'default', 'leftToRight', 'topToBottom', 'crossfade' ];\n\
\n\
\n\
    TRANSITION_MODE_IN  = 1;\n\
    TRANSITION_MODE_OUT = 0;\n\
\n\
    aTransitionModeInMap = { 'out': TRANSITION_MODE_OUT, 'in': TRANSITION_MODE_IN };\n\
    aTransitionModeOutMap = [ 'out', 'in' ];\n\
\n\
\n\
\n\
    var aStateTransitionTable_Never_Freeze =\n\
        [\n\
         INVALID_NODE,\n\
         RESOLVED_NODE | ENDED_NODE,         // active successors for UNRESOLVED\n\
         ACTIVE_NODE | ENDED_NODE,           // active successors for RESOLVED\n\
         INVALID_NODE,\n\
         FROZEN_NODE | ENDED_NODE,           // active successors for ACTIVE: freeze object\n\
         INVALID_NODE,\n\
         INVALID_NODE,\n\
         INVALID_NODE,\n\
         ENDED_NODE,                         // active successors for FROZEN: end\n\
         INVALID_NODE,\n\
         INVALID_NODE,\n\
         INVALID_NODE,\n\
         INVALID_NODE,\n\
         INVALID_NODE,\n\
         INVALID_NODE,\n\
         INVALID_NODE,\n\
         ENDED_NODE                          // active successors for ENDED:\n\
         ];\n\
\n\
\n\
    var aTableGuide =\n\
        [\n\
         null,\n\
         null,\n\
         null,\n\
         aStateTransitionTable_Never_Freeze,\n\
         null,\n\
         null\n\
         ];\n\
\n\
\n\
\n\
    function getTransitionTable( eRestartMode, eFillMode )\n\
    {\n\
        var nRestartValue = 0;  // never\n\
\n\
        var nFillValue = 1;     // frozen\n\
\n\
        return aTableGuide[ 3*nFillValue + nRestartValue ];\n\
    }\n\
\n\
\n\
\n\
\n\
    var EVENT_TRIGGER_UNKNOWN               = 0;\n\
    var EVENT_TRIGGER_ON_SLIDE_BEGIN        = 1;\n\
    var EVENT_TRIGGER_ON_SLIDE_END          = 2;\n\
    var EVENT_TRIGGER_BEGIN_EVENT           = 3;\n\
    var EVENT_TRIGGER_END_EVENT             = 4;\n\
    var EVENT_TRIGGER_ON_CLICK              = 5;\n\
    var EVENT_TRIGGER_ON_DBL_CLICK          = 6;\n\
    var EVENT_TRIGGER_ON_MOUSE_ENTER        = 7;\n\
    var EVENT_TRIGGER_ON_MOUSE_LEAVE        = 8;\n\
    var EVENT_TRIGGER_ON_NEXT_EFFECT        = 9;\n\
    var EVENT_TRIGGER_ON_PREV_EFFECT        = 10;\n\
    var EVENT_TRIGGER_REPEAT                = 11;\n\
\n\
    aEventTriggerOutMap = [ 'unknown', 'slideBegin', 'slideEnd', 'begin', 'end', 'click',\n\
                            'doubleClick', 'mouseEnter', 'mouseLeave', 'next', 'previous', 'repeat' ];\n\
\n\
\n\
    function getEventTriggerType( sEventTrigger )\n\
    {\n\
        if( sEventTrigger == 'begin' )\n\
            return EVENT_TRIGGER_BEGIN_EVENT;\n\
        else if( sEventTrigger == 'end' )\n\
            return EVENT_TRIGGER_END_EVENT;\n\
        else if( sEventTrigger == 'next' )\n\
            return EVENT_TRIGGER_ON_NEXT_EFFECT;\n\
        else if( sEventTrigger == 'prev' )\n\
            return EVENT_TRIGGER_ON_PREV_EFFECT;\n\
        else if( sEventTrigger == 'click' )\n\
            return EVENT_TRIGGER_ON_CLICK;\n\
        else\n\
            return EVENT_TRIGGER_UNKNOWN;\n\
    }\n\
\n\
\n\
\n\
\n\
    var UNKNOWN_TIMING          = 0;\n\
    var OFFSET_TIMING           = 1;\n\
    var WALLCLOCK_TIMING        = 2;\n\
    var INDEFINITE_TIMING       = 3;\n\
    var EVENT_TIMING            = 4;\n\
    var SYNCBASE_TIMING         = 5;\n\
    var MEDIA_TIMING            = 6;\n\
\n\
    aTimingTypeOutMap = [ 'unknown', 'offset', 'wallclock', 'indefinite', 'event', 'syncbase', 'media' ];\n\
\n\
\n\
    var CHARCODE_PLUS       = '+'.charCodeAt(0);\n\
    var CHARCODE_MINUS      = '-'.charCodeAt(0);\n\
    var CHARCODE_0          = '0'.charCodeAt(0);\n\
    var CHARCODE_9          = '9'.charCodeAt(0);\n\
\n\
\n\
\n\
    function Timing( aAnimationNode, sTimingAttribute )\n\
    {\n\
        this.aAnimationNode = aAnimationNode;     // the node, the timing attribute belongs to\n\
        this.sTimingDescription = removeWhiteSpaces( sTimingAttribute );\n\
        this.eTimingType = UNKNOWN_TIMING;\n\
        this.nOffset = 0.0;                       // in seconds\n\
        this.sEventBaseElementId = '';            // the element id for event based timing\n\
        this.eEventType = EVENT_TRIGGER_UNKNOWN;  // the event type\n\
    }\n\
\n\
    Timing.prototype.getAnimationNode = function()\n\
    {\n\
        return this.aAnimationNode;\n\
    };\n\
\n\
    Timing.prototype.getType = function()\n\
    {\n\
";

static const char aSVGScript12[] =
"\
        return this.eTimingType;\n\
    };\n\
\n\
    Timing.prototype.getOffset = function()\n\
    {\n\
        return this.nOffset;\n\
    };\n\
\n\
    Timing.prototype.getEventBaseElementId = function()\n\
    {\n\
        return this.sEventBaseElementId;\n\
    };\n\
\n\
    Timing.prototype.getEventType = function()\n\
    {\n\
        return this.eEventType;\n\
    };\n\
\n\
    Timing.prototype.parse = function()\n\
    {\n\
        if( !this.sTimingDescription )\n\
        {\n\
            this.eTimingType = OFFSET_TIMING;\n\
            return;\n\
        }\n\
\n\
        if( this.sTimingDescription == 'indefinite' )\n\
            this.eTimingType = INDEFINITE_TIMING;\n\
        else\n\
        {\n\
            var nFisrtCharCode = this.sTimingDescription.charCodeAt(0);\n\
            var bPositiveOffset = !( nFisrtCharCode == CHARCODE_MINUS );\n\
            if ( ( nFisrtCharCode == CHARCODE_PLUS ) ||\n\
                    ( nFisrtCharCode == CHARCODE_MINUS ) ||\n\
                    ( ( nFisrtCharCode >= CHARCODE_0 ) && ( nFisrtCharCode <= CHARCODE_9 ) ) )\n\
            {\n\
                var sClockValue\n\
                = ( ( nFisrtCharCode == CHARCODE_PLUS ) || ( nFisrtCharCode == CHARCODE_MINUS ) )\n\
                ? this.sTimingDescription.substr( 1 )\n\
                        : this.sTimingDescription;\n\
\n\
                var TimeInSec = Timing.parseClockValue( sClockValue );\n\
                if( TimeInSec != undefined )\n\
                {\n\
                    this.eTimingType = OFFSET_TIMING;\n\
                    this.nOffset = bPositiveOffset ? TimeInSec : -TimeInSec;\n\
                }\n\
            }\n\
            else\n\
            {\n\
                var aTimingSplit = new Array();\n\
                bPositiveOffset = true;\n\
                if( this.sTimingDescription.indexOf( '+' ) != -1 )\n\
                {\n\
                    aTimingSplit = this.sTimingDescription.split( '+' );\n\
                }\n\
                else if( this.sTimingDescription.indexOf( '-' ) != -1 )\n\
                {\n\
                    aTimingSplit = this.sTimingDescription.split( '-' );\n\
                    bPositiveOffset = false;\n\
                }\n\
                else\n\
                {\n\
                    aTimingSplit[0] = this.sTimingDescription;\n\
                    aTimingSplit[1] = '';\n\
                }\n\
\n\
                if( aTimingSplit[0].indexOf( '.' ) != -1 )\n\
                {\n\
                    var aEventSplit = aTimingSplit[0].split( '.' );\n\
                    this.sEventBaseElementId = aEventSplit[0];\n\
                    this.eEventType = getEventTriggerType( aEventSplit[1] );\n\
                }\n\
                else\n\
                {\n\
                    this.eEventType = getEventTriggerType( aTimingSplit[0] );\n\
                }\n\
\n\
                if( this.eEventType == EVENT_TRIGGER_UNKNOWN )\n\
                    return;\n\
\n\
                if( ( this.eEventType == EVENT_TRIGGER_BEGIN_EVENT ) ||\n\
                        ( this.eEventType == EVENT_TRIGGER_END_EVENT ) )\n\
                {\n\
                    this.eTimingType = SYNCBASE_TIMING;\n\
                }\n\
                else\n\
                {\n\
                    this.eTimingType = EVENT_TIMING;\n\
                }\n\
\n\
                if( aTimingSplit[1] )\n\
                {\n\
                    var sClockValue = aTimingSplit[1];\n\
                    var TimeInSec = Timing.parseClockValue( sClockValue );\n\
                    if( TimeInSec != undefined )\n\
                    {\n\
                        this.nOffset = ( bPositiveOffset ) ? TimeInSec : -TimeInSec;\n\
                    }\n\
                    else\n\
                    {\n\
                        this.eTimingType = UNKNOWN_TIMING;\n\
                    }\n\
\n\
                }\n\
            }\n\
        }\n\
\n\
    };\n\
\n\
    Timing.parseClockValue = function( sClockValue )\n\
    {\n\
        if( !sClockValue )\n\
            return 0.0;\n\
\n\
        var nTimeInSec = undefined;\n\
\n\
        var reFullClockValue = /^([0-9]+):([0-5][0-9]):([0-5][0-9])(.[0-9]+)?$/;\n\
        var rePartialClockValue = /^([0-5][0-9]):([0-5][0-9])(.[0-9]+)?$/;\n\
        var reTimecountValue = /^([0-9]+)(.[0-9]+)?(h|min|s|ms)?$/;\n\
\n\
        if( reFullClockValue.test( sClockValue ) )\n\
        {\n\
            var aClockTimeParts = reFullClockValue.exec( sClockValue );\n\
\n\
            var nHours = parseInt( aClockTimeParts[1] );\n\
            var nMinutes = parseInt( aClockTimeParts[2] );\n\
            var nSeconds = parseInt( aClockTimeParts[3] );\n\
            if( aClockTimeParts[4] )\n\
                nSeconds += parseFloat( aClockTimeParts[4] );\n\
\n\
            nTimeInSec = ( ( nHours * 60 ) +  nMinutes ) * 60 + nSeconds;\n\
\n\
        }\n\
        else if( rePartialClockValue.test( sClockValue ) )\n\
        {\n\
            var aClockTimeParts = rePartialClockValue.exec( sClockValue );\n\
\n\
            var nMinutes = parseInt( aClockTimeParts[1] );\n\
            var nSeconds = parseInt( aClockTimeParts[2] );\n\
            if( aClockTimeParts[3] )\n\
                nSeconds += parseFloat( aClockTimeParts[3] );\n\
\n\
            nTimeInSec = nMinutes * 60 + nSeconds;\n\
        }\n\
        else if( reTimecountValue.test( sClockValue ) )\n\
        {\n\
            var aClockTimeParts = reTimecountValue.exec( sClockValue );\n\
\n\
            var nTimecount = parseInt( aClockTimeParts[1] );\n\
            if( aClockTimeParts[2] )\n\
                nTimecount += parseFloat( aClockTimeParts[2] );\n\
\n\
            if( aClockTimeParts[3] )\n\
            {\n\
                if( aClockTimeParts[3] == 'h' )\n\
                {\n\
                    nTimeInSec = nTimecount * 3600;\n\
                }\n\
                else if( aClockTimeParts[3] == 'min' )\n\
                {\n\
                    nTimeInSec = nTimecount * 60;\n\
                }\n\
                else if( aClockTimeParts[3] == 's' )\n\
                {\n\
                    nTimeInSec = nTimecount;\n\
                }\n\
                else if( aClockTimeParts[3] == 'ms' )\n\
                {\n\
                    nTimeInSec = nTimecount / 1000;\n\
                }\n\
            }\n\
            else\n\
            {\n\
                nTimeInSec = nTimecount;\n\
            }\n\
\n\
        }\n\
\n\
        if( nTimeInSec )\n\
            nTimeInSec = parseFloat( nTimeInSec.toFixed( 3 ) );\n\
        return nTimeInSec;\n\
    };\n\
\n\
    Timing.prototype.info = function( bVerbose )\n\
    {\n\
\n\
        var sInfo = '';\n\
\n\
        if( bVerbose )\n\
        {\n\
            sInfo = 'description: ' + this.sTimingDescription + ', ';\n\
\n\
            sInfo += ', type: ' +  aTimingTypeOutMap[ this.getType() ];\n\
            sInfo += ', offset: ' + this.getOffset();\n\
            sInfo += ', event base element id: ' + this.getEventBaseElementId();\n\
            sInfo += ', timing event type: ' + aEventTriggerOutMap[ this.getEventType() ];\n\
        }\n\
        else\n\
        {\n\
";

static const char aSVGScript13[] =
"\
            switch( this.getType() )\n\
            {\n\
                case INDEFINITE_TIMING:\n\
                    sInfo += 'indefinite';\n\
                    break;\n\
                case OFFSET_TIMING:\n\
                    sInfo += this.getOffset();\n\
                    break;\n\
                case EVENT_TIMING:\n\
                case SYNCBASE_TIMING:\n\
                    if( this.getEventBaseElementId() )\n\
                        sInfo += this.getEventBaseElementId() + '.';\n\
                    sInfo += aEventTriggerOutMap[ this.getEventType() ];\n\
                    if( this.getOffset() )\n\
                    {\n\
                        if( this.getOffset() > 0 )\n\
                            sInfo += '+';\n\
                        sInfo += this.getOffset();\n\
                    }\n\
            }\n\
        }\n\
\n\
        return sInfo;\n\
    };\n\
\n\
\n\
\n\
    function Duration( sDurationAttribute )\n\
    {\n\
        this.bIndefinite = false;\n\
        this.bMedia = false;\n\
        this.nValue = undefined;\n\
        this.bDefined = false;\n\
\n\
        if( !sDurationAttribute )\n\
            return;\n\
\n\
        if( sDurationAttribute == 'indefinite' )\n\
            this.bIndefinite = true;\n\
        else if( sDurationAttribute == 'media' )\n\
            this.bMedia = true;\n\
        else\n\
        {\n\
            this.nValue = Timing.parseClockValue( sDurationAttribute );\n\
            if( this.nValue <= 0.0 )\n\
                this.nValue = 0.001;  // duration must be always greater than 0\n\
        }\n\
        this.bDefined = true;\n\
    }\n\
\n\
\n\
    Duration.prototype.isSet = function()\n\
    {\n\
        return this.bDefined;\n\
    };\n\
\n\
    Duration.prototype.isIndefinite = function()\n\
    {\n\
        return this.bIndefinite;\n\
    };\n\
\n\
    Duration.prototype.isMedia = function()\n\
    {\n\
        return this.bMedia;\n\
    };\n\
\n\
    Duration.prototype.isValue = function()\n\
    {\n\
        return this.nValue != undefined;\n\
    };\n\
\n\
    Duration.prototype.getValue= function()\n\
    {\n\
        return this.nValue;\n\
    };\n\
\n\
    Duration.prototype.info= function()\n\
    {\n\
        var sInfo;\n\
\n\
        if( this.isIndefinite() )\n\
            sInfo = 'indefinite';\n\
        else if( this.isMedia() )\n\
            sInfo = 'media';\n\
        else if( this.getValue() )\n\
            sInfo = this.getValue();\n\
\n\
        return sInfo;\n\
    };\n\
\n\
\n\
\n\
    function AnimationNode()\n\
    {\n\
    }\n\
\n\
    AnimationNode.prototype.init = function() {};\n\
    AnimationNode.prototype.resolve = function() {};\n\
    AnimationNode.prototype.activate = function() {};\n\
    AnimationNode.prototype.deactivate = function() {};\n\
    AnimationNode.prototype.end = function() {};\n\
    AnimationNode.prototype.getState = function() {};\n\
    AnimationNode.prototype.registerDeactivatingListener = function() {};\n\
    AnimationNode.prototype.notifyDeactivating = function() {};\n\
\n\
\n\
\n\
    function NodeContext( aSlideShowContext )\n\
    {\n\
        this.aContext = aSlideShowContext;\n\
        this.aAnimationNodeMap = null;\n\
        this.aAnimatedElementMap = null;\n\
        this.aSourceEventElementMap = null;\n\
        this.nStartDelay = 0.0;\n\
        this.bFirstRun = undefined;\n\
        this.aSlideHeight = HEIGHT;\n\
        this.aSlideWidth = WIDTH;\n\
    }\n\
\n\
\n\
    NodeContext.prototype.makeSourceEventElement = function( sId, aEventBaseElem )\n\
    {\n\
        if( !aEventBaseElem )\n\
        {\n\
            log( 'NodeContext.makeSourceEventElement: event base element is not valid' );\n\
            return null;\n\
        }\n\
\n\
        if( !this.aContext.aEventMultiplexer )\n\
        {\n\
            log( 'NodeContext.makeSourceEventElement: event multiplexer not initialized' );\n\
            return null;\n\
        }\n\
\n\
        if( !this.aAnimationNodeMap[ sId ] )\n\
        {\n\
            this.aAnimationNodeMap[ sId ] = new SourceEventElement( aEventBaseElem, this.aContext.aEventMultiplexer );\n\
        }\n\
        return this.aAnimationNodeMap[ sId ];\n\
    };\n\
\n\
\n\
\n\
    function StateTransition( aBaseNode )\n\
    {\n\
        this.aNode = aBaseNode;\n\
        this.eToState = INVALID_NODE;\n\
    }\n\
\n\
    StateTransition.prototype.enter = function( eNodeState, bForce )\n\
    {\n\
        if( !bForce ) bForce = false;\n\
\n\
        if( this.eToState != INVALID_NODE )\n\
        {\n\
            log( 'StateTransition.enter: commit() before enter()ing again!' );\n\
            return false;\n\
        }\n\
        if( !bForce && !this.aNode.isTransition( this.aNode.getState(), eNodeState  ) )\n\
            return false;\n\
\n\
        if( ( this.aNode.nCurrentStateTransition & eNodeState ) != 0 )\n\
            return false; // already in wanted transition\n\
\n\
        this.aNode.nCurrentStateTransition |= eNodeState;\n\
        this.eToState = eNodeState;\n\
        return true;\n\
    };\n\
\n\
    StateTransition.prototype.commit = function()\n\
    {\n\
        if( this.eToState != INVALID_NODE )\n\
        {\n\
            this.aNode.eCurrentState = this.eToState;\n\
            this.clear();\n\
        }\n\
    };\n\
\n\
    StateTransition.prototype.clear = function()\n\
    {\n\
        if( this.eToState != INVALID_NODE )\n\
        {\n\
            this.aNode.nCurrentStateTransition &= ~this.eToState;\n\
            this.eToState = INVALID_NODE;\n\
        }\n\
    };\n\
\n\
\n\
\n\
    function BaseNode( aAnimElem, aParentNode, aNodeContext )\n\
    {\n\
        this.nId = getUniqueId();\n\
        this.sClassName = 'BaseNode';\n\
\n\
        if( !aAnimElem )\n\
            log( 'BaseNode(id:' + this.nId + ') constructor: aAnimElem is not valid' );\n\
\n\
        if( !aNodeContext )\n\
            log( 'BaseNode(id:' + this.nId + ') constructor: aNodeContext is not valid' );\n\
\n\
";

static const char aSVGScript14[] =
"\
        if( !aNodeContext.aContext )\n\
            log( 'BaseNode(id:' + this.nId + ') constructor: aNodeContext.aContext is not valid' );\n\
\n\
\n\
        this.bIsContainer;\n\
        this.aElement = aAnimElem;\n\
        this.aParentNode = aParentNode;\n\
        this.aNodeContext = aNodeContext;\n\
        this.aContext = aNodeContext.aContext;\n\
        this.nStartDelay = aNodeContext.nStartDelay;\n\
        this.eCurrentState = UNRESOLVED_NODE;\n\
        this.nCurrentStateTransition = 0;\n\
        this.aDeactivatingListenerArray = new Array();\n\
        this.aActivationEvent = null;\n\
        this.aDeactivationEvent = null;\n\
\n\
        this.aBegin = null;\n\
        this.aDuration = null;\n\
        this.aEnd = null;\n\
        this.bMainSequenceRootNode = false;\n\
        this.eFillMode = FILL_MODE_FREEZE;\n\
        this.eRestartMode = RESTART_MODE_NEVER;\n\
        this.nReapeatCount = undefined;\n\
        this.nAccelerate = 0.0;\n\
        this.nDecelerate = 0.0;\n\
        this.bAutoReverse = false;\n\
\n\
    }\n\
    extend( BaseNode, AnimationNode );\n\
\n\
\n\
    BaseNode.prototype.getId = function()\n\
    {\n\
        return this.nId;\n\
    };\n\
\n\
    BaseNode.prototype.parseElement = function()\n\
    {\n\
        var aAnimElem = this.aElement;\n\
\n\
        var sIdAttr = aAnimElem.getAttributeNS( NSS['xml'], 'id' );\n\
        if( sIdAttr )\n\
            this.aNodeContext.aAnimationNodeMap[ sIdAttr ] = this;\n\
\n\
        this.aBegin = null;\n\
        var sBeginAttr = aAnimElem.getAttribute( 'begin' );\n\
        this.aBegin = new Timing( this, sBeginAttr );\n\
        this.aBegin.parse();\n\
\n\
        this.aEnd = null;\n\
        var sEndAttr = aAnimElem.getAttribute( 'end' );\n\
        if( sEndAttr )\n\
        {\n\
            this.aEnd = new Timing( this, sEndAttr );\n\
            this.aEnd.parse();\n\
        }\n\
\n\
        this.aDuration = null;\n\
        var sDurAttr = aAnimElem.getAttribute( 'dur' );\n\
        this.aDuration = new Duration( sDurAttr );\n\
        if( !this.aDuration.isSet() )\n\
        {\n\
            if( this.isContainer() )\n\
                this.aDuration = null;\n\
            else\n\
                this.aDuration = new Duration( 'indefinite' );\n\
        }\n\
\n\
        var sFillAttr = aAnimElem.getAttribute( 'fill' );\n\
        if( sFillAttr && aFillModeInMap[ sFillAttr ])\n\
            this.eFillMode = aFillModeInMap[ sFillAttr ];\n\
        else\n\
            this.eFillMode = FILL_MODE_DEFAULT;\n\
\n\
        var sRestartAttr = aAnimElem.getAttribute( 'restart' );\n\
        if( sRestartAttr && aRestartModeInMap[ sRestartAttr ] )\n\
            this.eRestartMode = aRestartModeInMap[ sRestartAttr ];\n\
        else\n\
            this.eRestartMode = RESTART_MODE_DEFAULT;\n\
\n\
        var sRepeatCount = aAnimElem.getAttribute( 'repeatCount' );\n\
        if( !sRepeatCount )\n\
            this.nReapeatCount = 1;\n\
        else\n\
            this.nReapeatCount = parseFloat( sRepeatCount );\n\
        if( ( this.nReapeatCount == NaN ) && ( sRepeatCount != 'indefinite' ) )\n\
            this.nReapeatCount = 1;\n\
\n\
        this.nAccelerate = 0.0;\n\
        var sAccelerateAttr = aAnimElem.getAttribute( 'accelerate' );\n\
        if( sAccelerateAttr )\n\
            this.nAccelerate = parseFloat( sAccelerateAttr );\n\
        if( this.nAccelerate == NaN )\n\
            this.nAccelerate = 0.0;\n\
\n\
        this.nDecelerate = 0.0;\n\
        var sDecelerateAttr = aAnimElem.getAttribute( 'decelerate' );\n\
        if( sDecelerateAttr )\n\
            this.nDecelerate = parseFloat( sDecelerateAttr );\n\
        if( this.nDecelerate == NaN )\n\
            this.nDecelerate = 0.0;\n\
\n\
        this.bAutoreverse = false;\n\
        var sAutoReverseAttr = aAnimElem.getAttribute( 'autoReverse' );\n\
        if( sAutoReverseAttr == 'true' )\n\
            this.bAutoreverse = true;\n\
\n\
\n\
        if( this.eFillMode == FILL_MODE_DEFAULT )\n\
            if( this.getParentNode() )\n\
                this.eFillMode = this.getParentNode().getFillMode();\n\
            else\n\
                this.eFillMode = FILL_MODE_AUTO;\n\
\n\
        if( this.eFillMode ==  FILL_MODE_AUTO ) // see SMIL recommendation document\n\
        {\n\
            this.eFillMode = ( this.aEnd ||\n\
                                ( this.nReapeatCount != 1) ||\n\
                                this.aDuration )\n\
                                ? FILL_MODE_REMOVE\n\
                                : FILL_MODE_FREEZE;\n\
        }\n\
\n\
        if( this.eRestartMode == RESTART_MODE_DEFAULT )\n\
            if( this.getParentNode() )\n\
                this.eRestartMode = this.getParentNode().getRestartMode();\n\
            else\n\
                this.eRestartMode = RESTART_MODE_NEVER;\n\
\n\
        if( ( this.nAccelerate + this.nDecelerate ) > 1.0 )\n\
        {\n\
            this.nAccelerate = 0.0;\n\
            this.nDecelerate = 0.0;\n\
        }\n\
\n\
        this.eFillMode = FILL_MODE_FREEZE;\n\
        this.eRestartMode = RESTART_MODE_NEVER;\n\
        this.aStateTransTable = getTransitionTable( this.getRestartMode(), this.getFillMode() );\n\
\n\
        return true;\n\
    };\n\
\n\
    BaseNode.prototype.getParentNode = function()\n\
    {\n\
        return this.aParentNode;\n\
    };\n\
\n\
    BaseNode.prototype.init = function()\n\
    {\n\
        if( ! this.checkValidNode() )\n\
            return false;\n\
        if( this.aActivationEvent )\n\
            this.aActivationEvent.dispose();\n\
        if( this.aDeactivationEvent )\n\
            this.aDeactivationEvent.dispose();\n\
\n\
        this.eCurrentState = UNRESOLVED_NODE;\n\
\n\
        return this.init_st();\n\
    };\n\
\n\
    BaseNode.prototype.resolve = function()\n\
    {\n\
        if( ! this.checkValidNode() )\n\
            return false;\n\
\n\
        this.DBG( this.callInfo( 'resolve' ) );\n\
\n\
        if( this.eCurrentState == RESOLVED_NODE )\n\
            log( 'BaseNode.resolve: already in RESOLVED state' );\n\
\n\
        var aStateTrans = new StateTransition( this );\n\
\n\
        if( aStateTrans.enter( RESOLVED_NODE ) &&\n\
                this.isTransition( RESOLVED_NODE, ACTIVE_NODE ) &&\n\
                this.resolve_st() )\n\
        {\n\
            aStateTrans.commit();\n\
\n\
            if( this.aActivationEvent )\n\
            {\n\
                this.aActivationEvent.charge();\n\
            }\n\
            else\n\
            {\n\
                this.aActivationEvent = makeDelay( bind( this, this.activate ), this.getBegin().getOffset() + this.nStartDelay );\n\
            }\n\
            registerEvent( this.getBegin(), this.aActivationEvent, this.aNodeContext );\n\
\n\
            return true;\n\
        }\n\
\n\
        return false;\n\
    };\n\
\n\
    BaseNode.prototype.activate = function()\n\
    {\n\
        if( ! this.checkValidNode() )\n\
            return false;\n\
\n\
";

static const char aSVGScript15[] =
"\
        if( this.eCurrentState == ACTIVE_NODE )\n\
            log( 'BaseNode.activate: already in ACTIVE state' );\n\
\n\
        this.DBG( this.callInfo( 'activate' ), getCurrentSystemTime() );\n\
\n\
        var aStateTrans = new StateTransition( this );\n\
\n\
        if( aStateTrans.enter( ACTIVE_NODE ) )\n\
        {\n\
            this.activate_st();\n\
            aStateTrans.commit();\n\
            if( !this.aContext.aEventMultiplexer )\n\
                log( 'BaseNode.activate: this.aContext.aEventMultiplexer is not valid' );\n\
            this.aContext.aEventMultiplexer.notifyEvent( EVENT_TRIGGER_BEGIN_EVENT, this.getId() );\n\
            return true;\n\
        }\n\
        return false;\n\
    };\n\
\n\
    BaseNode.prototype.deactivate = function()\n\
    {\n\
        if( this.inStateOrTransition( ENDED_NODE | FROZEN_NODE ) || !this.checkValidNode() )\n\
            return;\n\
\n\
        if( this.isTransition( this.eCurrentState, FROZEN_NODE ) )\n\
        {\n\
            this.DBG( this.callInfo( 'deactivate' ), getCurrentSystemTime() );\n\
\n\
            var aStateTrans = new StateTransition( this );\n\
            if( aStateTrans.enter( FROZEN_NODE, true /* FORCE */ ) )\n\
            {\n\
                this.deactivate_st();\n\
                aStateTrans.commit();\n\
\n\
                this.notifyEndListeners();\n\
\n\
                if( this.aActivationEvent )\n\
                    this.aActivationEvent.dispose();\n\
                if( this.aDeactivationEvent )\n\
                    this.aDeactivationEvent.dispose();\n\
            }\n\
        }\n\
        else\n\
        {\n\
            this.end();\n\
        }\n\
    };\n\
\n\
    BaseNode.prototype.end = function()\n\
    {\n\
        var bIsFrozenOrInTransitionToFrozen = this.inStateOrTransition( FROZEN_NODE );\n\
        if( this.inStateOrTransition( ENDED_NODE ) || !this.checkValidNode() )\n\
            return;\n\
\n\
        if( !(this.isTransition( this.eCurrentState, ENDED_NODE ) ) )\n\
            log( 'BaseNode.end: end state not reachable in transition table' );\n\
\n\
        this.DBG( this.callInfo( 'end' ), getCurrentSystemTime() );\n\
\n\
        var aStateTrans = new StateTransition( this );\n\
        if( aStateTrans.enter( ENDED_NODE, true /* FORCE */ ) )\n\
        {\n\
            this.deactivate_st( ENDED_NODE );\n\
            aStateTrans.commit();\n\
\n\
            if( !bIsFrozenOrInTransitionToFrozen )\n\
                this.notifyEndListeners();\n\
\n\
            if( this.aActivationEvent )\n\
                this.aActivationEvent.dispose();\n\
            if( this.aDeactivationEvent )\n\
                this.aDeactivationEvent.dispose();\n\
        }\n\
    };\n\
\n\
    BaseNode.prototype.dispose = function()\n\
    {\n\
        if( this.aActivationEvent )\n\
            this.aActivationEvent.dispose();\n\
        if( this.aDeactivationEvent )\n\
            this.aDeactivationEvent.dispose();\n\
        this.aDeactivatingListenerArray = new Array();\n\
    };\n\
\n\
    BaseNode.prototype.getState = function()\n\
    {\n\
        return this.eCurrentState;\n\
    };\n\
\n\
    BaseNode.prototype.registerDeactivatingListener = function( aNotifiee )\n\
    {\n\
        if (! this.checkValidNode())\n\
            return false;\n\
\n\
        if( !aNotifiee )\n\
        {\n\
            log( 'BaseNode.registerDeactivatingListener(): invalid notifee' );\n\
            return false;\n\
        }\n\
        this.aDeactivatingListenerArray.push( aNotifiee );\n\
\n\
        return true;\n\
    };\n\
\n\
    BaseNode.prototype.notifyDeactivating = function( aNotifier )\n\
    {\n\
        assert( ( aNotifier.getState() == FROZEN_NODE ) || ( aNotifier.getState() == ENDED_NODE ),\n\
        'BaseNode.notifyDeactivating: Notifier node is neither in FROZEN nor in ENDED state' );\n\
    };\n\
\n\
    BaseNode.prototype.isMainSequenceRootNode = function()\n\
    {\n\
        return this.bMainSequenceRootNode;\n\
    };\n\
\n\
    BaseNode.prototype.makeDeactivationEvent = function( nDelay )\n\
    {\n\
        if( this.aDeactivationEvent )\n\
        {\n\
            this.aDeactivationEvent.charge();\n\
        }\n\
        else\n\
        {\n\
            if( typeof( nDelay ) == typeof(0) )\n\
                this.aDeactivationEvent = makeDelay( bind( this, this.deactivate ), nDelay );\n\
            else\n\
                this.aDeactivationEvent = null;\n\
        }\n\
        return this.aDeactivationEvent;\n\
    };\n\
\n\
    BaseNode.prototype.scheduleDeactivationEvent = function( aEvent )\n\
    {\n\
        this.DBG( this.callInfo( 'scheduleDeactivationEvent' ) );\n\
\n\
        if( !aEvent )\n\
        {\n\
            if( this.getDuration() && this.getDuration().isValue() )\n\
                aEvent = this.makeDeactivationEvent( this.getDuration().getValue() );\n\
        }\n\
        if( aEvent )\n\
        {\n\
            this.aContext.aTimerEventQueue.addEvent( aEvent );\n\
        }\n\
    };\n\
\n\
    BaseNode.prototype.checkValidNode = function()\n\
    {\n\
        return ( this.eCurrentState != INVALID_NODE );\n\
    };\n\
\n\
    BaseNode.prototype.init_st = function()\n\
    {\n\
        return true;\n\
    };\n\
\n\
    BaseNode.prototype.resolve_st = function()\n\
    {\n\
        return true;\n\
    };\n\
\n\
    BaseNode.prototype.activate_st = function()\n\
    {\n\
        this.scheduleDeactivationEvent();\n\
    };\n\
\n\
    BaseNode.prototype.deactivate_st = function( aNodeState )\n\
    {\n\
    };\n\
\n\
    BaseNode.prototype.notifyEndListeners = function()\n\
    {\n\
        var nDeactivatingListenerCount = this.aDeactivatingListenerArray.length;\n\
\n\
        for( var i = 0; i < nDeactivatingListenerCount; ++i )\n\
        {\n\
            this.aDeactivatingListenerArray[i].notifyDeactivating( this );\n\
        }\n\
\n\
        this.aContext.aEventMultiplexer.notifyEvent( EVENT_TRIGGER_END_EVENT, this.getId() );\n\
    };\n\
\n\
    BaseNode.prototype.getContext = function()\n\
    {\n\
        return this.aContext;\n\
    };\n\
\n\
    BaseNode.prototype.isTransition = function( eFromState, eToState )\n\
    {\n\
        return ( ( this.aStateTransTable[ eFromState ] & eToState ) != 0 );\n\
    };\n\
\n\
    BaseNode.prototype.inStateOrTransition = function( nMask )\n\
    {\n\
        return ( ( ( this.eCurrentState & nMask ) != 0 ) || ( ( this.nCurrentStateTransition & nMask ) != 0 ) );\n\
    };\n\
\n\
    BaseNode.prototype.isContainer = function()\n\
    {\n\
        return this.bIsContainer;\n\
";

static const char aSVGScript16[] =
"\
    };\n\
\n\
    BaseNode.prototype.getBegin = function()\n\
    {\n\
        return this.aBegin;\n\
    };\n\
\n\
    BaseNode.prototype.getDuration = function()\n\
    {\n\
        return this.aDuration;\n\
    };\n\
\n\
    BaseNode.prototype.getEnd = function()\n\
    {\n\
        return this.aEnd;\n\
    };\n\
\n\
    BaseNode.prototype.getFillMode = function()\n\
    {\n\
        return this.eFillMode;\n\
    };\n\
\n\
    BaseNode.prototype.getRestartMode = function()\n\
    {\n\
        return this.eRestartMode;\n\
    };\n\
\n\
    BaseNode.prototype.getRepeatCount = function()\n\
    {\n\
        return this.nReapeatCount;\n\
    };\n\
\n\
    BaseNode.prototype.getAccelerateValue = function()\n\
    {\n\
        return this.nAccelerate;\n\
    };\n\
\n\
    BaseNode.prototype.getDecelerateValue = function()\n\
    {\n\
        return this.nDecelerate;\n\
    };\n\
\n\
    BaseNode.prototype.isAutoReverseEnabled = function()\n\
    {\n\
        return this.bAutoreverse;\n\
    };\n\
\n\
    BaseNode.prototype.info = function( bVerbose )\n\
    {\n\
        var sInfo = 'class name: ' + this.sClassName;\n\
        sInfo += ';  element name: ' + this.aElement.localName;\n\
        sInfo += ';  id: ' + this.getId();\n\
        sInfo += ';  state: ' + getNodeStateName( this.getState() );\n\
\n\
        if( bVerbose )\n\
        {\n\
            sInfo += ';  is container: ' + this.isContainer();\n\
\n\
            if( this.getBegin() )\n\
                sInfo += ';  begin: ' + this.getBegin().info();\n\
\n\
            if( this.getDuration() )\n\
                sInfo += ';  dur: ' + this.getDuration().info();\n\
\n\
            if( this.getEnd() )\n\
                sInfo += ';  end: ' + this.getEnd().info();\n\
\n\
            if( this.getFillMode() )\n\
                sInfo += ';  fill: ' + aFillModeOutMap[ this.getFillMode() ];\n\
\n\
            if( this.getRestartMode() )\n\
                sInfo += ';  restart: ' + aRestartModeOutMap[ this.getRestartMode() ];\n\
\n\
            if( this.getRepeatCount() && ( this.getRepeatCount() != 1.0 ) )\n\
                sInfo += ';  repeatCount: ' + this.getRepeatCount();\n\
\n\
            if( this.getAccelerateValue() )\n\
                sInfo += ';  accelerate: ' + this.getAccelerateValue();\n\
\n\
            if( this.getDecelerateValue() )\n\
                sInfo += ';  decelerate: ' + this.getDecelerateValue();\n\
\n\
            if( this.isAutoReverseEnabled() )\n\
                sInfo += ';  autoReverse: true';\n\
\n\
        }\n\
\n\
        return sInfo;\n\
    };\n\
\n\
    BaseNode.prototype.callInfo = function( sMethodName )\n\
    {\n\
        var sInfo = this.sClassName +\n\
                    '( ' + this.getId() +\n\
                    ', ' + getNodeStateName( this.getState() ) +\n\
                    ' ).' + sMethodName;\n\
        return sInfo;\n\
    };\n\
\n\
    BaseNode.prototype.DBG = function( sMessage, nTime )\n\
    {\n\
        ANIMDBG.print( sMessage, nTime );\n\
    };\n\
\n\
\n\
\n\
    function AnimationBaseNode( aAnimElem, aParentNode, aNodeContext )\n\
    {\n\
        AnimationBaseNode.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );\n\
\n\
        this.sClassName = 'AnimationBaseNode';\n\
        this.bIsContainer = false;\n\
        this.aTargetElement = null;\n\
        this.aAnimatedElement = null;\n\
        this.aActivity = null;\n\
\n\
        this.nMinFrameCount;\n\
        this.eAdditiveMode;\n\
\n\
    }\n\
    extend( AnimationBaseNode, BaseNode );\n\
\n\
\n\
    AnimationBaseNode.prototype.parseElement = function()\n\
    {\n\
        var bRet = AnimationBaseNode.superclass.parseElement.call( this );\n\
\n\
        var aAnimElem = this.aElement;\n\
\n\
        this.aTargetElement = null;\n\
        var sTargetElementAttr = aAnimElem.getAttribute( 'targetElement' );\n\
        if( sTargetElementAttr )\n\
            this.aTargetElement = document.getElementById( sTargetElementAttr );\n\
\n\
        if( !this.aTargetElement )\n\
        {\n\
            this.eCurrentState = INVALID_NODE;\n\
            log( 'AnimationBaseNode.parseElement: target element not found: ' + sTargetElementAttr );\n\
        }\n\
\n\
        var sAdditiveAttr = aAnimElem.getAttribute( 'additive' );\n\
        if( sAdditiveAttr && aAddittiveModeInMap[sAdditiveAttr] )\n\
            this.eAdditiveMode = aAddittiveModeInMap[sAdditiveAttr];\n\
        else\n\
            this.eAdditiveMode = ADDITIVE_MODE_REPLACE;\n\
\n\
        this.nMinFrameCount = ( this.getDuration().isValue() )\n\
                ? ( this.getDuration().getValue() * MINIMUM_FRAMES_PER_SECONDS )\n\
                : MINIMUM_FRAMES_PER_SECONDS;\n\
        if( this.nMinFrameCount < 1.0 )\n\
            this.nMinFrameCount = 1;\n\
        else if( this.nMinFrameCount > MINIMUM_FRAMES_PER_SECONDS )\n\
            this.nMinFrameCount = MINIMUM_FRAMES_PER_SECONDS;\n\
\n\
\n\
        if( this.aTargetElement )\n\
        {\n\
            if( true && aAnimElem.getAttribute( 'attributeName' ) === 'visibility' )\n\
            {\n\
                if( aAnimElem.getAttribute( 'to' ) === 'visible' )\n\
                    this.aTargetElement.setAttribute( 'visibility', 'hidden' );\n\
            }\n\
\n\
            if( !this.aNodeContext.aAnimatedElementMap[ sTargetElementAttr ] )\n\
            {\n\
                this.aNodeContext.aAnimatedElementMap[ sTargetElementAttr ]\n\
                        = new AnimatedElement( this.aTargetElement );\n\
            }\n\
            this.aAnimatedElement = this.aNodeContext.aAnimatedElementMap[ sTargetElementAttr ];\n\
\n\
            this.aAnimatedElement.setAdditiveMode( this.eAdditiveMode );\n\
        }\n\
\n\
\n\
        return bRet;\n\
    };\n\
\n\
    AnimationBaseNode.prototype.init_st = function()\n\
    {\n\
        if( this.aActivity )\n\
            this.aActivity.activate( makeEvent( bind( this, this.deactivate ) ) );\n\
        else\n\
            this.aActivity = this.createActivity();\n\
        return true;\n\
    };\n\
\n\
    AnimationBaseNode.prototype.resolve_st = function()\n\
    {\n\
        return true;\n\
    };\n\
\n\
    AnimationBaseNode.prototype.activate_st = function()\n\
    {\n\
        if( this.aActivity )\n\
        {\n\
            this.aActivity.setTargets( this.getAnimatedElement() );\n\
            this.getContext().aActivityQueue.addActivity( this.aActivity );\n\
        }\n\
        else\n\
        {\n\
";

static const char aSVGScript17[] =
"\
            AnimationBaseNode.superclass.scheduleDeactivationEvent.call( this );\n\
        }\n\
\n\
    };\n\
\n\
    AnimationBaseNode.prototype.deactivate_st = function( eDestState )\n\
    {\n\
        if( eDestState == FROZEN_NODE )\n\
        {\n\
            if( this.aActivity )\n\
                this.aActivity.end();\n\
        }\n\
        if( eDestState == ENDED_NODE )\n\
        {\n\
            if( this.aActivity )\n\
                this.aActivity.dispose();\n\
        }\n\
    };\n\
\n\
    AnimationBaseNode.prototype.createActivity = function()\n\
    {\n\
        log( 'AnimationBaseNode.createActivity: abstract method called' );\n\
    };\n\
\n\
    AnimationBaseNode.prototype.fillActivityParams = function()\n\
    {\n\
\n\
        var nDuration = 0.001;\n\
        if( this.getDuration().isValue() )\n\
        {\n\
            nDuration = this.getDuration().getValue();\n\
        }\n\
        else\n\
        {\n\
            log( 'AnimationBaseNode.fillActivityParams: duration is not a number' );\n\
        }\n\
\n\
        var aActivityParamSet = new ActivityParamSet();\n\
\n\
        aActivityParamSet.aEndEvent             = makeEvent( bind( this, this.deactivate ) );\n\
        aActivityParamSet.aTimerEventQueue      = this.aContext.aTimerEventQueue;\n\
        aActivityParamSet.aActivityQueue        = this.aContext.aActivityQueue;\n\
        aActivityParamSet.nMinDuration          = nDuration;\n\
        aActivityParamSet.nMinNumberOfFrames    = this.getMinFrameCount();\n\
        aActivityParamSet.bAutoReverse          = this.isAutoReverseEnabled();\n\
        aActivityParamSet.nRepeatCount          = this.getRepeatCount();\n\
        aActivityParamSet.nAccelerationFraction = this.getAccelerateValue();\n\
        aActivityParamSet.nDecelerationFraction = this.getDecelerateValue();\n\
        aActivityParamSet.nSlideWidth           = this.aNodeContext.aSlideWidth;\n\
        aActivityParamSet.nSlideHeight          = this.aNodeContext.aSlideHeight;\n\
\n\
        return aActivityParamSet;\n\
    };\n\
\n\
    AnimationBaseNode.prototype.hasPendingAnimation = function()\n\
    {\n\
        return true;\n\
    };\n\
\n\
    AnimationBaseNode.prototype.getTargetElement = function()\n\
    {\n\
        return this.aTargetElement;\n\
    };\n\
\n\
    AnimationBaseNode.prototype.getAnimatedElement = function()\n\
    {\n\
        return this.aAnimatedElement;\n\
    };\n\
\n\
    AnimationBaseNode.prototype.dispose= function()\n\
    {\n\
        if( this.aActivity )\n\
            this.aActivity.dispose();\n\
\n\
        AnimationBaseNode.superclass.dispose.call( this );\n\
    };\n\
\n\
    AnimationBaseNode.prototype.getMinFrameCount = function()\n\
    {\n\
        return this.nMinFrameCount;\n\
    };\n\
\n\
    AnimationBaseNode.prototype.getAdditiveMode = function()\n\
    {\n\
        return this.eAdditiveMode;\n\
    };\n\
\n\
    AnimationBaseNode.prototype.info = function( bVerbose )\n\
    {\n\
        var sInfo = AnimationBaseNode.superclass.info.call( this, bVerbose );\n\
\n\
        if( bVerbose )\n\
        {\n\
            if( this.getMinFrameCount() )\n\
                sInfo += ';  min frame count: ' + this.getMinFrameCount();\n\
\n\
            sInfo += ';  additive: ' + aAddittiveModeOutMap[ this.getAdditiveMode() ];\n\
\n\
            if( this.getShape() )\n\
            {\n\
                sElemId = this.getShape().getAttribute( 'id' );\n\
                sInfo += ';  targetElement: ' +  sElemId;\n\
            }\n\
        }\n\
\n\
        return sInfo;\n\
    };\n\
\n\
\n\
    function AnimationBaseNode2( aAnimElem, aParentNode, aNodeContext )\n\
    {\n\
        AnimationBaseNode2.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );\n\
\n\
        this.sAttributeName;\n\
        this.aToValue;\n\
\n\
    }\n\
    extend( AnimationBaseNode2, AnimationBaseNode );\n\
\n\
\n\
    AnimationBaseNode2.prototype.parseElement = function()\n\
    {\n\
        var bRet = AnimationBaseNode2.superclass.parseElement.call( this );\n\
\n\
        var aAnimElem = this.aElement;\n\
\n\
        this.sAttributeName = aAnimElem.getAttribute( 'attributeName' );\n\
        if( !this.sAttributeName )\n\
        {\n\
            this.eCurrentState = INVALID_NODE;\n\
            log( 'AnimationBaseNode2.parseElement: target attribute name not found: ' + this.sAttributeName );\n\
        }\n\
\n\
        this.aToValue = aAnimElem.getAttribute( 'to' );\n\
\n\
        return bRet;\n\
    };\n\
\n\
    AnimationBaseNode2.prototype.getAttributeName = function()\n\
    {\n\
        return this.sAttributeName;\n\
    };\n\
\n\
    AnimationBaseNode2.prototype.getToValue = function()\n\
    {\n\
        return this.aToValue;\n\
    };\n\
\n\
    AnimationBaseNode2.prototype.info = function( bVerbose )\n\
    {\n\
        var sInfo = AnimationBaseNode2.superclass.info.call( this, bVerbose );\n\
\n\
        if( bVerbose )\n\
        {\n\
            if( this.getAttributeName() )\n\
                sInfo += ';  attributeName: ' + this.getAttributeName();\n\
\n\
            if( this.getToValue() )\n\
                sInfo += ';  to: ' + this.getToValue();\n\
        }\n\
\n\
        return sInfo;\n\
    };\n\
\n\
\n\
\n\
    function AnimationBaseNode3( aAnimElem, aParentNode, aNodeContext )\n\
    {\n\
        AnimationBaseNode3.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );\n\
\n\
        this.eAccumulate;\n\
        this.eCalcMode;\n\
        this.aFromValue;\n\
        this.aByValue;\n\
        this.aKeyTimes;\n\
        this.aValues;\n\
\n\
    }\n\
    extend( AnimationBaseNode3, AnimationBaseNode2 );\n\
\n\
\n\
    AnimationBaseNode3.prototype.parseElement = function()\n\
    {\n\
        var bRet = AnimationBaseNode3.superclass.parseElement.call( this );\n\
\n\
        var aAnimElem = this.aElement;\n\
\n\
        this.eAccumulate = ACCUMULATE_MODE_NONE;\n\
        var sAccumulateAttr = aAnimElem.getAttribute( 'accumulate' );\n\
        if( sAccumulateAttr == 'sum' )\n\
            this.eAccumulate = ACCUMULATE_MODE_SUM;\n\
\n\
        this.eCalcMode = CALC_MODE_LINEAR;\n\
        var sCalcModeAttr = aAnimElem.getAttribute( 'calcMode' );\n\
        if( sCalcModeAttr && aCalcModeInMap[ sCalcModeAttr ] )\n\
            this.eCalcMode = aCalcModeInMap[ sCalcModeAttr ];\n\
\n\
        this.aFromValue = aAnimElem.getAttribute( 'from' );\n\
\n\
        this.aByValue = aAnimElem.getAttribute( 'by' );\n\
";

static const char aSVGScript18[] =
"\
\n\
        this.aKeyTimes = new Array();\n\
        var sKeyTimesAttr = aAnimElem.getAttribute( 'keyTimes' );\n\
        sKeyTimesAttr = removeWhiteSpaces( sKeyTimesAttr );\n\
        if( sKeyTimesAttr )\n\
        {\n\
            var aKeyTimes = sKeyTimesAttr.split( ';' );\n\
            for( var i = 0; i < aKeyTimes.length; ++i )\n\
                this.aKeyTimes.push( parseFloat( aKeyTimes[i] ) );\n\
        }\n\
\n\
        var sValuesAttr = aAnimElem.getAttribute( 'values' );\n\
        if( sValuesAttr )\n\
        {\n\
            this.aValues = sValuesAttr.split( ';' );\n\
        }\n\
        else\n\
        {\n\
            this.aValues = new Array();\n\
        }\n\
\n\
        return bRet;\n\
    };\n\
\n\
    AnimationBaseNode3.prototype.getAccumulate = function()\n\
    {\n\
        return this.eAccumulate;\n\
    };\n\
\n\
    AnimationBaseNode3.prototype.getCalcMode = function()\n\
    {\n\
        return this.eCalcMode;\n\
    };\n\
\n\
    AnimationBaseNode3.prototype.getFromValue = function()\n\
    {\n\
        return this.aFromValue;\n\
    };\n\
\n\
    AnimationBaseNode3.prototype.getByValue = function()\n\
    {\n\
        return this.aByValue;\n\
    };\n\
\n\
    AnimationBaseNode3.prototype.getKeyTimes = function()\n\
    {\n\
        return this.aKeyTimes;\n\
    };\n\
\n\
    AnimationBaseNode3.prototype.getValues = function()\n\
    {\n\
        return this.aValues;\n\
    };\n\
\n\
    AnimationBaseNode3.prototype.info = function( bVerbose )\n\
    {\n\
        var sInfo = AnimationBaseNode3.superclass.info.call( this, bVerbose );\n\
\n\
        if( bVerbose )\n\
        {\n\
            if( this.getAccumulate() )\n\
                sInfo += ';  accumulate: ' + aAccumulateModeOutMap[ this.getAccumulate() ];\n\
\n\
            sInfo += ';  calcMode: ' + aCalcModeOutMap[ this.getCalcMode() ];\n\
\n\
            if( this.getFromValue() )\n\
                sInfo += ';  from: ' + this.getFromValue();\n\
\n\
            if( this.getByValue() )\n\
                sInfo += ';  by: ' + this.getByValue();\n\
\n\
            if( this.getKeyTimes().length )\n\
                sInfo += ';  keyTimes: ' + this.getKeyTimes().join( ',' );\n\
\n\
            if( this.getKeyTimes().length )\n\
                sInfo += ';  values: ' + this.getValues().join( ',' );\n\
        }\n\
\n\
        return sInfo;\n\
    };\n\
\n\
\n\
\n\
    function BaseContainerNode( aAnimElem, aParentNode, aNodeContext )\n\
    {\n\
        BaseContainerNode.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );\n\
\n\
        this.sClassName = 'BaseContainerNode';\n\
        this.bIsContainer = true;\n\
        this.aChildrenArray = new Array();\n\
        this.nFinishedChildren = 0;\n\
        this.bDurationIndefinite = false;\n\
\n\
        this.eImpressNodeType = undefined;\n\
        this.ePresetClass =  undefined;\n\
        this.ePresetId =  undefined;\n\
    }\n\
    extend( BaseContainerNode, BaseNode );\n\
\n\
\n\
    BaseContainerNode.prototype.parseElement= function()\n\
    {\n\
        var bRet = BaseContainerNode.superclass.parseElement.call( this );\n\
\n\
        var aAnimElem = this.aElement;\n\
\n\
        this.eImpressNodeType = IMPRESS_DEFAULT_NODE;\n\
        var sNodeTypeAttr = aAnimElem.getAttribute( 'node-type' );\n\
        if( sNodeTypeAttr && aImpressNodeTypeInMap[ sNodeTypeAttr ] )\n\
            this.eImpressNodeType = aImpressNodeTypeInMap[ sNodeTypeAttr ];\n\
        this.bMainSequenceRootNode = ( this.eImpressNodeType == IMPRESS_MAIN_SEQUENCE_NODE );\n\
\n\
        this.ePresetClass =  undefined;\n\
        var sPresetClassAttr = aAnimElem.getAttribute( 'preset-class' );\n\
        if( sPresetClassAttr && aPresetClassInMap[ sPresetClassAttr ] )\n\
            this.ePresetClass = aPresetClassInMap[ sPresetClassAttr ];\n\
\n\
        this.ePresetId =  undefined;\n\
        var sPresetIdAttr = aAnimElem.getAttribute( 'preset-id' );\n\
        if( sPresetIdAttr && aPresetIdInMap[ sPresetIdAttr ] )\n\
            this.ePresetId = aPresetIdInMap[ sPresetIdAttr ];\n\
\n\
\n\
        var nChildrenCount = this.aChildrenArray.length;\n\
        for( var i = 0; i < nChildrenCount; ++i )\n\
        {\n\
            this.aChildrenArray[i].parseElement();\n\
        }\n\
\n\
\n\
        this.bDurationIndefinite\n\
                = ( !this.getDuration() || this.getDuration().isIndefinite()  ) &&\n\
                  ( !this.getEnd() || ( this.getEnd().getType() != OFFSET_TIMING ) );\n\
\n\
        return bRet;\n\
    };\n\
\n\
    BaseContainerNode.prototype.appendChildNode = function( aAnimationNode )\n\
    {\n\
        if( ! this.checkValidNode() )\n\
            return ;\n\
\n\
        if( aAnimationNode.registerDeactivatingListener( this ) )\n\
            this.aChildrenArray.push( aAnimationNode );\n\
    };\n\
\n\
    BaseContainerNode.prototype.init_st = function()\n\
    {\n\
        this.nFinishedChildren = 0;\n\
        var nChildrenCount = this.aChildrenArray.length;\n\
        var nInitChildren = 0;\n\
        for( var i = 0; i < nChildrenCount; ++i )\n\
        {\n\
            if( this.aChildrenArray[i].init() )\n\
            {\n\
                ++nInitChildren;\n\
            }\n\
        }\n\
        return ( nChildrenCount == nInitChildren );\n\
    };\n\
\n\
    BaseContainerNode.prototype.deactivate_st = function( eDestState )\n\
    {\n\
        if( eDestState == FROZEN_NODE )\n\
        {\n\
            this.forEachChildNode( mem_fn( 'deactivate' ), ~( FROZEN_NODE | ENDED_NODE ) );\n\
        }\n\
        else\n\
        {\n\
            this.forEachChildNode( mem_fn( 'end' ), ~ENDED_NODE );\n\
        }\n\
    };\n\
\n\
    BaseContainerNode.prototype.hasPendingAnimation = function()\n\
    {\n\
        var nChildrenCount = this.aChildrenArray.length;\n\
        for( var i = 0; i < nChildrenCount; ++i )\n\
        {\n\
            if( this.aChildrenArray[i].hasPendingAnimation() )\n\
                return true;\n\
        }\n\
        return false;\n\
    };\n\
\n\
    BaseContainerNode.prototype.activate_st = function()\n\
    {\n\
        log( 'BaseContainerNode.activate_st: abstract method called' );\n\
    };\n\
\n\
    BaseContainerNode.prototype.notifyDeactivating = function( aAnimationNode )\n\
    {\n\
        log( 'BaseContainerNode.notifyDeactivating: abstract method called' );\n\
    };\n\
\n\
    BaseContainerNode.prototype.isDurationIndefinite = function()\n\
    {\n\
        return this.bDurationIndefinite;\n\
    };\n\
\n\
    BaseContainerNode.prototype.isChildNode = function( aAnimationNode )\n\
";

static const char aSVGScript19[] =
"\
    {\n\
        var nChildrenCount = this.aChildrenArray.length;\n\
        for( var i = 0; i < nChildrenCount; ++i )\n\
        {\n\
            if( this.aChildrenArray[i].getId() == aAnimationNode.getId() )\n\
                return true;\n\
        }\n\
        return false;\n\
    };\n\
\n\
    BaseContainerNode.prototype.notifyDeactivatedChild = function( aChildNode )\n\
    {\n\
        assert( ( aChildNode.getState() == FROZEN_NODE ) || ( aChildNode.getState() == ENDED_NODE ),\n\
        'BaseContainerNode.notifyDeactivatedChild: passed child node is neither in FROZEN nor in ENDED state' );\n\
\n\
        assert( this.getState() != INVALID_NODE,\n\
        'BaseContainerNode.notifyDeactivatedChild: this node is invalid' );\n\
\n\
        if( !this.isChildNode( aChildNode ) )\n\
        {\n\
            log( 'BaseContainerNode.notifyDeactivatedChild: unknown child notifier!' );\n\
            return false;\n\
        }\n\
\n\
        var nChildrenCount = this.aChildrenArray.length;\n\
\n\
        assert( ( this.nFinishedChildren < nChildrenCount ),\n\
        'BaseContainerNode.notifyDeactivatedChild: assert(this.nFinishedChildren < nChildrenCount) failed' );\n\
\n\
        ++this.nFinishedChildren;\n\
        var bFinished = ( this.nFinishedChildren >= nChildrenCount );\n\
\n\
        if( bFinished && this.isDurationIndefinite() )\n\
        {\n\
            this.deactivate();\n\
        }\n\
\n\
        return bFinished;\n\
    };\n\
\n\
    BaseContainerNode.prototype.forEachChildNode = function( aFunction, eNodeStateMask )\n\
    {\n\
        if( !eNodeStateMask )\n\
            eNodeStateMask = -1;\n\
\n\
        var nChildrenCount = this.aChildrenArray.length;\n\
        for( var i = 0; i < nChildrenCount; ++i )\n\
        {\n\
            if( ( eNodeStateMask != -1 ) && ( ( this.aChildrenArray[i].getState() & eNodeStateMask ) == 0 ) )\n\
                continue;\n\
            aFunction( this.aChildrenArray[i] );\n\
        }\n\
    };\n\
\n\
    BaseContainerNode.prototype.dispose = function()\n\
    {\n\
        var nChildrenCount = this.aChildrenArray.length;\n\
        for( var i = 0; i < nChildrenCount; ++i )\n\
        {\n\
            this.aChildrenArray[i].dispose();\n\
        }\n\
\n\
        BaseContainerNode.superclass.dispose.call( this );\n\
    };\n\
\n\
    BaseContainerNode.prototype.getImpressNodeType = function()\n\
    {\n\
        return this.eImpressNodeType;\n\
    };\n\
\n\
    BaseContainerNode.prototype.info = function( bVerbose )\n\
    {\n\
        var sInfo = BaseContainerNode.superclass.info.call( this, bVerbose );\n\
\n\
        if( bVerbose )\n\
        {\n\
            if( this.getImpressNodeType() )\n\
                sInfo += ';  node-type: ' + aImpressNodeTypeOutMap[ this.getImpressNodeType() ];\n\
        }\n\
\n\
        var nChildrenCount = this.aChildrenArray.length;\n\
        for( var i = 0; i < nChildrenCount; ++i )\n\
        {\n\
            sInfo += '\\n';\n\
            sInfo += this.aChildrenArray[i].info( bVerbose );\n\
        }\n\
\n\
        return sInfo;\n\
    };\n\
\n\
    function ParallelTimeContainer( aAnimElem, aParentNode, aNodeContext )\n\
    {\n\
        ParallelTimeContainer.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );\n\
\n\
        this.sClassName = 'ParallelTimeContainer';\n\
    }\n\
    extend( ParallelTimeContainer, BaseContainerNode );\n\
\n\
\n\
    ParallelTimeContainer.prototype.activate_st = function()\n\
    {\n\
        var nChildrenCount = this.aChildrenArray.length;\n\
        var nResolvedChildren = 0;\n\
        for( var i = 0; i < nChildrenCount; ++i )\n\
        {\n\
            if( this.aChildrenArray[i].resolve() )\n\
            {\n\
                ++nResolvedChildren;\n\
            }\n\
        }\n\
\n\
        if( nChildrenCount != nResolvedChildren )\n\
        {\n\
            log( 'ParallelTimeContainer.activate_st: resolving all children failed' );\n\
            return;\n\
        }\n\
\n\
\n\
        if( this.isDurationIndefinite() && ( nChildrenCount == 0  ) )\n\
        {\n\
            this.scheduleDeactivationEvent( this.makeDeactivationEvent( 0.0 ) );\n\
        }\n\
        else\n\
        {\n\
            this.scheduleDeactivationEvent();\n\
        }\n\
    };\n\
\n\
    ParallelTimeContainer.prototype.notifyDeactivating = function( aAnimationNode )\n\
    {\n\
        this.notifyDeactivatedChild( aAnimationNode );\n\
    };\n\
\n\
\n\
\n\
    function SequentialTimeContainer( aAnimElem, aParentNode, aNodeContext )\n\
    {\n\
        SequentialTimeContainer.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );\n\
\n\
        this.sClassName = 'SequentialTimeContainer';\n\
    }\n\
    extend( SequentialTimeContainer, BaseContainerNode );\n\
\n\
\n\
    SequentialTimeContainer.prototype.activate_st = function()\n\
    {\n\
        var nChildrenCount = this.aChildrenArray.length;\n\
        for( ; this.nFinishedChildren < nChildrenCount; ++this.nFinishedChildren )\n\
        {\n\
            if( this.resolveChild( this.aChildrenArray[ this.nFinishedChildren ] ) )\n\
                break;\n\
            else\n\
                log( 'SequentialTimeContainer.activate_st: resolving child failed!' );\n\
        }\n\
\n\
        if( this.isDurationIndefinite() && ( ( nChildrenCount == 0 ) || ( this.nFinishedChildren >= nChildrenCount ) ) )\n\
        {\n\
            this.scheduleDeactivationEvent( this.makeDeactivationEvent( 0.0 ) );\n\
        }\n\
        else\n\
        {\n\
            this.scheduleDeactivationEvent();\n\
        }\n\
    };\n\
\n\
    SequentialTimeContainer.prototype.notifyDeactivating = function( aNotifier )\n\
    {\n\
        if( this.notifyDeactivatedChild( aNotifier ) )\n\
            return;\n\
\n\
        assert( this.nFinishedChildren < this.aChildrenArray.length,\n\
        'SequentialTimeContainer.notifyDeactivating: assertion (this.nFinishedChildren < this.aChildrenArray.length) failed' );\n\
\n\
        var aNextChild = this.aChildrenArray[ this.nFinishedChildren ];\n\
\n\
        assert( aNextChild.getState() == UNRESOLVED_NODE,\n\
        'SequentialTimeContainer.notifyDeactivating: assertion (aNextChild.getState == UNRESOLVED_NODE) failed' );\n\
\n\
        if( !this.resolveChild( aNextChild ) )\n\
        {\n\
            this.deactivate();\n\
        }\n\
    };\n\
\n\
    SequentialTimeContainer.prototype.skipEffect = function( aChildNode )\n\
    {\n\
    };\n\
\n\
    SequentialTimeContainer.prototype.rewindEffect = function( aChildNode )\n\
    {\n\
    };\n\
\n\
    SequentialTimeContainer.prototype.resolveChild = function( aChildNode )\n\
    {\n\
        var bResolved = aChildNode.resolve();\n\
\n\
        if( bResolved && this.isMainSequenceRootNode() )\n\
        {\n\
        }\n\
        return bResolved;\n\
";

static const char aSVGScript20[] =
"\
    };\n\
\n\
\n\
\n\
    function PropertyAnimationNode(  aAnimElem, aParentNode, aNodeContext )\n\
    {\n\
        PropertyAnimationNode.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );\n\
\n\
        this.sClassName = 'PropertyAnimationNode';\n\
    }\n\
    extend( PropertyAnimationNode, AnimationBaseNode3 );\n\
\n\
\n\
    PropertyAnimationNode.prototype.createActivity = function()\n\
    {\n\
\n\
\n\
\n\
\n\
\n\
\n\
        var aActivityParamSet = this.fillActivityParams();\n\
\n\
        var aAnimation = createPropertyAnimation( this.getAttributeName(),\n\
                                                  this.getAnimatedElement(),\n\
                                                  this.aNodeContext.aSlideWidth,\n\
                                                  this.aNodeContext.aSlideHeight );\n\
\n\
        var aInterpolator = null;  // createActivity will compute it;\n\
        return createActivity( aActivityParamSet, this, aAnimation, aInterpolator );\n\
\n\
    };\n\
\n\
\n\
\n\
    function AnimationSetNode(  aAnimElem, aParentNode, aNodeContext )\n\
    {\n\
        AnimationSetNode.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );\n\
\n\
        this.sClassName = 'AnimationSetNode';\n\
    }\n\
    extend( AnimationSetNode, AnimationBaseNode2 );\n\
\n\
\n\
    AnimationSetNode.prototype.createActivity = function()\n\
    {\n\
        var aAnimation = createPropertyAnimation( this.getAttributeName(),\n\
                                                  this.getAnimatedElement(),\n\
                                                  this.aNodeContext.aSlideWidth,\n\
                                                  this.aNodeContext.aSlideHeight );\n\
\n\
        var aActivityParamSet = this.fillActivityParams();\n\
\n\
        return new SetActivity( aActivityParamSet, aAnimation, this.getToValue() );\n\
    };\n\
\n\
\n\
\n\
    function AnimationColorNode(  aAnimElem, aParentNode, aNodeContext )\n\
    {\n\
        AnimationColorNode.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );\n\
\n\
        this.sClassName = 'AnimationColorNode';\n\
\n\
        this.eColorInterpolation;\n\
        this.eColorInterpolationDirection;\n\
    }\n\
    extend( AnimationColorNode, AnimationBaseNode3 );\n\
\n\
\n\
    AnimationColorNode.prototype.parseElement = function()\n\
    {\n\
        var bRet = AnimationColorNode.superclass.parseElement.call( this );\n\
\n\
        var aAnimElem = this.aElement;\n\
\n\
        this.eColorInterpolation = COLOR_SPACE_RGB;\n\
        var sColorInterpolationAttr = aAnimElem.getAttribute( 'color-interpolation' );\n\
        if( sColorInterpolationAttr && aColorSpaceInMap[ sColorInterpolationAttr ] )\n\
            this.eColorInterpolation = aColorSpaceInMap[ sColorInterpolationAttr ];\n\
\n\
        this.eColorInterpolationDirection = CLOCKWISE;\n\
        var sColorInterpolationDirectionAttr = aAnimElem.getAttribute( 'color-interpolation-direction' );\n\
        if( sColorInterpolationDirectionAttr && aClockDirectionInMap[ sColorInterpolationDirectionAttr ] )\n\
            this.eColorInterpolationDirection = aClockDirectionInMap[ sColorInterpolationDirectionAttr ];\n\
\n\
        return bRet;\n\
    };\n\
\n\
    AnimationColorNode.prototype.createActivity = function()\n\
    {\n\
\n\
\n\
\n\
        var aActivityParamSet = this.fillActivityParams();\n\
\n\
        var aAnimation = createPropertyAnimation( this.getAttributeName(),\n\
                                                  this.getAnimatedElement(),\n\
                                                  this.aNodeContext.aSlideWidth,\n\
                                                  this.aNodeContext.aSlideHeight );\n\
\n\
        var aColorAnimation;\n\
        var aInterpolator;\n\
        if( this.getColorInterpolation() === COLOR_SPACE_HSL )\n\
        {\n\
            ANIMDBG.print( 'AnimationColorNode.createActivity: color space hsl'  );\n\
            aColorAnimation = new HSLAnimationWrapper( aAnimation );\n\
            var aInterpolatorMaker = aInterpolatorHandler.getInterpolator( this.getCalcMode(),\n\
                                                                           COLOR_PROPERTY,\n\
                                                                           COLOR_SPACE_HSL );\n\
            aInterpolator = aInterpolatorMaker( this.getColorInterpolationDirection() );\n\
        }\n\
        else\n\
        {\n\
            ANIMDBG.print( 'AnimationColorNode.createActivity: color space rgb'  );\n\
            aColorAnimation = aAnimation;\n\
            aInterpolator = aInterpolatorHandler.getInterpolator( this.getCalcMode(),\n\
                                                                  COLOR_PROPERTY,\n\
                                                                  COLOR_SPACE_RGB );\n\
        }\n\
\n\
        return createActivity( aActivityParamSet, this, aColorAnimation, aInterpolator );\n\
\n\
\n\
    };\n\
\n\
    AnimationColorNode.prototype.getColorInterpolation = function()\n\
    {\n\
        return this.eColorInterpolation;\n\
    };\n\
\n\
    AnimationColorNode.prototype.getColorInterpolationDirection = function()\n\
    {\n\
        return this.eColorInterpolationDirection;\n\
    };\n\
\n\
    AnimationColorNode.prototype.info = function( bVerbose )\n\
    {\n\
        var sInfo = AnimationColorNode.superclass.info.call( this, bVerbose );\n\
\n\
        if( bVerbose )\n\
        {\n\
            sInfo += ';  color-interpolation: ' + aColorSpaceOutMap[ this.getColorInterpolation() ];\n\
\n\
            sInfo += ';  color-interpolation-direction: ' + aClockDirectionOutMap[ this.getColorInterpolationDirection() ];\n\
        }\n\
        return sInfo;\n\
    };\n\
\n\
\n\
\n\
    function AnimationTransitionFilterNode(  aAnimElem, aParentNode, aNodeContext )\n\
    {\n\
        AnimationTransitionFilterNode.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );\n\
\n\
        this.sClassName = 'AnimationTransitionFilterNode';\n\
\n\
        this.eTransitionType;\n\
        this.eTransitionSubType;\n\
        this.bReverseDirection;\n\
        this.eTransitionMode;\n\
    }\n\
    extend( AnimationTransitionFilterNode, AnimationBaseNode );\n\
\n\
\n\
    AnimationTransitionFilterNode.prototype.createActivity = function()\n\
    {\n\
        var aActivityParamSet = this.fillActivityParams();\n\
\n\
        var aAnimation = createPropertyAnimation( 'opacity',\n\
                this.getAnimatedElement(),\n\
                this.aNodeContext.aSlideWidth,\n\
                this.aNodeContext.aSlideHeight );\n\
\n\
        var eDirection = this.getTransitionMode() ? FORWARD : BACKWARD;\n\
\n\
        return new SimpleActivity( aActivityParamSet, aAnimation, eDirection );\n\
\n\
    };\n\
\n\
    AnimationTransitionFilterNode.prototype.parseElement = function()\n\
    {\n\
        var bRet = AnimationTransitionFilterNode.superclass.parseElement.call( this );\n\
\n\
        var aAnimElem = this.aElement;\n\
\n\
        this.eTransitionType = undefined;\n\
        var sTypeAttr = aAnimElem.getAttribute( 'type' );\n\
        if( sTypeAttr && aTransitionTypeInMap[ sTypeAttr ] )\n\
        {\n\
            this.eTransitionType = aTransitionTypeInMap[ sTypeAttr ];\n\
        }\n\
        else\n\
        {\n\
            this.eCurrentState = INVALID_NODE;\n\
            log( 'AnimationTransitionFilterNode.parseElement: transition type not valid: ' + sTypeAttr );\n\
        }\n\
\n\
        this.eTransitionSubType = undefined;\n\
        var sSubTypeAttr = aAnimElem.getAttribute( 'subtype' );\n\
";

static const char aSVGScript21[] =
"\
        if( sSubTypeAttr && aTransitionSubtypeInMap[ sSubTypeAttr ] )\n\
        {\n\
            this.eTransitionSubType = aTransitionSubtypeInMap[ sSubTypeAttr ];\n\
        }\n\
        else\n\
        {\n\
            this.eCurrentState = INVALID_NODE;\n\
            log( 'AnimationTransitionFilterNode.parseElement: transition subtype not valid: ' + sSubTypeAttr );\n\
        }\n\
\n\
        this.bReverseDirection = false;\n\
        var sDirectionAttr = aAnimElem.getAttribute( 'direction' );\n\
        if( sDirectionAttr == 'reverse' )\n\
            this.bReverseDirection = true;\n\
\n\
        this.eTransitionMode = TRANSITION_MODE_IN;\n\
        var sModeAttr = aAnimElem.getAttribute( 'mode' );\n\
        if( sModeAttr === 'out' )\n\
            this.eTransitionMode = TRANSITION_MODE_OUT;\n\
\n\
        return bRet;\n\
    };\n\
\n\
    AnimationTransitionFilterNode.prototype.getTransitionType = function()\n\
    {\n\
        return this.eTransitionType;\n\
    };\n\
\n\
    AnimationTransitionFilterNode.prototype.getTransitionSubType = function()\n\
    {\n\
        return this.eTransitionSubType;\n\
    };\n\
\n\
    AnimationTransitionFilterNode.prototype.getTransitionMode = function()\n\
    {\n\
        return this.eTransitionMode;\n\
    };\n\
\n\
    AnimationTransitionFilterNode.prototype.getReverseDirection = function()\n\
    {\n\
        return this.bReverseDirection;\n\
    };\n\
\n\
    AnimationTransitionFilterNode.prototype.info = function( bVerbose )\n\
    {\n\
        var sInfo = AnimationTransitionFilterNode.superclass.info.call( this, bVerbose );\n\
\n\
        if( bVerbose )\n\
        {\n\
            sInfo += ';  type: ' + aTransitionTypeOutMap[ String( this.getTransitionType() ) ];\n\
\n\
            sInfo += ';  subtype: ' + aTransitionSubtypeOutMap[ this.getTransitionSubType() ];\n\
\n\
            if( this.getReverseDirection() )\n\
                sInfo += ';  direction: reverse';\n\
        }\n\
\n\
        return sInfo;\n\
    };\n\
\n\
\n\
\n\
\n\
    function createAnimationTree( aRootElement, aNodeContext )\n\
    {\n\
        return createAnimationNode( aRootElement, null, aNodeContext );\n\
    }\n\
\n\
\n\
\n\
    function createAnimationNode( aElement, aParentNode, aNodeContext )\n\
    {\n\
        assert( aElement, 'createAnimationNode: invalid animation element' );\n\
\n\
        var eAnimationNodeType = getAnimationElementType( aElement );\n\
\n\
        var aCreatedNode = null;\n\
        var aCreatedContainer = null;\n\
\n\
        switch( eAnimationNodeType )\n\
        {\n\
        case ANIMATION_NODE_PAR:\n\
            aCreatedNode = aCreatedContainer =\n\
                new ParallelTimeContainer( aElement, aParentNode, aNodeContext );\n\
            break;\n\
        case ANIMATION_NODE_ITERATE:\n\
            aCreatedNode = aCreatedContainer =\n\
                new ParallelTimeContainer( aElement, aParentNode, aNodeContext );\n\
            break;\n\
        case ANIMATION_NODE_SEQ:\n\
            aCreatedNode = aCreatedContainer =\n\
                new SequentialTimeContainer( aElement, aParentNode, aNodeContext );\n\
            break;\n\
        case ANIMATION_NODE_ANIMATE:\n\
            aCreatedNode = new PropertyAnimationNode( aElement, aParentNode, aNodeContext );\n\
            break;\n\
        case ANIMATION_NODE_SET:\n\
            aCreatedNode = new AnimationSetNode( aElement, aParentNode, aNodeContext );\n\
            break;\n\
        case ANIMATION_NODE_ANIMATEMOTION:\n\
            log( 'createAnimationNode: ANIMATEMOTION not implemented' );\n\
            return null;\n\
        case ANIMATION_NODE_ANIMATECOLOR:\n\
            aCreatedNode = new AnimationColorNode( aElement, aParentNode, aNodeContext );\n\
            break;\n\
        case ANIMATION_NODE_ANIMATETRANSFORM:\n\
            log( 'createAnimationNode: ANIMATETRANSFORM not implemented' );\n\
            return null;\n\
        case ANIMATION_NODE_TRANSITIONFILTER:\n\
            aCreatedNode = new AnimationTransitionFilterNode( aElement, aParentNode, aNodeContext );\n\
            break;\n\
        default:\n\
            log( 'createAnimationNode: invalid Animation Node Type: ' + eAnimationNodeType );\n\
            return null;\n\
        }\n\
\n\
        if( aCreatedContainer )\n\
        {\n\
            if( eAnimationNodeType == ANIMATION_NODE_ITERATE )\n\
            {\n\
                createIteratedNodes( aElement, aCreatedContainer, aNodeContext );\n\
            }\n\
            else\n\
            {\n\
                var aChildrenArray = getElementChildren( aElement );\n\
                for( var i = 0; i < aChildrenArray.length; ++i )\n\
                {\n\
                    if( !createChildNode( aChildrenArray[i], aCreatedContainer, aNodeContext ) )\n\
                    {\n\
                        return null;\n\
                    }\n\
                }\n\
            }\n\
        }\n\
\n\
        return aCreatedNode;\n\
    }\n\
\n\
\n\
\n\
    function createChildNode( aElement, aParentNode, aNodeContext )\n\
    {\n\
        var aChildNode = createAnimationNode( aElement, aParentNode, aNodeContext );\n\
\n\
        if( !aChildNode )\n\
        {\n\
            log( 'createChildNode: child node creation failed' );\n\
            return false;\n\
        }\n\
        else\n\
        {\n\
            aParentNode.appendChildNode( aChildNode );\n\
            return true;\n\
        }\n\
    }\n\
\n\
\n\
\n\
    function createIteratedNodes( aElement, aContainerNode, aNodeContext )\n\
    {\n\
    }\n\
\n\
\n\
\n\
\n\
\n\
    function makeScaler( nScale )\n\
    {\n\
        if( ( typeof( nScale ) !== typeof( 0 ) ) || !isFinite( nScale ) )\n\
        {\n\
            log( 'makeScaler: not valid param passed: ' + nScale );\n\
            return null;\n\
        }\n\
\n\
        return  function( nValue )\n\
                {\n\
                    return ( nScale * nValue );\n\
                };\n\
    }\n\
\n\
\n\
\n\
    function createPropertyAnimation( sAttrName, aAnimatedElement, nWidth, nHeight )\n\
    {\n\
        if( !aAttributeMap[ sAttrName ] )\n\
        {\n\
            log( 'createPropertyAnimation: attribute is unknown' );\n\
            return;\n\
        }\n\
\n\
\n\
        var aFunctorSet = aAttributeMap[ sAttrName ];\n\
\n\
        var sGetValueMethod =   aFunctorSet.get;\n\
        var sSetValueMethod =   aFunctorSet.set;\n\
\n\
        if( !sGetValueMethod || !sSetValueMethod  )\n\
        {\n\
            log( 'createPropertyAnimation: attribute is not handled' );\n\
            return;\n\
";

static const char aSVGScript22[] =
"\
        }\n\
\n\
        var aGetModifier =  eval( aFunctorSet.getmod );\n\
        var aSetModifier =  eval( aFunctorSet.setmod );\n\
\n\
\n\
        return new GenericAnimation( bind( aAnimatedElement, aAnimatedElement[ sGetValueMethod ] ),\n\
                                     bind( aAnimatedElement, aAnimatedElement[ sSetValueMethod ] ),\n\
                                     aGetModifier,\n\
                                     aSetModifier);\n\
    }\n\
\n\
\n\
\n\
    function GenericAnimation( aGetValueFunc, aSetValueFunc, aGetModifier, aSetModifier )\n\
    {\n\
        assert( aGetValueFunc && aSetValueFunc,\n\
                'GenericAnimation constructor: get value functor and/or set value functor are not valid' );\n\
\n\
        this.aGetValueFunc = aGetValueFunc;\n\
        this.aSetValueFunc = aSetValueFunc;\n\
        this.aGetModifier = aGetModifier;\n\
        this.aSetModifier = aSetModifier;\n\
        this.aAnimatableElement = null;\n\
        this.bAnimationStarted = false;\n\
    }\n\
\n\
\n\
    GenericAnimation.prototype.start = function( aAnimatableElement )\n\
    {\n\
        assert( aAnimatableElement, 'GenericAnimation.start: animatable element is not valid' );\n\
\n\
        this.aAnimatableElement = aAnimatableElement;\n\
        this.aAnimatableElement.notifyAnimationStart();\n\
\n\
        if( !this.bAnimationStarted )\n\
            this.bAnimationStarted = true;\n\
    };\n\
\n\
    GenericAnimation.prototype.end = function()\n\
    {\n\
        if( this.bAnimationStarted )\n\
            this.bAnimationStarted = false;\n\
    };\n\
\n\
    GenericAnimation.prototype.perform = function( aValue )\n\
    {\n\
        if( this.aSetModifier )\n\
            aValue = this.aSetModifier( aValue );\n\
\n\
        this.aSetValueFunc( aValue );\n\
    };\n\
\n\
    GenericAnimation.prototype.getUnderlyingValue = function()\n\
    {\n\
        var aValue = this.aGetValueFunc();\n\
        if( this.aGetModifier )\n\
            aValue = this.aGetModifier( aValue );\n\
        return aValue;\n\
    };\n\
\n\
\n\
\n\
    function HSLAnimationWrapper( aColorAnimation )\n\
    {\n\
        assert( aColorAnimation,\n\
                'HSLAnimationWrapper constructor: invalid color animation delegate' );\n\
\n\
        this.aAnimation = aColorAnimation;\n\
    }\n\
\n\
\n\
    HSLAnimationWrapper.prototype.start = function( aAnimatableElement )\n\
    {\n\
        this.aAnimation.start( aAnimatableElement );\n\
    };\n\
\n\
    HSLAnimationWrapper.prototype.end = function()\n\
    {\n\
        this.aAnimation.end();\n\
    };\n\
    HSLAnimationWrapper.prototype.perform = function( aHSLValue )\n\
    {\n\
        this.aAnimation.perform( aHSLValue.convertToRGB() );\n\
    };\n\
\n\
    HSLAnimationWrapper.prototype.getUnderlyingValue = function()\n\
    {\n\
        return this.aAnimation.getUnderlyingValue().convertToHSL();\n\
    };\n\
\n\
\n\
\n\
    function AnimatedElement( aElement )\n\
    {\n\
        if( !aElement )\n\
        {\n\
            log( 'AnimatedElement constructor: element is not valid' );\n\
        }\n\
\n\
        this.aActiveElement = aElement;\n\
        this.initElement();\n\
\n\
        this.aBaseBBox = this.aActiveElement.getBBox();\n\
        this.nBaseCenterX = this.aBaseBBox.x + this.aBaseBBox.width / 2;\n\
        this.nBaseCenterY = this.aBaseBBox.y + this.aBaseBBox.height / 2;\n\
        this.nCenterX = this.nBaseCenterX;\n\
        this.nCenterY = this.nBaseCenterY;\n\
        this.nScaleFactorX = 1.0;\n\
        this.nScaleFactorY = 1.0;\n\
\n\
        this.aPreviousElement = null;\n\
        this.aElementArray = new Array();\n\
        this.nCurrentState = 0;\n\
        this.eAdditiveMode = ADDITIVE_MODE_REPLACE;\n\
        this.bIsUpdated = true;\n\
\n\
        this.aTMatrix = document.documentElement.createSVGMatrix();\n\
        this.aCTM = document.documentElement.createSVGMatrix();\n\
        this.aICTM = document.documentElement.createSVGMatrix();\n\
        this.setCTM();\n\
\n\
        this.aElementArray[0] = this.aActiveElement.cloneNode( true );\n\
    }\n\
\n\
    AnimatedElement.prototype.initElement = function()\n\
    {\n\
        this.aActiveElement.setAttribute( 'transform', makeMatrixString( 1, 0, 0, 1, 0, 0 ) );\n\
    };\n\
\n\
    AnimatedElement.prototype.getId = function()\n\
    {\n\
        return this.aActiveElement.getAttribute( 'id' );\n\
    };\n\
\n\
    AnimatedElement.prototype.isUpdated = function()\n\
    {\n\
        return this.bIsUpdated;\n\
    };\n\
\n\
    AnimatedElement.prototype.getAdditiveMode = function()\n\
    {\n\
        return this.eAdditiveMode;\n\
    };\n\
\n\
    AnimatedElement.prototype.setAdditiveMode = function( eAdditiveMode )\n\
    {\n\
        this.eAdditiveMode = eAdditiveMode;\n\
    };\n\
\n\
    AnimatedElement.prototype.setToElement = function( aElement )\n\
    {\n\
        if( !aElement )\n\
        {\n\
            log( 'AnimatedElement(' + this.getId() + ').setToElement: element is not valid' );\n\
            return false;\n\
        }\n\
\n\
        var aClone = aElement.cloneNode( true );\n\
        this.aPreviousElement = this.aActiveElement.parentNode.replaceChild( aClone, this.aActiveElement );\n\
        this.aActiveElement = aClone;\n\
\n\
        return true;\n\
    };\n\
\n\
    AnimatedElement.prototype.notifySlideStart = function()\n\
    {\n\
        this.setToFirst();\n\
        this.DBG( '.notifySlideStart invoked' );\n\
    };\n\
\n\
    AnimatedElement.prototype.notifyAnimationStart = function()\n\
    {\n\
\n\
        this.DBG( '.notifyAnimationStart invoked' );\n\
        this.bIsUpdated = false;\n\
    };\n\
\n\
    AnimatedElement.prototype.notifyAnimationEnd = function()\n\
    {\n\
    };\n\
\n\
    AnimatedElement.prototype.notifyNextEffectStart = function( nEffectIndex )\n\
    {\n\
        assert( this.nCurrentState === nEffectIndex,\n\
                'AnimatedElement(' + this.getId() + ').notifyNextEffectStart: assertion (current state == effect index) failed' );\n\
\n\
        if( this.isUpdated() )\n\
        {\n\
            if( !this.aElementArray[ nEffectIndex ] )\n\
            {\n\
                this.aElementArray[ nEffectIndex ] =  this.aElementArray[ this.nCurrentState ];\n\
                this.DBG( '.notifyNextEffectStart(' + nEffectIndex + '): new state set to previous one ' );\n\
            }\n\
        }\n\
        else\n\
        {\n\
            if( !this.aElementArray[ nEffectIndex ] )\n\
            {\n\
                this.aElementArray[ nEffectIndex ] = this.aActiveElement.cloneNode( true );\n\
";

static const char aSVGScript23[] =
"\
                this.DBG( '.notifyNextEffectStart(' + nEffectIndex + '): cloned active state ' );\n\
            }\n\
        }\n\
        ++this.nCurrentState;\n\
    };\n\
\n\
    AnimatedElement.prototype.setToFirst = function()\n\
    {\n\
        this.setTo( 0 );\n\
    };\n\
\n\
    AnimatedElement.prototype.setToLast = function()\n\
    {\n\
        this.setTo( this.aElementArray.length - 1 );\n\
    };\n\
\n\
    AnimatedElement.prototype.setTo = function( nEffectIndex )\n\
    {\n\
        var bRet = this.setToElement( this.aElementArray[ nEffectIndex ] );\n\
        if( bRet )\n\
        {\n\
            this.nCurrentState = nEffectIndex;\n\
\n\
            var aBBox = this.getBBox();\n\
            var aBaseBBox = this.getBaseBBox();\n\
            this.nCenterX = aBBox.x + aBBox.width / 2;\n\
            this.nCenterY = aBBox.y + aBBox.height / 2;\n\
            this.nScaleFactorX = aBBox.width / aBaseBBox.width;\n\
            this.nScaleFactorY = aBBox.height / aBaseBBox.height;\n\
        }\n\
    };\n\
\n\
    AnimatedElement.prototype.getBaseBBox = function()\n\
    {\n\
        return this.aBaseBBox;\n\
    };\n\
\n\
    AnimatedElement.prototype.getBaseCenterX = function()\n\
    {\n\
        return this.nBaseCenterX;\n\
    };\n\
\n\
    AnimatedElement.prototype.getBaseCenterY = function()\n\
    {\n\
        return this.nBaseCenterY;\n\
    };\n\
\n\
    AnimatedElement.prototype.getBBox = function()\n\
    {\n\
        return this.aActiveElement.parentNode.getBBox();\n\
    };\n\
\n\
    AnimatedElement.prototype.getX = function()\n\
    {\n\
        return this.nCenterX;\n\
    };\n\
\n\
    AnimatedElement.prototype.getY = function()\n\
    {\n\
        return this.nCenterY;\n\
    };\n\
\n\
    AnimatedElement.prototype.getWidth = function()\n\
    {\n\
        return this.nScaleFactorX * this.getBaseBBox().width;\n\
    };\n\
\n\
    AnimatedElement.prototype.getHeight = function()\n\
    {\n\
        return this.nScaleFactorY * this.getBaseBBox().height;\n\
    };\n\
\n\
    AnimatedElement.prototype.setCTM = function()\n\
    {\n\
\n\
        this.aICTM.e = this.getBaseCenterX();\n\
        this.aICTM.f = this.getBaseCenterY();\n\
\n\
        this.aCTM.e = -this.aICTM.e;\n\
        this.aCTM.f = -this.aICTM.f;\n\
    };\n\
\n\
    AnimatedElement.prototype.updateTransformAttribute = function()\n\
    {\n\
        this.aTransformAttrList = this.aActiveElement.transform.baseVal;\n\
        this.aTransformAttr = this.aTransformAttrList.getItem( 0 );\n\
        this.aTransformAttr.setMatrix( this.aTMatrix );\n\
    };\n\
\n\
    AnimatedElement.prototype.setX = function( nXNewPos )\n\
    {\n\
        this.aTransformAttrList = this.aActiveElement.transform.baseVal;\n\
        this.aTransformAttr = this.aTransformAttrList.getItem( 0 );\n\
        this.aTransformAttr.matrix.e += ( nXNewPos - this.getX() );\n\
        this.nCenterX = nXNewPos;\n\
    };\n\
\n\
    AnimatedElement.prototype.setY = function( nYNewPos )\n\
    {\n\
        this.aTransformAttrList = this.aActiveElement.transform.baseVal;\n\
        this.aTransformAttr = this.aTransformAttrList.getItem( 0 );\n\
        this.aTransformAttr.matrix.f += ( nYNewPos - this.getY() );\n\
        this.nCenterY = nYNewPos;\n\
    };\n\
\n\
    AnimatedElement.prototype.setWidth = function( nNewWidth )\n\
    {\n\
        var nBaseWidth = this.getBaseBBox().width;\n\
        if( nBaseWidth <= 0 )\n\
            return;\n\
\n\
        this.nScaleFactorX = nNewWidth / nBaseWidth;\n\
        this.implScale();\n\
    };\n\
\n\
    AnimatedElement.prototype.setHeight = function( nNewHeight )\n\
    {\n\
        var nBaseHeight = this.getBaseBBox().height;\n\
        if( nBaseHeight <= 0 )\n\
            return;\n\
\n\
        this.nScaleFactorY = nNewHeight / nBaseHeight;\n\
        this.implScale();\n\
    };\n\
\n\
    AnimatedElement.prototype.implScale = function( )\n\
    {\n\
        this.aTMatrix = document.documentElement.createSVGMatrix();\n\
        this.aTMatrix.a = this.nScaleFactorX;\n\
        this.aTMatrix.d = this.nScaleFactorY;\n\
        this.aTMatrix = this.aICTM.multiply( this.aTMatrix.multiply( this.aCTM ) );\n\
\n\
        var nDeltaX = this.getX() - this.getBaseCenterX();\n\
        var nDeltaY = this.getY() - this.getBaseCenterY();\n\
        this.aTMatrix = this.aTMatrix.translate( nDeltaX, nDeltaY );\n\
        this.updateTransformAttribute();\n\
    };\n\
\n\
    AnimatedElement.prototype.setWidth2 = function( nNewWidth )\n\
    {\n\
        if( nNewWidth < 0 )\n\
            log( 'AnimatedElement(' + this.getId() + ').setWidth: negative width!' );\n\
        if( nNewWidth < 0.001 )\n\
            nNewWidth = 0.001;\n\
\n\
        this.setCTM();\n\
\n\
        var nCurWidth = this.getWidth();\n\
        if( nCurWidth <= 0 )\n\
            nCurWidth = 0.001;\n\
\n\
        var nScaleFactor = nNewWidth / nCurWidth;\n\
        if( nScaleFactor < 1e-5 )\n\
            nScaleFactor = 1e-5;\n\
        this.aTMatrix = document.documentElement.createSVGMatrix();\n\
        this.aTMatrix.a = nScaleFactor;\n\
        this.aTMatrix = this.aICTM.multiply( this.aTMatrix.multiply( this.aCTM ) );\n\
        this.updateTransformAttribute();\n\
    };\n\
\n\
    AnimatedElement.prototype.setHeight2 = function( nNewHeight )\n\
    {\n\
        ANIMDBG.print( 'AnimatedElement.setHeight: nNewHeight = ' + nNewHeight );\n\
        if( nNewHeight < 0 )\n\
            log( 'AnimatedElement(' + this.getId() + ').setWidth: negative height!' );\n\
        if( nNewHeight < 0.001 )\n\
            nNewHeight = 0.001;\n\
\n\
        this.setCTM();\n\
\n\
        var nCurHeight = this.getHeight();\n\
        ANIMDBG.print( 'AnimatedElement.setHeight: nCurHeight = ' + nCurHeight );\n\
        if( nCurHeight <= 0 )\n\
            nCurHeight = 0.001;\n\
\n\
        var nScaleFactor = nNewHeight / nCurHeight;\n\
        ANIMDBG.print( 'AnimatedElement.setHeight: nScaleFactor = ' + nScaleFactor );\n\
        if( nScaleFactor < 1e-5 )\n\
            nScaleFactor = 1e-5;\n\
        this.aTMatrix = document.documentElement.createSVGMatrix();\n\
        this.aTMatrix.d = nScaleFactor;\n\
        this.aTMatrix = this.aICTM.multiply( this.aTMatrix.multiply( this.aCTM ) );\n\
        this.updateTransformAttribute();\n\
    };\n\
\n\
    AnimatedElement.prototype.getOpacity = function()\n\
    {\n\
        return this.aActiveElement.getAttribute( 'opacity' );\n\
    };\n\
\n\
    AnimatedElement.prototype.setOpacity = function( nValue )\n\
    {\n\
        this.aActiveElement.setAttribute( 'opacity', nValue );\n\
    };\n\
\n\
    AnimatedElement.prototype.getVisibility = function()\n\
    {\n\
\n\
        var sVisibilityValue = this.aActiveElement.getAttribute( 'visibility' );\n\
        if( !sVisibilityValue || ( sVisibilityValue === 'inherit' ) )\n\
";

static const char aSVGScript24[] =
"\
            return 'visible'; // TODO: look for parent visibility!\n\
        else\n\
            return sVisibilityValue;\n\
    };\n\
\n\
    AnimatedElement.prototype.setVisibility = function( sValue )\n\
    {\n\
        if( sValue == 'visible' )\n\
            sValue = 'inherit';\n\
        this.aActiveElement.setAttribute( 'visibility', sValue );\n\
    };\n\
\n\
    AnimatedElement.prototype.getStrokeStyle = function()\n\
    {\n\
        return 'solid';\n\
    };\n\
\n\
    AnimatedElement.prototype.setStrokeStyle = function( sValue )\n\
    {\n\
        ANIMDBG.print( 'AnimatedElement.setStrokeStyle(' + sValue + ')' );\n\
    };\n\
\n\
    AnimatedElement.prototype.getFillStyle = function()\n\
    {\n\
        return 'solid';\n\
    };\n\
\n\
    AnimatedElement.prototype.setFillStyle = function( sValue )\n\
    {\n\
        ANIMDBG.print( 'AnimatedElement.setFillStyle(' + sValue + ')' );\n\
    };\n\
\n\
    AnimatedElement.prototype.getFillColor = function()\n\
    {\n\
        var aChildSet = getElementChildren( this.aActiveElement );\n\
        var sFillColorValue = '';\n\
        for( var i = 0; i <  aChildSet.length; ++i )\n\
        {\n\
            sFillColorValue = aChildSet[i].getAttribute( 'fill' );\n\
            if( sFillColorValue && ( sFillColorValue !== 'none' ) )\n\
                break;\n\
        }\n\
\n\
        return colorParser( sFillColorValue );\n\
    };\n\
\n\
    AnimatedElement.prototype.setFillColor = function( aRGBValue )\n\
    {\n\
        assert( aRGBValue instanceof RGBColor,\n\
                'AnimatedElement.setFillColor: value argument is not an instance of RGBColor' );\n\
\n\
        var sValue = aRGBValue.toString( true /* clamped values */ );\n\
        var aChildSet = getElementChildren( this.aActiveElement );\n\
\n\
        var sFillColorValue = '';\n\
        for( var i = 0; i <  aChildSet.length; ++i )\n\
        {\n\
            sFillColorValue = aChildSet[i].getAttribute( 'fill' );\n\
            if( sFillColorValue && ( sFillColorValue !== 'none' ) )\n\
            {\n\
                aChildSet[i].setAttribute( 'fill', sValue );\n\
            }\n\
        }\n\
    };\n\
\n\
    AnimatedElement.prototype.getStrokeColor = function()\n\
    {\n\
        var aChildSet = getElementChildren( this.aActiveElement );\n\
        var sStrokeColorValue = '';\n\
        for( var i = 0; i <  aChildSet.length; ++i )\n\
        {\n\
            sStrokeColorValue = aChildSet[i].getAttribute( 'stroke' );\n\
            if( sStrokeColorValue && ( sStrokeColorValue !== 'none' ) )\n\
                break;\n\
        }\n\
\n\
        return colorParser( sStrokeColorValue );\n\
    };\n\
\n\
    AnimatedElement.prototype.setStrokeColor = function( aRGBValue )\n\
    {\n\
        assert( aRGBValue instanceof RGBColor,\n\
                'AnimatedElement.setFillColor: value argument is not an instance of RGBColor' );\n\
\n\
        var sValue = aRGBValue.toString( true /* clamped values */ );\n\
        var aChildSet = getElementChildren( this.aActiveElement );\n\
\n\
        var sStrokeColorValue = '';\n\
        for( var i = 0; i <  aChildSet.length; ++i )\n\
        {\n\
            sStrokeColorValue = aChildSet[i].getAttribute( 'stroke' );\n\
            if( sStrokeColorValue && ( sStrokeColorValue !== 'none' ) )\n\
            {\n\
                aChildSet[i].setAttribute( 'stroke', sValue );\n\
            }\n\
        }\n\
    };\n\
\n\
    AnimatedElement.prototype.getFontColor = function()\n\
    {\n\
        return new RGBColor( 0, 0, 0 );\n\
    };\n\
\n\
    AnimatedElement.prototype.setFontColor = function( sValue )\n\
    {\n\
        ANIMDBG.print( 'AnimatedElement.setFontColor(' + sValue + ')' );\n\
    };\n\
\n\
    AnimatedElement.prototype.DBG = function( sMessage, nTime )\n\
    {\n\
        aAnimatedElementDebugPrinter.print( 'AnimatedElement(' + this.getId() + ')' + sMessage, nTime );\n\
    };\n\
\n\
\n\
\n\
\n\
    function SlideAnimations( aSlideShowContext )\n\
    {\n\
        this.aContext = new NodeContext( aSlideShowContext );\n\
        this.aAnimationNodeMap = new Object();\n\
        this.aAnimatedElementMap = new Object();\n\
        this.aSourceEventElementMap = new Object();\n\
        this.aNextEffectEventArray = new NextEffectEventArray();\n\
        this.aEventMultiplexer = new EventMultiplexer( aSlideShowContext.aTimerEventQueue );\n\
        this.aRootNode = null;\n\
        this.bElementsParsed = false;\n\
\n\
        this.aContext.aAnimationNodeMap = this.aAnimationNodeMap;\n\
        this.aContext.aAnimatedElementMap = this.aAnimatedElementMap;\n\
        this.aContext.aSourceEventElementMap = this.aSourceEventElementMap;\n\
    }\n\
\n\
\n\
    SlideAnimations.prototype.importAnimations = function( aAnimationRootElement )\n\
    {\n\
        if( !aAnimationRootElement )\n\
            return false;\n\
\n\
        this.aRootNode = createAnimationTree( aAnimationRootElement, this.aContext );\n\
\n\
        return ( this.aRootNode ? true : false );\n\
    };\n\
\n\
    SlideAnimations.prototype.parseElements = function()\n\
    {\n\
        if( !this.aRootNode )\n\
            return false;\n\
\n\
        if( !this.aRootNode.parseElement() )\n\
            return false;\n\
        else\n\
            this.bElementsParsed = true;\n\
    };\n\
\n\
    SlideAnimations.prototype.elementsParsed = function()\n\
    {\n\
        return this.bElementsParsed;\n\
    };\n\
\n\
    SlideAnimations.prototype.isFirstRun = function()\n\
    {\n\
        return this.aContext.bFirstRun;\n\
    };\n\
\n\
    SlideAnimations.prototype.isAnimated = function()\n\
    {\n\
        if( !this.bElementsParsed )\n\
            return false;\n\
\n\
        return this.aRootNode.hasPendingAnimation();\n\
    };\n\
\n\
    SlideAnimations.prototype.start = function()\n\
    {\n\
        if( !this.bElementsParsed )\n\
            return false;\n\
\n\
        aSlideShow.setSlideEvents( this.aNextEffectEventArray, this.aEventMultiplexer );\n\
\n\
        if( this.aContext.bFirstRun == undefined )\n\
            this.aContext.bFirstRun = true;\n\
        else if( this.aContext.bFirstRun )\n\
            this.aContext.bFirstRun = false;\n\
\n\
        if( !this.aRootNode.init() )\n\
            return false;\n\
\n\
        if( !this.aRootNode.resolve() )\n\
            return false;\n\
\n\
        return true;\n\
    };\n\
\n\
    SlideAnimations.prototype.end = function( bLeftEffectsSkipped )\n\
    {\n\
        if( !this.bElementsParsed )\n\
            return; // no animations there\n\
\n\
        this.aRootNode.deactivate();\n\
        this.aRootNode.end();\n\
";

static const char aSVGScript25[] =
"\
\n\
        if( bLeftEffectsSkipped && this.isFirstRun() )\n\
        {\n\
            this.aContext.bFirstRun = undefined;\n\
        }\n\
        else if( this.isFirstRun() )\n\
        {\n\
            this.aContext.bFirstRun = false;\n\
        }\n\
\n\
    };\n\
\n\
    SlideAnimations.prototype.dispose = function()\n\
    {\n\
        if( this.aRootNode )\n\
        {\n\
            this.aRootNode.dispose();\n\
        }\n\
    };\n\
\n\
    SlideAnimations.prototype.clearNextEffectEvents = function()\n\
    {\n\
        ANIMDBG.print( 'SlideAnimations.clearNextEffectEvents: current slide: ' + nCurSlide );\n\
        this.aNextEffectEventArray.clear();\n\
        this.aContext.bFirstRun = undefined;\n\
    };\n\
\n\
\n\
\n\
\n\
    function Event()\n\
    {\n\
        this.nId = Event.getUniqueId();\n\
    }\n\
\n\
\n\
    Event.CURR_UNIQUE_ID = 0;\n\
\n\
    Event.getUniqueId = function()\n\
    {\n\
        ++Event.CURR_UNIQUE_ID;\n\
        return Event.CURR_UNIQUE_ID;\n\
    };\n\
\n\
    Event.prototype.getId = function()\n\
    {\n\
        return this.nId;\n\
    };\n\
\n\
\n\
    function DelayEvent( aFunctor, nTimeout )\n\
    {\n\
        DelayEvent.superclass.constructor.call( this );\n\
\n\
        this.aFunctor = aFunctor;\n\
        this.nTimeout = nTimeout;\n\
        this.bWasFired = false;\n\
    }\n\
    extend( DelayEvent, Event );\n\
\n\
\n\
    DelayEvent.prototype.fire = function()\n\
    {\n\
        assert( this.isCharged(), 'DelayEvent.fire: assertion isCharged failed' );\n\
\n\
        this.bWasFired = true;\n\
        this.aFunctor();\n\
        return true;\n\
    };\n\
\n\
    DelayEvent.prototype.isCharged = function()\n\
    {\n\
        return !this.bWasFired;\n\
    };\n\
\n\
    DelayEvent.prototype.getActivationTime = function( nCurrentTime )\n\
    {\n\
        return ( this.nTimeout + nCurrentTime );\n\
    };\n\
\n\
    DelayEvent.prototype.dispose = function()\n\
    {\n\
        if( this.isCharged() )\n\
            this.bWasFired = true;\n\
    };\n\
\n\
    DelayEvent.prototype.charge = function()\n\
    {\n\
        if( !this.isCharged() )\n\
            this.bWasFired = false;\n\
    };\n\
\n\
\n\
    function makeEvent( aFunctor )\n\
    {\n\
        return new DelayEvent( aFunctor, 0.0 );\n\
    }\n\
\n\
\n\
\n\
    function makeDelay( aFunctor, nTimeout )\n\
    {\n\
        return new DelayEvent( aFunctor, nTimeout );\n\
    }\n\
\n\
\n\
\n\
    function registerEvent( aTiming, aEvent, aNodeContext )\n\
    {\n\
        var aSlideShowContext = aNodeContext.aContext;\n\
        var eTimingType = aTiming.getType();\n\
\n\
        registerEvent.DBG( aTiming );\n\
\n\
        if( eTimingType == OFFSET_TIMING )\n\
        {\n\
            aSlideShowContext.aTimerEventQueue.addEvent( aEvent );\n\
        }\n\
        else if ( aNodeContext.bFirstRun )\n\
        {\n\
            var aEventMultiplexer = aSlideShowContext.aEventMultiplexer;\n\
            if( !aEventMultiplexer )\n\
            {\n\
                log( 'registerEvent: event multiplexer not initialized' );\n\
                return;\n\
            }\n\
            var aNextEffectEventArray = aSlideShowContext.aNextEffectEventArray;\n\
            if( !aNextEffectEventArray )\n\
            {\n\
                log( 'registerEvent: next effect event array not initialized' );\n\
                return;\n\
            }\n\
            switch( eTimingType )\n\
            {\n\
                case EVENT_TIMING:\n\
                    var eEventType = aTiming.getEventType();\n\
                    var sEventBaseElemId = aTiming.getEventBaseElementId();\n\
                    if( sEventBaseElemId )\n\
                    {\n\
                        var aEventBaseElem = document.getElementById( sEventBaseElemId );\n\
                        if( !aEventBaseElem )\n\
                        {\n\
                            log( 'generateEvent: EVENT_TIMING: event base element not found: ' + sEventBaseElemId );\n\
                            return;\n\
                        }\n\
                        var aSourceEventElement = aNodeContext.makeSourceEventElement( sEventBaseElemId, aEventBaseElem );\n\
\n\
                        var bEventRegistered = false;\n\
                        switch( eEventType )\n\
                        {\n\
                            case EVENT_TRIGGER_ON_CLICK:\n\
                                aEventMultiplexer.registerEvent( eEventType, aSourceEventElement.getId(), aEvent );\n\
                                bEventRegistered = true;\n\
                                break;\n\
                            default:\n\
                                log( 'generateEvent: not handled event type: ' + eEventType );\n\
                        }\n\
                        if( bEventRegistered )\n\
                            aSourceEventElement.addEventListener( eEventType  );\n\
                    }\n\
                    else  // no base event element present\n\
                    {\n\
                        switch( eEventType )\n\
                        {\n\
                            case EVENT_TRIGGER_ON_NEXT_EFFECT:\n\
                                aNextEffectEventArray.appendEvent( aEvent );\n\
                                break;\n\
                            default:\n\
                                log( 'generateEvent: not handled event type: ' + eEventType );\n\
                        }\n\
                    }\n\
                    break;\n\
                case SYNCBASE_TIMING:\n\
                    var eEventType = aTiming.getEventType();\n\
                    var sEventBaseElemId = aTiming.getEventBaseElementId();\n\
                    if( sEventBaseElemId )\n\
                    {\n\
                        var aAnimationNode = aNodeContext.aAnimationNodeMap[ sEventBaseElemId ];\n\
                        if( !aAnimationNode )\n\
                        {\n\
                            log( 'generateEvent: SYNCBASE_TIMING: event base element not found: ' + sEventBaseElemId );\n\
                            return;\n\
                        }\n\
                        aEventMultiplexer.registerEvent( eEventType, aAnimationNode.getId(), aEvent );\n\
                    }\n\
                    else\n\
                    {\n\
                        log( 'generateEvent: SYNCBASE_TIMING: event base element not specified' );\n\
                    }\n\
                    break;\n\
                default:\n\
                    log( 'generateEvent: not handled timing type: ' + eTimingType );\n\
            }\n\
        }\n\
    }\n\
\n\
    registerEvent.DEBUG = aRegisterEventDebugPrinter.isEnabled();\n\
\n\
    registerEvent.DBG = function( aTiming, nTime )\n\
    {\n\
";

static const char aSVGScript26[] =
"\
        if( registerEvent.DEBUG )\n\
        {\n\
            aRegisterEventDebugPrinter.print( 'registerEvent( timing: ' + aTiming.info() + ' )', nTime );\n\
        }\n\
    };\n\
\n\
\n\
\n\
    function SourceEventElement( aElement, aEventMulyiplexer )\n\
    {\n\
        this.nId = getUniqueId();\n\
        this.aElement = aElement;\n\
        this.aEventMulyiplexer = aEventMulyiplexer;\n\
        this.aEventListenerStateArray = new Array();\n\
    }\n\
\n\
\n\
    SourceEventElement.prototype.getId = function()\n\
    {\n\
        return this.nId;\n\
    };\n\
\n\
    SourceEventElement.prototype.isEqualTo = function( aSourceEventElement )\n\
    {\n\
        return ( this.getId() == aSourceEventElement.getId() );\n\
    };\n\
\n\
    SourceEventElement.prototype.onClick = function()\n\
    {\n\
        aEventMulyiplexer.notifyClickEvent( this );\n\
    };\n\
\n\
    SourceEventElement.prototype.isEventListenerRegistered = function( eEventType )\n\
    {\n\
        return this.aEventListenerStateArray[ eEventType ];\n\
    };\n\
\n\
    SourceEventElement.prototype.addEventListener = function( eEventType )\n\
    {\n\
        if( !this.aElement )\n\
            return false;\n\
\n\
        this.aEventListenerStateArray[ eEventType ] = true;\n\
        switch( eEventType )\n\
        {\n\
            case EVENT_TRIGGER_ON_CLICK:\n\
                this.aElement.addEventListener( 'click', this.onClick, false );\n\
                break;\n\
            default:\n\
                log( 'SourceEventElement.addEventListener: not handled event type: ' + eEventType );\n\
                return false;\n\
        }\n\
        return true;\n\
    };\n\
\n\
    SourceEventElement.prototype.removeEventListener = function( eEventType )\n\
    {\n\
        if( !this.aElement )\n\
            return false;\n\
\n\
        this.aEventListenerStateArray[ eEventType ] = false;\n\
        switch( eEventType )\n\
        {\n\
            case EVENT_TRIGGER_ON_CLICK:\n\
                this.aElement.removeEventListener( 'click', this.onClick, false );\n\
                break;\n\
            default:\n\
                log( 'SourceEventElement.removeEventListener: not handled event type: ' + eEventType );\n\
                return false;\n\
        }\n\
        return true;\n\
    };\n\
\n\
\n\
    function EventMultiplexer( aTimerEventQueue )\n\
    {\n\
        this.aTimerEventQueue = aTimerEventQueue;\n\
        this.aEventMap = new Object();\n\
\n\
    }\n\
\n\
\n\
    EventMultiplexer.prototype.registerEvent = function( eEventType, aNotifierId, aEvent )\n\
    {\n\
        this.DBG( 'registerEvent', eEventType, aNotifierId );\n\
        if( !this.aEventMap[ eEventType ] )\n\
        {\n\
            this.aEventMap[ eEventType ] = new Object();\n\
        }\n\
        if( !this.aEventMap[ eEventType ][ aNotifierId ] )\n\
        {\n\
            this.aEventMap[ eEventType ][ aNotifierId ] = new Array();\n\
        }\n\
        this.aEventMap[ eEventType ][ aNotifierId ].push( aEvent );\n\
    };\n\
\n\
\n\
    EventMultiplexer.prototype.notifyEvent = function( eEventType, aNotifierId )\n\
    {\n\
        this.DBG( 'notifyEvent', eEventType, aNotifierId );\n\
        if( this.aEventMap[ eEventType ] )\n\
        {\n\
            if( this.aEventMap[ eEventType ][ aNotifierId ] )\n\
            {\n\
                var aEventArray = this.aEventMap[ eEventType ][ aNotifierId ];\n\
                var nSize = aEventArray.length;\n\
                for( var i = 0; i < nSize; ++i )\n\
                {\n\
                    this.aTimerEventQueue.addEvent( aEventArray[i] );\n\
                }\n\
            }\n\
        }\n\
    };\n\
\n\
    EventMultiplexer.DEBUG = aEventMultiplexerDebugPrinter.isEnabled();\n\
\n\
    EventMultiplexer.prototype.DBG = function( sMethodName, eEventType, aNotifierId, nTime )\n\
    {\n\
        if( EventMultiplexer.DEBUG )\n\
        {\n\
            var sInfo = 'EventMultiplexer.' + sMethodName;\n\
            sInfo += '( type: ' + aEventTriggerOutMap[ eEventType ];\n\
            sInfo += ', notifier: ' + aNotifierId + ' )';\n\
            aEventMultiplexerDebugPrinter.print( sInfo, nTime );\n\
        }\n\
    };\n\
\n\
\n\
\n\
\n\
    var aInterpolatorHandler = new Object();\n\
\n\
    aInterpolatorHandler.getInterpolator = function( eCalcMode, eValueType, eValueSubtype )\n\
    {\n\
        var bHasSubtype = ( typeof( eValueSubtype ) === typeof( 0 ) );\n\
\n\
        if( !bHasSubtype && aInterpolatorHandler.aLerpFunctorMap[ eCalcMode ][ eValueType ] )\n\
        {\n\
            return aInterpolatorHandler.aLerpFunctorMap[ eCalcMode ][ eValueType ];\n\
        }\n\
        else if( bHasSubtype && aInterpolatorHandler.aLerpFunctorMap[ eCalcMode ][ eValueType ][ eValueSubtype ] )\n\
        {\n\
            return aInterpolatorHandler.aLerpFunctorMap[ eCalcMode ][ eValueType ][ eValueSubtype ];\n\
        }\n\
        else\n\
        {\n\
            log( 'aInterpolatorHandler.getInterpolator: not found any valid interpolator for clalc mode '\n\
                    + aCalcModeOutMap[eCalcMode]  + 'and value type ' + aValueTypeOutMap[eValueType]  );\n\
            return null;\n\
        }\n\
    };\n\
\n\
    aInterpolatorHandler.aLerpFunctorMap = new Array();\n\
    aInterpolatorHandler.aLerpFunctorMap[ CALC_MODE_DISCRETE ] = new Array();\n\
    aInterpolatorHandler.aLerpFunctorMap[ CALC_MODE_LINEAR ] = new Array();\n\
\n\
\n\
\n\
    aInterpolatorHandler.aLerpFunctorMap[ CALC_MODE_LINEAR ][ NUMBER_PROPERTY ] =\n\
        function ( nFrom, nTo, nT )\n\
        {\n\
            return ( ( 1.0 - nT )* nFrom + nT * nTo );\n\
        };\n\
\n\
    aInterpolatorHandler.aLerpFunctorMap[ CALC_MODE_LINEAR ][ COLOR_PROPERTY ] = new Array();\n\
\n\
    aInterpolatorHandler.aLerpFunctorMap[ CALC_MODE_LINEAR ][ COLOR_PROPERTY ][ COLOR_SPACE_RGB ] =\n\
        function ( nFrom, nTo, nT )\n\
        {\n\
            return RGBColor.interpolate( nFrom, nTo, nT );\n\
        };\n\
\n\
    aInterpolatorHandler.aLerpFunctorMap[ CALC_MODE_LINEAR ][ COLOR_PROPERTY ][ COLOR_SPACE_HSL ] =\n\
        function ( bCCW  )\n\
        {\n\
            return  function ( nFrom, nTo, nT )\n\
                    {\n\
                        return HSLColor.interpolate( nFrom, nTo, nT, bCCW );\n\
                    };\n\
        };\n\
\n\
\n\
\n\
    function KeyStopLerp( aValueList )\n\
    {\n\
        KeyStopLerp.validateInput( aValueList );\n\
\n\
        this.aKeyStopList = new Array();\n\
        this.nLastIndex = 0;\n\
        this.nKeyStopDistance = aValueList[1] - aValueList[0];\n\
        if( this.nKeyStopDistance <= 0 )\n\
            this.nKeyStopDistance = 0.001;\n\
\n\
        for( var i = 0; i < aValueList.length; ++i )\n\
            this.aKeyStopList.push( aValueList[i] );\n\
\n\
        this.nUpperBoundIndex = this.aKeyStopList.length - 2;\n\
    }\n\
\n\
\n\
";

static const char aSVGScript27[] =
"\
    KeyStopLerp.validateInput = function( aValueList )\n\
    {\n\
        var nSize = aValueList.length;\n\
        assert( nSize > 1, 'KeyStopLerp.validateInput: key stop vector must have two entries or more' );\n\
\n\
        for( var i = 1; i < nSize; ++i )\n\
        {\n\
            if( aValueList[i-1] > aValueList[i] )\n\
                log( 'KeyStopLerp.validateInput: time vector is not sorted in ascending order!' );\n\
        }\n\
    };\n\
\n\
    KeyStopLerp.prototype.reset = function()\n\
    {\n\
        KeyStopLerp.validateInput( this.aKeyStopList );\n\
        this.nLastIndex = 0;\n\
        this.nKeyStopDistance = this.aKeyStopList[1] - this.aKeyStopList[0];\n\
        if( this.nKeyStopDistance <= 0 )\n\
            this.nKeyStopDistance = 0.001;\n\
\n\
    };\n\
\n\
    KeyStopLerp.prototype.lerp = function( nAlpha )\n\
    {\n\
        if( nAlpha > this.aKeyStopList[ this.nLastIndex + 1 ] )\n\
        {\n\
            do\n\
            {\n\
                var nIndex = this.nLastIndex + 1;\n\
                this.nLastIndex = clamp( nIndex, 0, this.nUpperBoundIndex );\n\
                this.nKeyStopDistance = this.aKeyStopList[ this.nLastIndex + 1 ] - this.aKeyStopList[ this.nLastIndex ];\n\
            }\n\
            while( ( this.nKeyStopDistance <= 0 ) && ( this.nLastIndex < this.nUpperBoundIndex ) );\n\
        }\n\
\n\
        var nRawLerp = ( nAlpha - this.aKeyStopList[ this.nLastIndex ] ) / this.nKeyStopDistance;\n\
\n\
        nRawLerp = clamp( nRawLerp, 0.0, 1.0 );\n\
\n\
        var aResult = new Object();\n\
        aResult.nIndex = this.nLastIndex;\n\
        aResult.nLerp = nRawLerp;\n\
\n\
        return aResult;\n\
    };\n\
\n\
    KeyStopLerp.prototype.lerp_ported = function( nAlpha )\n\
    {\n\
        if( ( this.aKeyStopList[ this.nLastIndex ] < nAlpha ) ||\n\
            ( this.aKeyStopList[ this.nLastIndex + 1 ] >= nAlpha ) )\n\
        {\n\
            var i = 0;\n\
            for( ; i < this.aKeyStopList.length; ++i )\n\
            {\n\
                if( this.aKeyStopList[i] >= nAlpha )\n\
                    break;\n\
            }\n\
            if( this.aKeyStopList[i] > nAlpha )\n\
                --i;\n\
            var nIndex = i - 1;\n\
            this.nLastIndex = clamp( nIndex, 0, this.aKeyStopList.length - 2 );\n\
        }\n\
\n\
        var nRawLerp = ( nAlpha - this.aKeyStopList[ this.nLastIndex ] ) /\n\
                        ( this.aKeyStopList[ this.nLastIndex+1 ] - this.aKeyStopList[ this.nLastIndex ] );\n\
\n\
        nRawLerp = clamp( nRawLerp, 0.0, 1.0 );\n\
\n\
        var aResult = new Object();\n\
        aResult.nIndex = this.nLastIndex;\n\
        aResult.nLerp = nRawLerp;\n\
\n\
        return aResult;\n\
    };\n\
\n\
\n\
\n\
\n\
    var aOperatorSetMap = new Array();\n\
\n\
    aOperatorSetMap[ NUMBER_PROPERTY ] = new Object();\n\
\n\
    aOperatorSetMap[ NUMBER_PROPERTY ].add = function( a, b )\n\
    {\n\
        return ( a + b );\n\
    };\n\
\n\
    aOperatorSetMap[ NUMBER_PROPERTY ].scale = function( k, v )\n\
    {\n\
        return ( k * v );\n\
    };\n\
\n\
    aOperatorSetMap[ COLOR_PROPERTY ] = new Object();\n\
\n\
    aOperatorSetMap[ COLOR_PROPERTY ].add = function( a, b )\n\
    {\n\
        var c = a.clone();\n\
        c.add( b );\n\
        return c;\n\
    };\n\
\n\
    aOperatorSetMap[ COLOR_PROPERTY ].scale = function( k, v )\n\
    {\n\
        var r = v.clone();\n\
        r.scale( k );\n\
        return r;\n\
    };\n\
\n\
\n\
\n\
\n\
    function ActivityParamSet()\n\
    {\n\
        this.aEndEvent = null;\n\
        this.aTimerEventQueue = null;\n\
        this.aActivityQueue = null;\n\
        this.nRepeatCount = 1.0;\n\
        this.nAccelerationFraction = 0.0;\n\
        this.nDecelerationFraction = 0.0;\n\
        this.bAutoReverse = false;\n\
        this.nMinDuration = undefined;\n\
        this.nMinNumberOfFrames = MINIMUM_FRAMES_PER_SECONDS;\n\
        this.aDiscreteTimes = new Array();\n\
    }\n\
\n\
    function AnimationActivity()\n\
    {\n\
        this.nId = AnimationActivity.getUniqueId();\n\
    }\n\
\n\
\n\
    AnimationActivity.CURR_UNIQUE_ID = 0;\n\
\n\
    AnimationActivity.getUniqueId = function()\n\
    {\n\
        ++AnimationActivity.CURR_UNIQUE_ID;\n\
        return AnimationActivity.CURR_UNIQUE_ID;\n\
    };\n\
\n\
    AnimationActivity.prototype.getId = function()\n\
    {\n\
        return this.nId;\n\
    };\n\
\n\
\n\
\n\
    function SetActivity( aCommonParamSet, aAnimation, aToAttr  )\n\
    {\n\
        SetActivity.superclass.constructor.call( this );\n\
\n\
        this.aAnimation = aAnimation;\n\
        this.aTargetElement = null;\n\
        this.aEndEvent = aCommonParamSet.aEndEvent;\n\
        this.aTimerEventQueue = aCommonParamSet.aTimerEventQueue;\n\
        this.aToAttr = aToAttr;\n\
        this.bIsActive = true;\n\
    }\n\
    extend( SetActivity, AnimationActivity );\n\
\n\
\n\
    SetActivity.prototype.activate = function( aEndEvent )\n\
    {\n\
        this.aEndEvent = aEndEvent;\n\
        this.bIsActive = true;\n\
    };\n\
\n\
    SetActivity.prototype.dispose = function()\n\
    {\n\
        this.bIsActive = false;\n\
        if( this.aEndEvent && this.aEndEvent.isCharged() )\n\
            this.aEndEvent.dispose();\n\
    };\n\
\n\
    SetActivity.prototype.calcTimeLag = function()\n\
    {\n\
        return 0.0;\n\
    };\n\
\n\
    SetActivity.prototype.perform = function()\n\
    {\n\
        if( !this.isActive() )\n\
            return false;\n\
\n\
        this.bIsActive = false;\n\
\n\
        if( this.aAnimation && this.aTargetElement )\n\
        {\n\
            this.aAnimation.start( this.aTargetElement );\n\
            this.aAnimation.perform( this.aToAttr );\n\
            this.aAnimation.end();\n\
        }\n\
\n\
        if( this.aEndEvent )\n\
            this.aTimerEventQueue.addEvent( this.aEndEvent );\n\
\n\
    };\n\
\n\
    SetActivity.prototype.isActive = function()\n\
    {\n\
        return this.bIsActive;\n\
";

static const char aSVGScript28[] =
"\
    };\n\
\n\
    SetActivity.prototype.dequeued = function()\n\
    {\n\
    };\n\
\n\
    SetActivity.prototype.end = function()\n\
    {\n\
        this.perform();\n\
    };\n\
\n\
    SetActivity.prototype.setTargets = function( aTargetElement )\n\
    {\n\
        assert( aTargetElement, 'SetActivity.setTargets: target element is not valid' );\n\
        this.aTargetElement = aTargetElement;\n\
    };\n\
\n\
\n\
\n\
    function ActivityBase( aCommonParamSet )\n\
    {\n\
        ActivityBase.superclass.constructor.call( this );\n\
\n\
        this.aTargetElement = null;\n\
        this.aEndEvent = aCommonParamSet.aEndEvent;\n\
        this.aTimerEventQueue = aCommonParamSet.aTimerEventQueue;\n\
        this.nRepeats = aCommonParamSet.nRepeatCount;\n\
        this.nAccelerationFraction = aCommonParamSet.nAccelerationFraction;\n\
        this.nDecelerationFraction = aCommonParamSet.nDecelerationFraction;\n\
        this.bAutoReverse = aCommonParamSet.bAutoReverse;\n\
\n\
        this.bFirstPerformCall = true;\n\
        this.bIsActive = true;\n\
\n\
    }\n\
    extend( ActivityBase, AnimationActivity );\n\
\n\
\n\
    ActivityBase.prototype.activate = function( aEndEvent )\n\
    {\n\
        this.aEndEvent = aEndEvent;\n\
        this.bFirstPerformCall = true;\n\
        this.bIsActive = true;\n\
    };\n\
\n\
    ActivityBase.prototype.dispose = function()\n\
    {\n\
        this.bIsActive = false;\n\
\n\
        if( this.aEndEvent )\n\
            this.aEndEvent.dispose();\n\
\n\
        this.aEndEvent = null;\n\
    };\n\
\n\
    ActivityBase.prototype.perform = function()\n\
    {\n\
        if( !this.isActive() )\n\
            return false; // no, early exit.\n\
\n\
        assert( !this.FirstPerformCall, 'ActivityBase.perform: assertion (!this.FirstPerformCall) failed' );\n\
\n\
        return true;\n\
    };\n\
\n\
    ActivityBase.prototype.calcTimeLag = function()\n\
    {\n\
        if( this.isActive() && this.bFirstPerformCall )\n\
        {\n\
            this.bFirstPerformCall = false;\n\
\n\
            this.startAnimation();\n\
        }\n\
        return 0.0;\n\
    };\n\
\n\
    ActivityBase.prototype.isActive = function()\n\
    {\n\
        return this.bIsActive;\n\
    };\n\
\n\
    ActivityBase.prototype.isDisposed = function()\n\
    {\n\
        return ( !this.bIsActive && !this.aEndEvent );\n\
    };\n\
\n\
    ActivityBase.prototype.dequeued = function()\n\
    {\n\
        if( !this.isActive() )\n\
            this.endAnimation();\n\
    };\n\
\n\
    ActivityBase.prototype.setTargets = function( aTargetElement )\n\
    {\n\
        assert( aTargetElement, 'ActivityBase.setTargets: target element is not valid' );\n\
\n\
        this.aTargetElement = aTargetElement;\n\
    };\n\
\n\
    ActivityBase.prototype.startAnimation = function()\n\
    {\n\
        throw ( 'ActivityBase.startAnimation: abstract method invoked' );\n\
    };\n\
\n\
    ActivityBase.prototype.endAnimation = function()\n\
    {\n\
        throw ( 'ActivityBase.endAnimation: abstract method invoked' );\n\
    };\n\
\n\
    ActivityBase.prototype.endActivity = function()\n\
    {\n\
        this.bIsActive = false;\n\
\n\
        if( this.aEndEvent )\n\
            this.aTimerEventQueue.addEvent( this.aEndEvent );\n\
\n\
        this.aEndEvent = null;\n\
\n\
    };\n\
\n\
    ActivityBase.prototype.calcAcceleratedTime = function( nT )\n\
    {\n\
\n\
\n\
        nT = clamp( nT, 0.0, 1.0 );\n\
\n\
        if( ( this.nAccelerationFraction > 0.0 || this.nDecelerationFraction > 0.0 ) &&\n\
             ( this.nAccelerationFraction + this.nDecelerationFraction <= 1.0 ) )\n\
        {\n\
            var nC = 1.0 - 0.5*this.nAccelerationFraction - 0.5*this.nDecelerationFraction;\n\
\n\
            var nTPrime = 0.0;\n\
\n\
            if( nT < this.nAccelerationFraction )\n\
            {\n\
                nTPrime += 0.5 * nT * nT / this.nAccelerationFraction; // partial first interval\n\
            }\n\
            else\n\
            {\n\
                nTPrime += 0.5 * this.nAccelerationFraction; // full first interval\n\
\n\
                if( nT <= ( 1.0 - this.nDecelerationFraction ) )\n\
                {\n\
                    nTPrime += nT - this.nAccelerationFraction; // partial second interval\n\
                }\n\
                else\n\
                {\n\
                    nTPrime += 1.0 - this.nAccelerationFraction - this.nDecelerationFraction; // full second interval\n\
\n\
                    var nTRelative = nT - 1.0 + this.nDecelerationFraction;\n\
\n\
                    nTPrime += nTRelative - 0.5*nTRelative*nTRelative / this.nDecelerationFraction;\n\
                }\n\
            }\n\
\n\
            nT = nTPrime / nC;\n\
\n\
        }\n\
        return nT;\n\
    };\n\
\n\
    ActivityBase.prototype.getEventQueue = function()\n\
    {\n\
        return this.aTimerEventQueue;\n\
    };\n\
\n\
    ActivityBase.prototype.getTargetElement = function()\n\
    {\n\
        return this.aTargetElement;\n\
    };\n\
\n\
    ActivityBase.prototype.isRepeatCountValid = function()\n\
    {\n\
        if( this.nRepeats )\n\
            return true;\n\
        else\n\
            return false;\n\
    };\n\
\n\
    ActivityBase.prototype.getRepeatCount = function()\n\
    {\n\
        return this.nRepeats;\n\
    };\n\
\n\
    ActivityBase.prototype.isAutoReverse = function()\n\
    {\n\
        return this.bAutoReverse;\n\
    };\n\
\n\
    ActivityBase.prototype.end = function()\n\
    {\n\
        if( !this.isActive() || this.isDisposed() )\n\
            return;\n\
\n\
        if( this.bFirstPerformCall )\n\
        {\n\
            this.bFirstPerformCall = false;\n\
            this.startAnimation();\n\
        }\n\
\n\
";

static const char aSVGScript29[] =
"\
        this.performEnd();\n\
        this.endAnimation();\n\
        this.endActivity();\n\
    };\n\
\n\
    ActivityBase.prototype.performEnd = function()\n\
    {\n\
        throw ( 'ActivityBase.performEnd: abstract method invoked' );\n\
    };\n\
\n\
\n\
\n\
    function SimpleContinuousActivityBase( aCommonParamSet )\n\
    {\n\
        SimpleContinuousActivityBase.superclass.constructor.call( this, aCommonParamSet );\n\
\n\
        this.aTimer = new ElapsedTime( aCommonParamSet.aActivityQueue.getTimer() );\n\
        this.nMinSimpleDuration = aCommonParamSet.nMinDuration;\n\
        this.nMinNumberOfFrames = aCommonParamSet.nMinNumberOfFrames;\n\
        this.nCurrPerformCalls = 0;\n\
\n\
    }\n\
    extend( SimpleContinuousActivityBase, ActivityBase );\n\
\n\
\n\
    SimpleContinuousActivityBase.prototype.startAnimation = function()\n\
    {\n\
        this.aTimer.reset();\n\
\n\
    };\n\
\n\
    SimpleContinuousActivityBase.prototype.calcTimeLag = function()\n\
    {\n\
        SimpleContinuousActivityBase.superclass.calcTimeLag.call( this );\n\
\n\
        if( !this.isActive() )\n\
            return 0.0;\n\
\n\
        var nCurrElapsedTime = this.aTimer.getElapsedTime();\n\
\n\
\n\
        var nFractionElapsedTime = nCurrElapsedTime / this.nMinSimpleDuration;\n\
\n\
        var nFractionRequiredCalls = this.nCurrPerformCalls / this.nMinNumberOfFrames;\n\
\n\
\n\
        if( nFractionElapsedTime < nFractionRequiredCalls )\n\
        {\n\
            return 0.0;\n\
        }\n\
        else\n\
        {\n\
            return ( ( nFractionElapsedTime - nFractionRequiredCalls ) * this.nMinSimpleDuration );\n\
        }\n\
    };\n\
\n\
    SimpleContinuousActivityBase.prototype.perform = function()\n\
    {\n\
        if( !SimpleContinuousActivityBase.superclass.perform.call( this ) )\n\
            return false; // done, we're ended\n\
\n\
        var nCurrElapsedTime = this.aTimer.getElapsedTime();\n\
        var nT = nCurrElapsedTime / this.nMinSimpleDuration;\n\
\n\
\n\
\n\
        var bActivityEnding = false;\n\
\n\
        if( this.isRepeatCountValid() )\n\
        {\n\
\n\
            var nRepeatCount = this.getRepeatCount();\n\
            var nEffectiveRepeat = this.isAutoReverse() ? 2.0 * nRepeatCount : nRepeatCount;\n\
\n\
            if( nEffectiveRepeat <= nT )\n\
            {\n\
                bActivityEnding = true;\n\
\n\
                nT = nEffectiveRepeat;\n\
            }\n\
        }\n\
\n\
\n\
\n\
        var nRepeats;\n\
        var nRelativeSimpleTime;\n\
        if( this.isAutoReverse() )\n\
        {\n\
            nRepeats = Math.floor( nT );\n\
            var nFractionalActiveDuration =  nT - nRepeats;\n\
\n\
            if( nRepeats % 2 )\n\
            {\n\
                nRelativeSimpleTime = 1.0 - nFractionalActiveDuration;\n\
            }\n\
            else\n\
            {\n\
                nRelativeSimpleTime = nFractionalActiveDuration;\n\
            }\n\
\n\
            nRepeats /= 2;\n\
        }\n\
        else\n\
        {\n\
\n\
            nRepeats = Math.floor( nT );\n\
            nRelativeSimpleTime = nT - nRepeats;\n\
\n\
            if( this.isRepeatCountValid() && ( nRepeats >= this.getRepeatCount() ) )\n\
            {\n\
\n\
                nRelativeSimpleTime = 1.0;\n\
                nRepeats -= 1.0;\n\
            }\n\
        }\n\
\n\
\n\
\n\
        this.simplePerform( nRelativeSimpleTime, nRepeats );\n\
\n\
        if( bActivityEnding )\n\
            this.endActivity();\n\
\n\
        ++this.nCurrPerformCalls;\n\
\n\
        return this.isActive();\n\
    };\n\
\n\
    SimpleContinuousActivityBase.prototype.simplePerform = function( nSimpleTime, nRepeatCount )\n\
    {\n\
        throw ( 'SimpleContinuousActivityBase.simplePerform: abstract method invoked' );\n\
    };\n\
\n\
\n\
\n\
    function ContinuousKeyTimeActivityBase( aCommonParamSet )\n\
    {\n\
        var nSize = aCommonParamSet.aDiscreteTimes.length;\n\
        assert( nSize > 1,\n\
                'ContinuousKeyTimeActivityBase constructor: assertion (aDiscreteTimes.length > 1) failed' );\n\
\n\
        assert( aCommonParamSet.aDiscreteTimes[0] == 0.0,\n\
                'ContinuousKeyTimeActivityBase constructor: assertion (aDiscreteTimes.front() == 0.0) failed' );\n\
\n\
        assert( aCommonParamSet.aDiscreteTimes[ nSize - 1 ] <= 1.0,\n\
                'ContinuousKeyTimeActivityBase constructor: assertion (aDiscreteTimes.back() <= 1.0) failed' );\n\
\n\
        ContinuousKeyTimeActivityBase.superclass.constructor.call( this, aCommonParamSet );\n\
\n\
        this.aLerper = new KeyStopLerp( aCommonParamSet.aDiscreteTimes );\n\
    }\n\
    extend( ContinuousKeyTimeActivityBase, SimpleContinuousActivityBase );\n\
\n\
\n\
    ContinuousKeyTimeActivityBase.prototype.activate = function( aEndElement )\n\
    {\n\
        ContinuousKeyTimeActivityBase.superclass.activate.call( this, aEndElement );\n\
\n\
        this.aLerper.reset();\n\
    };\n\
\n\
    ContinuousKeyTimeActivityBase.prototype.performHook = function( nIndex, nFractionalIndex, nRepeatCount )\n\
    {\n\
        throw ( 'ContinuousKeyTimeActivityBase.performHook: abstract method invoked' );\n\
    };\n\
\n\
    ContinuousKeyTimeActivityBase.prototype.simplePerform = function( nSimpleTime, nRepeatCount )\n\
    {\n\
        var nAlpha = this.calcAcceleratedTime( nSimpleTime );\n\
\n\
        var aLerpResult = this.aLerper.lerp( nAlpha );\n\
\n\
        this.performHook( aLerpResult.nIndex, aLerpResult.nLerp, nRepeatCount );\n\
    };\n\
\n\
\n\
\n\
    function ContinuousActivityBase( aCommonParamSet )\n\
    {\n\
        ContinuousActivityBase.superclass.constructor.call( this, aCommonParamSet );\n\
\n\
    }\n\
    extend( ContinuousActivityBase, SimpleContinuousActivityBase );\n\
\n\
\n\
    ContinuousActivityBase.prototype.performHook = function( nModifiedTime, nRepeatCount )\n\
    {\n\
        throw ( 'ContinuousActivityBase.performHook: abstract method invoked' );\n\
    };\n\
\n\
    ContinuousActivityBase.prototype.simplePerform = function( nSimpleTime, nRepeatCount )\n\
    {\n\
        this.performHook( this.calcAcceleratedTime( nSimpleTime ), nRepeatCount );\n\
    };\n\
\n\
\n\
\n\
    function SimpleActivity( aCommonParamSet, aNumberAnimation, eDirection )\n\
    {\n\
        assert( ( eDirection == BACKWARD ) || ( eDirection == FORWARD ),\n\
";

static const char aSVGScript30[] =
"\
                'SimpleActivity constructor: animation direction is not valid' );\n\
\n\
        assert( aNumberAnimation, 'SimpleActivity constructor: animation object is not valid' );\n\
\n\
        SimpleActivity.superclass.constructor.call( this, aCommonParamSet );\n\
\n\
        this.aAnimation = aNumberAnimation;\n\
        this.nDirection = ( eDirection == FORWARD ) ? 1.0 : 0.0;\n\
    }\n\
    extend( SimpleActivity, ContinuousActivityBase );\n\
\n\
\n\
    SimpleActivity.prototype.startAnimation = function()\n\
    {\n\
        if( this.isDisposed() || !this.aAnimation )\n\
            return;\n\
\n\
        ANIMDBG.print( 'SimpleActivity.startAnimation invoked' );\n\
        SimpleActivity.superclass.startAnimation.call( this );\n\
\n\
        this.aAnimation.start( this.getTargetElement() );\n\
    };\n\
\n\
    SimpleActivity.prototype.endAnimation = function()\n\
    {\n\
        if( this.aAnimation )\n\
            this.aAnimation.end();\n\
\n\
    };\n\
\n\
    SimpleActivity.prototype.performHook = function( nModifiedTime, nRepeatCount )\n\
    {\n\
\n\
        if( this.isDisposed() || !this.aAnimation )\n\
            return;\n\
\n\
        var nT = 1.0 - this.nDirection + nModifiedTime * ( 2.0*this.nDirection - 1.0 );\n\
        this.aAnimation.perform( nT );\n\
    };\n\
\n\
    SimpleActivity.prototype.performEnd = function()\n\
    {\n\
        if( this.aAnimation )\n\
            this.aAnimation.perform( this.nDirection );\n\
    };\n\
\n\
\n\
\n\
\n\
\n\
    function FromToByActivityTemplate( BaseType ) // template parameter\n\
    {\n\
\n\
        function FromToByActivity( aFromValue, aToValue, aByValue,\n\
                                   aActivityParamSet, aAnimation,\n\
                                   aInterpolator, aOperatorSet, bAccumulate )\n\
        {\n\
            assert( aAnimation, 'FromToByActivity constructor: invalid animation object' );\n\
            assert( ( aToValue != undefined ) || ( aByValue != undefined ),\n\
                    'FromToByActivity constructor: one of aToValue or aByValue must be valid' );\n\
\n\
            FromToByActivity.superclass.constructor.call( this, aActivityParamSet );\n\
\n\
            this.aFrom = aFromValue;\n\
            this.aTo = aToValue;\n\
            this.aBy = aByValue;\n\
            this.aStartValue;\n\
            this.aEndValue;\n\
            this.aAnimation = aAnimation;\n\
            this.aInterpolator = aInterpolator;\n\
            this.add = aOperatorSet.add;\n\
            this.scale = aOperatorSet.scale;\n\
            this.bDynamicStartValue = false;\n\
            this.bCumulative = bAccumulate;\n\
\n\
            this.initAnimatedElement();\n\
\n\
        }\n\
        extend( FromToByActivity, BaseType );\n\
\n\
        FromToByActivity.prototype.initAnimatedElement = function()\n\
        {\n\
            if( this.aAnimation && this.aFrom )\n\
                this.aAnimation.perform( this.aFrom );\n\
        };\n\
\n\
        FromToByActivity.prototype.startAnimation = function()\n\
        {\n\
            if( this.isDisposed() || !this.aAnimation  )\n\
            {\n\
                log( 'FromToByActivity.startAnimation: activity disposed or not valid animation' );\n\
                return;\n\
            }\n\
\n\
            FromToByActivity.superclass.startAnimation.call( this );\n\
\n\
            this.aAnimation.start( this.getTargetElement() );\n\
\n\
\n\
            var aAnimationStartValue = this.aAnimation.getUnderlyingValue();\n\
\n\
            if( this.aFrom )\n\
            {\n\
                if( this.aTo )\n\
                {\n\
                    this.aStartValue = this.aFrom;\n\
                    this.aEndValue = this.aTo;\n\
                }\n\
                else if( this.aBy )\n\
                {\n\
                    this.aStartValue = this.aFrom;\n\
\n\
                    this.aEndValue = this.add( this.aStartValue, this.aBy );\n\
                }\n\
            }\n\
            else\n\
            {\n\
                if( this.aTo )\n\
                {\n\
\n\
                    this.bDynamicStartValue = true;\n\
                    this.aEndValue = this.aTo;\n\
                }\n\
                else if( this.aBy )\n\
                {\n\
                    this.aStartValue = aAnimationStartValue;\n\
\n\
                    this.aEndValue = this.add( this.aStartValue, this.aBy );\n\
                }\n\
            }\n\
\n\
            ANIMDBG.print( 'FromToByActivity.startAnimation: aStartValue = ' + this.aStartValue + ', aEndValue = ' + this.aEndValue );\n\
        };\n\
\n\
        FromToByActivity.prototype.endAnimation = function()\n\
        {\n\
            if( this.aAnimation )\n\
                this.aAnimation.end();\n\
        };\n\
\n\
        FromToByActivity.prototype.performHook = function( nModifiedTime, nRepeatCount )\n\
        {\n\
            if( this.isDisposed() || !this.aAnimation  )\n\
            {\n\
                log( 'FromToByActivity.performHook: activity disposed or not valid animation' );\n\
                return;\n\
            }\n\
\n\
            var aValue = this.bDynamicStartValue ? this.aAnimation.getUnderlyingValue()\n\
                                                 : this.aStartValue;\n\
\n\
            aValue = this.aInterpolator( aValue, this.aEndValue, nModifiedTime );\n\
\n\
            if( this.bCumulative )\n\
            {\n\
                aValue = this.add( this.scale( nRepeatCount, this.aEndValue ), aValue );\n\
            }\n\
\n\
            this.aAnimation.perform( aValue );\n\
        };\n\
\n\
        FromToByActivity.prototype.performEnd = function()\n\
        {\n\
            if( this.aAnimation )\n\
            {\n\
                if( this.isAutoreverse() )\n\
                    this.aAnimation.perform( this.aStartValue );\n\
                else\n\
                    this.aAnimation.perform( this.aEndValue );\n\
            }\n\
        };\n\
\n\
        FromToByActivity.prototype.dispose = function()\n\
        {\n\
            FromToByActivity.superclass.dispose.call( this );\n\
        };\n\
\n\
\n\
        return FromToByActivity;\n\
    }\n\
\n\
\n\
    var LinearFromToByActivity = instantiate( FromToByActivityTemplate, ContinuousActivityBase );\n\
\n\
\n\
\n\
\n\
\n\
    function  ValueListActivityTemplate( BaseType ) // template parameter\n\
    {\n\
\n\
        function ValueListActivity( aValueList, aActivityParamSet,\n\
                                    aAnimation, aInterpolator,\n\
                                    aOperatorSet, bAccumulate )\n\
        {\n\
            assert( aAnimation, 'ValueListActivity constructor: invalid animation object' );\n\
            assert( aValueList.length != 0, 'ValueListActivity: value list is empty' );\n\
\n\
            ValueListActivity.superclass.constructor.call( this, aActivityParamSet );\n\
\n\
";

static const char aSVGScript31[] =
"\
            this.aValueList = aValueList;\n\
            this.aAnimation = aAnimation;\n\
            this.aInterpolator = aInterpolator;\n\
            this.add = aOperatorSet.add;\n\
            this.scale = aOperatorSet.scale;\n\
            this.bCumulative = bAccumulate;\n\
            this.aLastValue = this.aValueList[ this.aValueList.length - 1 ];\n\
\n\
            this.initAnimatedElement();\n\
        }\n\
        extend( ValueListActivity, BaseType );\n\
\n\
        ValueListActivity.prototype.activate = function( aEndEvent )\n\
        {\n\
            ValueListActivity.superclass.activate.call( this, aEndEvent );\n\
            for( var i = 0; i < this.aValueList.length; ++i )\n\
            {\n\
                ANIMDBG.print( 'createValueListActivity: value[' + i + '] = ' + this.aValueList[i] );\n\
            }\n\
\n\
            this.initAnimatedElement();\n\
        };\n\
\n\
        ValueListActivity.prototype.initAnimatedElement = function()\n\
        {\n\
            if( this.aAnimation )\n\
                this.aAnimation.perform( this.aValueList[0] );\n\
        };\n\
\n\
        ValueListActivity.prototype.startAnimation = function()\n\
        {\n\
            if( this.isDisposed() || !this.aAnimation  )\n\
            {\n\
                log( 'ValueListActivity.startAnimation: activity disposed or not valid animation' );\n\
                return;\n\
            }\n\
\n\
            ValueListActivity.superclass.startAnimation.call( this );\n\
\n\
            this.aAnimation.start( this.getTargetElement() );\n\
        };\n\
\n\
        ValueListActivity.prototype.endAnimation = function()\n\
        {\n\
            if( this.aAnimation )\n\
                this.aAnimation.end();\n\
        };\n\
\n\
        ValueListActivity.prototype.performHook = function( nIndex, nFractionalIndex, nRepeatCount )\n\
        {\n\
            if( this.isDisposed() || !this.aAnimation  )\n\
            {\n\
                log( 'ValueListActivity.performHook: activity disposed or not valid animation' );\n\
                return;\n\
            }\n\
\n\
            assert( ( nIndex + 1 ) < this.aValueList.length,\n\
                    'ValueListActivity.performHook: assertion (nIndex + 1 < this.aValueList.length) failed' );\n\
\n\
\n\
            var aValue = this.aInterpolator( this.aValueList[ nIndex ],\n\
                                             this.aValueList[ nIndex+1 ],\n\
                                             nFractionalIndex );\n\
\n\
            if( this.bCumulative )\n\
            {\n\
                aValue = this.add( aValue, this.scale( nRepeatCount, this.aLastValue ) );\n\
            }\n\
            this.aAnimation.perform( aValue );\n\
        };\n\
\n\
        ValueListActivity.prototype.performEnd = function()\n\
        {\n\
            if( this.aAnimation )\n\
            {\n\
                this.aAnimation.perform( this.aLastValue );\n\
            }\n\
        };\n\
\n\
        ValueListActivity.prototype.dispose = function()\n\
        {\n\
            ValueListActivity.superclass.dispose.call( this );\n\
        };\n\
\n\
\n\
        return ValueListActivity;\n\
    }\n\
\n\
\n\
    var LinearValueListActivity = instantiate( ValueListActivityTemplate, ContinuousKeyTimeActivityBase );\n\
\n\
\n\
\n\
\n\
    function BaseClassOne( aValue )\n\
    {\n\
        this.aBasevalue = aValue;\n\
    }\n\
\n\
    BaseClassOne.prototype.print = function()\n\
    {\n\
        log( 'BaseClassOne value: ' + this.aBasevalue );\n\
    };\n\
\n\
\n\
    function Outer( BaseType )\n\
    {\n\
        function Inner( aValue )\n\
        {\n\
            Inner.superclass.constructor.call( this, aValue - 1 );\n\
            this.aValue = aValue;\n\
        }\n\
\n\
        extend( Inner, BaseType );\n\
\n\
        Inner.prototype.print = function()\n\
        {\n\
            Inner.superclass.print.call( this );\n\
            log( 'Inner value: ' + this.aValue );\n\
        };\n\
\n\
        return Inner;\n\
    }\n\
\n\
\n\
\n\
\n\
\n\
\n\
\n\
    function createActivity( aActivityParamSet, aAnimationNode, aAnimation, aInterpolator )\n\
    {\n\
        var eCalcMode = aAnimationNode.getCalcMode();\n\
\n\
        var sAttributeName = aAnimationNode.getAttributeName();\n\
        var aAttributeProp = aAttributeMap[ sAttributeName ];\n\
\n\
        var eValueType = aAttributeProp[ 'type' ];\n\
        var eValueSubtype = aAttributeProp[ 'subtype' ];\n\
\n\
        if( ! aInterpolator )\n\
        {\n\
            aInterpolator = aInterpolatorHandler.getInterpolator( eCalcMode,\n\
                                                                  eValueType,\n\
                                                                  eValueSubtype );\n\
        }\n\
\n\
        var bAccumulate = ( aAnimationNode.getAccumulate() === ACCUMULATE_MODE_SUM )\n\
                                && !( eValueType === BOOL_PROPERTY ||\n\
                                      eValueType === STRING_PROPERTY ||\n\
                                      eValueType === ENUM_PROPERTY );\n\
\n\
\n\
\n\
        aActivityParamSet.aDiscreteTimes = aAnimationNode.getKeyTimes();\n\
\n\
        var aValueSet = aAnimationNode.getValues();\n\
        var nValueSetSize = aValueSet.length;\n\
\n\
        if( nValueSetSize != 0 )\n\
        {\n\
\n\
            if( aActivityParamSet.aDiscreteTimes.length == 0 )\n\
            {\n\
                for( var i = 0; i < nValueSetSize; ++i )\n\
                    aActivityParamSet.aDiscreteTimes[i].push( i / nValueSetSize );\n\
            }\n\
\n\
            switch( eCalcMode )\n\
            {\n\
                case CALC_MODE_DISCRETE:\n\
                    log( 'createActivity: discrete calculation case not yet implemented' );\n\
                    break;\n\
\n\
                default:\n\
                    log( 'createActivity: unexpected calculation mode: ' + eCalcMode );\n\
                case CALC_MODE_PACED :\n\
                case CALC_MODE_SPLINE :\n\
                case CALC_MODE_LINEAR:\n\
                    return createValueListActivity( aActivityParamSet,\n\
                                                    aAnimationNode,\n\
                                                    aAnimation,\n\
                                                    aInterpolator,\n\
                                                    LinearValueListActivity,\n\
                                                    bAccumulate,\n\
                                                    eValueType );\n\
            }\n\
        }\n\
        else\n\
        {\n\
\n\
            switch( eCalcMode )\n\
            {\n\
                case CALC_MODE_DISCRETE:\n\
                    log( 'createActivity: discrete calculation case not yet implemented' );\n\
                    break;\n\
\n\
                default:\n\
                    log( 'createActivity: unexpected calculation mode: ' + eCalcMode );\n\
                case CALC_MODE_PACED :\n\
";

static const char aSVGScript32[] =
"\
                case CALC_MODE_SPLINE :\n\
                case CALC_MODE_LINEAR:\n\
                    return createFromToByActivity(  aActivityParamSet,\n\
                                                    aAnimationNode,\n\
                                                    aAnimation,\n\
                                                    aInterpolator,\n\
                                                    LinearFromToByActivity,\n\
                                                    bAccumulate,\n\
                                                    eValueType );\n\
            }\n\
        }\n\
    }\n\
\n\
\n\
\n\
    function createValueListActivity( aActivityParamSet, aAnimationNode, aAnimation,\n\
                                      aInterpolator, ClassTemplateInstance, bAccumulate, eValueType )\n\
    {\n\
        var aAnimatedElement = aAnimationNode.getAnimatedElement();\n\
        var aOperatorSet = aOperatorSetMap[ eValueType ];\n\
        assert( aOperatorSet, 'createFromToByActivity: no operator set found' );\n\
\n\
        var aValueSet = aAnimationNode.getValues();\n\
\n\
        var aValueList = new Array();\n\
\n\
        extractAttributeValues( eValueType,\n\
                                aValueList,\n\
                                aValueSet,\n\
                                aAnimatedElement.getBaseBBox(),\n\
                                aActivityParamSet.nSlideWidth,\n\
                                aActivityParamSet.nSlideHeight );\n\
\n\
        for( var i = 0; i < aValueList.length; ++i )\n\
        {\n\
            ANIMDBG.print( 'createValueListActivity: value[' + i + '] = ' + aValueList[i] );\n\
        }\n\
\n\
        return new ClassTemplateInstance( aValueList, aActivityParamSet, aAnimation,\n\
                                          aInterpolator, aOperatorSet, bAccumulate );\n\
    }\n\
\n\
\n\
\n\
    function createFromToByActivity( aActivityParamSet, aAnimationNode, aAnimation,\n\
                                     aInterpolator, ClassTemplateInstance, bAccumulate, eValueType )\n\
    {\n\
\n\
        var aAnimatedElement = aAnimationNode.getAnimatedElement();\n\
        var aOperatorSet = aOperatorSetMap[ eValueType ];\n\
        assert( aOperatorSet, 'createFromToByActivity: no operator set found' );\n\
\n\
        var aValueSet = new Array();\n\
        aValueSet[0] = aAnimationNode.getFromValue();\n\
        aValueSet[1] = aAnimationNode.getToValue();\n\
        aValueSet[2] = aAnimationNode.getByValue();\n\
\n\
        ANIMDBG.print( 'createFromToByActivity: value type: ' + aValueTypeOutMap[eValueType] +\n\
                        ', aFrom = ' + aValueSet[0] +\n\
                        ', aTo = ' + aValueSet[1] +\n\
                        ', aBy = ' + aValueSet[2] );\n\
\n\
        var aValueList = new Array();\n\
\n\
        extractAttributeValues( eValueType,\n\
                                aValueList,\n\
                                aValueSet,\n\
                                aAnimatedElement.getBaseBBox(),\n\
                                aActivityParamSet.nSlideWidth,\n\
                                aActivityParamSet.nSlideHeight );\n\
\n\
        ANIMDBG.print( 'createFromToByActivity: ' +\n\
                        ', aFrom = ' + aValueList[0] +\n\
                        ', aTo = ' + aValueList[1] +\n\
                        ', aBy = ' + aValueList[2] );\n\
\n\
        return new ClassTemplateInstance( aValueList[0], aValueList[1], aValueList[2],\n\
                                          aActivityParamSet, aAnimation,\n\
                                          aInterpolator, aOperatorSet, bAccumulate );\n\
    }\n\
\n\
\n\
    function extractAttributeValues( eValueType, aValueList, aValueSet, aBBox, nSlideWidth, nSlideHeight )\n\
    {\n\
        switch( eValueType )\n\
        {\n\
            case NUMBER_PROPERTY :\n\
                evalValuesAttribute( aValueList, aValueSet, aBBox, nSlideWidth, nSlideHeight );\n\
                break;\n\
            case BOOL_PROPERTY :\n\
                for( var i = 0; i < aValueSet.length; ++i )\n\
                {\n\
                    var aValue = booleanParser( aValueSet[i] );\n\
                    aValueList.push( aValue );\n\
                }\n\
                break;\n\
            case STRING_PROPERTY :\n\
                for( var i = 0; i < aValueSet.length; ++i )\n\
                {\n\
                    aValueList.push( aValueSet[i] );\n\
                }\n\
                break;\n\
            case ENUM_PROPERTY :\n\
                for( var i = 0; i < aValueSet.length; ++i )\n\
                {\n\
                    aValueList.push( aValueSet[i] );\n\
                }\n\
                break;\n\
            case COLOR_PROPERTY :\n\
                for( var i = 0; i < aValueSet.length; ++i )\n\
                {\n\
                    var aValue = colorParser( aValueSet[i] );\n\
                    aValueList.push( aValue );\n\
                }\n\
                break;\n\
            default:\n\
                log( 'createValueListActivity: unexpeded value type: ' + eValueType );\n\
        }\n\
\n\
    }\n\
\n\
    function evalValuesAttribute( aValueList, aValueSet, aBBox, nSlideWidth, nSlideHeight )\n\
    {\n\
        var width = aBBox.width / nSlideWidth;\n\
        var height = aBBox.height / nSlideHeight;\n\
        var x = ( aBBox.x + aBBox.width / 2 ) / nSlideWidth;\n\
        var y = ( aBBox.y + aBBox.height / 2 ) / nSlideHeight;\n\
\n\
        for( var i = 0; i < aValueSet.length; ++i )\n\
        {\n\
            var aValue =  eval( aValueSet[i] );\n\
            aValueList.push( aValue );\n\
        }\n\
    }\n\
\n\
\n\
\n\
\n\
\n\
    var BACKWARD    = 0;\n\
    var FORWARD     = 1;\n\
\n\
    var MAXIMUM_FRAME_COUNT                 = 60;\n\
    var MINIMUM_TIMEOUT                     = 1.0 / MAXIMUM_FRAME_COUNT;\n\
    var MAXIMUM_TIMEOUT                     = 4.0;\n\
    var MINIMUM_FRAMES_PER_SECONDS          = 10;\n\
    var PREFERRED_FRAMES_PER_SECONDS        = 50;\n\
    var PREFERRED_FRAME_RATE                = 1.0 / PREFERRED_FRAMES_PER_SECONDS;\n\
\n\
\n\
\n\
    function SlideShow()\n\
    {\n\
        this.aTimer = new ElapsedTime();\n\
        this.aFrameSynchronization = new FrameSynchronization( PREFERRED_FRAME_RATE );\n\
        this.aTimerEventQueue = new TimerEventQueue( this.aTimer );\n\
        this.aActivityQueue = new ActivityQueue( this.aTimer );\n\
        this.aNextEffectEventArray = null;\n\
        this.aEventMultiplexer = null; new EventMultiplexer( this.aTimerEventQueue );\n\
\n\
        this.aContext = new SlideShowContext( this.aTimerEventQueue, this.aEventMultiplexer,\n\
                                              this.aNextEffectEventArray, this.aActivityQueue );\n\
        this.nCurrentSlide = 0;\n\
        this.nCurrentEffect = 0;\n\
        this.eDirection = FORWARD;\n\
        this.bIsIdle = true;\n\
        this.bIsEnabled = true;\n\
    }\n\
\n\
\n\
    SlideShow.prototype.setSlideEvents = function( aNextEffectEventArray, aEventMultiplexer )\n\
    {\n\
        if( !aNextEffectEventArray )\n\
            log( 'SlideShow.setSlideEvents: aNextEffectEventArray is not valid' );\n\
\n\
        if( !aEventMultiplexer )\n\
            log( 'SlideShow.setSlideEvents: aEventMultiplexer is not valid' );\n\
\n\
        this.aContext.aNextEffectEventArray = aNextEffectEventArray;\n\
        this.aNextEffectEventArray = aNextEffectEventArray;\n\
        this.aContext.aEventMultiplexer = aEventMultiplexer;\n\
        this.aEventMultiplexer = aEventMultiplexer;\n\
        this.nCurrentEffect = 0;\n\
    };\n\
\n\
    SlideShow.prototype.isRunning = function()\n\
    {\n\
        return !this.bIsIdle;\n\
    };\n\
\n\
    SlideShow.prototype.isEnabled = function()\n\
    {\n\
        return this.bIsEnabled;\n\
    };\n\
\n\
    SlideShow.prototype.notifyNextEffectStart = function()\n\
    {\n\
        var aAnimatedElementMap = theMetaDoc.aMetaSlideSet[nCurSlide].aSlideAnimationsHandler.aAnimatedElementMap;\n\
        for( sId in aAnimatedElementMap )\n\
            aAnimatedElementMap[ sId ].notifyNextEffectStart( this.nCurrentEffect );\n\
";

static const char aSVGScript33[] =
"\
    };\n\
\n\
    SlideShow.prototype.notifySlideStart = function()\n\
    {\n\
        var aAnimatedElementMap = theMetaDoc.aMetaSlideSet[nCurSlide].aSlideAnimationsHandler.aAnimatedElementMap;\n\
        for( sId in aAnimatedElementMap )\n\
            aAnimatedElementMap[ sId ].notifySlideStart();\n\
    };\n\
\n\
    SlideShow.prototype.nextEffect = function()\n\
    {\n\
        if( !this.isEnabled() )\n\
            return false;\n\
\n\
        if( this.isRunning() )\n\
            return true;\n\
\n\
        if( !this.aNextEffectEventArray )\n\
            return false;\n\
\n\
        this.notifyNextEffectStart();\n\
\n\
        if( this.nCurrentEffect >= this.aNextEffectEventArray.size() )\n\
            return false;\n\
\n\
        this.eDirection = FORWARD;\n\
        this.aNextEffectEventArray.at( this.nCurrentEffect ).fire();\n\
        ++this.nCurrentEffect;\n\
        this.update();\n\
        return true;\n\
    };\n\
\n\
    SlideShow.prototype.previousEffect = function()\n\
    {\n\
        if( this.nCurrentEffect <= 0 )\n\
            return false;\n\
        this.eDirection = BACKWARD;\n\
        this.aNextEffectEventArray.at( this.nCurrentEffect ).fire();\n\
        --this.nCurrentEffect;\n\
        return true;\n\
    };\n\
\n\
    SlideShow.prototype.displaySlide = function( nNewSlide, bSkipSlideTransition )\n\
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
        if( !this.isEnabled() )\n\
        {\n\
            oldMetaSlide.hide();\n\
            newMetaSlide.show();\n\
            return;\n\
        }\n\
\n\
        oldMetaSlide.hide();\n\
        oldMetaSlide.aSlideAnimationsHandler.end( bSkipSlideTransition );\n\
\n\
        this.aTimerEventQueue.clear();\n\
        this.aActivityQueue.clear();\n\
        this.aNextEffectEventArray = null;\n\
        this.aEventMultiplexer = null;\n\
\n\
        this.notifySlideStart();\n\
\n\
        if( !bSkipSlideTransition )\n\
        {\n\
        }\n\
\n\
        newMetaSlide.show();\n\
        newMetaSlide.aSlideAnimationsHandler.start();\n\
        this.update();\n\
    };\n\
\n\
    SlideShow.prototype.update = function()\n\
    {\n\
        this.aTimer.holdTimer();\n\
        var suspendHandle = ROOT_NODE.suspendRedraw( PREFERRED_FRAME_RATE * 1000 );\n\
\n\
        this.aTimerEventQueue.process();\n\
        this.aActivityQueue.process();\n\
\n\
        this.aFrameSynchronization.synchronize();\n\
\n\
        ROOT_NODE.unsuspendRedraw(suspendHandle);\n\
        ROOT_NODE.forceRedraw();\n\
        this.aTimer.releaseTimer();\n\
\n\
        var bActivitiesLeft = ( ! this.aActivityQueue.isEmpty() );\n\
        var bTimerEventsLeft = ( ! this.aTimerEventQueue.isEmpty() );\n\
        var bEventsLeft = ( bActivitiesLeft || bTimerEventsLeft );\n\
\n\
\n\
        if( bEventsLeft )\n\
        {\n\
            var nNextTimeout;\n\
            if( bActivitiesLeft )\n\
            {\n\
                nNextTimeout = MINIMUM_TIMEOUT;\n\
                this.aFrameSynchronization.activate();\n\
            }\n\
            else\n\
            {\n\
                nNextTimeout = this.aTimerEventQueue.nextTimeout();\n\
                if( nNextTimeout < MINIMUM_TIMEOUT )\n\
                    nNextTimeout = MINIMUM_TIMEOUT;\n\
                else if( nNextTimeout > MAXIMUM_TIMEOUT )\n\
                    nNextTimeout = MAXIMUM_TIMEOUT;\n\
                this.aFrameSynchronization.deactivate();\n\
            }\n\
\n\
            this.bIsIdle = false;\n\
            window.setTimeout( 'aSlideShow.update()', nNextTimeout * 1000 );\n\
        }\n\
        else\n\
        {\n\
            this.bIsIdle = true;\n\
        }\n\
    };\n\
\n\
    SlideShow.prototype.getContext = function()\n\
    {\n\
        return this.aContext;\n\
    };\n\
\n\
    var aSlideShow = null;\n\
\n\
\n\
\n\
    function SlideShowContext( aTimerEventQueue, aEventMultiplexer, aNextEffectEventArray, aActivityQueue)\n\
    {\n\
        this.aTimerEventQueue = aTimerEventQueue;\n\
        this.aEventMultiplexer = aEventMultiplexer;\n\
        this.aNextEffectEventArray = aNextEffectEventArray;\n\
        this.aActivityQueue = aActivityQueue;\n\
    }\n\
\n\
\n\
\n\
    function FrameSynchronization( nFrameDuration )\n\
    {\n\
        this.nFrameDuration = nFrameDuration;\n\
        this.aTimer = new ElapsedTime();\n\
        this.nNextFrameTargetTime = 0.0;\n\
        this.bIsActive = false;\n\
\n\
        this.markCurrentFrame();\n\
    }\n\
\n\
\n\
    FrameSynchronization.prototype.markCurrentFrame = function()\n\
    {\n\
        this.nNextFrameTargetTime = this.aTimer.getElapsedTime() + this.nFrameDuration;\n\
    };\n\
\n\
    FrameSynchronization.prototype.synchronize = function()\n\
    {\n\
        if( this.bIsActive )\n\
        {\n\
            while( this.aTimer.getElapsedTime() < this.nNextFrameTargetTime )\n\
                ;\n\
        }\n\
\n\
        this.markCurrentFrame();\n\
\n\
    };\n\
\n\
    FrameSynchronization.prototype.activate = function()\n\
    {\n\
        this.bIsActive = true;\n\
    };\n\
\n\
    FrameSynchronization.prototype.deactivate = function()\n\
    {\n\
        this.bIsActive = false;\n\
    };\n\
\n\
\n\
\n\
\n\
    function NextEffectEventArray()\n\
    {\n\
        this.aEventArray = new Array();\n\
    }\n\
\n\
\n\
    NextEffectEventArray.prototype.size = function()\n\
    {\n\
        return this.aEventArray.length;\n\
    };\n\
";

static const char aSVGScript34[] =
"\
\n\
    NextEffectEventArray.prototype.at = function( nIndex )\n\
    {\n\
        return this.aEventArray[ nIndex ];\n\
    };\n\
\n\
    NextEffectEventArray.prototype.appendEvent = function( aEvent )\n\
    {\n\
        var nSize = this.size();\n\
        for( var i = 0; i < nSize; ++i )\n\
        {\n\
            if( this.aEventArray[i].getId() == aEvent.getId() )\n\
            {\n\
                aNextEffectEventArrayDebugPrinter.print( 'NextEffectEventArray.appendEvent: event already present' );\n\
                return false;\n\
            }\n\
        }\n\
        this.aEventArray.push( aEvent );\n\
        aNextEffectEventArrayDebugPrinter.print( 'NextEffectEventArray.appendEvent: event appended' );\n\
        return true;\n\
    };\n\
\n\
    NextEffectEventArray.prototype.clear = function( )\n\
    {\n\
        this.aEventArray = new Array();\n\
    };\n\
\n\
\n\
\n\
    function TimerEventQueue( aTimer )\n\
    {\n\
        this.aTimer = aTimer;\n\
        this.aEventSet = new PriorityQueue( EventEntry.compare );\n\
    }\n\
\n\
\n\
    TimerEventQueue.prototype.addEvent = function( aEvent )\n\
    {\n\
        this.DBG( 'TimerEventQueue.addEvent invoked' );\n\
        if( !aEvent )\n\
        {\n\
            log( 'error: TimerEventQueue.addEvent: null event' );\n\
            return false;\n\
        }\n\
\n\
        var nTime = aEvent.getActivationTime( this.aTimer.getElapsedTime() );\n\
        var aEventEntry = new EventEntry( aEvent, nTime );\n\
        this.aEventSet.push( aEventEntry );\n\
\n\
        return true;\n\
    };\n\
\n\
    TimerEventQueue.prototype.process = function()\n\
    {\n\
        var nCurrentTime = this.aTimer.getElapsedTime();\n\
\n\
        while( !this.isEmpty() && ( this.aEventSet.top().nActivationTime <= nCurrentTime ) )\n\
        {\n\
            var aEventEntry = this.aEventSet.top();\n\
            this.aEventSet.pop();\n\
\n\
            var aEvent = aEventEntry.aEvent;\n\
            if( aEvent.isCharged() )\n\
                aEvent.fire();\n\
        }\n\
    };\n\
\n\
    TimerEventQueue.prototype.isEmpty = function()\n\
    {\n\
        return this.aEventSet.isEmpty();\n\
    };\n\
\n\
    TimerEventQueue.prototype.nextTimeout = function()\n\
    {\n\
        var nTimeout = Number.MAX_VALUE;\n\
        var nCurrentTime = this.aTimer.getElapsedTime();\n\
        if( !this.isEmpty() )\n\
            nTimeout = this.aEventSet.top().nActivationTime - nCurrentTime;\n\
        return nTimeout;\n\
    };\n\
\n\
    TimerEventQueue.prototype.clear = function()\n\
    {\n\
        this.DBG( 'TimerEventQueue.clear invoked' );\n\
        this.aEventSet.clear();\n\
    };\n\
\n\
    TimerEventQueue.prototype.getTimer = function()\n\
    {\n\
        return this.aTimer;\n\
    };\n\
\n\
    TimerEventQueue.prototype.DBG = function( sMessage, nTime )\n\
    {\n\
        aTimerEventQueueDebugPrinter.print( sMessage, nTime );\n\
    };\n\
\n\
\n\
    TimerEventQueue.prototype.insert = function( aEventEntry )\n\
    {\n\
        var nHoleIndex = this.aEventSet.length;\n\
        var nParent = Math.floor( ( nHoleIndex - 1 ) / 2 );\n\
\n\
        while( ( nHoleIndex > 0 ) && this.aEventSet[ nParent ].compare( aEventEntry ) )\n\
        {\n\
            this.aEventSet[ nHoleIndex ] = this.aEventSet[ nParent ];\n\
            nHoleIndex = nParent;\n\
            nParent = Math.floor( ( nHoleIndex - 1 ) / 2 );\n\
        }\n\
        this.aEventSet[ nHoleIndex ] = aEventEntry;\n\
    };\n\
\n\
\n\
\n\
    function EventEntry( aEvent, nTime )\n\
    {\n\
        this.aEvent = aEvent;\n\
        this.nActivationTime = nTime;\n\
    }\n\
\n\
\n\
    EventEntry.compare = function( aLhsEventEntry, aRhsEventEntry )\n\
    {\n\
        return ( aLhsEventEntry.nActivationTime > aRhsEventEntry.nActivationTime );\n\
    };\n\
\n\
\n\
\n\
    function PriorityQueue( aCompareFunc )\n\
    {\n\
        this.aSequence = new Array();\n\
        this.aCompareFunc = aCompareFunc;\n\
    }\n\
\n\
    PriorityQueue.prototype.top = function()\n\
    {\n\
        return this.aSequence[0];\n\
    };\n\
\n\
    PriorityQueue.prototype.isEmpty = function()\n\
    {\n\
        return ( this.size() === 0 );\n\
    };\n\
\n\
    PriorityQueue.prototype.size = function()\n\
    {\n\
        return this.aSequence.length;\n\
    };\n\
\n\
    PriorityQueue.prototype.push = function( aValue )\n\
    {\n\
        this.implPushHeap( 0, this.aSequence.length, 0, aValue );\n\
    };\n\
\n\
    PriorityQueue.prototype.clear = function()\n\
    {\n\
        return this.aSequence = new Array();\n\
    };\n\
\n\
\n\
    PriorityQueue.prototype.pop = function()\n\
    {\n\
        if( this.isEmpty() )\n\
            return;\n\
\n\
        var nLast = this.aSequence.length - 1;\n\
        var aValue = this.aSequence[ nLast ];\n\
        this.aSequence[ nLast ] = this.aSequence[ 0 ];\n\
        this.implAdjustHeap( 0, 0, nLast, aValue );\n\
        this.aSequence.pop();\n\
    };\n\
\n\
    PriorityQueue.prototype.implAdjustHeap = function( nFirst, nHoleIndex, nLength, aValue )\n\
    {\n\
        var nTopIndex = nHoleIndex;\n\
        var nSecondChild = nHoleIndex;\n\
\n\
        while( nSecondChild < Math.floor( ( nLength - 1 ) / 2 ) )\n\
        {\n\
            nSecondChild = 2 * ( nSecondChild + 1 );\n\
            if( this.aCompareFunc( this.aSequence[ nFirst + nSecondChild ],\n\
                                   this.aSequence[ nFirst + nSecondChild - 1] ) )\n\
            {\n\
                --nSecondChild;\n\
            }\n\
            this.aSequence[ nFirst + nHoleIndex ] = this.aSequence[ nFirst + nSecondChild ];\n\
            nHoleIndex = nSecondChild;\n\
        }\n\
\n\
        if( ( ( nLength & 1 ) === 0 ) && ( nSecondChild === Math.floor( ( nLength - 2 ) / 2 ) ) )\n\
        {\n\
            nSecondChild = 2 * ( nSecondChild + 1 );\n\
            this.aSequence[ nFirst + nHoleIndex ] = this.aSequence[ nFirst + nSecondChild - 1];\n\
            nHoleIndex = nSecondChild - 1;\n\
        }\n\
\n\
        this.implPushHeap( nFirst, nHoleIndex, nTopIndex, aValue );\n\
    };\n\
\n\
    PriorityQueue.prototype.implPushHeap = function( nFirst, nHoleIndex, nTopIndex, aValue )\n\
";

static const char aSVGScript35[] =
"\
    {\n\
        var nParent = Math.floor( ( nHoleIndex - 1 ) / 2 );\n\
\n\
        while( ( nHoleIndex > nTopIndex ) &&\n\
               this.aCompareFunc( this.aSequence[ nFirst + nParent ], aValue ) )\n\
        {\n\
            this.aSequence[ nFirst + nHoleIndex ] = this.aSequence[ nFirst + nParent ];\n\
            nHoleIndex = nParent;\n\
            nParent = Math.floor( ( nHoleIndex - 1 ) / 2 );\n\
        }\n\
        this.aSequence[ nFirst + nHoleIndex ] = aValue;\n\
    };\n\
\n\
\n\
\n\
    function ActivityQueue( aTimer )\n\
    {\n\
        this.aTimer = aTimer;\n\
        this.aCurrentActivityWaitingSet = new Array();\n\
        this.aCurrentActivityReinsertSet = new Array();\n\
        this.aDequeuedActivitySet = new Array();\n\
    }\n\
\n\
\n\
    ActivityQueue.prototype.dispose = function()\n\
    {\n\
        var nSize = this.aCurrentActivityWaitingSet.length;\n\
        for( var i = 0; i < nSize; ++i )\n\
            this.aCurrentActivityWaitingSet[i].dispose();\n\
\n\
        nSize = this.aCurrentActivityReinsertSet.length;\n\
        for( var i = 0; i < nSize; ++i )\n\
            this.aCurrentActivityReinsertSet[i].dispose();\n\
    };\n\
\n\
    ActivityQueue.prototype.addActivity = function( aActivity )\n\
    {\n\
        if( !aActivity )\n\
        {\n\
            log( 'ActivityQueue.addActivity: activity is not valid' );\n\
            return false;\n\
        }\n\
\n\
        this.aCurrentActivityWaitingSet.push( aActivity );\n\
        aActivityQueueDebugPrinter.print( 'ActivityQueue.addActivity: activity appended' );\n\
        return true;\n\
    };\n\
\n\
    ActivityQueue.prototype.process = function()\n\
    {\n\
        var nSize = this.aCurrentActivityWaitingSet.length;\n\
        var nLag = 0.0;\n\
        for( var i = 0; i < nSize; ++i )\n\
        {\n\
            nLag = Math.max( nLag,this.aCurrentActivityWaitingSet[i].calcTimeLag()  );\n\
        }\n\
\n\
        if( nLag > 0.0 )\n\
            this.aTimer.adjustTimer( -nLag, true );\n\
\n\
\n\
        while( this.aCurrentActivityWaitingSet.length != 0 )\n\
        {\n\
            var aActivity = this.aCurrentActivityWaitingSet.shift();\n\
            var bReinsert = false;\n\
\n\
            bReinsert = aActivity.perform();\n\
\n\
            if( bReinsert )\n\
            {\n\
                this.aCurrentActivityReinsertSet.push( aActivity );\n\
            }\n\
            else\n\
            {\n\
                this.aDequeuedActivitySet.push( aActivity );\n\
            }\n\
        }\n\
\n\
        if( this.aCurrentActivityReinsertSet.length != 0 )\n\
        {\n\
            this.aCurrentActivityWaitingSet = this.aCurrentActivityReinsertSet;\n\
            this.aCurrentActivityReinsertSet = new Array();\n\
        }\n\
    };\n\
\n\
    ActivityQueue.prototype.processDequeued = function()\n\
    {\n\
        var nSize = this.aDequeuedActivitySet.length;\n\
        for( var i = 0; i < nSize; ++i )\n\
            this.aDequeuedActivitySet[i].dequeued();\n\
\n\
        this.aDequeuedActivitySet = new Array();\n\
    };\n\
\n\
    ActivityQueue.prototype.isEmpty = function()\n\
    {\n\
        return ( ( this.aCurrentActivityWaitingSet.length == 0 ) &&\n\
                 ( this.aCurrentActivityReinsertSet.length == 0 ) );\n\
    };\n\
\n\
    ActivityQueue.prototype.clear = function()\n\
    {\n\
        aActivityQueueDebugPrinter.print( 'ActivityQueue.clear invoked' );\n\
        var nSize = this.aCurrentActivityWaitingSet.length;\n\
        for( var i = 0; i < nSize; ++i )\n\
            this.aCurrentActivityWaitingSet[i].dequeued();\n\
        this.aCurrentActivityWaitingSet = new Array();\n\
\n\
        nSize = this.aCurrentActivityReinsertSet.length;\n\
        for( var i = 0; i < nSize; ++i )\n\
            this.aCurrentActivityReinsertSet[i].dequeued();\n\
        this.aCurrentActivityReinsertSet = new Array();\n\
    };\n\
\n\
    ActivityQueue.prototype.getTimer = function()\n\
    {\n\
        return this.aTimer;\n\
    };\n\
\n\
    ActivityQueue.prototype.size = function()\n\
    {\n\
        return ( this.aCurrentActivityWaitingSet.length +\n\
                 this.aCurrentActivityReinsertSet.length +\n\
                 this.aDequeuedActivitySet.length );\n\
    };\n\
\n\
\n\
\n\
    function ElapsedTime( aTimeBase )\n\
    {\n\
        this.aTimeBase = aTimeBase;\n\
        this.nLastQueriedTime = 0.0;\n\
        this.nStartTime = this.getCurrentTime();\n\
        this.nFrozenTime = 0.0;\n\
        this.bInPauseMode = false;\n\
        this.bInHoldMode = false;\n\
    }\n\
\n\
\n\
    ElapsedTime.prototype.getTimeBase = function()\n\
    {\n\
        return aTimeBase;\n\
    };\n\
\n\
    ElapsedTime.prototype.reset = function()\n\
    {\n\
        this.nLastQueriedTime = 0.0;\n\
        this.nStartTime = this.getCurrentTime();\n\
        this.nFrozenTime = 0.0;\n\
        this.bInPauseMode = false;\n\
        this.bInHoldMode = false;\n\
    };\n\
\n\
    ElapsedTime.prototype.getElapsedTime = function()\n\
    {\n\
        this.nLastQueriedTime = this.getElapsedTimeImpl();\n\
        return this.nLastQueriedTime;\n\
    };\n\
\n\
    ElapsedTime.prototype.pauseTimer = function()\n\
    {\n\
        this.nFrozenTime = this.getElapsedTimeImpl();\n\
        this.bInPauseMode = true;\n\
    };\n\
\n\
    ElapsedTime.prototype.continueTimer = function()\n\
    {\n\
        this.bInPauseMode = false;\n\
\n\
        var nPauseDuration = this.getElapsedTimeImpl() - this.nFrozenTime;\n\
\n\
        this.nStartTime += nPauseDuration;\n\
    };\n\
\n\
    ElapsedTime.prototype.adjustTimer = function( nOffset, bLimitToLastQueriedTime )\n\
    {\n\
        if( bLimitToLastQueriedTime == undefined )\n\
            bLimitToLastQueriedTime = true;\n\
\n\
        this.nStartTime -= nOffset;\n\
\n\
        if( this.bInHoldMode || this.bInPauseMode )\n\
            this.nFrozenTime += nOffset;\n\
    };\n\
\n\
    ElapsedTime.prototype.holdTimer = function()\n\
    {\n\
        this.nFrozenTime = this.getElapsedTimeImpl();\n\
        this.bInHoldMode = true;\n\
    };\n\
\n\
    ElapsedTime.prototype.releaseTimer = function()\n\
    {\n\
        this.bInHoldMode = false;\n\
    };\n\
\n\
    ElapsedTime.prototype.getSystemTime = function()\n\
    {\n\
        return ( getCurrentSystemTime() / 1000.0 );\n\
    };\n\
";

static const char aSVGScript36[] =
"\
\n\
    ElapsedTime.prototype.getCurrentTime = function()\n\
    {\n\
        var nCurrentTime;\n\
        if ( !this.aTimeBase )\n\
        {\n\
            nCurrentTime = this.getSystemTime();\n\
        }\n\
        else\n\
        {\n\
            nCurrentTime = this.aTimeBase.getElapsedTimeImpl();\n\
        }\n\
\n\
        assert( ( typeof( nCurrentTime ) === typeof( 0 ) ) && isFinite( nCurrentTime ),\n\
                'ElapsedTime.getCurrentTime: assertion failed: nCurrentTime == ' + nCurrentTime );\n\
\n\
\n\
        return nCurrentTime;\n\
    };\n\
\n\
    ElapsedTime.prototype.getElapsedTimeImpl = function()\n\
    {\n\
        if( this.bInHoldMode || this.bInPauseMode )\n\
        {\n\
\n\
            return this.nFrozenTime;\n\
        }\n\
\n\
        var nCurTime = this.getCurrentTime();\n\
        return ( nCurTime - this.nStartTime );\n\
    };\n\
\n\
\n\
\n\
\n\
]]>";



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
