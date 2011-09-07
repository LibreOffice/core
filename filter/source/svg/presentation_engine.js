
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                      - Presentation Engine -                            *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /

/**
 *  WARNING: any comment that should not be striped out by the script
 *  generating the C++ header file must starts with a '/' and exactly 5 '*'
 *  not striped examples: '/*****', '/***** *'
 *  striped examples: '/** ***' (not contiguous), '/******' (more than 5)
 */



/*****
 * @licstart
 *
 * The following is the license notice for the part of JavaScript code of this
 * page included between the '@jessyinkstart' and the '@jessyinkend' notes.
 */

/*****  ******************************************************************
 *
 *   Copyright 2008, 2009 Hannes Hochreiner
 *
 *   The JavaScript code included between the start note '@jessyinkstart'
 *   and the end note '@jessyinkend' is free software: you can
 *   redistribute it and/or modify it under the terms of the GNU
 *   General Public License (GNU GPL) as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your option)
 *   any later version.  The code is distributed WITHOUT ANY WARRANTY;
 *   without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU GPL for more details.
 *
 *   As additional permission under GNU GPL version 3 section 7, you
 *   may distribute non-source (e.g., minimized or compacted) forms of
 *   that code without the copy of the GNU GPL normally required by
 *   section 4, provided you include this license notice and a URL
 *   through which recipients can access the Corresponding Source.
 *
 *************************************************************************/

/*****
 *  You can find the complete source code of the JessyInk project at:
 *  @source http://code.google.com/p/jessyink/
 */

/*****
 * @licend
 *
 * The above is the license notice for the part of JavaScript code of this
 * page included between the '@jessyinkstart' and the '@jessyinkend' notes.
 */



/*****
 * @jessyinkstart
 *
 *  The following code is a derivative work of some parts of the JessyInk
 *  project.
 *  @source http://code.google.com/p/jessyink/
 */


/** Convenience function to get an element depending on whether it has a
 *  property with a particular name.
 *
 *  @param node:   element of the document
 *  @param name:   attribute name
 *
 *  @returns   an array containing all the elements of the tree with root
 *             'node' that own the property 'name'
 */
function getElementsByProperty( node, name )
{
    var elems = new Array();

    if( node.getAttribute( name ) )
        elems.push( node );

    for( var counter = 0; counter < node.childNodes.length; ++counter )
    {
        if( node.childNodes[counter].nodeType == 1 )
        {
            var subElems = getElementsByProperty( node.childNodes[counter], name );
            elems = elems.concat( subElems );
        }
    }
    return elems;
}

/** Event handler for key press.
 *
 *  @param aEvt the event
 */
function onKeyDown( aEvt )
{
   if ( !aEvt )
       aEvt = window.event;

   code = aEvt.keyCode || aEvt.charCode;

   if ( !processingEffect && keyCodeDictionary[currentMode] && keyCodeDictionary[currentMode][code] )
       return keyCodeDictionary[currentMode][code]();
   else
       document.onkeypress = onKeyPress;
}
//Set event handler for key down.
document.onkeydown = onKeyDown;

/** Event handler for key press.
 *
 *  @param aEvt the event
 */
function onKeyPress( aEvt )
{
   document.onkeypress = null;

   if ( !aEvt )
       aEvt = window.event;

   str = String.fromCharCode( aEvt.keyCode || aEvt.charCode );

   if ( !processingEffect && charCodeDictionary[currentMode] && charCodeDictionary[currentMode][str] )
       return charCodeDictionary[currentMode][str]();
}

/** Function to supply the default key code dictionary.
 *
 *  @returns default key code dictionary
 */
function getDefaultKeyCodeDictionary()
{
   var keyCodeDict = new Object();

   keyCodeDict[SLIDE_MODE] = new Object();
   keyCodeDict[INDEX_MODE] = new Object();

   // slide mode
   keyCodeDict[SLIDE_MODE][LEFT_KEY]
       = function() { return dispatchEffects(-1); };
   keyCodeDict[SLIDE_MODE][RIGHT_KEY]
       = function() { return dispatchEffects(1); };
   keyCodeDict[SLIDE_MODE][UP_KEY]
       = function() { return skipEffects(-1); };
   keyCodeDict[SLIDE_MODE][DOWN_KEY]
       = function() { return skipEffects(1); };
   keyCodeDict[SLIDE_MODE][PAGE_UP_KEY]
       = function() { return switchSlide( -1, true ); };
   keyCodeDict[SLIDE_MODE][PAGE_DOWN_KEY]
       = function() { return switchSlide( 1, true ); };
   keyCodeDict[SLIDE_MODE][HOME_KEY]
       = function() { return aSlideShow.displaySlide( 0, true ); };
   keyCodeDict[SLIDE_MODE][END_KEY]
       = function() { return aSlideShow.displaySlide( theMetaDoc.nNumberOfSlides - 1, true ); };
   keyCodeDict[SLIDE_MODE][SPACE_KEY]
       = function() { return dispatchEffects(1); };

   // index mode
   keyCodeDict[INDEX_MODE][LEFT_KEY]
       = function() { return indexSetPageSlide( theSlideIndexPage.selectedSlideIndex - 1 ); };
   keyCodeDict[INDEX_MODE][RIGHT_KEY]
       = function() { return indexSetPageSlide( theSlideIndexPage.selectedSlideIndex + 1 ); };
   keyCodeDict[INDEX_MODE][UP_KEY]
       = function() { return indexSetPageSlide( theSlideIndexPage.selectedSlideIndex - theSlideIndexPage.indexColumns ); };
   keyCodeDict[INDEX_MODE][DOWN_KEY]
       = function() { return indexSetPageSlide( theSlideIndexPage.selectedSlideIndex + theSlideIndexPage.indexColumns ); };
   keyCodeDict[INDEX_MODE][PAGE_UP_KEY]
       = function() { return indexSetPageSlide( theSlideIndexPage.selectedSlideIndex - theSlideIndexPage.getTotalThumbnails() ); };
   keyCodeDict[INDEX_MODE][PAGE_DOWN_KEY]
       = function() { return indexSetPageSlide( theSlideIndexPage.selectedSlideIndex + theSlideIndexPage.getTotalThumbnails() ); };
   keyCodeDict[INDEX_MODE][HOME_KEY]
       = function() { return indexSetPageSlide( 0 ); };
   keyCodeDict[INDEX_MODE][END_KEY]
       = function() { return indexSetPageSlide( theMetaDoc.nNumberOfSlides - 1 ); };
   keyCodeDict[INDEX_MODE][ENTER_KEY]
       = function() { return toggleSlideIndex(); };
   keyCodeDict[INDEX_MODE][SPACE_KEY]
       = function() { return toggleSlideIndex(); };
   keyCodeDict[INDEX_MODE][ESCAPE_KEY]
       = function() { return abandonIndexMode(); };

   return keyCodeDict;
}

/** Function to supply the default char code dictionary.
 *
 *  @returns default char code dictionary
 */
function getDefaultCharCodeDictionary()
{
   var charCodeDict = new Object();

   charCodeDict[SLIDE_MODE] = new Object();
   charCodeDict[INDEX_MODE] = new Object();

   // slide mode
   charCodeDict[SLIDE_MODE]['i']
       = function () { return toggleSlideIndex(); };

   // index mode
   charCodeDict[INDEX_MODE]['i']
       = function () { return toggleSlideIndex(); };
   charCodeDict[INDEX_MODE]['-']
       = function () { return theSlideIndexPage.decreaseNumberOfColumns(); };
   charCodeDict[INDEX_MODE]['=']
       = function () { return theSlideIndexPage.increaseNumberOfColumns(); };
   charCodeDict[INDEX_MODE]['+']
       = function () { return theSlideIndexPage.increaseNumberOfColumns(); };
   charCodeDict[INDEX_MODE]['0']
       = function () { return theSlideIndexPage.resetNumberOfColumns(); };

   return charCodeDict;
}


function slideOnMouseDown( aEvt )
{
   if (!aEvt)
       aEvt = window.event;

   var nOffset = 0;

   if( aEvt.button == 0 )
       nOffset = 1;
   else if( aEvt.button == 2 )
       nOffset = -1;

   if( 0 != nOffset )
       dispatchEffects( nOffset );
}

/** Event handler for mouse wheel events in slide mode.
 *  based on http://adomas.org/javascript-mouse-wheel/
 *
 *  @param aEvt the event
 */
function slideOnMouseWheel(aEvt)
{
   var delta = 0;

   if (!aEvt)
       aEvt = window.event;

   if (aEvt.wheelDelta)
   { // IE Opera
       delta = aEvt.wheelDelta/120;
   }
   else if (aEvt.detail)
   { // MOZ
       delta = -aEvt.detail/3;
   }

   if (delta > 0)
       skipEffects(-1);
   else if (delta < 0)
       skipEffects(1);

   if (aEvt.preventDefault)
       aEvt.preventDefault();

   aEvt.returnValue = false;
}

//Mozilla
if( window.addEventListener )
{
   window.addEventListener( 'DOMMouseScroll', function( aEvt ) { return mouseHandlerDispatch( aEvt, MOUSE_WHEEL ); }, false );
}

//Opera Safari OK - may not work in IE
window.onmousewheel
    = function( aEvt ) { return mouseHandlerDispatch( aEvt, MOUSE_WHEEL ); };

/** Function to handle all mouse events.
 *
 *  @param  aEvt    event
 *  @param  anAction  type of event (e.g. mouse up, mouse wheel)
 */
function mouseHandlerDispatch( aEvt, anAction )
{
   if( !aEvt )
       aEvt = window.event;

   var retVal = true;

   if ( mouseHandlerDictionary[currentMode] && mouseHandlerDictionary[currentMode][anAction] )
   {
       var subRetVal = mouseHandlerDictionary[currentMode][anAction]( aEvt );

       if( subRetVal != null && subRetVal != undefined )
           retVal = subRetVal;
   }

   if( aEvt.preventDefault && !retVal )
       aEvt.preventDefault();

   aEvt.returnValue = retVal;

   return retVal;
}

//Set mouse event handler.
document.onmousedown = function( aEvt ) { return mouseHandlerDispatch( aEvt, MOUSE_DOWN ); };
//document.onmousemove = function( aEvt ) { return mouseHandlerDispatch( aEvt, MOUSE_MOVE ); };

/** Function to supply the default mouse handler dictionary.
 *
 *  @returns default mouse handler dictionary
 */
function getDefaultMouseHandlerDictionary()
{
   var mouseHandlerDict = new Object();

   mouseHandlerDict[SLIDE_MODE] = new Object();
   mouseHandlerDict[INDEX_MODE] = new Object();

  // slide mode
   mouseHandlerDict[SLIDE_MODE][MOUSE_DOWN]
       = function( aEvt ) { return slideOnMouseDown( aEvt ); };
   mouseHandlerDict[SLIDE_MODE][MOUSE_WHEEL]
       = function( aEvt ) { return slideOnMouseWheel( aEvt ); };

   // index mode
   mouseHandlerDict[INDEX_MODE][MOUSE_DOWN]
       = function( aEvt ) { return toggleSlideIndex(); };

   return mouseHandlerDict;
}

/** Function to set the page and active slide in index view.
*
*  @param nIndex index of the active slide
*
*  NOTE: To force a redraw,
*  set INDEX_OFFSET to -1 before calling indexSetPageSlide().
*
*  This is necessary for zooming (otherwise the index might not
*  get redrawn) and when switching to index mode.
*
*  INDEX_OFFSET = -1
*  indexSetPageSlide(activeSlide);
*/
function indexSetPageSlide( nIndex )
{
   var aMetaSlideSet = theMetaDoc.aMetaSlideSet;
   nIndex = getSafeIndex( nIndex, 0, aMetaSlideSet.length - 1 );

   //calculate the offset
   var nSelectedThumbnailIndex = nIndex % theSlideIndexPage.getTotalThumbnails();
   var offset = nIndex - nSelectedThumbnailIndex;

   if( offset < 0 )
       offset = 0;

   //if different from kept offset, then record and change the page
   if( offset != INDEX_OFFSET )
   {
       INDEX_OFFSET = offset;
       displayIndex( INDEX_OFFSET );
   }

   //set the selected thumbnail and the current slide
   theSlideIndexPage.setSelection( nSelectedThumbnailIndex );
}


/*****
 * @jessyinkend
 *
 *  The above code is a derivative work of some parts of the JessyInk project.
 *  @source http://code.google.com/p/jessyink/
 */





/*****
 * @licstart
 *
 * The following is the license notice for the part of JavaScript code of this
 * page included between the '@stdlibstart' and the '@stdlibend' notes.
 */

/*****  ******************************************************************
 *
 *  Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009
 *  Free Software Foundation, Inc.
 *
 *  The code included between the start note '@stdlibstart' and the end
 *  note '@stdlibend' is a derivative work of the GNU ISO C++ Library.
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  Under Section 7 of GPL version 3, you are granted additional
 *  permissions described in the GCC Runtime Library Exception, version
 *  3.1, as published by the Free Software Foundation.
 *
 *  You should have received a copy of the GNU General Public License and
 *  a copy of the GCC Runtime Library Exception along with this program;
 *  see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 *************************************************************************/

/*****
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided 'as is' without express or implied warranty.
 *
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided 'as is' without express or implied warranty.
 *
 ************************************************************************/

/*****
 * @licend
 *
 * The above is the license notice for the part of JavaScript code of this
 * page included between the '@stdlibstart' and the '@stdlibend' notes.
 */



/*****
 * @stdlibstart
 *
 * The following code is a porting, performed on August 2011, of a part of
 * the C++ code included into the source file stl_queue.h that is part of
 * the GNU ISO C++ Library.
 */


function PriorityQueue( aCompareFunc )
{
 this.aSequence = new Array();
 this.aCompareFunc = aCompareFunc;
}

PriorityQueue.prototype.top = function()
{
 return this.aSequence[0];
};

PriorityQueue.prototype.isEmpty = function()
{
 return ( this.size() === 0 );
};

PriorityQueue.prototype.size = function()
{
 return this.aSequence.length;
};

PriorityQueue.prototype.push = function( aValue )
{
 this.implPushHeap( 0, this.aSequence.length, 0, aValue );
};

PriorityQueue.prototype.clear = function()
{
 return this.aSequence = new Array();
};


PriorityQueue.prototype.pop = function()
{
 if( this.isEmpty() )
     return;

 var nLast = this.aSequence.length - 1;
 var aValue = this.aSequence[ nLast ];
 this.aSequence[ nLast ] = this.aSequence[ 0 ];
 this.implAdjustHeap( 0, 0, nLast, aValue );
 this.aSequence.pop();
};

PriorityQueue.prototype.implAdjustHeap = function( nFirst, nHoleIndex, nLength, aValue )
{
 var nTopIndex = nHoleIndex;
 var nSecondChild = nHoleIndex;

 while( nSecondChild < Math.floor( ( nLength - 1 ) / 2 ) )
 {
     nSecondChild = 2 * ( nSecondChild + 1 );
     if( this.aCompareFunc( this.aSequence[ nFirst + nSecondChild ],
                            this.aSequence[ nFirst + nSecondChild - 1] ) )
     {
         --nSecondChild;
     }
     this.aSequence[ nFirst + nHoleIndex ] = this.aSequence[ nFirst + nSecondChild ];
     nHoleIndex = nSecondChild;
 }

 if( ( ( nLength & 1 ) === 0 ) && ( nSecondChild === Math.floor( ( nLength - 2 ) / 2 ) ) )
 {
     nSecondChild = 2 * ( nSecondChild + 1 );
     this.aSequence[ nFirst + nHoleIndex ] = this.aSequence[ nFirst + nSecondChild - 1];
     nHoleIndex = nSecondChild - 1;
 }

 this.implPushHeap( nFirst, nHoleIndex, nTopIndex, aValue );
};

PriorityQueue.prototype.implPushHeap = function( nFirst, nHoleIndex, nTopIndex, aValue )
{
 var nParent = Math.floor( ( nHoleIndex - 1 ) / 2 );

 while( ( nHoleIndex > nTopIndex ) &&
        this.aCompareFunc( this.aSequence[ nFirst + nParent ], aValue ) )
 {
     this.aSequence[ nFirst + nHoleIndex ] = this.aSequence[ nFirst + nParent ];
     nHoleIndex = nParent;
     nParent = Math.floor( ( nHoleIndex - 1 ) / 2 );
 }
 this.aSequence[ nFirst + nHoleIndex ] = aValue;
};


/*****
 * @stdlibend
 *
 * The above code is a porting, performed on August 2011, of a part of
 * the C++ code included into the source file stl_queue.h that is part of
 * the GNU ISO C++ Library.
 */





/*****
 * @licstart
 *
 * The following is the license notice for the part of JavaScript code  of
 * this page included between the '@libreofficestart' and the '@libreofficeend'
 * notes.
 */

/*****  ******************************************************************
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

/*****
 * @licend
 *
 * The above is the license notice for the part of JavaScript code  of
 * this page included between the '@libreofficestart' and the '@libreofficeend'
 * notes.
 */



/*****
 * @libreofficestart
 *
 * Several parts of the following code are the result of the porting,
 * started on August 2011, of the C++ code included in the source files
 * placed under the folder '/slideshow/source' and subfolders.
 * @source http://cgit.freedesktop.org/libreoffice/core/tree/slideshow/source
 *
 */


window.onload = init;


// ooo elements
var aOOOElemMetaSlides = 'ooo:meta_slides';
var aOOOElemMetaSlide = 'ooo:meta_slide';
var aOOOElemTextField = 'ooo:text_field';

// ooo attributes
var aOOOAttrNumberOfSlides = 'number-of-slides';
var aOOOAttrNumberingType = 'page-numbering-type';

var aOOOAttrSlide = 'slide';
var aOOOAttrMaster = 'master';
var aOOOAttrBackgroundVisibility = 'background-visibility';
var aOOOAttrMasterObjectsVisibility = 'master-objects-visibility';
var aOOOAttrPageNumberVisibility = 'page-number-visibility';
var aOOOAttrDateTimeVisibility = 'date-time-visibility';
var aOOOAttrFooterVisibility = 'footer-visibility';
var aOOOAttrHeaderVisibility = 'header-visibility';
var aOOOAttrDateTimeField = 'date-time-field';
var aOOOAttrFooterField = 'footer-field';
var aOOOAttrHeaderField = 'header-field';

var aOOOAttrDateTimeFormat = 'date-time-format';

var aOOOAttrTextAdjust = 'text-adjust';

// Placeholder class names
var aSlideNumberClassName = 'Slide_Number';
var aDateTimeClassName = 'Date/Time';
var aFooterClassName = 'Footer';
var aHeaderClassName = 'Header';

// Creating a namespace dictionary.
var NSS = new Object();
NSS['svg']='http://www.w3.org/2000/svg';
NSS['rdf']='http://www.w3.org/1999/02/22-rdf-syntax-ns#';
NSS['xlink']='http://www.w3.org/1999/xlink';
NSS['xml']='http://www.w3.org/XML/1998/namespace';
NSS['ooo'] = 'http://xml.openoffice.org/svg/export';

// Presentation modes.
var SLIDE_MODE = 1;
var INDEX_MODE = 2;

// Mouse handler actions.
var MOUSE_UP = 1;
var MOUSE_DOWN = 2;
var MOUSE_MOVE = 3;
var MOUSE_WHEEL = 4;

// Keycodes.
var LEFT_KEY = 37;          // cursor left keycode
var UP_KEY = 38;            // cursor up keycode
var RIGHT_KEY = 39;         // cursor right keycode
var DOWN_KEY = 40;          // cursor down keycode
var PAGE_UP_KEY = 33;       // page up keycode
var PAGE_DOWN_KEY = 34;     // page down keycode
var HOME_KEY = 36;          // home keycode
var END_KEY = 35;           // end keycode
var ENTER_KEY = 13;
var SPACE_KEY = 32;
var ESCAPE_KEY = 27;

// Visibility Values
var HIDDEN = 0;
var VISIBLE = 1;
var INHERIT = 2;
var aVisibilityAttributeValue = [ 'hidden', 'visible', 'inherit' ];
var aVisibilityValue = { 'hidden' : HIDDEN, 'visible' : VISIBLE, 'inherit' : INHERIT };

// Parameters
var ROOT_NODE = document.getElementsByTagNameNS( NSS['svg'], 'svg' )[0];
var WIDTH = 0;
var HEIGHT = 0;
var INDEX_COLUMNS_DEFAULT = 3;
var INDEX_OFFSET = 0;

// Initialization.
var theMetaDoc;
var theSlideIndexPage;
var currentMode = SLIDE_MODE;
var processingEffect = false;
var nCurSlide = undefined;

// Initialize char and key code dictionaries.
var charCodeDictionary = getDefaultCharCodeDictionary();
var keyCodeDictionary = getDefaultKeyCodeDictionary();

// Initialize mouse handler dictionary.
var mouseHandlerDictionary = getDefaultMouseHandlerDictionary();

/***************************
 ** OOP support functions **
 ***************************/

function object( aObject )
{
    var F = function() {};
    F.prototype = aObject;
    return new F();
}


function extend( aSubType, aSuperType )
{
    if (!aSuperType || !aSubType)
    {
        alert('extend failed, verify dependencies');
    }
    var OP = Object.prototype;
    var sp = aSuperType.prototype;
    var rp = object( sp );
    aSubType.prototype = rp;

    rp.constructor = aSubType;
    aSubType.superclass = sp;

    // assign constructor property
    if (aSuperType != Object && sp.constructor == OP.constructor)
    {
        sp.constructor = aSuperType;
    }

    return aSubType;
}


function instantiate( TemplateClass, BaseType )
{
    if( !TemplateClass.instanceSet )
        TemplateClass.instanceSet = new Array();

    var nSize = TemplateClass.instanceSet.length;

    for( var i = 0; i < nSize; ++i )
    {
        if( TemplateClass.instanceSet[i].base === BaseType )
            return TemplateClass.instanceSet[i].instance;
    }

    TemplateClass.instanceSet[ nSize ] = new Object();
    TemplateClass.instanceSet[ nSize ].base = BaseType;
    TemplateClass.instanceSet[ nSize ].instance = TemplateClass( BaseType );

    return TemplateClass.instanceSet[ nSize ].instance;
};

// ------------------------------------------------------------------------------------------ //
/**********************************
 ** Helper functions and classes **
 **********************************/

function Rectangle( aSVGRectElem )
{
    var x = parseInt( aSVGRectElem.getAttribute( 'x' ) );
    var y = parseInt( aSVGRectElem.getAttribute( 'y' ) );
    var width = parseInt( aSVGRectElem.getAttribute( 'width' ) );
    var height = parseInt( aSVGRectElem.getAttribute( 'height' ) );

    this.left = x;
    this.right = x + width;
    this.top = y;
    this.bottom = y + height;
}

function log( message )
{
    if( typeof console == 'object' )
    {
        console.log( message );
    }
    else if( typeof opera == 'object' )
    {
        opera.postError( message );
    }
    else if( typeof java == 'object' && typeof java.lang == 'object' )
    {
        java.lang.System.out.println( message );
    }
}

function getNSAttribute( sNSPrefix, aElem, sAttrName )
{
    if( !aElem ) return null;
    if( aElem.hasAttributeNS( NSS[sNSPrefix], sAttrName ) )
    {
        return aElem.getAttributeNS( NSS[sNSPrefix], sAttrName );
    }
    return null;
}

function getOOOAttribute( aElem, sAttrName )
{
    return getNSAttribute( 'ooo', aElem, sAttrName );
}

function setNSAttribute( sNSPrefix, aElem, sAttrName, aValue )
{
    if( !aElem ) return false;
    if( 'setAttributeNS' in aElem )
    {
        aElem.setAttributeNS( NSS[sNSPrefix], sAttrName, aValue );
        return true;
    }
    else
    {
        aElem.setAttribute(sNSPrefix + ':' + sAttrName, aValue );
        return true;
    }
}

function setOOOAttribute( aElem, sAttrName, aValue )
{
    return setNSAttribute( 'ooo', aElem, sAttrName, aValue );
}

function checkElemAndSetAttribute( aElem, sAttrName, aValue )
{
    if( aElem )
        aElem.setAttribute( sAttrName, aValue );
}

function getElementsByClassName( aElem, sClassName )
{

    var aElementSet = new Array();
    // not all browsers support the 'getElementsByClassName' method
    if( 'getElementsByClassName' in aElem )
    {
        aElementSet = aElem.getElementsByClassName( sClassName );
    }
    else
    {
        var aElementSetByClassProperty = getElementsByProperty( aElem, 'class' );
        for( var i = 0; i < aElementSetByClassProperty.length; ++i )
        {
            var sAttrClassName = aElementSetByClassProperty[i].getAttribute( 'class' );
            if( sAttrClassName == sClassName )
            {
                aElementSet.push( aElementSetByClassProperty[i] );
            }
        }
    }
    return aElementSet;
}

function getElementByClassName( aElem, sClassName /*, sTagName */)
{
    var aElementSet = getElementsByClassName( aElem, sClassName );
    if ( aElementSet.length == 1 )
        return aElementSet[0];
    else
        return null;
}

function getClassAttribute(  aElem )
{
    if( aElem )
        return aElem.getAttribute( 'class' );
    return '';
}

function initVisibilityProperty( aElement )
{
    var nVisibility = VISIBLE;
    var sVisibility = aElement.getAttribute( 'visibility' );
    if( sVisibility ) nVisibility = aVisibilityValue[ sVisibility ];
    return nVisibility;
}

function setElementVisibility( aElement, nCurrentVisibility, nNewVisibility )
{
    if( nCurrentVisibility !=  nNewVisibility )
    {
        checkElemAndSetAttribute( aElement, 'visibility', aVisibilityAttributeValue[nNewVisibility] );
        return nNewVisibility;
    }
    return nCurrentVisibility;
}

function getSafeIndex( nIndex, nMin, nMax )
{
    if( nIndex < nMin )
        return nMin;
    else if( nIndex > nMax )
        return nMax;
    else
        return nIndex;
}



// ------------------------------------------------------------------------------------------ //
/*********************
 ** Debug Utilities **
 *********************/

function DebugPrinter()
{
    this.bEnabled = false;
}


DebugPrinter.prototype.on = function()
{
    this.bEnabled = true;
};

DebugPrinter.prototype.off = function()
{
    this.bEnabled = false;
};

DebugPrinter.prototype.isEnabled = function()
{
    return this.bEnabled;
};

DebugPrinter.prototype.print = function( sMessage, nTime )
{
    if( this.isEnabled() )
    {
        sInfo = 'DBG: ' + sMessage;
        if( nTime )
            sInfo += ' (at: ' + String( nTime / 1000 ) + 's)';
            log( sInfo );
        }
    };


// - Debug Printers -
var NAVDBG = new DebugPrinter();
NAVDBG.off();

var ANIMDBG = new DebugPrinter();
ANIMDBG.off();

var aRegisterEventDebugPrinter = new DebugPrinter();
aRegisterEventDebugPrinter.off();

var aTimerEventQueueDebugPrinter = new DebugPrinter();
aTimerEventQueueDebugPrinter.off();

var aEventMultiplexerDebugPrinter = new DebugPrinter();
aEventMultiplexerDebugPrinter.off();

var aNextEffectEventArrayDebugPrinter = new DebugPrinter();
aNextEffectEventArrayDebugPrinter.off();

var aActivityQueueDebugPrinter = new DebugPrinter();
aActivityQueueDebugPrinter.off();

var aAnimatedElementDebugPrinter = new DebugPrinter();
aAnimatedElementDebugPrinter.off();



// ------------------------------------------------------------------------------------------ //
/******************
 ** Core Classes **
 ******************/

/** Class MetaDocument **
 *  This class provides a pool of properties related to the whole presentation and
 *  it is responsible for initializing the set of MetaSlide objects that handle
 *  the meta information for each slide.
 */
function MetaDocument( aMetaDocElem )
{
    this.nNumberOfSlides = parseInt( aMetaDocElem.getAttributeNS( NSS['ooo'], aOOOAttrNumberOfSlides ) );
    assert( typeof this.nNumberOfSlides == 'number' && this.nNumberOfSlides > 0,
            'MetaDocument: number of slides is zero or undefined.' );
    this.startSlideNumber = 0;
    this.sPageNumberingType = aMetaDocElem.getAttributeNS( NSS['ooo'], aOOOAttrNumberingType ) || 'arabic';
    this.aMetaSlideSet = new Array();
    this.aMasterPageSet = new Object();
    this.aTextFieldSet = new Array();
    this.slideNumberField =  new SlideNumberField( this.startSlideNumber + 1, this.sPageNumberingType );

    this.aSlideAnimationsMap = new Object();
    this.initSlideAnimationsMap();


    for( var i = 0; i < this.nNumberOfSlides; ++i )
    {
        var sMetaSlideId = aOOOElemMetaSlide + '_' + i;
        this.aMetaSlideSet.push( new MetaSlide( sMetaSlideId, this ) );
    }
    assert( this.aMetaSlideSet.length == this.nNumberOfSlides,
            'MetaDocument: aMetaSlideSet.length != nNumberOfSlides.' );
    //this.aMetaSlideSet[ this.startSlideNumber ].show();
}

MetaDocument.prototype.initPlaceholderShapes = function()
{
    this.aMetaSlideSet[0].initPlaceholderShapes();
};

MetaDocument.prototype.initSlideAnimationsMap = function()
{
    var aAnimationsSection = document.getElementById( 'presentation-animations' );
    if( aAnimationsSection )
    {
        var aAnimationsDefSet = aAnimationsSection.getElementsByTagName( 'defs' );

        for( var i = 0; i < aAnimationsDefSet.length; ++i )
        {
            var sSlideId = aAnimationsDefSet[i].getAttributeNS( NSS['ooo'], aOOOAttrSlide );
            var aChildSet = getElementChildren( aAnimationsDefSet[i] );
            if( sSlideId && ( aChildSet.length === 1 ) )
            {
                this.aSlideAnimationsMap[ sSlideId ] = aChildSet[0];
            }
        }
    }
};



/** Class MetaSlide **
 *  This class is responsible for managing the visibility of all master page shapes
 *  and background related to a given slide element; it performs the creation and
 *  the initialization of each Text Field object.
 */
function MetaSlide( sMetaSlideId, aMetaDoc )
{
    this.theDocument = document;
    this.id = sMetaSlideId;
    this.theMetaDoc = aMetaDoc;
    this.element = this.theDocument.getElementById( this.id );
    assert( this.element, 'MetaSlide: meta_slide element <' + this.id + '> not found.' );
    // - Initialize the Slide Element -
    this.slideId = this.element.getAttributeNS( NSS['ooo'], aOOOAttrSlide );
    this.slideElement = this.theDocument.getElementById( this.slideId );
    assert( this.slideElement, 'MetaSlide: slide element <' + this.slideId + '> not found.' );
    // - Initialize the Target Master Page Element -
    this.masterPage = this.initMasterPage();
    // - Initialize Background -
    //this.aBackground                 = getElementByClassName( this.aSlide, 'Background' );
    // - Initialize Visibility Properties -
    this.nAreMasterObjectsVisible     = this.initVisibilityProperty( aOOOAttrMasterObjectsVisibility,  VISIBLE );
    this.nIsBackgroundVisible         = this.initVisibilityProperty( aOOOAttrBackgroundVisibility,     VISIBLE );
    this.nIsPageNumberVisible         = this.initVisibilityProperty( aOOOAttrPageNumberVisibility,     HIDDEN );
    this.nIsDateTimeVisible           = this.initVisibilityProperty( aOOOAttrDateTimeVisibility,       VISIBLE );
    this.nIsFooterVisible             = this.initVisibilityProperty( aOOOAttrFooterVisibility,         VISIBLE );
    this.nIsHeaderVisible             = this.initVisibilityProperty( aOOOAttrHeaderVisibility,         VISIBLE );
    // - Initialize Master Page Text Fields (Placeholders)-
    this.aMPTextFieldSet = new Object();
    this.aMPTextFieldSet[aSlideNumberClassName]   = this.initSlideNumberField();
    this.aMPTextFieldSet[aDateTimeClassName]      = this.initDateTimeField( aOOOAttrDateTimeField );
    this.aMPTextFieldSet[aFooterClassName]        = this.initFixedTextField( aOOOAttrFooterField );
    this.aMPTextFieldSet[aHeaderClassName]        = this.initFixedTextField( aOOOAttrHeaderField );

    // - Initialize Slide Animations Handler
    this.aSlideAnimationsHandler = new SlideAnimations( aSlideShow.getContext() );
    this.aSlideAnimationsHandler.importAnimations( this.getSlideAnimationsRoot() );
    this.aSlideAnimationsHandler.parseElements();
    if( false && this.aSlideAnimationsHandler.aRootNode )
        log( this.aSlideAnimationsHandler.aRootNode.info( true ) );
}

/*** MetaSlide methods ***/
MetaSlide.prototype =
{
    /*** public methods ***/
    hide : function()
    {
        checkElemAndSetAttribute( this.slideElement, 'visibility', 'hidden' );

        this.masterPage.hide();
        this.masterPage.hideBackground();

        var aFieldSet = this.aMPTextFieldSet;
        var aShapeSet = this.masterPage.aPlaceholderShapeSet;
        if( aFieldSet[aSlideNumberClassName] )         aFieldSet[aSlideNumberClassName].hide( aShapeSet[aSlideNumberClassName] );
        if( aFieldSet[aDateTimeClassName] )            aFieldSet[aDateTimeClassName].hide( aShapeSet[aDateTimeClassName] );
        if( aFieldSet[aFooterClassName] )              aFieldSet[aFooterClassName].hide( aShapeSet[aFooterClassName] );
        if( aFieldSet[aHeaderClassName] )              aFieldSet[aHeaderClassName].hide( aShapeSet[aHeaderClassName] );
    },

    hideExceptMaster : function()
    {
        checkElemAndSetAttribute( this.slideElement, 'visibility', 'hidden' );
    },

    show : function()
    {
        checkElemAndSetAttribute( this.slideElement, 'visibility', 'visible' );

        this.masterPage.setVisibility( this.nAreMasterObjectsVisible );
        this.masterPage.setVisibilityBackground( this.nIsBackgroundVisible );


        this.setTextFieldVisibility( aSlideNumberClassName, this.nIsPageNumberVisible );
        this.setTextFieldVisibility( aDateTimeClassName, this.nIsDateTimeVisible );
        this.setTextFieldVisibility( aFooterClassName, this.nIsFooterVisible );
        this.setTextFieldVisibility( aHeaderClassName, this.nIsHeaderVisible );
    },

    getMasterPageId : function()
    {
        return this.masterPage.id;
    },

    getMasterPageElement : function()
    {
        return this.masterPage.element;
    },

    getBackground : function()
    {
        return getElementByClassName( this.slideElement, 'Background' );
    },

    getMasterPageBackground : function()
    {
        return this.masterPage.background;
    },

    /*** private methods ***/
    initMasterPage : function()
    {
        var sMasterPageId = this.element.getAttributeNS( NSS['ooo'], aOOOAttrMaster );
        if( !this.theMetaDoc.aMasterPageSet.hasOwnProperty( sMasterPageId ) )
            this.theMetaDoc.aMasterPageSet[ sMasterPageId ] = new MasterPage( sMasterPageId );
        return this.theMetaDoc.aMasterPageSet[ sMasterPageId ];
    },

    initVisibilityProperty : function( aVisibilityAttribute, nDefaultValue )
    {
        var nVisibility = nDefaultValue;
        var sVisibility = getOOOAttribute( this.element, aVisibilityAttribute );
        if( sVisibility )
            nVisibility = aVisibilityValue[ sVisibility ];
        return nVisibility;
    },

    initSlideNumberField : function()
    {
        return this.theMetaDoc.slideNumberField;
    },

    initDateTimeField : function( aOOOAttrDateTimeField )
    {
        var sTextFieldId = getOOOAttribute( this.element, aOOOAttrDateTimeField );
        if( !sTextFieldId )  return null;

        var nLength = aOOOElemTextField.length + 1;
        var nIndex = parseInt(sTextFieldId.substring( nLength ) );
        if( typeof nIndex != 'number') return null;

        if( !this.theMetaDoc.aTextFieldSet[ nIndex ] )
        {
            var aTextField;
            var aTextFieldElem = document.getElementById( sTextFieldId );
            var sClassName = getClassAttribute( aTextFieldElem );
            if( sClassName == 'FixedDateTimeField' )
            {
                aTextField = new FixedTextField( aTextFieldElem );
            }
            else if( sClassName == 'VariableDateTimeField' )
            {
                aTextField = new VariableDateTimeField( aTextFieldElem );
            }
            else
            {
                aTextField = null;
            }
            this.theMetaDoc.aTextFieldSet[ nIndex ] = aTextField;
        }
        return this.theMetaDoc.aTextFieldSet[ nIndex ];
    },

    initFixedTextField : function( aOOOAttribute )
    {
        var sTextFieldId = getOOOAttribute( this.element, aOOOAttribute );
        if( !sTextFieldId ) return null;

        var nLength = aOOOElemTextField.length + 1;
        var nIndex = parseInt( sTextFieldId.substring( nLength ) );
        if( typeof nIndex != 'number') return null;

        if( !this.theMetaDoc.aTextFieldSet[ nIndex ] )
        {
            var aTextFieldElem = document.getElementById( sTextFieldId );
            this.theMetaDoc.aTextFieldSet[ nIndex ]
                = new FixedTextField( aTextFieldElem );
        }
        return this.theMetaDoc.aTextFieldSet[ nIndex ];
    },

    setTextFieldVisibility : function( sClassName, nVisible )
    {
        var aTextField = this.aMPTextFieldSet[ sClassName ];
        var aPlaceholderShape = this.masterPage.aPlaceholderShapeSet[ sClassName ];
        if( !aTextField ) return;
        aTextField.setVisibility( this.nAreMasterObjectsVisible & nVisible, aPlaceholderShape );
    },

    getSlideAnimationsRoot : function()
    {
        return this.theMetaDoc.aSlideAnimationsMap[ this.slideId ];
    }

};

/** Class MasterPage **
 *  This class gives access to a master page element, its background and
 *  each placeholder shape present in the master page element.
 */
function MasterPage( sMasterPageId )
{
    this.id = sMasterPageId;
    this.element = document.getElementById( this.id );
    assert( this.element, 'MasterPage: master page element <' + this.id + '> not found.' );
    this.background = getElementByClassName( this.element, 'Background' );
    this.backgroundId = this.background.getAttribute( 'id' );
    this.backgroundVisibility = initVisibilityProperty( this.background );
    this.backgroundObjects = getElementByClassName( this.element, 'BackgroundObjects' );
    this.backgroundObjectsId = this.backgroundObjects.getAttribute( 'id' );
    this.backgroundObjectsVisibility = initVisibilityProperty( this.backgroundObjects );
    this.aPlaceholderShapeSet = new Object();
    this.initPlaceholderShapes();
}

/*** MasterPage methods ***/
MasterPage.prototype =
{
     /*** public method ***/
    setVisibility : function( nVisibility )
    {
        this.backgroundObjectsVisibility = setElementVisibility( this.backgroundObjects, this.backgroundObjectsVisibility, nVisibility );
    },

    setVisibilityBackground : function( nVisibility )
    {
        this.backgroundVisibility = setElementVisibility( this.background, this.backgroundVisibility, nVisibility );
    },

    hide : function()
    {
        this.setVisibility( HIDDEN );
    },

    show : function()
    {
        this.setVisibility( VISIBLE );
    },

    hideBackground : function()
    {
        this.setVisibilityBackground( HIDDEN );
    },

    showBackground : function()
    {
        this.setVisibilityBackground( VISIBLE );
    },

    /*** private method ***/
    initPlaceholderShapes : function()
    {
        this.aPlaceholderShapeSet[ aSlideNumberClassName ] = new PlaceholderShape( this, aSlideNumberClassName );
        this.aPlaceholderShapeSet[ aDateTimeClassName ] = new PlaceholderShape( this, aDateTimeClassName );
        this.aPlaceholderShapeSet[ aFooterClassName ] = new PlaceholderShape( this, aFooterClassName );
        this.aPlaceholderShapeSet[ aHeaderClassName ] = new PlaceholderShape( this, aHeaderClassName );
    }
};

/** Class PlaceholderShape **
 *  This class manages the visibility and the text content of a placeholder shape.
 */
function PlaceholderShape( aMasterPage, sClassName )
{
    this.masterPage = aMasterPage;
    this.className = sClassName;
    this.element = null;
    this.textElement = null;

    this.init();
}

/* public methods */
PlaceholderShape.prototype.setTextContent = function( sText )
{
    if( !this.textElement )
    {
        log( 'error: PlaceholderShape.setTextContent: text element is not valid in placeholder of type '
                + this.className + ' that belongs to master slide ' + this.masterPage.id );
        return;
    }
    this.textElement.textContent = sText;
};

PlaceholderShape.prototype.setVisibility = function( nVisibility )
{
    this.element.setAttribute( 'visibility', aVisibilityAttributeValue[nVisibility] );
};

PlaceholderShape.prototype.show = function()
{
    this.element.setAttribute( 'visibility', 'visible' );
};

PlaceholderShape.prototype.hide = function()
{
    this.element.setAttribute( 'visibility', 'hidden' );
};

/* private methods */
PlaceholderShape.prototype.init = function()
{
    var aShapeElem = getElementByClassName( this.masterPage.backgroundObjects, this.className );
    if( !aShapeElem ) return;

    this.element = aShapeElem;
    this.element.setAttribute( 'visibility', 'hidden' );

    this.textElement = getElementByClassName( this.element , 'PlaceholderText' );
    if( !this.textElement )  return;


    var aSVGRectElemSet = this.element.getElementsByTagName( 'rect' );
    if( aSVGRectElemSet.length != 1) return;

    var aRect = new Rectangle( aSVGRectElemSet[0] );

    var sTextAdjust = getOOOAttribute( this.element, aOOOAttrTextAdjust ) || 'left';
    var sTextAnchor, sX;
    if( sTextAdjust == 'left' )
    {
        sTextAnchor = 'start';
        sX = String( aRect.left );
    }
    else if( sTextAdjust == 'right' )
    {
        sTextAnchor = 'end';
        sX = String( aRect.right );
    }
    else if( sTextAdjust == 'center' )
    {
        sTextAnchor = 'middle';
        var nMiddle = ( aRect.left + aRect.right ) / 2;
        sX = String( parseInt( String( nMiddle ) ) );
    }


    this.textElement.setAttribute( 'text-anchor', sTextAnchor );
    this.textElement.setAttribute( 'x', sX );
};


// ------------------------------------------------------------------------------------------ //
/********************************
 ** Text Field Class Hierarchy **
 ********************************/

/** Class TextField **
 *  This class is the root abstract class of the hierarchy.
 *  The 'shapeElement' property is the shape element to which
 *  this TextField object provides the text content.
 */
function TextField( aTextFieldElem )
{
    this.bIsUpdated = false;
}

/*** TextField public methods ***/
TextField.prototype.getShapeElement = function()
{
    return this.shapeElement;
};

TextField.prototype.setVisibility = function( nVisibility, aPlaceholderShape )
{
    if( !this.bIsUpdated )
    {
        if( nVisibility )
        {
            this.update( aPlaceholderShape );
            this.bIsUpdated = true;
        }
        aPlaceholderShape.setVisibility( nVisibility );
    }
    else if( !nVisibility )
    {
        aPlaceholderShape.hide();
        this.bIsUpdated = false;
    }
};

TextField.prototype.show = function( aPlaceholderShape )
{
    this.setVisibility( VISIBLE, aPlaceholderShape );
};

TextField.prototype.hide = function( aPlaceholderShape )
{
    this.setVisibility( HIDDEN, aPlaceholderShape );
};


/** Class FixedTextField **
 *  This class handles text field with a fixed text.
 *  The text content is provided by the 'text' property.
 */
function FixedTextField( aTextFieldElem )
{
    TextField.call( this, aTextFieldElem );
    this.text = aTextFieldElem.textContent;
}
extend( FixedTextField, TextField );

FixedTextField.prototype.update = function( aPlaceholderShape )
{
    aPlaceholderShape.setTextContent( this.text );
};


/** Class VariableDateTimeField **
 *  Provide the text content for the related shape by generating the current
 *  date/time in the format specified by the 'dateTimeFormat' property.
 */
function VariableDateTimeField( aTextFieldElem )
{
    VariableDateTimeField.superclass.constructor.call( this, aTextFieldElem );
    this.dateTimeFormat = getOOOAttribute( aTextFieldElem, aOOOAttrDateTimeFormat );
}
extend( VariableDateTimeField, TextField );

/*** public methods ***/
VariableDateTimeField.prototype.update = function( aPlaceholderShape )
{
    var sText = this.createDateTimeText( this.dateTimeFormat );
    aPlaceholderShape.setTextContent( sText );
};

VariableDateTimeField.prototype.createDateTimeText = function( sDateTimeFormat )
{
    // TODO handle date/time format
    var aDate = Date();
    var sDate = aDate.toLocaleString();
    return sDate;
};

/** Class SlideNumberField **
 *  Provides the text content to the related shape by generating
 *  the current page number in the given page numbering type.
 */
function SlideNumberField( nInitialSlideNumber, sPageNumberingType )
{
    SlideNumberField.superclass.constructor.call( this, null );
    this.nInitialSlideNumber = nInitialSlideNumber;
    this.pageNumberingType = sPageNumberingType;

}
extend( SlideNumberField, TextField );

/*** public methods ***/
SlideNumberField.prototype.getNumberingType = function()
{
    return this.pageNumberingType;
};

SlideNumberField.prototype.update = function( aPlaceholderShape )
{
    var nSlideNumber;
    if( nCurSlide === undefined )
        nSlideNumber = this.nInitialSlideNumber;
    else
        nSlideNumber = nCurSlide + 1;
    var sText = this.createSlideNumberText( nSlideNumber, this.getNumberingType() );
    aPlaceholderShape.setTextContent( sText );
};

SlideNumberField.prototype.createSlideNumberText = function( nSlideNumber, sNumberingType )
{
    // TODO handle page numbering type
    return String( nSlideNumber );
};



//------------------------------------------------------------------------------------------- //
/********************************
 ** Slide Index Classes **
 ********************************/

/** Class SlideIndePagex **
 *  This class is responsible for handling the slide index page
 */
function SlideIndexPage()
{
    this.pageElementId = 'slide_index';
    this.pageBgColor = 'rgb(252,252,252)';
    this.pageElement = this.createPageElement();
    assert( this.pageElement, 'SlideIndexPage: pageElement is not valid' );
    this.indexColumns = INDEX_COLUMNS_DEFAULT;
    this.totalThumbnails = this.indexColumns * this.indexColumns;
    this.selectedSlideIndex = undefined;

    // set up layout paramers
    this.xSpacingFactor = 600/28000;
    this.ySpacingFactor = 450/21000;
    this.xSpacing = WIDTH * this.xSpacingFactor;
    this.ySpacing = HEIGHT * this.ySpacingFactor;
    this.halfBorderWidthFactor = ( 300/28000 ) * ( this.indexColumns / 3 );
    this.halfBorderWidth = WIDTH * this.halfBorderWidthFactor;
    this.borderWidth = 2 * this.halfBorderWidth;
    // the following formula is used to compute the slide shrinking factor:
    // scaleFactor = ( WIDTH - ( columns + 1 ) * xSpacing ) / ( columns * ( WIDTH + borderWidth ) )
    // indeed we can divide everything by WIDTH:
    this.scaleFactor = ( 1 - ( this.indexColumns + 1 ) * this.xSpacingFactor ) /
                            ( this.indexColumns * ( 1 + 2 * this.halfBorderWidthFactor ) );

    // We create a Thumbnail Border and Thumbnail MouseArea rectangle template that will be
    // used by every Thumbnail. The Mouse Area rectangle is used in order to trigger the
    // mouseover event properly even when the slide background is hidden.
    this.thumbnailMouseAreaTemplateId = 'thumbnail_mouse_area';
    this.thumbnailMouseAreaTemplateElement = null;
    this.thumbnailBorderTemplateId = 'thumbnail_border';
    this.thumbnailBorderTemplateElement = null;
    this.createTemplateElements();

    // Now we create the grid of thumbnails
    this.aThumbnailSet = new Array( this.totalThumbnails );
    for( var i = 0; i < this.totalThumbnails; ++i )
    {
        this.aThumbnailSet[i] = new Thumbnail( this, i );
        this.aThumbnailSet[i].updateView();
    }

//  this.curThumbnailIndex = this.selectedSlideIndex % this.totalThumbnails;
    this.curThumbnailIndex = 0;
//    this.aThumbnailSet[ this.curThumbnailIndex ].select();
}


/* public methods */
SlideIndexPage.prototype.getTotalThumbnails = function()
{
    return this.totalThumbnails;
};

SlideIndexPage.prototype.show = function()
{
    this.pageElement.setAttribute( 'display', 'inherit' );
};

SlideIndexPage.prototype.hide = function()
{
    this.pageElement.setAttribute( 'display', 'none' );
};

/** setSelection
 *
 * Change the selected thumbnail from the current one to the thumbnail with index nIndex.
 *
 * @param nIndex - the thumbnail index
 */
SlideIndexPage.prototype.setSelection = function( nIndex )
{
    nIndex = getSafeIndex( nIndex, 0, this.getTotalThumbnails() - 1 );
    if( this.curThumbnailIndex != nIndex )
    {
        this.aThumbnailSet[ this.curThumbnailIndex ].unselect();
        this.aThumbnailSet[ nIndex ].select();
        this.curThumbnailIndex = nIndex;
    }
    this.selectedSlideIndex = this.aThumbnailSet[ nIndex ].slideIndex;
};

SlideIndexPage.prototype.createPageElement = function()
{
    var aPageElement = document.createElementNS( NSS['svg'], 'g' );
    aPageElement.setAttribute( 'id', this.pageElementId );
    aPageElement.setAttribute( 'display', 'none' );

    // the slide index page background
    var sPageBgColor = this.pageBgColor + ';';
    var aRectElement = document.createElementNS( NSS['svg'], 'rect' );
    aRectElement.setAttribute( 'x', 0 );
    aRectElement.setAttribute( 'y', 0 );
    aRectElement.setAttribute( 'width', WIDTH );
    aRectElement.setAttribute( 'height', HEIGHT );
    aRectElement.setAttribute( 'style', 'stroke:none;fill:' + sPageBgColor );

    aPageElement.appendChild( aRectElement );
    // The index page is appended after all slide elements
    // so when it is displayed it covers them all
    ROOT_NODE.appendChild( aPageElement );
    return( document.getElementById( this.pageElementId ) );
};

SlideIndexPage.prototype.createTemplateElements = function()
{
    // We define a Rect element as a template of thumbnail border for all slide-thumbnails.
    // The stroke color is defined individually by each thumbnail according to
    // its selection status.
    var aDefsElement = document.createElementNS( NSS['svg'], 'defs' );
    var aRectElement = document.createElementNS( NSS['svg'], 'rect' );
    aRectElement.setAttribute( 'id', this.thumbnailBorderTemplateId );
    aRectElement.setAttribute( 'x', -this.halfBorderWidth );
    aRectElement.setAttribute( 'y', -this.halfBorderWidth );
    aRectElement.setAttribute( 'rx', this.halfBorderWidth );
    aRectElement.setAttribute( 'ry', this.halfBorderWidth );
    aRectElement.setAttribute( 'width', WIDTH + this.halfBorderWidth );
    aRectElement.setAttribute( 'height', HEIGHT + this.halfBorderWidth );
    aRectElement.setAttribute( 'stroke-width', this.borderWidth );
    aRectElement.setAttribute( 'fill', 'none' );
    aDefsElement.appendChild( aRectElement );

    // We define a Rect element as a template of mouse area for triggering the mouseover event.
    // A copy is used by each thumbnail element.
    aRectElement = document.createElementNS( NSS['svg'], 'rect' );
    aRectElement.setAttribute( 'id', this.thumbnailMouseAreaTemplateId );
    aRectElement.setAttribute( 'x', 0 );
    aRectElement.setAttribute( 'y', 0 );
    aRectElement.setAttribute( 'width', WIDTH );
    aRectElement.setAttribute( 'height', HEIGHT );
    aRectElement.setAttribute( 'fill', this.pageBgColor );
    aDefsElement.appendChild( aRectElement );

    this.pageElement.appendChild( aDefsElement );

    this.thumbnailMouseAreaTemplateElement = document.getElementById( this.thumbnailMouseAreaTemplateId );
    this.thumbnailBorderTemplateElement = document.getElementById( this.thumbnailBorderTemplateId );
};

SlideIndexPage.prototype.decreaseNumberOfColumns  = function()
{
    this.setNumberOfColumns( this.indexColumns - 1 );
};

SlideIndexPage.prototype.increaseNumberOfColumns  = function()
{
    this.setNumberOfColumns( this.indexColumns + 1 );
};

SlideIndexPage.prototype.resetNumberOfColumns  = function()
{
    this.setNumberOfColumns( INDEX_COLUMNS_DEFAULT );
};

/** setNumberOfColumns
 *
 * Change the size of the thumbnail grid.
 *
 * @param nNumberOfColumns - the new number of columns/rows of the thumbnail grid
 */
SlideIndexPage.prototype.setNumberOfColumns  = function( nNumberOfColumns )
{
    if( this.indexColumns == nNumberOfColumns )  return;
    if( nNumberOfColumns < 2 || nNumberOfColumns > 6 ) return;

    var suspendHandle = ROOT_NODE.suspendRedraw(500);

    var nOldTotalThumbnails = this.totalThumbnails;
    this.indexColumns = nNumberOfColumns;
    this.totalThumbnails = nNumberOfColumns * nNumberOfColumns;;

    this.aThumbnailSet[this.curThumbnailIndex].unselect();

    // if we decreased the number of used columns we remove the exceding thumbnail elements
    for( var i = this.totalThumbnails; i < nOldTotalThumbnails; ++i )
    {
        this.aThumbnailSet[i].removeElement();
    };

    // if we increased the number of used columns we create the needed thumbnail objects
    for( var i = nOldTotalThumbnails; i < this.totalThumbnails; ++i )
    {
        this.aThumbnailSet[i] = new Thumbnail( this, i );
    };

    // we set up layout parameters that depend on the number of columns
    this.halfBorderWidthFactor = ( 300/28000 ) * ( this.indexColumns / 3 );
    this.halfBorderWidth = WIDTH * this.halfBorderWidthFactor;
    this.borderWidth = 2 * this.halfBorderWidth;
    // scaleFactor = ( WIDTH - ( columns + 1 ) * xSpacing ) / ( columns * ( WIDTH + borderWidth ) )
    this.scaleFactor = ( 1 - ( this.indexColumns + 1 ) * this.xSpacingFactor ) /
                            ( this.indexColumns * ( 1 + 2 * this.halfBorderWidthFactor ) );

    // update the thumbnail border size
    var aRectElement = this.thumbnailBorderTemplateElement;
    aRectElement.setAttribute( 'x', -this.halfBorderWidth );
    aRectElement.setAttribute( 'y', -this.halfBorderWidth );
    aRectElement.setAttribute( 'rx', this.halfBorderWidth );
    aRectElement.setAttribute( 'ry', this.halfBorderWidth );
    aRectElement.setAttribute( 'width', WIDTH + this.halfBorderWidth );
    aRectElement.setAttribute( 'height', HEIGHT + this.halfBorderWidth );
    aRectElement.setAttribute( 'stroke-width', this.borderWidth );

    // now we update the displacement on the index page of each thumbnail (old and new)
    for( var i = 0; i < this.totalThumbnails; ++i )
    {
        this.aThumbnailSet[i].updateView();
    }

    this.curThumbnailIndex = this.selectedSlideIndex % this.totalThumbnails;
    this.aThumbnailSet[this.curThumbnailIndex].select();

    // needed for forcing the indexSetPageSlide routine to update the INDEX_OFFSET
    INDEX_OFFSET = -1;
    indexSetPageSlide( this.selectedSlideIndex );

    ROOT_NODE.unsuspendRedraw( suspendHandle );
    ROOT_NODE.forceRedraw();
};


/** Class Thumbnail **
 *  This class handles a slide thumbnail.
 */
function Thumbnail( aSlideIndexPage, nIndex )
{
    this.container = aSlideIndexPage;
    this.index = nIndex;//= getSafeIndex( nIndex, 0, this.container.getTotalThumbnails() );
    this.pageElement = this.container.pageElement;
    this.thumbnailId = 'thumbnail' + this.index;
    this.thumbnailElement = this.createThumbnailElement();
    this.slideElement = getElementByClassName( this.thumbnailElement, 'Slide' );
    this.backgroundElement = getElementByClassName( this.thumbnailElement, 'Background' );
    this.backgroundObjectsElement = getElementByClassName( this.thumbnailElement, 'BackgroundObjects' );
    this.borderElement = getElementByClassName( this.thumbnailElement, 'Border' );
    this.aTransformSet = new Array( 3 );
    this.visibility = VISIBLE;
    this.isSelected = false;
};

/* static const class member */
Thumbnail.prototype.sNormalBorderColor = 'rgb(216,216,216)';
Thumbnail.prototype.sSelectionBorderColor = 'rgb(92,92,255)';

/* public methods */
Thumbnail.prototype.removeElement = function()
{
    if( this.thumbnailElement )
        this.container.pageElement.removeChild( this.thumbnailElement );
};

Thumbnail.prototype.show = function()
{
    if( this.visibility == HIDDEN )
    {
        this.thumbnailElement.setAttribute( 'display', 'inherit' );
        this.visibility = VISIBLE;
    }
};

Thumbnail.prototype.hide = function()
{
    if( this.visibility == VISIBLE )
    {
        this.thumbnailElement.setAttribute( 'display', 'none' );
            this.visibility = HIDDEN;
        }
    };

    Thumbnail.prototype.select = function()
    {
        if( !this.isSelected )
        {
            this.borderElement.setAttribute( 'stroke', this.sSelectionBorderColor );
        this.isSelected = true;
    }
};

Thumbnail.prototype.unselect = function()
{
    if( this.isSelected )
    {
        this.borderElement.setAttribute( 'stroke', this.sNormalBorderColor );
        this.isSelected = false;
    }
};

/** updateView
 *
 *  This method updates the displacement of the thumbnail on the slide index page,
 *  the value of the row, column coordinates of the thumbnail in the grid, and
 *  the onmouseover property of the thumbnail element.
 *
 */
Thumbnail.prototype.updateView = function()
{
    this.column = this.index % this.container.indexColumns;
    this.row = ( this.index - this.column ) / this.container.indexColumns;
    this.halfBorderWidth = this.container.halfBorderWidth;
    this.borderWidth = this.container.borderWidth;
    this.width = ( WIDTH + this.borderWidth ) * this.container.scaleFactor;
    this.height = ( HEIGHT + this.borderWidth ) * this.container.scaleFactor;
    this.aTransformSet[2] = 'translate(' + this.halfBorderWidth + ' ' + this.halfBorderWidth + ')';
    this.aTransformSet[1] = 'scale(' + this.container.scaleFactor + ')';
    var sTransformAttrValue = this.computeTransform();
    this.thumbnailElement.setAttribute( 'transform', sTransformAttrValue );
    this.thumbnailElement.setAttribute( 'onmouseover', 'theSlideIndexPage.aThumbnailSet[' + this.index  + '].onMouseOver()' );
};

/** update
 *
 * This method update the content of the thumbnail view
 *
 * @param nIndex - the index of the slide to be shown in the thumbnail
 */
Thumbnail.prototype.update = function( nIndex )
{
   if( this.slideIndex == nIndex )  return;

   var aMetaSlide = theMetaDoc.aMetaSlideSet[nIndex];
   setNSAttribute( 'xlink', this.slideElement, 'href', '#' + aMetaSlide.slideId );
   if( aMetaSlide.nIsBackgroundVisible )
   {
       setNSAttribute( 'xlink', this.backgroundElement, 'href', '#' + aMetaSlide.masterPage.backgroundId );
       this.backgroundElement.setAttribute( 'visibility', 'inherit' );
   }
   else
   {
       this.backgroundElement.setAttribute( 'visibility', 'hidden' );
   }
   if( aMetaSlide.nAreMasterObjectsVisible )
   {
       setNSAttribute( 'xlink',  this.backgroundObjectsElement, 'href', '#' + aMetaSlide.masterPage.backgroundObjectsId );
       this.backgroundObjectsElement.setAttribute( 'visibility', 'inherit' );
   }
   else
   {
       this.backgroundObjectsElement.setAttribute( 'visibility', 'hidden' );
   }
   this.slideIndex = nIndex;
};

Thumbnail.prototype.clear = function( nIndex )
{
   setNSAttribute( 'xlink', this.slideElement, 'href', '' );
   setNSAttribute( 'xlink', this.backgroundElement, 'href', '' );
   setNSAttribute( 'xlink', this.backgroundObjectsElement, 'href', '' );
};

/* private methods */
Thumbnail.prototype.createThumbnailElement = function()
{
    var aThumbnailElement = document.createElementNS( NSS['svg'], 'g' );
    aThumbnailElement.setAttribute( 'id', this.thumbnailId );
    aThumbnailElement.setAttribute( 'display', 'inherit' );

    var aMouseAreaElement = document.createElementNS( NSS['svg'], 'use' );
    setNSAttribute( 'xlink', aMouseAreaElement, 'href', '#' + this.container.thumbnailMouseAreaTemplateId );
    aMouseAreaElement.setAttribute( 'class', 'MouseArea' );
    aThumbnailElement.appendChild( aMouseAreaElement );

    var aBackgroundElement = document.createElementNS( NSS['svg'], 'use' );
    setNSAttribute( 'xlink', aBackgroundElement, 'href', '' );
    aBackgroundElement.setAttribute( 'visibility', 'inherit');
    aBackgroundElement.setAttribute( 'class', 'Background' );
    aThumbnailElement.appendChild( aBackgroundElement );

    var aBackgroundObjectsElement = document.createElementNS( NSS['svg'], 'use' );
    setNSAttribute( 'xlink', aBackgroundObjectsElement, 'href', '' );
    aBackgroundObjectsElement.setAttribute( 'visibility', 'inherit');
    aBackgroundObjectsElement.setAttribute( 'class', 'BackgroundObjects' );
    aThumbnailElement.appendChild( aBackgroundObjectsElement );

    var aSlideElement = document.createElementNS( NSS['svg'], 'use' );
    setNSAttribute( 'xlink', aSlideElement, 'href', '' );
    aSlideElement.setAttribute( 'class', 'Slide' );
    aThumbnailElement.appendChild( aSlideElement );

    var aBorderElement = document.createElementNS( NSS['svg'], 'use' );
    setNSAttribute( 'xlink', aBorderElement, 'href', '#' + this.container.thumbnailBorderTemplateId );
    aBorderElement.setAttribute( 'stroke', this.sNormalBorderColor );
    aBorderElement.setAttribute( 'class', 'Border' );
    aThumbnailElement.appendChild( aBorderElement );

    this.container.pageElement.appendChild( aThumbnailElement );
    return( document.getElementById( this.thumbnailId ) );
};

Thumbnail.prototype.computeTransform = function()
{
    var nXSpacing = this.container.xSpacing;
    var nYSpacing = this.container.ySpacing;

    var nXOffset = nXSpacing + ( this.width + nXSpacing ) * this.column;
    var nYOffset = nYSpacing + ( this.height + nYSpacing ) * this.row;

    this.aTransformSet[0] = 'translate(' + nXOffset + ' ' + nYOffset + ')';

    sTransform = this.aTransformSet.join( ' ' );

    return sTransform;
};

Thumbnail.prototype.onMouseOver = function()
{
    if( ( currentMode == INDEX_MODE ) && ( this.container.curThumbnailIndex !=  this.index ) )
    {
        this.container.setSelection( this.index );
    }
};




// ------------------------------------------------------------------------------------------ //
/** Initialization function.
 *  The whole presentation is set-up in this function.
 */
function init()
{
    var VIEWBOX = ROOT_NODE.getAttribute('viewBox');

    if( VIEWBOX )
    {
        WIDTH = ROOT_NODE.viewBox.animVal.width;
        HEIGHT = ROOT_NODE.viewBox.animVal.height;
    }

    var aMetaDocElem = document.getElementById( aOOOElemMetaSlides );
    assert( aMetaDocElem, 'init: meta document element not found' );
    aSlideShow = new SlideShow();
    theMetaDoc =  new MetaDocument( aMetaDocElem );
    theSlideIndexPage = new SlideIndexPage();
    aSlideShow.displaySlide( theMetaDoc.startSlideNumber );

    //=====================================//
    //      ===== timing test =====        //
    //=====================================//
//        var aTimingAttributeList = [ '0.001s', '-12:16.123', 'next', 'id23.click', 'id3.end + 5s', 'id4.begin - 12:45' ];
//
//        for( var i = 0; i < aTimingAttributeList.length; ++i)
//        {
//            var aTiming = new Timing( aTimingAttributeList[i] );
//            aTiming.parse();
//            aTiming.info();
//        }


    //=====================================//
    //  == animations parsing test ==      //
    //=====================================//

//        var aSlideShowContext = aSlideShow.getContext();
//        var aSlideAnimations = new SlideAnimations( aSlideShowContext );
//        aSlideAnimations.importAnimations( getSlideAnimationsRoot( 'id7' ) );
//        aSlideAnimations.parseElements();
//        log( aSlideAnimations.aRootNode.info( true ) );

}

function presentationEngineStop()
{
    alert( 'We are sorry! An unexpected error occurred.\nThe presentation engine will be stopped' );
    document.onkeydown = null;
    document.onkeypress = null;
    document.onclick = null;
    window.onmousewheel = null;
}

function assert( condition, message )
{
   if (!condition)
   {
       presentationEngineStop();
       if (typeof console == 'object')
           console.trace();
       throw new Error( message );
   }
}

function dispatchEffects(dir)
{
    // TODO to be implemented

    if( dir == 1 )
    {
        var bRet = aSlideShow.nextEffect();

        if( !bRet )
        {
            switchSlide( 1 );
        }
    }
    else
    {
        switchSlide( dir );
    }
}

function skipEffects(dir)
{
    // TODO to be implemented
    switchSlide(dir);
}

function switchSlide( nOffset, bSkipTransition )
{
    var nNextSlide = nCurSlide + nOffset;
    aSlideShow.displaySlide( nNextSlide, bSkipTransition );
}

/** Function to display the index sheet.
*
*  @param offsetNumber offset number
*/
   function displayIndex( offsetNumber )
   {
       var aMetaSlideSet = theMetaDoc.aMetaSlideSet;
       offsetNumber = getSafeIndex( offsetNumber, 0, aMetaSlideSet.length - 1 );

       var nTotalThumbnails = theSlideIndexPage.getTotalThumbnails();
       var nEnd = Math.min( offsetNumber + nTotalThumbnails, aMetaSlideSet.length);

       var aThumbnailSet = theSlideIndexPage.aThumbnailSet;
       var j = 0;
       for( var i = offsetNumber; i < nEnd; ++i, ++j )
       {
           aThumbnailSet[j].update( i );
           aThumbnailSet[j].show();
       }
       for( ; j < nTotalThumbnails; ++j )
       {
           aThumbnailSet[j].hide();
       }

       //do we need to save the current offset?
       if (INDEX_OFFSET != offsetNumber)
           INDEX_OFFSET = offsetNumber;
   }

/** Function to toggle between index and slide mode.
 */
function toggleSlideIndex()
{
    var suspendHandle = ROOT_NODE.suspendRedraw(500);
    var aMetaSlideSet = theMetaDoc.aMetaSlideSet;

    if( currentMode == SLIDE_MODE )
    {
        aMetaSlideSet[nCurSlide].hide();
        for( var counter = 0; counter < aMetaSlideSet.length; ++counter )
        {
            checkElemAndSetAttribute( aMetaSlideSet[counter].slideElement, 'visibility', 'inherit' );
            aMetaSlideSet[counter].masterPage.setVisibilityBackground( INHERIT );
            aMetaSlideSet[counter].masterPage.setVisibility( INHERIT );
        }
        INDEX_OFFSET = -1;
        indexSetPageSlide( nCurSlide );
        theSlideIndexPage.show();
        currentMode = INDEX_MODE;
    }
    else if( currentMode == INDEX_MODE )
    {
        theSlideIndexPage.hide();
        var nNewSlide = theSlideIndexPage.selectedSlideIndex;

        for( var counter = 0; counter < aMetaSlideSet.length; ++counter )
        {
            var aMetaSlide = aMetaSlideSet[counter];
            aMetaSlide.slideElement.setAttribute( 'visibility', 'hidden' );
            aMetaSlide.masterPage.setVisibilityBackground( HIDDEN );
            aMetaSlide.masterPage.setVisibility( HIDDEN );
        }

        aSlideShow.displaySlide( nNewSlide, true );
        currentMode = SLIDE_MODE;
    }

    ROOT_NODE.unsuspendRedraw(suspendHandle);
    ROOT_NODE.forceRedraw();
}

/** Function that exit from the index mode without changing the shown slide
 *
 */
function abandonIndexMode()
{
    theSlideIndexPage.selectedSlideIndex = nCurSlide;
    toggleSlideIndex();
}





/*********************************************************************************************
 *********************************************************************************************
 *********************************************************************************************

                                  ***** ANIMATION ENGINE *****

 *********************************************************************************************
 *********************************************************************************************
 *********************************************************************************************/





// ------------------------------------------------------------------------------------------ //
// helper functions


var CURR_UNIQUE_ID = 0;

function getUniqueId()
{
    ++CURR_UNIQUE_ID;
    return CURR_UNIQUE_ID;
}

function mem_fn( sMethodName )
{
    return  function( aObject )
    {
        var aMethod = aObject[ sMethodName ];
        if( aMethod )
            aMethod.call( aObject );
        else
            log( 'method sMethodName not found' );
    };
}

function bind( aObject, aMethod )
{
    return  function()
            {
                return aMethod.call( aObject, arguments[0] );
            };
}

function getCurrentSystemTime()
{
    return ( new Date() ).getTime();
    //return ROOT_NODE.getCurrentTime();
}

function getSlideAnimationsRoot( sSlideId )
{
    return theMetaDoc.aSlideAnimationsMap[ sSlideId ];
}

/** This function return an array populated with all children nodes of the
 *  passed element that are elements
 *
 *  @param aElement:   any XML element
 *
 *  @returns   an array that contains all children elements
 */
function getElementChildren( aElement )
{
    var aChildrenArray = new Array();

    var nSize = aElement.childNodes.length;

    for( var i = 0; i < nSize; ++i )
    {
        if( aElement.childNodes[i].nodeType == 1 )
            aChildrenArray.push( aElement.childNodes[i] );
    }

    return aChildrenArray;
}

function removeWhiteSpaces( str )
{
    if( !str )
        return '';

    var re = / */;
    var aSplittedString = str.split( re );
    return aSplittedString.join('');
}

function clamp( nValue, nMinimum, nMaximum )
{
    if( nValue < nMinimum )
    {
        return nMinimum;
    }
    else if( nValue > nMaximum )
    {
        return nMaximum;
    }
    else
    {
        return nValue;
    }
}

function makeMatrixString( a, b, c, d, e, f )
{
    var s = 'matrix(';
    s += a + ', ';
    s += b + ', ';
    s += c + ', ';
    s += d + ', ';
    s += e + ', ';
    s += f + ')';

    return s;
}

function matrixToString( aSVGMatrix )
{
    return makeMatrixString( aSVGMatrix.a, aSVGMatrix.b, aSVGMatrix.c,
                             aSVGMatrix.d, aSVGMatrix.e, aSVGMatrix.f );
}



// ------------------------------------------------------------------------------------------ //
// Attribute Parsers

function numberParser( sValue )
{
    if( sValue === '.' )
        return undefined;
    var reFloatNumber = /^[+-]?[0-9]*[.]?[0-9]*$/;

    if( reFloatNumber.test( sValue ) )
        return parseFloat( sValue );
    else
        return undefined;
}

function booleanParser( sValue )
{
    sValue = sValue.toLowerCase();
    if( sValue === 'true' )
        return true;
    else if( sValue === 'false' )
        return false;
    else
        return undefined;
}

function colorParser( sValue )
{

    function hsl( nHue, nSaturation, nLuminance )
    {
        return new HSLColor( nHue, nSaturation / 100, nLuminance / 100 );
    }

    function rgb( nRed, nGreen, nBlue )
    {
        return new RGBColor( nRed / 255, nGreen / 255, nBlue / 255 );
    }

    function prgb( nRed, nGreen, nBlue )
    {
        return new RGBColor( nRed / 100, nGreen / 100, nBlue / 100 );
    }

    var sCommaPattern = ' *[,] *';
    var sIntegerPattern = '[+-]?[0-9]+';
    var sHexDigitPattern = '[0-9A-Fa-f]';

    var sHexColorPattern = '#(' + sHexDigitPattern + '{2})('
                                + sHexDigitPattern + '{2})('
                                + sHexDigitPattern + '{2})';

    var sRGBIntegerPattern = 'rgb[(] *' + sIntegerPattern + sCommaPattern
                                      + sIntegerPattern + sCommaPattern
                                      + sIntegerPattern + ' *[)]';

    var sRGBPercentPattern = 'rgb[(] *' + sIntegerPattern + '%' + sCommaPattern
                                        + sIntegerPattern + '%' + sCommaPattern
                                        + sIntegerPattern + '%' + ' *[)]';

    var sHSLPercentPattern = 'hsl[(] *' + sIntegerPattern + sCommaPattern
                                        + sIntegerPattern + '%' + sCommaPattern
                                        + sIntegerPattern + '%' + ' *[)]';

    var reHexColor = RegExp( sHexColorPattern );
    var reRGBInteger = RegExp( sRGBIntegerPattern );
    var reRGBPercent = RegExp( sRGBPercentPattern );
    var reHSLPercent = RegExp( sHSLPercentPattern );

    if( reHexColor.test( sValue ) )
    {
        var aRGBTriple = reHexColor.exec( sValue );

        var nRed    = parseInt( aRGBTriple[1], 16 ) / 255;
        var nGreen  = parseInt( aRGBTriple[2], 16 ) / 255;
        var nBlue   = parseInt( aRGBTriple[3], 16 ) / 255;

        return new RGBColor( nRed, nGreen, nBlue );
    }
    else if( reHSLPercent.test( sValue ) )
    {
        sValue = sValue.replace( '%', '' ).replace( '%', '' );
        return eval( sValue );
    }
    else if( reRGBInteger.test( sValue ) )
    {
        return eval( sValue );
    }
    else if( reRGBPercent.test( sValue ) )
    {
        sValue = 'p' + sValue.replace( '%', '' ).replace( '%', '' ).replace( '%', '' );
        return eval( sValue );
    }
    else
    {
        return null;
    }
}



/**********************************************************************************************
 *      RGB and HSL color classes
 **********************************************************************************************/

// ------------------------------------------------------------------------------------------ //
function RGBColor( nRed, nGreen, nBlue )
{
    this.eColorSpace = COLOR_SPACE_RGB;
    // values in the [0,1] range
    this.nRed = nRed;
    this.nGreen = nGreen;
    this.nBlue = nBlue;
}


RGBColor.prototype.clone = function()
{
    return new RGBColor( this.nRed, this.nGreen, this.nBlue );
};

RGBColor.prototype.add = function( aRGBColor )
{
    this.nRed += aRGBColor.nRed;
    this.nGreen += aRGBColor.nGreen;
    this.nBlue += aRGBColor.nBlue;
    return this;
};

RGBColor.prototype.scale = function( aT )
{
    this.nRed *= aT;
    this.nGreen *= aT;
    this.nBlue *= aT;
    return this;
};

RGBColor.clamp = function( aRGBColor )
{
    var aClampedRGBColor = new RGBColor( 0, 0, 0 );

    aClampedRGBColor.nRed   = clamp( aRGBColor.nRed, 0.0, 1.0 );
    aClampedRGBColor.nGreen = clamp( aRGBColor.nGreen, 0.0, 1.0 );
    aClampedRGBColor.nBlue  = clamp( aRGBColor.nBlue, 0.0, 1.0 );

    return aClampedRGBColor;
};

RGBColor.prototype.convertToHSL = function()
{
    var nRed   = clamp( this.nRed, 0.0, 1.0 );
    var nGreen = clamp( this.nGreen, 0.0, 1.0 );
    var nBlue  = clamp( this.nBlue, 0.0, 1.0 );

    var nMax = Math.max( nRed, nGreen, nBlue );
    var nMin = Math.min( nRed, nGreen, nBlue );
    var nDelta = nMax - nMin;

    var nLuminance  = ( nMax + nMin ) / 2.0;
    var nSaturation = 0.0;
    var nHue = 0.0;
    if( nDelta !== 0 )
    {
        nSaturation = ( nLuminance > 0.5 ) ?
                            ( nDelta / ( 2.0 - nMax - nMin) ) :
                            ( nDelta / ( nMax + nMin ) );

        if( nRed == nMax )
            nHue = ( nGreen - nBlue ) / nDelta;
        else if( nGreen == nMax )
            nHue = 2.0 + ( nBlue - nRed ) / nDelta;
        else if( nBlue == nMax )
            nHue = 4.0 + ( nRed - nGreen ) / nDelta;

        nHue *= 60.0;

        if( nHue < 0.0 )
            nHue += 360.0;
    }

    return new HSLColor( nHue, nSaturation, nLuminance );

};

RGBColor.prototype.toString = function( bClamped )
{
    var aRGBColor;
    if( bClamped )
    {
        aRGBColor = RGBColor.clamp( this );
    }
    else
    {
        aRGBColor = this;
    }

    var nRed = Math.round( aRGBColor.nRed * 255 );
    var nGreen = Math.round( aRGBColor.nGreen * 255 );
    var nBlue = Math.round( aRGBColor.nBlue * 255 );

    return ( 'rgb(' + nRed + ',' + nGreen + ',' + nBlue + ')' );
};

RGBColor.interpolate = function( aStartRGB , aEndRGB, nT )
{
    var aResult = aStartRGB.clone();
    var aTEndRGB = aEndRGB.clone();
    aResult.scale( 1.0 - nT );
    aTEndRGB.scale( nT );
    aResult.add( aTEndRGB );

    return aResult;
};



// ------------------------------------------------------------------------------------------ //
function HSLColor( nHue, nSaturation, nLuminance )
{
    this.eColorSpace = COLOR_SPACE_HSL;
    // Hue is in the [0,360[ range, Saturation and Luminance are in the [0,1] range
    this.nHue = nHue;
    this.nSaturation = nSaturation;
    this.nLuminance = nLuminance;

    this.normalizeHue();
}


HSLColor.prototype.clone = function()
{
    return new HSLColor( this.nHue, this.nSaturation, this.nLuminance );
};

HSLColor.prototype.add = function( aHSLColor )
{
    this.nHue += aHSLColor.nHue;
    this.nSaturation += aHSLColor.nSaturation;
    this.nLuminance += aHSLColor.nLuminance;
    this.normalizeHue();
    return this;
};

HSLColor.prototype.scale = function( aT )
{
    this.nHue *= aT;
    this.nSaturation *= aT;
    this.nLuminance *= aT;
    this.normalizeHue();
    return this;
};

HSLColor.clamp = function( aHSLColor )
{
    var aClampedHSLColor = new HSLColor( 0, 0, 0 );

    aClampedHSLColor.nHue = aHSLColor.nHue % 360;
    if( aClampedHSLColor.nHue < 0 )
        aClampedHSLColor.nHue += 360;
    aClampedHSLColor.nSaturation = clamp( aHSLColor.nSaturation, 0.0, 1.0 );
    aClampedHSLColor.nLuminance = clamp( aHSLColor.nLuminance, 0.0, 1.0 );
};

HSLColor.prototype.normalizeHue = function()
{
    this.nHue = this.nHue % 360;
    if( this.nHue < 0 ) this.nHue += 360;
};

HSLColor.prototype.toString = function()
{
    return 'hsl(' + this.nHue.toFixed( 3 ) + ','
                  + this.nSaturation.toFixed( 3 ) + ','
                  + this.nLuminance.toFixed( 3 ) + ')';
};

HSLColor.prototype.convertToRGB = function()
{

    var nHue = this.nHue % 360;
    if( nHue < 0 ) nHue += 360;
    var nSaturation =  clamp( this.nSaturation, 0.0, 1.0 );
    var nLuminance = clamp( this.nLuminance, 0.0, 1.0 );


    if( nSaturation === 0 )
    {
        return new RGBColor( nLuminance, nLuminance, nLuminance );
    }

    var nVal1 = ( nLuminance <= 0.5 ) ?
                        ( nLuminance * (1.0 + nSaturation) ) :
                        ( nLuminance + nSaturation - nLuminance * nSaturation );

    var nVal2 = 2.0 * nLuminance - nVal1;

    var nRed    = HSLColor.hsl2rgbHelper( nVal2, nVal1, nHue + 120 );
    var nGreen  = HSLColor.hsl2rgbHelper( nVal2, nVal1, nHue );
    var nBlue   = HSLColor.hsl2rgbHelper( nVal2, nVal1, nHue - 120 );

    return new RGBColor( nRed, nGreen, nBlue );
};

HSLColor.hsl2rgbHelper = function( nValue1, nValue2, nHue )
{
    nHue = nHue % 360;
    if( nHue < 0 )
        nHue += 360;

    if( nHue < 60.0 )
        return nValue1 + ( nValue2 - nValue1 ) * nHue / 60.0;
    else if( nHue < 180.0 )
        return nValue2;
    else if( nHue < 240.0 )
        return ( nValue1 + ( nValue2 - nValue1 ) * ( 240.0 - nHue ) / 60.0 );
    else
        return nValue1;
};

HSLColor.interpolate = function( aFrom, aTo, nT, bCCW )
{
    var nS = 1.0 - nT;

    var nHue = 0.0;
    if( aFrom.nHue <= aTo.nHue && !bCCW )
    {
        // interpolate hue clockwise. That is, hue starts at
        // high values and ends at low ones. Therefore, we
        // must 'cross' the 360 degrees and start at low
        // values again (imagine the hues to lie on the
        // circle, where values above 360 degrees are mapped
        // back to [0,360)).
        nHue = nS * (aFrom.nHue + 360.0) + nT * aTo.nHue;
    }
    else if( aFrom.nHue > aTo.nHue && bCCW )
    {
        // interpolate hue counter-clockwise. That is, hue
        // starts at high values and ends at low
        // ones. Therefore, we must 'cross' the 360 degrees
        // and start at low values again (imagine the hues to
        // lie on the circle, where values above 360 degrees
        // are mapped back to [0,360)).
        nHue = nS * aFrom.nHue + nT * (aTo.nHue + 360.0);
    }
    else
    {
        // interpolate hue counter-clockwise. That is, hue
        // starts at low values and ends at high ones (imagine
        // the hue value as degrees on a circle, with
        // increasing values going counter-clockwise)
            nHue = nS * aFrom.nHue + nT * aTo.nHue;
        }

        var nSaturation = nS * aFrom.nSaturation + nT * aTo.nSaturation;
        var nLuminance = nS * aFrom.nLuminance + nT * aTo.nLuminance;

        return new HSLColor( nHue, nSaturation, nLuminance );
    };




/**********************************************************************************************
 *      AnimationNode Class Hierarchy
 **********************************************************************************************/

// ------------------------------------------------------------------------------------------ //

// Node Types
var ANIMATION_NODE_CUSTOM               = 0;
var ANIMATION_NODE_PAR                  = 1;
var ANIMATION_NODE_SEQ                  = 2;
var ANIMATION_NODE_ITERATE              = 3;
var ANIMATION_NODE_ANIMATE              = 4;
var ANIMATION_NODE_SET                  = 5;
var ANIMATION_NODE_ANIMATEMOTION        = 6;
var ANIMATION_NODE_ANIMATECOLOR         = 7;
var ANIMATION_NODE_ANIMATETRANSFORM     = 8;
var ANIMATION_NODE_TRANSITIONFILTER     = 9;
var ANIMATION_NODE_AUDIO                = 10;
var ANIMATION_NODE_COMMAND              = 11;

aAnimationNodeTypeInMap = {
            'par'               : ANIMATION_NODE_PAR,
            'seq'               : ANIMATION_NODE_SEQ,
            'iterate'           : ANIMATION_NODE_ITERATE,
            'animate'           : ANIMATION_NODE_ANIMATE,
            'set'               : ANIMATION_NODE_SET,
            'animatemotion'     : ANIMATION_NODE_ANIMATEMOTION,
            'animatecolor'      : ANIMATION_NODE_ANIMATECOLOR,
            'animatetransform'  : ANIMATION_NODE_ANIMATETRANSFORM,
            'transitionfilter'  : ANIMATION_NODE_TRANSITIONFILTER
};



// ------------------------------------------------------------------------------------------ //
function getAnimationElementType( aElement )
{
    var sName = aElement.localName.toLowerCase();
    //log( 'getAnimationElementType: ' + sName );

    if( sName && aAnimationNodeTypeInMap[ sName ] )
        return aAnimationNodeTypeInMap[ sName ];
    else
        return ANIMATION_NODE_CUSTOM;
}



// ------------------------------------------------------------------------------------------ //

// Node States
var INVALID_NODE                = 0;
var UNRESOLVED_NODE             = 1;
var RESOLVED_NODE               = 2;
var ACTIVE_NODE                 = 4;
var FROZEN_NODE                 = 8;
var ENDED_NODE                  = 16;

function getNodeStateName( eNodeState )
{
    switch( eNodeState )
    {
        case INVALID_NODE:
            return 'INVALID';
        case UNRESOLVED_NODE:
            return 'UNRESOLVED';
        case RESOLVED_NODE:
            return 'RESOLVED';
        case ACTIVE_NODE:
            return 'ACTIVE';
        case FROZEN_NODE:
            return 'FROZEN';
        case ENDED_NODE:
            return 'ENDED';
        default:
            return 'UNKNOWN';
    }
}


// Impress Node Types
IMPRESS_DEFAULT_NODE                    = 0;
IMPRESS_ON_CLICK_NODE                   = 1;
IMPRESS_WITH_PREVIOUS_NODE              = 2;
IMPRESS_AFTER_PREVIOUS_NODE             = 3;
IMPRESS_MAIN_SEQUENCE_NODE              = 4;
IMPRESS_TIMING_ROOT_NODE                = 5;
IMPRESS_INTERACTIVE_SEQUENCE_NODE       = 6;

aImpressNodeTypeInMap = {
        'on-click'                  : IMPRESS_ON_CLICK_NODE,
        'with-previous'             : IMPRESS_WITH_PREVIOUS_NODE,
        'after-previous'            : IMPRESS_AFTER_PREVIOUS_NODE,
        'main-sequence'             : IMPRESS_MAIN_SEQUENCE_NODE,
        'timing-root'               : IMPRESS_TIMING_ROOT_NODE,
        'interactive-sequence'      : IMPRESS_INTERACTIVE_SEQUENCE_NODE
};

aImpressNodeTypeOutMap = [ 'default', 'on-click', 'with-previous', 'after-previous',
                            'main-sequence', 'timing-root', 'interactive-sequence' ];


// Preset Classes
aPresetClassInMap = {};


// Preset Ids
aPresetIdInMap = {};


// Restart Modes
RESTART_MODE_DEFAULT            = 0;
RESTART_MODE_INHERIT            = 0;
RESTART_MODE_ALWAYS             = 1;
RESTART_MODE_WHEN_NOT_ACTIVE    = 2;
RESTART_MODE_NEVER              = 3;

aRestartModeInMap = {
        'inherit'       : RESTART_MODE_DEFAULT,
        'always'        : RESTART_MODE_ALWAYS,
        'whenNotActive' : RESTART_MODE_WHEN_NOT_ACTIVE,
        'never'         : RESTART_MODE_NEVER
};

aRestartModeOutMap = [ 'inherit','always', 'whenNotActive', 'never' ];


// Fill Modes
var FILL_MODE_DEFAULT           = 0;
var FILL_MODE_INHERIT           = 0;
var FILL_MODE_REMOVE            = 1;
var FILL_MODE_FREEZE            = 2;
var FILL_MODE_HOLD              = 3;
var FILL_MODE_TRANSITION        = 4;
var FILL_MODE_AUTO              = 5;

aFillModeInMap = {
        'inherit'       : FILL_MODE_DEFAULT,
        'remove'        : FILL_MODE_REMOVE,
        'freeze'        : FILL_MODE_FREEZE,
        'hold'          : FILL_MODE_HOLD,
        'transition'    : FILL_MODE_TRANSITION,
        'auto'          : FILL_MODE_AUTO
};

aFillModeOutMap = [ 'inherit', 'remove', 'freeze', 'hold', 'transition', 'auto' ];


// Additive Modes
var ADDITIVE_MODE_BASE          = 0;
var ADDITIVE_MODE_SUM           = 1;
var ADDITIVE_MODE_REPLACE       = 2;
var ADDITIVE_MODE_MULTIPLY      = 3;
var ADDITIVE_MODE_NONE          = 4;

aAddittiveModeInMap = {
        'base'          : ADDITIVE_MODE_BASE,
        'sum'           : ADDITIVE_MODE_SUM,
        'replace'       : ADDITIVE_MODE_REPLACE,
        'multiply'      : ADDITIVE_MODE_MULTIPLY,
        'none'          : ADDITIVE_MODE_NONE
};

aAddittiveModeOutMap = [ 'base', 'sum', 'replace', 'multiply', 'none' ];


// Accumulate Modes
var ACCUMULATE_MODE_NONE        = 0;
var ACCUMULATE_MODE_SUM         = 1;

aAccumulateModeOutMap = [ 'none', 'sum' ];

// Calculation Modes
var CALC_MODE_DISCRETE          = 0;
var CALC_MODE_LINEAR            = 1;
var CALC_MODE_PACED             = 2;
var CALC_MODE_SPLINE            = 3;

aCalcModeInMap = {
        'discrete'      : CALC_MODE_DISCRETE,
        'linear'        : CALC_MODE_LINEAR,
        'paced'         : CALC_MODE_PACED,
        'spline'        : CALC_MODE_SPLINE
};

aCalcModeOutMap = [ 'discrete', 'linear', 'paced', 'spline' ];


// Color Spaces
var COLOR_SPACE_RGB = 0;
var COLOR_SPACE_HSL = 1;

aColorSpaceInMap = { 'rgb': COLOR_SPACE_RGB, 'hsl': COLOR_SPACE_HSL };

aColorSpaceOutMap = [ 'rgb', 'hsl' ];


// Clock Directions
var CLOCKWISE               = 0;
var COUNTERCLOCKWISE        = 1;

aClockDirectionInMap = { 'clockwise': CLOCKWISE, 'counterclockwise': COUNTERCLOCKWISE };

aClockDirectionOutMap = [ 'clockwise', 'counterclockwise' ];


// Attribute Value Types
UNKNOWN_PROPERTY        = 0;
NUMBER_PROPERTY         = 1;
ENUM_PROPERTY           = 2;
COLOR_PROPERTY          = 3;
STRING_PROPERTY         = 4;
BOOL_PROPERTY           = 5;

aValueTypeOutMap = [ 'unknown', 'number', 'enum', 'color', 'string', 'boolean' ];


// Attribute Map
var aAttributeMap =
{
        'height':           {   'type':         NUMBER_PROPERTY,
                                'get':          'getHeight',
                                'set':          'setHeight',
                                'getmod':       'makeScaler( 1/nHeight )',
                                'setmod':       'makeScaler( nHeight)'          },

        'opacity':          {   'type':         NUMBER_PROPERTY,
                                'get':          'getOpacity',
                                'set':          'setOpacity'                    },

        'width':            {   'type':         NUMBER_PROPERTY,
                                'get':          'getWidth',
                                'set':          'setWidth',
                                'getmod':       'makeScaler( 1/nWidth )',
                                'setmod':       'makeScaler( nWidth)'           },

        'x':                {   'type':         NUMBER_PROPERTY,
                                'get':          'getX',
                                'set':          'setX',
                                'getmod':       'makeScaler( 1/nWidth )',
                                'setmod':       'makeScaler( nWidth)'           },

        'y':                {   'type':         NUMBER_PROPERTY,
                                'get':          'getY',
                                'set':          'setY',
                                'getmod':       'makeScaler( 1/nHeight )',
                                'setmod':       'makeScaler( nHeight)'          },

        'fill':             {   'type':         ENUM_PROPERTY,
                                'get':          'getFillStyle',
                                'set':          'setFillStyle'                  },

        'stroke':           {   'type':         ENUM_PROPERTY,
                                'get':          'getStrokeStyle',
                                'set':          'setStrokeStyle'                },

        'visibility':       {   'type':         ENUM_PROPERTY,
                                'get':          'getVisibility',
                                'set':          'setVisibility'                 },

        'fill-color':       {   'type':         COLOR_PROPERTY,
                                'get':          'getFillColor',
                                'set':          'setFillColor'                  },

        'stroke-color':     {   'type':         COLOR_PROPERTY,
                                'get':          'getStrokeColor',
                                'set':          'setStrokeColor'                },

        'color':            {   'type':         COLOR_PROPERTY,
                                'get':          'getFontColor',
                                'set':          'setFontColor'                  },

};


// Transition Types
BARWIPE_TRANSITION          = 1;
FADE_TRANSITION             = 2; // 37

aTransitionTypeInMap = {
            'barWipe'           : BARWIPE_TRANSITION,
            'fade'              : FADE_TRANSITION
};

aTransitionTypeOutMap = [ '', 'barWipe', 'fade' ];


// Transition Subtypes
DEFAULT_TRANS_SUBTYPE               = 0;
LEFTTORIGHT_TRANS_SUBTYPE           = 1;
TOPTOBOTTOM_TRANS_SUBTYPE           = 2;
CROSSFADE_TRANS_SUBTYPE             = 3; // 101

aTransitionSubtypeInMap = {
            'leftToRight'       : LEFTTORIGHT_TRANS_SUBTYPE,
            'topToBottom'       : TOPTOBOTTOM_TRANS_SUBTYPE,
            'crossfade'         : CROSSFADE_TRANS_SUBTYPE
};

aTransitionSubtypeOutMap = [ 'default', 'leftToRight', 'topToBottom', 'crossfade' ];


// Transition Modes
TRANSITION_MODE_IN  = 1;
TRANSITION_MODE_OUT = 0;

aTransitionModeInMap = { 'out': TRANSITION_MODE_OUT, 'in': TRANSITION_MODE_IN };
aTransitionModeOutMap = [ 'out', 'in' ];


// ------------------------------------------------------------------------------------------ //
// Transition tables

// transition table for restart=NEVER, fill=FREEZE
var aStateTransitionTable_Never_Freeze =
[
     INVALID_NODE,
     RESOLVED_NODE | ENDED_NODE,         // active successors for UNRESOLVED
     ACTIVE_NODE | ENDED_NODE,           // active successors for RESOLVED
     INVALID_NODE,
     FROZEN_NODE | ENDED_NODE,           // active successors for ACTIVE: freeze object
     INVALID_NODE,
     INVALID_NODE,
     INVALID_NODE,
     ENDED_NODE,                         // active successors for FROZEN: end
     INVALID_NODE,
     INVALID_NODE,
     INVALID_NODE,
     INVALID_NODE,
     INVALID_NODE,
     INVALID_NODE,
     INVALID_NODE,
     ENDED_NODE                          // active successors for ENDED:
     // this state is a sink here (cannot restart)
];


// Table guide
var aTableGuide =
[
     null,
     null,
     null,
     aStateTransitionTable_Never_Freeze,
     null,
     null
];



// ------------------------------------------------------------------------------------------ //
function getTransitionTable( eRestartMode, eFillMode )
{
    var nRestartValue = 0;  // never

    var nFillValue = 1;     // frozen

    return aTableGuide[ 3*nFillValue + nRestartValue ];
}



// ------------------------------------------------------------------------------------------ //

// Event Triggers
var EVENT_TRIGGER_UNKNOWN               = 0;
var EVENT_TRIGGER_ON_SLIDE_BEGIN        = 1;
var EVENT_TRIGGER_ON_SLIDE_END          = 2;
var EVENT_TRIGGER_BEGIN_EVENT           = 3;
var EVENT_TRIGGER_END_EVENT             = 4;
var EVENT_TRIGGER_ON_CLICK              = 5;
var EVENT_TRIGGER_ON_DBL_CLICK          = 6;
var EVENT_TRIGGER_ON_MOUSE_ENTER        = 7;
var EVENT_TRIGGER_ON_MOUSE_LEAVE        = 8;
var EVENT_TRIGGER_ON_NEXT_EFFECT        = 9;
var EVENT_TRIGGER_ON_PREV_EFFECT        = 10;
var EVENT_TRIGGER_REPEAT                = 11;

aEventTriggerOutMap = [ 'unknown', 'slideBegin', 'slideEnd', 'begin', 'end', 'click',
                        'doubleClick', 'mouseEnter', 'mouseLeave', 'next', 'previous', 'repeat' ];


function getEventTriggerType( sEventTrigger )
{
    if( sEventTrigger == 'begin' )
        return EVENT_TRIGGER_BEGIN_EVENT;
    else if( sEventTrigger == 'end' )
        return EVENT_TRIGGER_END_EVENT;
    else if( sEventTrigger == 'next' )
        return EVENT_TRIGGER_ON_NEXT_EFFECT;
    else if( sEventTrigger == 'prev' )
        return EVENT_TRIGGER_ON_PREV_EFFECT;
    else if( sEventTrigger == 'click' )
        return EVENT_TRIGGER_ON_CLICK;
    else
        return EVENT_TRIGGER_UNKNOWN;
}



// ------------------------------------------------------------------------------------------ //

// Timing Types
var UNKNOWN_TIMING          = 0;
var OFFSET_TIMING           = 1;
var WALLCLOCK_TIMING        = 2;
var INDEFINITE_TIMING       = 3;
var EVENT_TIMING            = 4;
var SYNCBASE_TIMING         = 5;
var MEDIA_TIMING            = 6;

aTimingTypeOutMap = [ 'unknown', 'offset', 'wallclock', 'indefinite', 'event', 'syncbase', 'media' ];


// Char Codes
var CHARCODE_PLUS       = '+'.charCodeAt(0);
var CHARCODE_MINUS      = '-'.charCodeAt(0);
var CHARCODE_0          = '0'.charCodeAt(0);
var CHARCODE_9          = '9'.charCodeAt(0);



function Timing( aAnimationNode, sTimingAttribute )
{
    this.aAnimationNode = aAnimationNode;     // the node, the timing attribute belongs to
    this.sTimingDescription = removeWhiteSpaces( sTimingAttribute );
    this.eTimingType = UNKNOWN_TIMING;
    this.nOffset = 0.0;                       // in seconds
    this.sEventBaseElementId = '';            // the element id for event based timing
    this.eEventType = EVENT_TRIGGER_UNKNOWN;  // the event type
}

Timing.prototype.getAnimationNode = function()
{
    return this.aAnimationNode;
};

Timing.prototype.getType = function()
{
    return this.eTimingType;
};

Timing.prototype.getOffset = function()
{
    return this.nOffset;
};

Timing.prototype.getEventBaseElementId = function()
{
    return this.sEventBaseElementId;
};

Timing.prototype.getEventType = function()
{
    return this.eEventType;
};

Timing.prototype.parse = function()
{
    if( !this.sTimingDescription )
    {
        this.eTimingType = OFFSET_TIMING;
        return;
    }

    if( this.sTimingDescription == 'indefinite' )
        this.eTimingType = INDEFINITE_TIMING;
    else
    {
        var nFisrtCharCode = this.sTimingDescription.charCodeAt(0);
        var bPositiveOffset = !( nFisrtCharCode == CHARCODE_MINUS );
        if ( ( nFisrtCharCode == CHARCODE_PLUS ) ||
                ( nFisrtCharCode == CHARCODE_MINUS ) ||
                ( ( nFisrtCharCode >= CHARCODE_0 ) && ( nFisrtCharCode <= CHARCODE_9 ) ) )
        {
            var sClockValue
            = ( ( nFisrtCharCode == CHARCODE_PLUS ) || ( nFisrtCharCode == CHARCODE_MINUS ) )
            ? this.sTimingDescription.substr( 1 )
                    : this.sTimingDescription;

            var TimeInSec = Timing.parseClockValue( sClockValue );
            if( TimeInSec != undefined )
            {
                this.eTimingType = OFFSET_TIMING;
                this.nOffset = bPositiveOffset ? TimeInSec : -TimeInSec;
            }
        }
        else
        {
            var aTimingSplit = new Array();
            bPositiveOffset = true;
            if( this.sTimingDescription.indexOf( '+' ) != -1 )
            {
                aTimingSplit = this.sTimingDescription.split( '+' );
            }
            else if( this.sTimingDescription.indexOf( '-' ) != -1 )
            {
                aTimingSplit = this.sTimingDescription.split( '-' );
                bPositiveOffset = false;
            }
            else
            {
                aTimingSplit[0] = this.sTimingDescription;
                aTimingSplit[1] = '';
            }

            if( aTimingSplit[0].indexOf( '.' ) != -1 )
            {
                var aEventSplit = aTimingSplit[0].split( '.' );
                this.sEventBaseElementId = aEventSplit[0];
                this.eEventType = getEventTriggerType( aEventSplit[1] );
            }
            else
            {
                this.eEventType = getEventTriggerType( aTimingSplit[0] );
            }

            if( this.eEventType == EVENT_TRIGGER_UNKNOWN )
                return;

            if( ( this.eEventType == EVENT_TRIGGER_BEGIN_EVENT ) ||
                    ( this.eEventType == EVENT_TRIGGER_END_EVENT ) )
            {
                this.eTimingType = SYNCBASE_TIMING;
            }
            else
            {
                this.eTimingType = EVENT_TIMING;
            }

            if( aTimingSplit[1] )
            {
                var sClockValue = aTimingSplit[1];
                var TimeInSec = Timing.parseClockValue( sClockValue );
                if( TimeInSec != undefined )
                {
                    this.nOffset = ( bPositiveOffset ) ? TimeInSec : -TimeInSec;
                }
                else
                {
                    this.eTimingType = UNKNOWN_TIMING;
                }

            }
        }
    }

};

Timing.parseClockValue = function( sClockValue )
{
    if( !sClockValue )
        return 0.0;

    var nTimeInSec = undefined;

    var reFullClockValue = /^([0-9]+):([0-5][0-9]):([0-5][0-9])(.[0-9]+)?$/;
    var rePartialClockValue = /^([0-5][0-9]):([0-5][0-9])(.[0-9]+)?$/;
    var reTimecountValue = /^([0-9]+)(.[0-9]+)?(h|min|s|ms)?$/;

    if( reFullClockValue.test( sClockValue ) )
    {
        var aClockTimeParts = reFullClockValue.exec( sClockValue );

        var nHours = parseInt( aClockTimeParts[1] );
        var nMinutes = parseInt( aClockTimeParts[2] );
        var nSeconds = parseInt( aClockTimeParts[3] );
        if( aClockTimeParts[4] )
            nSeconds += parseFloat( aClockTimeParts[4] );

        nTimeInSec = ( ( nHours * 60 ) +  nMinutes ) * 60 + nSeconds;

    }
    else if( rePartialClockValue.test( sClockValue ) )
    {
        var aClockTimeParts = rePartialClockValue.exec( sClockValue );

        var nMinutes = parseInt( aClockTimeParts[1] );
        var nSeconds = parseInt( aClockTimeParts[2] );
        if( aClockTimeParts[3] )
            nSeconds += parseFloat( aClockTimeParts[3] );

        nTimeInSec = nMinutes * 60 + nSeconds;
    }
    else if( reTimecountValue.test( sClockValue ) )
    {
        var aClockTimeParts = reTimecountValue.exec( sClockValue );

        var nTimecount = parseInt( aClockTimeParts[1] );
        if( aClockTimeParts[2] )
            nTimecount += parseFloat( aClockTimeParts[2] );

        if( aClockTimeParts[3] )
        {
            if( aClockTimeParts[3] == 'h' )
            {
                nTimeInSec = nTimecount * 3600;
            }
            else if( aClockTimeParts[3] == 'min' )
            {
                nTimeInSec = nTimecount * 60;
            }
            else if( aClockTimeParts[3] == 's' )
            {
                nTimeInSec = nTimecount;
            }
            else if( aClockTimeParts[3] == 'ms' )
            {
                nTimeInSec = nTimecount / 1000;
            }
        }
        else
        {
            nTimeInSec = nTimecount;
        }

    }

    if( nTimeInSec )
        nTimeInSec = parseFloat( nTimeInSec.toFixed( 3 ) );
    return nTimeInSec;
};

Timing.prototype.info = function( bVerbose )
{

    var sInfo = '';

    if( bVerbose )
    {
        sInfo = 'description: ' + this.sTimingDescription + ', ';

        sInfo += ', type: ' +  aTimingTypeOutMap[ this.getType() ];
        sInfo += ', offset: ' + this.getOffset();
        sInfo += ', event base element id: ' + this.getEventBaseElementId();
        sInfo += ', timing event type: ' + aEventTriggerOutMap[ this.getEventType() ];
    }
    else
    {
        switch( this.getType() )
        {
            case INDEFINITE_TIMING:
                sInfo += 'indefinite';
                break;
            case OFFSET_TIMING:
                sInfo += this.getOffset();
                break;
            case EVENT_TIMING:
            case SYNCBASE_TIMING:
                if( this.getEventBaseElementId() )
                    sInfo += this.getEventBaseElementId() + '.';
                sInfo += aEventTriggerOutMap[ this.getEventType() ];
                if( this.getOffset() )
                {
                    if( this.getOffset() > 0 )
                        sInfo += '+';
                    sInfo += this.getOffset();
                }
        }
    }

    return sInfo;
};



// ------------------------------------------------------------------------------------------ //
function Duration( sDurationAttribute )
{
    this.bIndefinite = false;
    this.bMedia = false;
    this.nValue = undefined;
    this.bDefined = false;

    if( !sDurationAttribute )
        return;

    if( sDurationAttribute == 'indefinite' )
        this.bIndefinite = true;
    else if( sDurationAttribute == 'media' )
        this.bMedia = true;
    else
    {
        this.nValue = Timing.parseClockValue( sDurationAttribute );
        if( this.nValue <= 0.0 )
            this.nValue = 0.001;  // duration must be always greater than 0
    }
    this.bDefined = true;
}


Duration.prototype.isSet = function()
{
    return this.bDefined;
};

Duration.prototype.isIndefinite = function()
{
    return this.bIndefinite;
};

Duration.prototype.isMedia = function()
{
    return this.bMedia;
};

Duration.prototype.isValue = function()
{
    return this.nValue != undefined;
};

Duration.prototype.getValue= function()
{
    return this.nValue;
};

Duration.prototype.info= function()
{
    var sInfo;

    if( this.isIndefinite() )
        sInfo = 'indefinite';
    else if( this.isMedia() )
        sInfo = 'media';
    else if( this.getValue() )
        sInfo = this.getValue();

    return sInfo;
};



// ------------------------------------------------------------------------------------------ //
function AnimationNode()
{
}

AnimationNode.prototype.init = function() {};
AnimationNode.prototype.resolve = function() {};
AnimationNode.prototype.activate = function() {};
AnimationNode.prototype.deactivate = function() {};
AnimationNode.prototype.end = function() {};
AnimationNode.prototype.getState = function() {};
AnimationNode.prototype.registerDeactivatingListener = function() {};
AnimationNode.prototype.notifyDeactivating = function() {};



// ------------------------------------------------------------------------------------------ //
function NodeContext( aSlideShowContext )
{
    this.aContext = aSlideShowContext;
    this.aAnimationNodeMap = null;
    this.aAnimatedElementMap = null;
    this.aSourceEventElementMap = null;
    this.nStartDelay = 0.0;
    this.bFirstRun = undefined;
    this.aSlideHeight = HEIGHT;
    this.aSlideWidth = WIDTH;
}


NodeContext.prototype.makeSourceEventElement = function( sId, aEventBaseElem )
{
    if( !aEventBaseElem )
    {
        log( 'NodeContext.makeSourceEventElement: event base element is not valid' );
        return null;
    }

    if( !this.aContext.aEventMultiplexer )
    {
        log( 'NodeContext.makeSourceEventElement: event multiplexer not initialized' );
        return null;
    }

    if( !this.aAnimationNodeMap[ sId ] )
    {
        this.aAnimationNodeMap[ sId ] = new SourceEventElement( aEventBaseElem, this.aContext.aEventMultiplexer );
    }
    return this.aAnimationNodeMap[ sId ];
};



// ------------------------------------------------------------------------------------------ //
function StateTransition( aBaseNode )
{
    this.aNode = aBaseNode;
    this.eToState = INVALID_NODE;
}

StateTransition.prototype.enter = function( eNodeState, bForce )
{
    if( !bForce ) bForce = false;

    if( this.eToState != INVALID_NODE )
    {
        log( 'StateTransition.enter: commit() before enter()ing again!' );
        return false;
    }
    if( !bForce && !this.aNode.isTransition( this.aNode.getState(), eNodeState  ) )
        return false;

    // recursion detection:
    if( ( this.aNode.nCurrentStateTransition & eNodeState ) != 0 )
        return false; // already in wanted transition

    // mark transition:
    this.aNode.nCurrentStateTransition |= eNodeState;
    this.eToState = eNodeState;
    return true;
};

StateTransition.prototype.commit = function()
{
    if( this.eToState != INVALID_NODE )
    {
        this.aNode.eCurrentState = this.eToState;
        this.clear();
    }
};

StateTransition.prototype.clear = function()
{
    if( this.eToState != INVALID_NODE )
    {
        this.aNode.nCurrentStateTransition &= ~this.eToState;
        this.eToState = INVALID_NODE;
    }
};



// ------------------------------------------------------------------------------------------ //
function BaseNode( aAnimElem, aParentNode, aNodeContext )
{
    this.nId = getUniqueId();
    this.sClassName = 'BaseNode';

    if( !aAnimElem )
        log( 'BaseNode(id:' + this.nId + ') constructor: aAnimElem is not valid' );

    if( !aNodeContext )
        log( 'BaseNode(id:' + this.nId + ') constructor: aNodeContext is not valid' );

    if( !aNodeContext.aContext )
        log( 'BaseNode(id:' + this.nId + ') constructor: aNodeContext.aContext is not valid' );


    this.bIsContainer;
    this.aElement = aAnimElem;
    this.aParentNode = aParentNode;
    this.aNodeContext = aNodeContext;
    this.aContext = aNodeContext.aContext;
    this.nStartDelay = aNodeContext.nStartDelay;
    this.eCurrentState = UNRESOLVED_NODE;
    this.nCurrentStateTransition = 0;
    this.aDeactivatingListenerArray = new Array();
    this.aActivationEvent = null;
    this.aDeactivationEvent = null;

    this.aBegin = null;
    this.aDuration = null;
    this.aEnd = null;
    this.bMainSequenceRootNode = false;
    this.eFillMode = FILL_MODE_FREEZE;
    this.eRestartMode = RESTART_MODE_NEVER;
    this.nReapeatCount = undefined;
    this.nAccelerate = 0.0;
    this.nDecelerate = 0.0;
    this.bAutoReverse = false;

}
extend( BaseNode, AnimationNode );


BaseNode.prototype.getId = function()
{
    return this.nId;
};

BaseNode.prototype.parseElement = function()
{
    var aAnimElem = this.aElement;

    // id attribute
    var sIdAttr = aAnimElem.getAttributeNS( NSS['xml'], 'id' );
    // we append the animation node to the Animation Node Map so that it can be retrieved
    // by the registerEvent routine for resolving begin values of type 'id.begin', 'id.end'
    if( sIdAttr )
        this.aNodeContext.aAnimationNodeMap[ sIdAttr ] = this;

    // begin attribute
    this.aBegin = null;
    var sBeginAttr = aAnimElem.getAttribute( 'begin' );
    this.aBegin = new Timing( this, sBeginAttr );
    this.aBegin.parse();

    // end attribute
    this.aEnd = null;
    var sEndAttr = aAnimElem.getAttribute( 'end' );
    if( sEndAttr )
    {
        this.aEnd = new Timing( this, sEndAttr );
        this.aEnd.parse();
    }

    // dur attribute
    this.aDuration = null;
    var sDurAttr = aAnimElem.getAttribute( 'dur' );
    this.aDuration = new Duration( sDurAttr );
    if( !this.aDuration.isSet() )
    {
        if( this.isContainer() )
            this.aDuration = null;
        else
            this.aDuration = new Duration( 'indefinite' );
    }

    // fill attribute
    var sFillAttr = aAnimElem.getAttribute( 'fill' );
    if( sFillAttr && aFillModeInMap[ sFillAttr ])
        this.eFillMode = aFillModeInMap[ sFillAttr ];
    else
        this.eFillMode = FILL_MODE_DEFAULT;

    // restart attribute
    var sRestartAttr = aAnimElem.getAttribute( 'restart' );
    if( sRestartAttr && aRestartModeInMap[ sRestartAttr ] )
        this.eRestartMode = aRestartModeInMap[ sRestartAttr ];
    else
        this.eRestartMode = RESTART_MODE_DEFAULT;

    // repeatCount attribute
    var sRepeatCount = aAnimElem.getAttribute( 'repeatCount' );
    if( !sRepeatCount )
        this.nReapeatCount = 1;
    else
        this.nReapeatCount = parseFloat( sRepeatCount );
    if( ( this.nReapeatCount == NaN ) && ( sRepeatCount != 'indefinite' ) )
        this.nReapeatCount = 1;

    // accelerate attribute
    this.nAccelerate = 0.0;
    var sAccelerateAttr = aAnimElem.getAttribute( 'accelerate' );
    if( sAccelerateAttr )
        this.nAccelerate = parseFloat( sAccelerateAttr );
    if( this.nAccelerate == NaN )
        this.nAccelerate = 0.0;

    // decelerate attribute
    this.nDecelerate = 0.0;
    var sDecelerateAttr = aAnimElem.getAttribute( 'decelerate' );
    if( sDecelerateAttr )
        this.nDecelerate = parseFloat( sDecelerateAttr );
    if( this.nDecelerate == NaN )
        this.nDecelerate = 0.0;

    // autoReverse attribute
    this.bAutoreverse = false;
    var sAutoReverseAttr = aAnimElem.getAttribute( 'autoReverse' );
    if( sAutoReverseAttr == 'true' )
        this.bAutoreverse = true;


    // resolve fill value
    if( this.eFillMode == FILL_MODE_DEFAULT )
        if( this.getParentNode() )
            this.eFillMode = this.getParentNode().getFillMode();
        else
            this.eFillMode = FILL_MODE_AUTO;

    if( this.eFillMode ==  FILL_MODE_AUTO ) // see SMIL recommendation document
    {
        this.eFillMode = ( this.aEnd ||
                            ( this.nReapeatCount != 1) ||
                            this.aDuration )
                            ? FILL_MODE_REMOVE
                            : FILL_MODE_FREEZE;
    }

    // resolve restart value
    if( this.eRestartMode == RESTART_MODE_DEFAULT )
        if( this.getParentNode() )
            this.eRestartMode = this.getParentNode().getRestartMode();
        else
            // SMIL recommendation document says to set it to 'always'
            // but we follow the slideshow engine C++ implementation
            this.eRestartMode = RESTART_MODE_NEVER;

    // resolve accelerate and decelerate attributes
    // from the SMIL recommendation document: if the individual values of the accelerate
    // and decelerate attributes are between 0 and 1 and the sum is greater than 1,
    // then both the accelerate and decelerate attributes will be ignored and the timed
    // element will behave as if neither attribute was specified.
    if( ( this.nAccelerate + this.nDecelerate ) > 1.0 )
    {
        this.nAccelerate = 0.0;
        this.nDecelerate = 0.0;
    }

    // TODO: at present we are able to manage only this case
    this.eFillMode = FILL_MODE_FREEZE;
    this.eRestartMode = RESTART_MODE_NEVER;
    this.aStateTransTable = getTransitionTable( this.getRestartMode(), this.getFillMode() );

    return true;
};

BaseNode.prototype.getParentNode = function()
{
    return this.aParentNode;
};

BaseNode.prototype.init = function()
{
    if( ! this.checkValidNode() )
        return false;
    if( this.aActivationEvent )
        this.aActivationEvent.dispose();
    if( this.aDeactivationEvent )
        this.aDeactivationEvent.dispose();

    this.eCurrentState = UNRESOLVED_NODE;

    return this.init_st();
};

BaseNode.prototype.resolve = function()
{
    if( ! this.checkValidNode() )
        return false;

    this.DBG( this.callInfo( 'resolve' ) );

    if( this.eCurrentState == RESOLVED_NODE )
        log( 'BaseNode.resolve: already in RESOLVED state' );

    var aStateTrans = new StateTransition( this );

    if( aStateTrans.enter( RESOLVED_NODE ) &&
            this.isTransition( RESOLVED_NODE, ACTIVE_NODE ) &&
            this.resolve_st() )
    {
        aStateTrans.commit();

        if( this.aActivationEvent )
        {
            this.aActivationEvent.charge();
        }
        else
        {
            this.aActivationEvent = makeDelay( bind( this, this.activate ), this.getBegin().getOffset() + this.nStartDelay );
        }
        registerEvent( this.getBegin(), this.aActivationEvent, this.aNodeContext );

        return true;
    }

    return false;
};

BaseNode.prototype.activate = function()
{
    if( ! this.checkValidNode() )
        return false;

    if( this.eCurrentState == ACTIVE_NODE )
        log( 'BaseNode.activate: already in ACTIVE state' );

    this.DBG( this.callInfo( 'activate' ), getCurrentSystemTime() );

    var aStateTrans = new StateTransition( this );

    if( aStateTrans.enter( ACTIVE_NODE ) )
    {
        this.activate_st();
        aStateTrans.commit();
        if( !this.aContext.aEventMultiplexer )
            log( 'BaseNode.activate: this.aContext.aEventMultiplexer is not valid' );
        this.aContext.aEventMultiplexer.notifyEvent( EVENT_TRIGGER_BEGIN_EVENT, this.getId() );
        return true;
    }
    return false;
};

BaseNode.prototype.deactivate = function()
{
    if( this.inStateOrTransition( ENDED_NODE | FROZEN_NODE ) || !this.checkValidNode() )
        return;

    if( this.isTransition( this.eCurrentState, FROZEN_NODE ) )
    {
        this.DBG( this.callInfo( 'deactivate' ), getCurrentSystemTime() );

        var aStateTrans = new StateTransition( this );
        if( aStateTrans.enter( FROZEN_NODE, true /* FORCE */ ) )
        {
            this.deactivate_st();
            aStateTrans.commit();

            this.notifyEndListeners();

            if( this.aActivationEvent )
                this.aActivationEvent.dispose();
            if( this.aDeactivationEvent )
                this.aDeactivationEvent.dispose();
        }
    }
    else
    {
        this.end();
    }
    // state has changed either to FROZEN or ENDED
};

BaseNode.prototype.end = function()
{
    var bIsFrozenOrInTransitionToFrozen = this.inStateOrTransition( FROZEN_NODE );
    if( this.inStateOrTransition( ENDED_NODE ) || !this.checkValidNode() )
        return;

    if( !(this.isTransition( this.eCurrentState, ENDED_NODE ) ) )
        log( 'BaseNode.end: end state not reachable in transition table' );

    this.DBG( this.callInfo( 'end' ), getCurrentSystemTime() );

    var aStateTrans = new StateTransition( this );
    if( aStateTrans.enter( ENDED_NODE, true /* FORCE */ ) )
    {
        this.deactivate_st( ENDED_NODE );
        aStateTrans.commit();

        // if is FROZEN or is to be FROZEN, then
        // will/already notified deactivating listeners
        if( !bIsFrozenOrInTransitionToFrozen )
            this.notifyEndListeners();

        if( this.aActivationEvent )
            this.aActivationEvent.dispose();
        if( this.aDeactivationEvent )
            this.aDeactivationEvent.dispose();
    }
};

BaseNode.prototype.dispose = function()
{
    if( this.aActivationEvent )
        this.aActivationEvent.dispose();
    if( this.aDeactivationEvent )
        this.aDeactivationEvent.dispose();
    this.aDeactivatingListenerArray = new Array();
};

BaseNode.prototype.getState = function()
{
    return this.eCurrentState;
};

BaseNode.prototype.registerDeactivatingListener = function( aNotifiee )
{
    if (! this.checkValidNode())
        return false;

    if( !aNotifiee )
    {
        log( 'BaseNode.registerDeactivatingListener(): invalid notifee' );
        return false;
    }
    this.aDeactivatingListenerArray.push( aNotifiee );

    return true;
};

BaseNode.prototype.notifyDeactivating = function( aNotifier )
{
    assert( ( aNotifier.getState() == FROZEN_NODE ) || ( aNotifier.getState() == ENDED_NODE ),
    'BaseNode.notifyDeactivating: Notifier node is neither in FROZEN nor in ENDED state' );
};

BaseNode.prototype.isMainSequenceRootNode = function()
{
    return this.bMainSequenceRootNode;
};

BaseNode.prototype.makeDeactivationEvent = function( nDelay )
{
    if( this.aDeactivationEvent )
    {
        this.aDeactivationEvent.charge();
    }
    else
    {
        if( typeof( nDelay ) == typeof(0) )
            this.aDeactivationEvent = makeDelay( bind( this, this.deactivate ), nDelay );
        else
            this.aDeactivationEvent = null;
    }
    return this.aDeactivationEvent;
};

BaseNode.prototype.scheduleDeactivationEvent = function( aEvent )
{
    this.DBG( this.callInfo( 'scheduleDeactivationEvent' ) );

    if( !aEvent )
    {
        if( this.getDuration() && this.getDuration().isValue() )
            aEvent = this.makeDeactivationEvent( this.getDuration().getValue() );
    }
    if( aEvent )
    {
        this.aContext.aTimerEventQueue.addEvent( aEvent );
    }
};

BaseNode.prototype.checkValidNode = function()
{
    return ( this.eCurrentState != INVALID_NODE );
};

BaseNode.prototype.init_st = function()
{
    return true;
};

BaseNode.prototype.resolve_st = function()
{
    return true;
};

BaseNode.prototype.activate_st = function()
{
    this.scheduleDeactivationEvent();
};

BaseNode.prototype.deactivate_st = function( aNodeState )
{
    // empty body
};

BaseNode.prototype.notifyEndListeners = function()
{
    var nDeactivatingListenerCount = this.aDeactivatingListenerArray.length;

    for( var i = 0; i < nDeactivatingListenerCount; ++i )
    {
        this.aDeactivatingListenerArray[i].notifyDeactivating( this );
    }

    this.aContext.aEventMultiplexer.notifyEvent( EVENT_TRIGGER_END_EVENT, this.getId() );
};

BaseNode.prototype.getContext = function()
{
    return this.aContext;
};

BaseNode.prototype.isTransition = function( eFromState, eToState )
{
    return ( ( this.aStateTransTable[ eFromState ] & eToState ) != 0 );
};

BaseNode.prototype.inStateOrTransition = function( nMask )
{
    return ( ( ( this.eCurrentState & nMask ) != 0 ) || ( ( this.nCurrentStateTransition & nMask ) != 0 ) );
};

BaseNode.prototype.isContainer = function()
{
    return this.bIsContainer;
};

BaseNode.prototype.getBegin = function()
{
    return this.aBegin;
};

BaseNode.prototype.getDuration = function()
{
    return this.aDuration;
};

BaseNode.prototype.getEnd = function()
{
    return this.aEnd;
};

BaseNode.prototype.getFillMode = function()
{
    return this.eFillMode;
};

BaseNode.prototype.getRestartMode = function()
{
    return this.eRestartMode;
};

BaseNode.prototype.getRepeatCount = function()
{
    return this.nReapeatCount;
};

BaseNode.prototype.getAccelerateValue = function()
{
    return this.nAccelerate;
};

BaseNode.prototype.getDecelerateValue = function()
{
    return this.nDecelerate;
};

BaseNode.prototype.isAutoReverseEnabled = function()
{
    return this.bAutoreverse;
};

BaseNode.prototype.info = function( bVerbose )
{
    var sInfo = 'class name: ' + this.sClassName;
    sInfo += ';  element name: ' + this.aElement.localName;
    sInfo += ';  id: ' + this.getId();
    sInfo += ';  state: ' + getNodeStateName( this.getState() );

    if( bVerbose )
    {
        // is container
        sInfo += ';  is container: ' + this.isContainer();

        // begin
        if( this.getBegin() )
            sInfo += ';  begin: ' + this.getBegin().info();

        // duration
        if( this.getDuration() )
            sInfo += ';  dur: ' + this.getDuration().info();

        // end
        if( this.getEnd() )
            sInfo += ';  end: ' + this.getEnd().info();

        // fill mode
        if( this.getFillMode() )
            sInfo += ';  fill: ' + aFillModeOutMap[ this.getFillMode() ];

        // restart mode
        if( this.getRestartMode() )
            sInfo += ';  restart: ' + aRestartModeOutMap[ this.getRestartMode() ];

        // repeatCount
        if( this.getRepeatCount() && ( this.getRepeatCount() != 1.0 ) )
            sInfo += ';  repeatCount: ' + this.getRepeatCount();

        // accelerate
        if( this.getAccelerateValue() )
            sInfo += ';  accelerate: ' + this.getAccelerateValue();

        // decelerate
        if( this.getDecelerateValue() )
            sInfo += ';  decelerate: ' + this.getDecelerateValue();

        // auto reverse
        if( this.isAutoReverseEnabled() )
            sInfo += ';  autoReverse: true';

    }

    return sInfo;
};

BaseNode.prototype.callInfo = function( sMethodName )
{
    var sInfo = this.sClassName +
                '( ' + this.getId() +
                ', ' + getNodeStateName( this.getState() ) +
                ' ).' + sMethodName;
    return sInfo;
};

BaseNode.prototype.DBG = function( sMessage, nTime )
{
    ANIMDBG.print( sMessage, nTime );
};



// ------------------------------------------------------------------------------------------ //
function AnimationBaseNode( aAnimElem, aParentNode, aNodeContext )
{
    AnimationBaseNode.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );

    this.sClassName = 'AnimationBaseNode';
    this.bIsContainer = false;
    this.aTargetElement = null;
    this.aAnimatedElement = null;
    this.aActivity = null;

    this.nMinFrameCount;
    this.eAdditiveMode;

}
extend( AnimationBaseNode, BaseNode );


AnimationBaseNode.prototype.parseElement = function()
{
    var bRet = AnimationBaseNode.superclass.parseElement.call( this );

    var aAnimElem = this.aElement;

    // targetElement attribute
    this.aTargetElement = null;
    var sTargetElementAttr = aAnimElem.getAttribute( 'targetElement' );
    if( sTargetElementAttr )
        this.aTargetElement = document.getElementById( sTargetElementAttr );

    if( !this.aTargetElement )
    {
        this.eCurrentState = INVALID_NODE;
        log( 'AnimationBaseNode.parseElement: target element not found: ' + sTargetElementAttr );
    }

    // additive attribute
    var sAdditiveAttr = aAnimElem.getAttribute( 'additive' );
    if( sAdditiveAttr && aAddittiveModeInMap[sAdditiveAttr] )
        this.eAdditiveMode = aAddittiveModeInMap[sAdditiveAttr];
    else
        this.eAdditiveMode = ADDITIVE_MODE_REPLACE;

    // set up min frame count value;
    this.nMinFrameCount = ( this.getDuration().isValue() )
            ? ( this.getDuration().getValue() * MINIMUM_FRAMES_PER_SECONDS )
            : MINIMUM_FRAMES_PER_SECONDS;
    if( this.nMinFrameCount < 1.0 )
        this.nMinFrameCount = 1;
    else if( this.nMinFrameCount > MINIMUM_FRAMES_PER_SECONDS )
        this.nMinFrameCount = MINIMUM_FRAMES_PER_SECONDS;


    if( this.aTargetElement )
    {
        // set up target element initial visibility
        if( true && aAnimElem.getAttribute( 'attributeName' ) === 'visibility' )
        {
            if( aAnimElem.getAttribute( 'to' ) === 'visible' )
                this.aTargetElement.setAttribute( 'visibility', 'hidden' );
        }

        // create animated element
        if( !this.aNodeContext.aAnimatedElementMap[ sTargetElementAttr ] )
        {
            this.aNodeContext.aAnimatedElementMap[ sTargetElementAttr ]
                    = new AnimatedElement( this.aTargetElement );
        }
        this.aAnimatedElement = this.aNodeContext.aAnimatedElementMap[ sTargetElementAttr ];

        // set additive mode
        this.aAnimatedElement.setAdditiveMode( this.eAdditiveMode );
    }


    return bRet;
};

AnimationBaseNode.prototype.init_st = function()
{
    if( this.aActivity )
        this.aActivity.activate( makeEvent( bind( this, this.deactivate ) ) );
    else
        this.aActivity = this.createActivity();
    return true;
};

AnimationBaseNode.prototype.resolve_st = function()
{
    return true;
};

AnimationBaseNode.prototype.activate_st = function()
{
    if( this.aActivity )
    {
        this.aActivity.setTargets( this.getAnimatedElement() );
        this.getContext().aActivityQueue.addActivity( this.aActivity );
    }
    else
    {
        AnimationBaseNode.superclass.scheduleDeactivationEvent.call( this );
    }

    // TODO: only for testing! to be removed!
    //AnimationBaseNode.superclass.scheduleDeactivationEvent.call( this );
};

AnimationBaseNode.prototype.deactivate_st = function( eDestState )
{
    if( eDestState == FROZEN_NODE )
    {
        if( this.aActivity )
            this.aActivity.end();
    }
    if( eDestState == ENDED_NODE )
    {
        if( this.aActivity )
            this.aActivity.dispose();
    }
};

AnimationBaseNode.prototype.createActivity = function()
{
    log( 'AnimationBaseNode.createActivity: abstract method called' );
};

AnimationBaseNode.prototype.fillActivityParams = function()
{

    // compute duration
    var nDuration = 0.001;
    if( this.getDuration().isValue() )
    {
        nDuration = this.getDuration().getValue();
    }
    else
    {
        log( 'AnimationBaseNode.fillActivityParams: duration is not a number' );
    }

    // create and set up activity params
        var aActivityParamSet = new ActivityParamSet();

        aActivityParamSet.aEndEvent             = makeEvent( bind( this, this.deactivate ) );
        aActivityParamSet.aTimerEventQueue      = this.aContext.aTimerEventQueue;
        aActivityParamSet.aActivityQueue        = this.aContext.aActivityQueue;
        aActivityParamSet.nMinDuration          = nDuration;
        aActivityParamSet.nMinNumberOfFrames    = this.getMinFrameCount();
        aActivityParamSet.bAutoReverse          = this.isAutoReverseEnabled();
        aActivityParamSet.nRepeatCount          = this.getRepeatCount();
        aActivityParamSet.nAccelerationFraction = this.getAccelerateValue();
        aActivityParamSet.nDecelerationFraction = this.getDecelerateValue();
        aActivityParamSet.nSlideWidth           = this.aNodeContext.aSlideWidth;
        aActivityParamSet.nSlideHeight          = this.aNodeContext.aSlideHeight;

        return aActivityParamSet;
    };

    AnimationBaseNode.prototype.hasPendingAnimation = function()
    {
        return true;
    };

    AnimationBaseNode.prototype.getTargetElement = function()
    {
        return this.aTargetElement;
    };

    AnimationBaseNode.prototype.getAnimatedElement = function()
    {
        return this.aAnimatedElement;
    };

    AnimationBaseNode.prototype.dispose= function()
    {
        if( this.aActivity )
            this.aActivity.dispose();

        AnimationBaseNode.superclass.dispose.call( this );
    };

    AnimationBaseNode.prototype.getMinFrameCount = function()
    {
        return this.nMinFrameCount;
    };

    AnimationBaseNode.prototype.getAdditiveMode = function()
    {
        return this.eAdditiveMode;
    };

    AnimationBaseNode.prototype.info = function( bVerbose )
    {
        var sInfo = AnimationBaseNode.superclass.info.call( this, bVerbose );

        if( bVerbose )
        {
            // min frame count
        if( this.getMinFrameCount() )
            sInfo += ';  min frame count: ' + this.getMinFrameCount();

        // additive mode
        sInfo += ';  additive: ' + aAddittiveModeOutMap[ this.getAdditiveMode() ];

        // target element
        if( this.getShape() )
        {
            sElemId = this.getShape().getAttribute( 'id' );
            sInfo += ';  targetElement: ' +  sElemId;
        }
    }

    return sInfo;
};


// ------------------------------------------------------------------------------------------ //
function AnimationBaseNode2( aAnimElem, aParentNode, aNodeContext )
{
    AnimationBaseNode2.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );

    this.sAttributeName;
    this.aToValue;

}
extend( AnimationBaseNode2, AnimationBaseNode );


AnimationBaseNode2.prototype.parseElement = function()
{
    var bRet = AnimationBaseNode2.superclass.parseElement.call( this );

    var aAnimElem = this.aElement;

    // attributeName attribute
    this.sAttributeName = aAnimElem.getAttribute( 'attributeName' );
    if( !this.sAttributeName )
    {
        this.eCurrentState = INVALID_NODE;
        log( 'AnimationBaseNode2.parseElement: target attribute name not found: ' + this.sAttributeName );
    }

    // to attribute
    this.aToValue = aAnimElem.getAttribute( 'to' );

    return bRet;
};

AnimationBaseNode2.prototype.getAttributeName = function()
{
    return this.sAttributeName;
};

AnimationBaseNode2.prototype.getToValue = function()
{
    return this.aToValue;
};

AnimationBaseNode2.prototype.info = function( bVerbose )
{
    var sInfo = AnimationBaseNode2.superclass.info.call( this, bVerbose );

    if( bVerbose )
    {
        // attribute name
        if( this.getAttributeName() )
            sInfo += ';  attributeName: ' + this.getAttributeName();

        // To
        if( this.getToValue() )
            sInfo += ';  to: ' + this.getToValue();
    }

    return sInfo;
};



// ------------------------------------------------------------------------------------------ //
function AnimationBaseNode3( aAnimElem, aParentNode, aNodeContext )
{
    AnimationBaseNode3.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );

    this.eAccumulate;
    this.eCalcMode;
    this.aFromValue;
    this.aByValue;
    this.aKeyTimes;
    this.aValues;

}
extend( AnimationBaseNode3, AnimationBaseNode2 );


AnimationBaseNode3.prototype.parseElement = function()
{
    var bRet = AnimationBaseNode3.superclass.parseElement.call( this );

    var aAnimElem = this.aElement;

    // accumulate attribute
    this.eAccumulate = ACCUMULATE_MODE_NONE;
    var sAccumulateAttr = aAnimElem.getAttribute( 'accumulate' );
    if( sAccumulateAttr == 'sum' )
        this.eAccumulate = ACCUMULATE_MODE_SUM;

    // calcMode attribute
    this.eCalcMode = CALC_MODE_LINEAR;
    var sCalcModeAttr = aAnimElem.getAttribute( 'calcMode' );
    if( sCalcModeAttr && aCalcModeInMap[ sCalcModeAttr ] )
        this.eCalcMode = aCalcModeInMap[ sCalcModeAttr ];

    // from attribute
    this.aFromValue = aAnimElem.getAttribute( 'from' );

    // by attribute
    this.aByValue = aAnimElem.getAttribute( 'by' );

    // keyTimes attribute
    this.aKeyTimes = new Array();
    var sKeyTimesAttr = aAnimElem.getAttribute( 'keyTimes' );
    sKeyTimesAttr = removeWhiteSpaces( sKeyTimesAttr );
    if( sKeyTimesAttr )
    {
        var aKeyTimes = sKeyTimesAttr.split( ';' );
        for( var i = 0; i < aKeyTimes.length; ++i )
            this.aKeyTimes.push( parseFloat( aKeyTimes[i] ) );
    }

    // values attribute
    var sValuesAttr = aAnimElem.getAttribute( 'values' );
    if( sValuesAttr )
    {
        this.aValues = sValuesAttr.split( ';' );
    }
    else
    {
        this.aValues = new Array();
    }

    return bRet;
};

AnimationBaseNode3.prototype.getAccumulate = function()
{
    return this.eAccumulate;
};

AnimationBaseNode3.prototype.getCalcMode = function()
{
    return this.eCalcMode;
};

AnimationBaseNode3.prototype.getFromValue = function()
{
    return this.aFromValue;
};

AnimationBaseNode3.prototype.getByValue = function()
{
    return this.aByValue;
};

AnimationBaseNode3.prototype.getKeyTimes = function()
{
    return this.aKeyTimes;
};

AnimationBaseNode3.prototype.getValues = function()
{
    return this.aValues;
};

AnimationBaseNode3.prototype.info = function( bVerbose )
{
    var sInfo = AnimationBaseNode3.superclass.info.call( this, bVerbose );

    if( bVerbose )
    {
        // accumulate mode
        if( this.getAccumulate() )
            sInfo += ';  accumulate: ' + aAccumulateModeOutMap[ this.getAccumulate() ];

        // calcMode
        sInfo += ';  calcMode: ' + aCalcModeOutMap[ this.getCalcMode() ];

        // from
        if( this.getFromValue() )
            sInfo += ';  from: ' + this.getFromValue();

        // by
        if( this.getByValue() )
            sInfo += ';  by: ' + this.getByValue();

        // keyTimes
        if( this.getKeyTimes().length )
            sInfo += ';  keyTimes: ' + this.getKeyTimes().join( ',' );

        // values
        if( this.getKeyTimes().length )
            sInfo += ';  values: ' + this.getValues().join( ',' );
    }

    return sInfo;
};



// ------------------------------------------------------------------------------------------ //
function BaseContainerNode( aAnimElem, aParentNode, aNodeContext )
{
    BaseContainerNode.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );

    this.sClassName = 'BaseContainerNode';
    this.bIsContainer = true;
    this.aChildrenArray = new Array();
    this.nFinishedChildren = 0;
    this.bDurationIndefinite = false;

    this.eImpressNodeType = undefined;
    this.ePresetClass =  undefined;
    this.ePresetId =  undefined;
}
extend( BaseContainerNode, BaseNode );


BaseContainerNode.prototype.parseElement= function()
{
    var bRet = BaseContainerNode.superclass.parseElement.call( this );

    var aAnimElem = this.aElement;

    // node-type attribute
    this.eImpressNodeType = IMPRESS_DEFAULT_NODE;
    var sNodeTypeAttr = aAnimElem.getAttribute( 'node-type' );
    if( sNodeTypeAttr && aImpressNodeTypeInMap[ sNodeTypeAttr ] )
        this.eImpressNodeType = aImpressNodeTypeInMap[ sNodeTypeAttr ];
    this.bMainSequenceRootNode = ( this.eImpressNodeType == IMPRESS_MAIN_SEQUENCE_NODE );

    // preset-class attribute
    this.ePresetClass =  undefined;
    var sPresetClassAttr = aAnimElem.getAttribute( 'preset-class' );
    if( sPresetClassAttr && aPresetClassInMap[ sPresetClassAttr ] )
        this.ePresetClass = aPresetClassInMap[ sPresetClassAttr ];

    // preset-id attribute
    this.ePresetId =  undefined;
    var sPresetIdAttr = aAnimElem.getAttribute( 'preset-id' );
    if( sPresetIdAttr && aPresetIdInMap[ sPresetIdAttr ] )
        this.ePresetId = aPresetIdInMap[ sPresetIdAttr ];


    // parse children elements
    var nChildrenCount = this.aChildrenArray.length;
    for( var i = 0; i < nChildrenCount; ++i )
    {
        this.aChildrenArray[i].parseElement();
    }


    // resolve duration
    this.bDurationIndefinite
            = ( !this.getDuration() || this.getDuration().isIndefinite()  ) &&
              ( !this.getEnd() || ( this.getEnd().getType() != OFFSET_TIMING ) );

    return bRet;
};

BaseContainerNode.prototype.appendChildNode = function( aAnimationNode )
{
    if( ! this.checkValidNode() )
        return ;

    if( aAnimationNode.registerDeactivatingListener( this ) )
        this.aChildrenArray.push( aAnimationNode );
};

BaseContainerNode.prototype.init_st = function()
{
    this.nFinishedChildren = 0;
    var nChildrenCount = this.aChildrenArray.length;
    var nInitChildren = 0;
    for( var i = 0; i < nChildrenCount; ++i )
    {
        if( this.aChildrenArray[i].init() )
        {
            ++nInitChildren;
        }
    }
    return ( nChildrenCount == nInitChildren );
};

BaseContainerNode.prototype.deactivate_st = function( eDestState )
{
    if( eDestState == FROZEN_NODE )
    {
        // deactivate all children that are not FROZEN or ENDED:
        this.forEachChildNode( mem_fn( 'deactivate' ), ~( FROZEN_NODE | ENDED_NODE ) );
    }
    else
    {
        // end all children that are not ENDED:
        this.forEachChildNode( mem_fn( 'end' ), ~ENDED_NODE );
    }
};

BaseContainerNode.prototype.hasPendingAnimation = function()
{
    var nChildrenCount = this.aChildrenArray.length;
    for( var i = 0; i < nChildrenCount; ++i )
    {
        if( this.aChildrenArray[i].hasPendingAnimation() )
            return true;
    }
    return false;
};

BaseContainerNode.prototype.activate_st = function()
{
    log( 'BaseContainerNode.activate_st: abstract method called' );
};

BaseContainerNode.prototype.notifyDeactivating = function( aAnimationNode )
{
    log( 'BaseContainerNode.notifyDeactivating: abstract method called' );
};

BaseContainerNode.prototype.isDurationIndefinite = function()
{
    return this.bDurationIndefinite;
};

BaseContainerNode.prototype.isChildNode = function( aAnimationNode )
{
    var nChildrenCount = this.aChildrenArray.length;
    for( var i = 0; i < nChildrenCount; ++i )
    {
        if( this.aChildrenArray[i].getId() == aAnimationNode.getId() )
            return true;
    }
    return false;
};

BaseContainerNode.prototype.notifyDeactivatedChild = function( aChildNode )
{
    assert( ( aChildNode.getState() == FROZEN_NODE ) || ( aChildNode.getState() == ENDED_NODE ),
    'BaseContainerNode.notifyDeactivatedChild: passed child node is neither in FROZEN nor in ENDED state' );

    assert( this.getState() != INVALID_NODE,
    'BaseContainerNode.notifyDeactivatedChild: this node is invalid' );

    if( !this.isChildNode( aChildNode ) )
    {
        log( 'BaseContainerNode.notifyDeactivatedChild: unknown child notifier!' );
        return false;
    }

    var nChildrenCount = this.aChildrenArray.length;

    assert( ( this.nFinishedChildren < nChildrenCount ),
    'BaseContainerNode.notifyDeactivatedChild: assert(this.nFinishedChildren < nChildrenCount) failed' );

    ++this.nFinishedChildren;
    var bFinished = ( this.nFinishedChildren >= nChildrenCount );

    if( bFinished && this.isDurationIndefinite() )
    {
        this.deactivate();
    }

    return bFinished;
};

BaseContainerNode.prototype.forEachChildNode = function( aFunction, eNodeStateMask )
{
    if( !eNodeStateMask )
        eNodeStateMask = -1;

    var nChildrenCount = this.aChildrenArray.length;
    for( var i = 0; i < nChildrenCount; ++i )
    {
        if( ( eNodeStateMask != -1 ) && ( ( this.aChildrenArray[i].getState() & eNodeStateMask ) == 0 ) )
            continue;
        aFunction( this.aChildrenArray[i] );
    }
};

BaseContainerNode.prototype.dispose = function()
{
    var nChildrenCount = this.aChildrenArray.length;
    for( var i = 0; i < nChildrenCount; ++i )
    {
        this.aChildrenArray[i].dispose();
    }

    BaseContainerNode.superclass.dispose.call( this );
};

BaseContainerNode.prototype.getImpressNodeType = function()
{
    return this.eImpressNodeType;
};

BaseContainerNode.prototype.info = function( bVerbose )
{
    var sInfo = BaseContainerNode.superclass.info.call( this, bVerbose );

    if( bVerbose )
    {
        // impress node type
        if( this.getImpressNodeType() )
            sInfo += ';  node-type: ' + aImpressNodeTypeOutMap[ this.getImpressNodeType() ];
    }

    var nChildrenCount = this.aChildrenArray.length;
    for( var i = 0; i < nChildrenCount; ++i )
    {
        sInfo += '\n';
        sInfo += this.aChildrenArray[i].info( bVerbose );
    }

    return sInfo;
};

// ------------------------------------------------------------------------------------------ //
function ParallelTimeContainer( aAnimElem, aParentNode, aNodeContext )
{
    ParallelTimeContainer.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );

    this.sClassName = 'ParallelTimeContainer';
}
extend( ParallelTimeContainer, BaseContainerNode );


ParallelTimeContainer.prototype.activate_st = function()
{
    var nChildrenCount = this.aChildrenArray.length;
    var nResolvedChildren = 0;
    for( var i = 0; i < nChildrenCount; ++i )
    {
        if( this.aChildrenArray[i].resolve() )
        {
            ++nResolvedChildren;
        }
    }

    if( nChildrenCount != nResolvedChildren )
    {
        log( 'ParallelTimeContainer.activate_st: resolving all children failed' );
        return;
    }


    if( this.isDurationIndefinite() && ( nChildrenCount == 0  ) )
    {
        this.scheduleDeactivationEvent( this.makeDeactivationEvent( 0.0 ) );
    }
    else
    {
        this.scheduleDeactivationEvent();
    }
};

ParallelTimeContainer.prototype.notifyDeactivating = function( aAnimationNode )
{
    this.notifyDeactivatedChild( aAnimationNode );
};



// ------------------------------------------------------------------------------------------ //
function SequentialTimeContainer( aAnimElem, aParentNode, aNodeContext )
{
    SequentialTimeContainer.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );

    this.sClassName = 'SequentialTimeContainer';
}
extend( SequentialTimeContainer, BaseContainerNode );


SequentialTimeContainer.prototype.activate_st = function()
{
    var nChildrenCount = this.aChildrenArray.length;
    for( ; this.nFinishedChildren < nChildrenCount; ++this.nFinishedChildren )
    {
        if( this.resolveChild( this.aChildrenArray[ this.nFinishedChildren ] ) )
            break;
        else
            log( 'SequentialTimeContainer.activate_st: resolving child failed!' );
    }

    if( this.isDurationIndefinite() && ( ( nChildrenCount == 0 ) || ( this.nFinishedChildren >= nChildrenCount ) ) )
    {
        // deactivate ASAP:
        this.scheduleDeactivationEvent( this.makeDeactivationEvent( 0.0 ) );
    }
    else
    {
        this.scheduleDeactivationEvent();
    }
};

SequentialTimeContainer.prototype.notifyDeactivating = function( aNotifier )
{
    if( this.notifyDeactivatedChild( aNotifier ) )
        return;

    assert( this.nFinishedChildren < this.aChildrenArray.length,
    'SequentialTimeContainer.notifyDeactivating: assertion (this.nFinishedChildren < this.aChildrenArray.length) failed' );

    var aNextChild = this.aChildrenArray[ this.nFinishedChildren ];

    assert( aNextChild.getState() == UNRESOLVED_NODE,
    'SequentialTimeContainer.notifyDeactivating: assertion (aNextChild.getState == UNRESOLVED_NODE) failed' );

    if( !this.resolveChild( aNextChild ) )
    {
        // could not resolve child - since we risk to
        // stall the chain of events here, play it safe
        // and deactivate this node (only if we have
        // indefinite duration - otherwise, we'll get a
        // deactivation event, anyways).
        this.deactivate();
    }
};

SequentialTimeContainer.prototype.skipEffect = function( aChildNode )
{
    // not implemented
};

SequentialTimeContainer.prototype.rewindEffect = function( aChildNode )
{
    // not implemented
};

SequentialTimeContainer.prototype.resolveChild = function( aChildNode )
{
    var bResolved = aChildNode.resolve();

    if( bResolved && this.isMainSequenceRootNode() )
    {
        // skip/rewind events handling
    }
    return bResolved;
};



// ------------------------------------------------------------------------------------------ //
function PropertyAnimationNode(  aAnimElem, aParentNode, aNodeContext )
{
    PropertyAnimationNode.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );

    this.sClassName = 'PropertyAnimationNode';
}
extend( PropertyAnimationNode, AnimationBaseNode3 );


PropertyAnimationNode.prototype.createActivity = function()
{

    /*
    var aActivityParamSet = this.fillActivityParams();
    var aAnimation = createPropertyAnimation( 'opacity',
                                              this.getAnimatedElement(),
                                              this.aNodeContext.aSlideWidth,
                                              this.aNodeContext.aSlideHeight );

    return new SimpleActivity( aActivityParamSet, aAnimation, FORWARD );
    */


    /*
        if( true && this.getAttributeName() === 'x' )
        {
            var sAttributeName = 'x';

            this.aDuration = new Duration( '2s' );
            this.sAttributeName = sAttributeName;
            this.aKeyTimes = [ 0.0, 0.25, 0.50, 0.75, 1.0 ];
            //this.aKeyTimes = [ 0.0, 1.0 ];
            var aM = 5000 / this.aNodeContext.aSlideWidth;
            this.aValues = [ 'x', 'x - ' + aM, 'x', 'x + ' + aM, 'x' ];
            //this.aValues = [ '0', 'width' ];

            //this.aFromValue = '';
            //this.aToValue = '0 + ' + aTranslationValue;
            //this.aByValue = aTranslationValue;
            //this.nRepeatCount = 3;

            var aActivityParamSet = this.fillActivityParams();

            var aAnimation = createPropertyAnimation( this.getAttributeName(),
                                                      this.getAnimatedElement(),
                                                      this.aNodeContext.aSlideWidth,
                                                      this.aNodeContext.aSlideHeight );

            var aInterpolator = null;
            return createActivity( aActivityParamSet, this, aAnimation, aInterpolator );
        }

        if( true && this.getAttributeName() === 'y' )
        {
            var sAttributeName = 'height';
            this.aDuration = new Duration( '2s' );
            this.sAttributeName = sAttributeName;
            this.aKeyTimes = [ 0.0, 0.25, 0.50, 0.75, 1.0 ];
            //this.aKeyTimes = [ 0.0, 1.0 ];
            var aM = 5000 / this.aNodeContext.aSlideHeight;
            this.aValues = new Array();
            //this.aValues = [ 'y', 'y', 'y - ' + aM, 'y - ' + aM, 'y' ];
            this.aValues = [ 'height', '0', 'height', '2*height', 'height' ];
            //this.aValues = [ '0', 'height' ];

            //this.aFromValue = '2 * height';
            //this.aToValue = 'width';
            //this.aByValue = 'width';//aTranslationValue;


            var aActivityParamSet = this.fillActivityParams();

            var aAnimation = createPropertyAnimation( this.getAttributeName(),
                                                      this.getAnimatedElement(),
                                                      this.aNodeContext.aSlideWidth,
                                                      this.aNodeContext.aSlideHeight );

            var aInterpolator = null;
            return createActivity( aActivityParamSet, this, aAnimation, aInterpolator );
        }
        */



    var aActivityParamSet = this.fillActivityParams();

    var aAnimation = createPropertyAnimation( this.getAttributeName(),
                                              this.getAnimatedElement(),
                                              this.aNodeContext.aSlideWidth,
                                              this.aNodeContext.aSlideHeight );

    var aInterpolator = null;  // createActivity will compute it;
    return createActivity( aActivityParamSet, this, aAnimation, aInterpolator );

};



// ------------------------------------------------------------------------------------------ //
function AnimationSetNode(  aAnimElem, aParentNode, aNodeContext )
{
    AnimationSetNode.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );

    this.sClassName = 'AnimationSetNode';
}
extend( AnimationSetNode, AnimationBaseNode2 );


AnimationSetNode.prototype.createActivity = function()
{
    var aAnimation = createPropertyAnimation( this.getAttributeName(),
                                              this.getAnimatedElement(),
                                              this.aNodeContext.aSlideWidth,
                                              this.aNodeContext.aSlideHeight );

    var aActivityParamSet = this.fillActivityParams();

    return new SetActivity( aActivityParamSet, aAnimation, this.getToValue() );
};



// ------------------------------------------------------------------------------------------ //
function AnimationColorNode(  aAnimElem, aParentNode, aNodeContext )
{
    AnimationColorNode.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );

    this.sClassName = 'AnimationColorNode';

    this.eColorInterpolation;
    this.eColorInterpolationDirection;
}
extend( AnimationColorNode, AnimationBaseNode3 );


AnimationColorNode.prototype.parseElement = function()
{
    var bRet = AnimationColorNode.superclass.parseElement.call( this );

    var aAnimElem = this.aElement;

    // color-interpolation attribute
    this.eColorInterpolation = COLOR_SPACE_RGB;
    var sColorInterpolationAttr = aAnimElem.getAttribute( 'color-interpolation' );
    if( sColorInterpolationAttr && aColorSpaceInMap[ sColorInterpolationAttr ] )
        this.eColorInterpolation = aColorSpaceInMap[ sColorInterpolationAttr ];

    // color-interpolation-direction attribute
    this.eColorInterpolationDirection = CLOCKWISE;
    var sColorInterpolationDirectionAttr = aAnimElem.getAttribute( 'color-interpolation-direction' );
    if( sColorInterpolationDirectionAttr && aClockDirectionInMap[ sColorInterpolationDirectionAttr ] )
        this.eColorInterpolationDirection = aClockDirectionInMap[ sColorInterpolationDirectionAttr ];

    return bRet;
};

AnimationColorNode.prototype.createActivity = function()
{
    /*
    var aActivityParamSet = this.fillActivityParams();

    var aAnimation = createPropertyAnimation( 'opacity',
            this.getAnimatedElement(),
            this.aNodeContext.aSlideWidth,
            this.aNodeContext.aSlideHeight );

    return new SimpleActivity( aActivityParamSet, aAnimation, FORWARD );
    */

    /*
    if( false && this.getAttributeName() === 'fill-color' )
    {
        var sAttributeName = 'stroke-color';

        this.aDuration = new Duration( '2s' );
        this.nAccelerate = 0.0;
        this.nDecelerate = 0.0;
        this.eColorInterpolation = COLOR_SPACE_RGB;
        this.eColorInterpolationDirection = COUNTERCLOCKWISE;

        this.sAttributeName = sAttributeName;

        this.aFromValue = 'rgb( 0%, 0%, 0% )';
        this.aToValue = 'rgb( 0%, 0%, 100% )';
        //this.aByValue = 'hsl( 0, -12%, -25% )';



        var aActivityParamSet = this.fillActivityParams();

        var aAnimation = createPropertyAnimation( this.getAttributeName(),
                                                  this.getAnimatedElement(),
                                                  this.aNodeContext.aSlideWidth,
                                                  this.aNodeContext.aSlideHeight );
        var aColorAnimation;
        var aInterpolator;
        if( this.getColorInterpolation() === COLOR_SPACE_HSL )
        {
            ANIMDBG.print( 'AnimationColorNode.createActivity: color space hsl'  );
            aColorAnimation = new HSLAnimationWrapper( aAnimation );
            var aInterpolatorMaker = aInterpolatorHandler.getInterpolator( this.getCalcMode(),
                                                                           COLOR_PROPERTY,
                                                                           COLOR_SPACE_HSL );
            aInterpolator = aInterpolatorMaker( this.getColorInterpolationDirection() );
        }
        else
        {
            ANIMDBG.print( 'AnimationColorNode.createActivity: color space rgb'  );
            aColorAnimation = aAnimation;
            aInterpolator = aInterpolatorHandler.getInterpolator( this.getCalcMode(),
                                                                  COLOR_PROPERTY,
                                                                  COLOR_SPACE_RGB );
        }

        return createActivity( aActivityParamSet, this, aColorAnimation, aInterpolator );
    }
     */


    var aActivityParamSet = this.fillActivityParams();

    var aAnimation = createPropertyAnimation( this.getAttributeName(),
                                              this.getAnimatedElement(),
                                              this.aNodeContext.aSlideWidth,
                                              this.aNodeContext.aSlideHeight );

    var aColorAnimation;
    var aInterpolator;
    if( this.getColorInterpolation() === COLOR_SPACE_HSL )
    {
        ANIMDBG.print( 'AnimationColorNode.createActivity: color space hsl'  );
        aColorAnimation = new HSLAnimationWrapper( aAnimation );
        var aInterpolatorMaker = aInterpolatorHandler.getInterpolator( this.getCalcMode(),
                                                                       COLOR_PROPERTY,
                                                                       COLOR_SPACE_HSL );
        aInterpolator = aInterpolatorMaker( this.getColorInterpolationDirection() );
    }
    else
    {
        ANIMDBG.print( 'AnimationColorNode.createActivity: color space rgb'  );
        aColorAnimation = aAnimation;
        aInterpolator = aInterpolatorHandler.getInterpolator( this.getCalcMode(),
                                                              COLOR_PROPERTY,
                                                              COLOR_SPACE_RGB );
    }

    return createActivity( aActivityParamSet, this, aColorAnimation, aInterpolator );


};

AnimationColorNode.prototype.getColorInterpolation = function()
{
    return this.eColorInterpolation;
};

AnimationColorNode.prototype.getColorInterpolationDirection = function()
{
    return this.eColorInterpolationDirection;
};

AnimationColorNode.prototype.info = function( bVerbose )
{
    var sInfo = AnimationColorNode.superclass.info.call( this, bVerbose );

    if( bVerbose )
    {
        // color interpolation
        sInfo += ';  color-interpolation: ' + aColorSpaceOutMap[ this.getColorInterpolation() ];

        // color interpolation direction
        sInfo += ';  color-interpolation-direction: ' + aClockDirectionOutMap[ this.getColorInterpolationDirection() ];
    }
    return sInfo;
};



// ------------------------------------------------------------------------------------------ //
function AnimationTransitionFilterNode(  aAnimElem, aParentNode, aNodeContext )
{
    AnimationTransitionFilterNode.superclass.constructor.call( this, aAnimElem, aParentNode, aNodeContext );

    this.sClassName = 'AnimationTransitionFilterNode';

    this.eTransitionType;
    this.eTransitionSubType;
    this.bReverseDirection;
    this.eTransitionMode;
}
extend( AnimationTransitionFilterNode, AnimationBaseNode );


AnimationTransitionFilterNode.prototype.createActivity = function()
{
    var aActivityParamSet = this.fillActivityParams();

    var aAnimation = createPropertyAnimation( 'opacity',
            this.getAnimatedElement(),
            this.aNodeContext.aSlideWidth,
            this.aNodeContext.aSlideHeight );

    var eDirection = this.getTransitionMode() ? FORWARD : BACKWARD;

    return new SimpleActivity( aActivityParamSet, aAnimation, eDirection );

};

AnimationTransitionFilterNode.prototype.parseElement = function()
{
    var bRet = AnimationTransitionFilterNode.superclass.parseElement.call( this );

    var aAnimElem = this.aElement;

    // type attribute
    this.eTransitionType = undefined;
    var sTypeAttr = aAnimElem.getAttribute( 'type' );
    if( sTypeAttr && aTransitionTypeInMap[ sTypeAttr ] )
    {
        this.eTransitionType = aTransitionTypeInMap[ sTypeAttr ];
    }
    else
    {
        this.eCurrentState = INVALID_NODE;
        log( 'AnimationTransitionFilterNode.parseElement: transition type not valid: ' + sTypeAttr );
    }

    // subtype attribute
    this.eTransitionSubType = undefined;
    var sSubTypeAttr = aAnimElem.getAttribute( 'subtype' );
    if( sSubTypeAttr && aTransitionSubtypeInMap[ sSubTypeAttr ] )
    {
        this.eTransitionSubType = aTransitionSubtypeInMap[ sSubTypeAttr ];
    }
    else
    {
        this.eCurrentState = INVALID_NODE;
        log( 'AnimationTransitionFilterNode.parseElement: transition subtype not valid: ' + sSubTypeAttr );
    }

    // direction attribute
    this.bReverseDirection = false;
    var sDirectionAttr = aAnimElem.getAttribute( 'direction' );
    if( sDirectionAttr == 'reverse' )
        this.bReverseDirection = true;

    // mode attribute:
    this.eTransitionMode = TRANSITION_MODE_IN;
    var sModeAttr = aAnimElem.getAttribute( 'mode' );
    if( sModeAttr === 'out' )
        this.eTransitionMode = TRANSITION_MODE_OUT;

    return bRet;
};

AnimationTransitionFilterNode.prototype.getTransitionType = function()
{
    return this.eTransitionType;
};

AnimationTransitionFilterNode.prototype.getTransitionSubType = function()
{
    return this.eTransitionSubType;
};

AnimationTransitionFilterNode.prototype.getTransitionMode = function()
{
    return this.eTransitionMode;
};

AnimationTransitionFilterNode.prototype.getReverseDirection = function()
{
    return this.bReverseDirection;
};

AnimationTransitionFilterNode.prototype.info = function( bVerbose )
{
    var sInfo = AnimationTransitionFilterNode.superclass.info.call( this, bVerbose );

    if( bVerbose )
    {
        // transition type
        sInfo += ';  type: ' + aTransitionTypeOutMap[ String( this.getTransitionType() ) ];

        // transition subtype
        sInfo += ';  subtype: ' + aTransitionSubtypeOutMap[ this.getTransitionSubType() ];

        // transition direction
        if( this.getReverseDirection() )
            sInfo += ';  direction: reverse';
    }

    return sInfo;
};



/**********************************************************************************************
 *      Animation Node Factory
 **********************************************************************************************/

// ------------------------------------------------------------------------------------------ //
function createAnimationTree( aRootElement, aNodeContext )
{
    return createAnimationNode( aRootElement, null, aNodeContext );
}



// ------------------------------------------------------------------------------------------ //
function createAnimationNode( aElement, aParentNode, aNodeContext )
{
    assert( aElement, 'createAnimationNode: invalid animation element' );

    var eAnimationNodeType = getAnimationElementType( aElement );

    var aCreatedNode = null;
    var aCreatedContainer = null;

    switch( eAnimationNodeType )
    {
    case ANIMATION_NODE_PAR:
        aCreatedNode = aCreatedContainer =
            new ParallelTimeContainer( aElement, aParentNode, aNodeContext );
        break;
    case ANIMATION_NODE_ITERATE:
        // map iterate container to ParallelTimeContainer.
        // the iterating functionality is to be found
        // below, (see method implCreateIteratedNodes)
        aCreatedNode = aCreatedContainer =
            new ParallelTimeContainer( aElement, aParentNode, aNodeContext );
        break;
    case ANIMATION_NODE_SEQ:
        aCreatedNode = aCreatedContainer =
            new SequentialTimeContainer( aElement, aParentNode, aNodeContext );
        break;
    case ANIMATION_NODE_ANIMATE:
        aCreatedNode = new PropertyAnimationNode( aElement, aParentNode, aNodeContext );
        break;
    case ANIMATION_NODE_SET:
        aCreatedNode = new AnimationSetNode( aElement, aParentNode, aNodeContext );
        break;
    case ANIMATION_NODE_ANIMATEMOTION:
        //aCreatedNode = new AnimationPathMotionNode( aElement, aParentNode, aNodeContext );
        //break;
        log( 'createAnimationNode: ANIMATEMOTION not implemented' );
        return null;
    case ANIMATION_NODE_ANIMATECOLOR:
        aCreatedNode = new AnimationColorNode( aElement, aParentNode, aNodeContext );
        break;
    case ANIMATION_NODE_ANIMATETRANSFORM:
        //aCreatedNode = new AnimationTransformNode( aElement, aParentNode, aNodeContext );
        //break;
        log( 'createAnimationNode: ANIMATETRANSFORM not implemented' );
        return null;
    case ANIMATION_NODE_TRANSITIONFILTER:
        aCreatedNode = new AnimationTransitionFilterNode( aElement, aParentNode, aNodeContext );
        break;
    default:
        log( 'createAnimationNode: invalid Animation Node Type: ' + eAnimationNodeType );
        return null;
    }

    if( aCreatedContainer )
    {
        if( eAnimationNodeType == ANIMATION_NODE_ITERATE )
        {
            createIteratedNodes( aElement, aCreatedContainer, aNodeContext );
        }
        else
        {
            var aChildrenArray = getElementChildren( aElement );
            for( var i = 0; i < aChildrenArray.length; ++i )
            {
                if( !createChildNode( aChildrenArray[i], aCreatedContainer, aNodeContext ) )
                {
                    return null;
                }
            }
        }
    }

    return aCreatedNode;
}



// ------------------------------------------------------------------------------------------ //
function createChildNode( aElement, aParentNode, aNodeContext )
{
    var aChildNode = createAnimationNode( aElement, aParentNode, aNodeContext );

    if( !aChildNode )
    {
        log( 'createChildNode: child node creation failed' );
        return false;
    }
    else
    {
        aParentNode.appendChildNode( aChildNode );
        return true;
    }
}



// ------------------------------------------------------------------------------------------ //
function createIteratedNodes( aElement, aContainerNode, aNodeContext )
{
    // not implemented
}



/**********************************************************************************************
 *      Animation Factory
 **********************************************************************************************/


// ------------------------------------------------------------------------------------------ //
function makeScaler( nScale )
{
    if( ( typeof( nScale ) !== typeof( 0 ) ) || !isFinite( nScale ) )
    {
        log( 'makeScaler: not valid param passed: ' + nScale );
        return null;
    }

    return  function( nValue )
            {
                return ( nScale * nValue );
            };
}



// ------------------------------------------------------------------------------------------ //
function createPropertyAnimation( sAttrName, aAnimatedElement, nWidth, nHeight )
{
    if( !aAttributeMap[ sAttrName ] )
    {
        log( 'createPropertyAnimation: attribute is unknown' );
        return;
    }


    var aFunctorSet = aAttributeMap[ sAttrName ];

    var sGetValueMethod =   aFunctorSet.get;
    var sSetValueMethod =   aFunctorSet.set;

    if( !sGetValueMethod || !sSetValueMethod  )
    {
        log( 'createPropertyAnimation: attribute is not handled' );
        return;
    }

    var aGetModifier =  eval( aFunctorSet.getmod );
    var aSetModifier =  eval( aFunctorSet.setmod );


    return new GenericAnimation( bind( aAnimatedElement, aAnimatedElement[ sGetValueMethod ] ),
                                 bind( aAnimatedElement, aAnimatedElement[ sSetValueMethod ] ),
                                 aGetModifier,
                                 aSetModifier);
}



// ------------------------------------------------------------------------------------------ //
function GenericAnimation( aGetValueFunc, aSetValueFunc, aGetModifier, aSetModifier )
{
    assert( aGetValueFunc && aSetValueFunc,
            'GenericAnimation constructor: get value functor and/or set value functor are not valid' );

    this.aGetValueFunc = aGetValueFunc;
    this.aSetValueFunc = aSetValueFunc;
    this.aGetModifier = aGetModifier;
    this.aSetModifier = aSetModifier;
    this.aAnimatableElement = null;
    this.bAnimationStarted = false;
}


GenericAnimation.prototype.start = function( aAnimatableElement )
{
    assert( aAnimatableElement, 'GenericAnimation.start: animatable element is not valid' );

    this.aAnimatableElement = aAnimatableElement;
    this.aAnimatableElement.notifyAnimationStart();

    if( !this.bAnimationStarted )
        this.bAnimationStarted = true;
};

GenericAnimation.prototype.end = function()
{
    if( this.bAnimationStarted )
        this.bAnimationStarted = false;
};

GenericAnimation.prototype.perform = function( aValue )
{
    if( this.aSetModifier )
        aValue = this.aSetModifier( aValue );

    this.aSetValueFunc( aValue );
};

GenericAnimation.prototype.getUnderlyingValue = function()
{
    var aValue = this.aGetValueFunc();
    if( this.aGetModifier )
        aValue = this.aGetModifier( aValue );
    return aValue;
};



// ------------------------------------------------------------------------------------------ //
function HSLAnimationWrapper( aColorAnimation )
{
    assert( aColorAnimation,
            'HSLAnimationWrapper constructor: invalid color animation delegate' );

    this.aAnimation = aColorAnimation;
}


HSLAnimationWrapper.prototype.start = function( aAnimatableElement )
{
    this.aAnimation.start( aAnimatableElement );
};

HSLAnimationWrapper.prototype.end = function()
{
    this.aAnimation.end();
};
HSLAnimationWrapper.prototype.perform = function( aHSLValue )
{
    this.aAnimation.perform( aHSLValue.convertToRGB() );
};

HSLAnimationWrapper.prototype.getUnderlyingValue = function()
{
    return this.aAnimation.getUnderlyingValue().convertToHSL();
};



// ------------------------------------------------------------------------------------------ //
function AnimatedElement( aElement )
{
    if( !aElement )
    {
        log( 'AnimatedElement constructor: element is not valid' );
    }

    this.aActiveElement = aElement;
    this.initElement();

    this.aBaseBBox = this.aActiveElement.getBBox();
    this.nBaseCenterX = this.aBaseBBox.x + this.aBaseBBox.width / 2;
    this.nBaseCenterY = this.aBaseBBox.y + this.aBaseBBox.height / 2;
    this.nCenterX = this.nBaseCenterX;
    this.nCenterY = this.nBaseCenterY;
    this.nScaleFactorX = 1.0;
    this.nScaleFactorY = 1.0;

    this.aPreviousElement = null;
    this.aElementArray = new Array();
    this.nCurrentState = 0;
    this.eAdditiveMode = ADDITIVE_MODE_REPLACE;
    this.bIsUpdated = true;

    this.aTMatrix = document.documentElement.createSVGMatrix();
    this.aCTM = document.documentElement.createSVGMatrix();
    this.aICTM = document.documentElement.createSVGMatrix();
    this.setCTM();

    this.aElementArray[0] = this.aActiveElement.cloneNode( true );
}

AnimatedElement.prototype.initElement = function()
{
    // add a transform attribute of type matrix
    this.aActiveElement.setAttribute( 'transform', makeMatrixString( 1, 0, 0, 1, 0, 0 ) );
};

AnimatedElement.prototype.getId = function()
{
    return this.aActiveElement.getAttribute( 'id' );
};

AnimatedElement.prototype.isUpdated = function()
{
    return this.bIsUpdated;
};

AnimatedElement.prototype.getAdditiveMode = function()
{
    return this.eAdditiveMode;
};

AnimatedElement.prototype.setAdditiveMode = function( eAdditiveMode )
{
    this.eAdditiveMode = eAdditiveMode;
};

AnimatedElement.prototype.setToElement = function( aElement )
{
    if( !aElement )
    {
        log( 'AnimatedElement(' + this.getId() + ').setToElement: element is not valid' );
        return false;
    }

    var aClone = aElement.cloneNode( true );
    this.aPreviousElement = this.aActiveElement.parentNode.replaceChild( aClone, this.aActiveElement );
    this.aActiveElement = aClone;

    return true;
};

AnimatedElement.prototype.notifySlideStart = function()
{
    this.setToFirst();
    this.DBG( '.notifySlideStart invoked' );
};

AnimatedElement.prototype.notifyAnimationStart = function()
{

    this.DBG( '.notifyAnimationStart invoked' );
    this.bIsUpdated = false;
};

AnimatedElement.prototype.notifyAnimationEnd = function()
{
    // empty body
};

AnimatedElement.prototype.notifyNextEffectStart = function( nEffectIndex )
{
    assert( this.nCurrentState === nEffectIndex,
            'AnimatedElement(' + this.getId() + ').notifyNextEffectStart: assertion (current state == effect index) failed' );

    if( this.isUpdated() )
    {
        if( !this.aElementArray[ nEffectIndex ] )
        {
            this.aElementArray[ nEffectIndex ] =  this.aElementArray[ this.nCurrentState ];
            this.DBG( '.notifyNextEffectStart(' + nEffectIndex + '): new state set to previous one ' );
        }
    }
    else
    {
        if( !this.aElementArray[ nEffectIndex ] )
        {
            this.aElementArray[ nEffectIndex ] = this.aActiveElement.cloneNode( true );
            this.DBG( '.notifyNextEffectStart(' + nEffectIndex + '): cloned active state ' );
            }
        }
        ++this.nCurrentState;
    };

    AnimatedElement.prototype.setToFirst = function()
    {
        this.setTo( 0 );
    };

    AnimatedElement.prototype.setToLast = function()
    {
        this.setTo( this.aElementArray.length - 1 );
    };

    AnimatedElement.prototype.setTo = function( nEffectIndex )
    {
        var bRet = this.setToElement( this.aElementArray[ nEffectIndex ] );
        if( bRet )
        {
            this.nCurrentState = nEffectIndex;

            var aBBox = this.getBBox();
            var aBaseBBox = this.getBaseBBox();
            this.nCenterX = aBBox.x + aBBox.width / 2;
            this.nCenterY = aBBox.y + aBBox.height / 2;
            this.nScaleFactorX = aBBox.width / aBaseBBox.width;
            this.nScaleFactorY = aBBox.height / aBaseBBox.height;
        }
    };

    AnimatedElement.prototype.getBaseBBox = function()
    {
        return this.aBaseBBox;
    };

    AnimatedElement.prototype.getBaseCenterX = function()
    {
        return this.nBaseCenterX;
    };

    AnimatedElement.prototype.getBaseCenterY = function()
    {
        return this.nBaseCenterY;
    };

    AnimatedElement.prototype.getBBox = function()
    {
        return this.aActiveElement.parentNode.getBBox();
    };

    AnimatedElement.prototype.getX = function()
    {
        return this.nCenterX;
    };

    AnimatedElement.prototype.getY = function()
    {
        return this.nCenterY;
    };

    AnimatedElement.prototype.getWidth = function()
    {
        return this.nScaleFactorX * this.getBaseBBox().width;
    };

    AnimatedElement.prototype.getHeight = function()
    {
        return this.nScaleFactorY * this.getBaseBBox().height;
    };

    AnimatedElement.prototype.setCTM = function()
    {

        this.aICTM.e = this.getBaseCenterX();
        this.aICTM.f = this.getBaseCenterY();

        this.aCTM.e = -this.aICTM.e;
        this.aCTM.f = -this.aICTM.f;
    };

    AnimatedElement.prototype.updateTransformAttribute = function()
    {
        this.aTransformAttrList = this.aActiveElement.transform.baseVal;
        this.aTransformAttr = this.aTransformAttrList.getItem( 0 );
        this.aTransformAttr.setMatrix( this.aTMatrix );
    };

    AnimatedElement.prototype.setX = function( nXNewPos )
    {
        this.aTransformAttrList = this.aActiveElement.transform.baseVal;
        this.aTransformAttr = this.aTransformAttrList.getItem( 0 );
        this.aTransformAttr.matrix.e += ( nXNewPos - this.getX() );
        this.nCenterX = nXNewPos;
    };

    AnimatedElement.prototype.setY = function( nYNewPos )
    {
        this.aTransformAttrList = this.aActiveElement.transform.baseVal;
        this.aTransformAttr = this.aTransformAttrList.getItem( 0 );
        this.aTransformAttr.matrix.f += ( nYNewPos - this.getY() );
        this.nCenterY = nYNewPos;
    };

    AnimatedElement.prototype.setWidth = function( nNewWidth )
    {
        var nBaseWidth = this.getBaseBBox().width;
        if( nBaseWidth <= 0 )
            return;

        this.nScaleFactorX = nNewWidth / nBaseWidth;
        this.implScale();
    };

    AnimatedElement.prototype.setHeight = function( nNewHeight )
    {
        var nBaseHeight = this.getBaseBBox().height;
        if( nBaseHeight <= 0 )
            return;

        this.nScaleFactorY = nNewHeight / nBaseHeight;
        this.implScale();
    };

    AnimatedElement.prototype.implScale = function( )
    {
        this.aTMatrix = document.documentElement.createSVGMatrix();
        this.aTMatrix.a = this.nScaleFactorX;
        this.aTMatrix.d = this.nScaleFactorY;
        this.aTMatrix = this.aICTM.multiply( this.aTMatrix.multiply( this.aCTM ) );

        var nDeltaX = this.getX() - this.getBaseCenterX();
        var nDeltaY = this.getY() - this.getBaseCenterY();
        this.aTMatrix = this.aTMatrix.translate( nDeltaX, nDeltaY );
        this.updateTransformAttribute();
    };

    AnimatedElement.prototype.setWidth2 = function( nNewWidth )
    {
        if( nNewWidth < 0 )
            log( 'AnimatedElement(' + this.getId() + ').setWidth: negative width!' );
    if( nNewWidth < 0.001 )
        nNewWidth = 0.001;

    this.setCTM();

    var nCurWidth = this.getWidth();
    if( nCurWidth <= 0 )
        nCurWidth = 0.001;

    var nScaleFactor = nNewWidth / nCurWidth;
    if( nScaleFactor < 1e-5 )
        nScaleFactor = 1e-5;
    this.aTMatrix = document.documentElement.createSVGMatrix();
    this.aTMatrix.a = nScaleFactor;
    this.aTMatrix = this.aICTM.multiply( this.aTMatrix.multiply( this.aCTM ) );
    this.updateTransformAttribute();
};

AnimatedElement.prototype.setHeight2 = function( nNewHeight )
{
    ANIMDBG.print( 'AnimatedElement.setHeight: nNewHeight = ' + nNewHeight );
    if( nNewHeight < 0 )
        log( 'AnimatedElement(' + this.getId() + ').setWidth: negative height!' );
    if( nNewHeight < 0.001 )
        nNewHeight = 0.001;

    this.setCTM();

    var nCurHeight = this.getHeight();
    ANIMDBG.print( 'AnimatedElement.setHeight: nCurHeight = ' + nCurHeight );
    if( nCurHeight <= 0 )
        nCurHeight = 0.001;

    var nScaleFactor = nNewHeight / nCurHeight;
    ANIMDBG.print( 'AnimatedElement.setHeight: nScaleFactor = ' + nScaleFactor );
    if( nScaleFactor < 1e-5 )
        nScaleFactor = 1e-5;
    this.aTMatrix = document.documentElement.createSVGMatrix();
    this.aTMatrix.d = nScaleFactor;
    this.aTMatrix = this.aICTM.multiply( this.aTMatrix.multiply( this.aCTM ) );
    this.updateTransformAttribute();
};

AnimatedElement.prototype.getOpacity = function()
{
    return this.aActiveElement.getAttribute( 'opacity' );
};

AnimatedElement.prototype.setOpacity = function( nValue )
{
    this.aActiveElement.setAttribute( 'opacity', nValue );
};

AnimatedElement.prototype.getVisibility = function()
{

    var sVisibilityValue = this.aActiveElement.getAttribute( 'visibility' );
    if( !sVisibilityValue || ( sVisibilityValue === 'inherit' ) )
        return 'visible'; // TODO: look for parent visibility!
    else
        return sVisibilityValue;
};

AnimatedElement.prototype.setVisibility = function( sValue )
{
    if( sValue == 'visible' )
        sValue = 'inherit';
    this.aActiveElement.setAttribute( 'visibility', sValue );
};

AnimatedElement.prototype.getStrokeStyle = function()
{
    // TODO: getStrokeStyle: implement it
    return 'solid';
};

AnimatedElement.prototype.setStrokeStyle = function( sValue )
{
    ANIMDBG.print( 'AnimatedElement.setStrokeStyle(' + sValue + ')' );
};

AnimatedElement.prototype.getFillStyle = function()
{
    // TODO: getFillStyle: implement it
    return 'solid';
};

AnimatedElement.prototype.setFillStyle = function( sValue )
{
    ANIMDBG.print( 'AnimatedElement.setFillStyle(' + sValue + ')' );
};

AnimatedElement.prototype.getFillColor = function()
{
    var aChildSet = getElementChildren( this.aActiveElement );
    var sFillColorValue = '';
    for( var i = 0; i <  aChildSet.length; ++i )
    {
        sFillColorValue = aChildSet[i].getAttribute( 'fill' );
        if( sFillColorValue && ( sFillColorValue !== 'none' ) )
            break;
    }

    return colorParser( sFillColorValue );
};

AnimatedElement.prototype.setFillColor = function( aRGBValue )
{
    assert( aRGBValue instanceof RGBColor,
            'AnimatedElement.setFillColor: value argument is not an instance of RGBColor' );

    var sValue = aRGBValue.toString( true /* clamped values */ );
    var aChildSet = getElementChildren( this.aActiveElement );

    var sFillColorValue = '';
    for( var i = 0; i <  aChildSet.length; ++i )
    {
        sFillColorValue = aChildSet[i].getAttribute( 'fill' );
        if( sFillColorValue && ( sFillColorValue !== 'none' ) )
        {
            aChildSet[i].setAttribute( 'fill', sValue );
        }
    }
};

AnimatedElement.prototype.getStrokeColor = function()
{
    var aChildSet = getElementChildren( this.aActiveElement );
    var sStrokeColorValue = '';
    for( var i = 0; i <  aChildSet.length; ++i )
    {
        sStrokeColorValue = aChildSet[i].getAttribute( 'stroke' );
        if( sStrokeColorValue && ( sStrokeColorValue !== 'none' ) )
            break;
    }

    return colorParser( sStrokeColorValue );
};

AnimatedElement.prototype.setStrokeColor = function( aRGBValue )
{
    assert( aRGBValue instanceof RGBColor,
            'AnimatedElement.setFillColor: value argument is not an instance of RGBColor' );

    var sValue = aRGBValue.toString( true /* clamped values */ );
    var aChildSet = getElementChildren( this.aActiveElement );

    var sStrokeColorValue = '';
    for( var i = 0; i <  aChildSet.length; ++i )
    {
        sStrokeColorValue = aChildSet[i].getAttribute( 'stroke' );
        if( sStrokeColorValue && ( sStrokeColorValue !== 'none' ) )
        {
            aChildSet[i].setAttribute( 'stroke', sValue );
        }
    }
};

AnimatedElement.prototype.getFontColor = function()
{
    // TODO: getFontColor implement it
    return new RGBColor( 0, 0, 0 );
};

AnimatedElement.prototype.setFontColor = function( sValue )
{
    ANIMDBG.print( 'AnimatedElement.setFontColor(' + sValue + ')' );
};

AnimatedElement.prototype.DBG = function( sMessage, nTime )
{
    aAnimatedElementDebugPrinter.print( 'AnimatedElement(' + this.getId() + ')' + sMessage, nTime );
};



// ------------------------------------------------------------------------------------------ //
// SlideAnimations

function SlideAnimations( aSlideShowContext )
{
    this.aContext = new NodeContext( aSlideShowContext );
    this.aAnimationNodeMap = new Object();
    this.aAnimatedElementMap = new Object();
    this.aSourceEventElementMap = new Object();
    this.aNextEffectEventArray = new NextEffectEventArray();
    this.aEventMultiplexer = new EventMultiplexer( aSlideShowContext.aTimerEventQueue );
    this.aRootNode = null;
    this.bElementsParsed = false;

    this.aContext.aAnimationNodeMap = this.aAnimationNodeMap;
    this.aContext.aAnimatedElementMap = this.aAnimatedElementMap;
    this.aContext.aSourceEventElementMap = this.aSourceEventElementMap;
}


SlideAnimations.prototype.importAnimations = function( aAnimationRootElement )
{
    if( !aAnimationRootElement )
        return false;

    this.aRootNode = createAnimationTree( aAnimationRootElement, this.aContext );

    return ( this.aRootNode ? true : false );
};

SlideAnimations.prototype.parseElements = function()
{
    if( !this.aRootNode )
        return false;

    // parse all nodes
    if( !this.aRootNode.parseElement() )
        return false;
    else
        this.bElementsParsed = true;
};

SlideAnimations.prototype.elementsParsed = function()
{
    return this.bElementsParsed;
};

SlideAnimations.prototype.isFirstRun = function()
{
    return this.aContext.bFirstRun;
};

SlideAnimations.prototype.isAnimated = function()
{
    if( !this.bElementsParsed )
        return false;

    return this.aRootNode.hasPendingAnimation();
};

SlideAnimations.prototype.start = function()
{
    if( !this.bElementsParsed )
        return false;

    aSlideShow.setSlideEvents( this.aNextEffectEventArray, this.aEventMultiplexer );

    if( this.aContext.bFirstRun == undefined )
        this.aContext.bFirstRun = true;
    else if( this.aContext.bFirstRun )
        this.aContext.bFirstRun = false;

    // init all nodes
    if( !this.aRootNode.init() )
        return false;

    // resolve root node
    if( !this.aRootNode.resolve() )
        return false;

    return true;
};

SlideAnimations.prototype.end = function( bLeftEffectsSkipped )
{
    if( !this.bElementsParsed )
        return; // no animations there

    // end root node
    this.aRootNode.deactivate();
    this.aRootNode.end();

    if( bLeftEffectsSkipped && this.isFirstRun() )
    {
        // in case this is the first run and left events have been skipped
        // some next effect events for the slide could not be collected
        // so the next time we should behave as it was the first run again
        this.aContext.bFirstRun = undefined;
    }
    else if( this.isFirstRun() )
    {
        this.aContext.bFirstRun = false;
    }

};

SlideAnimations.prototype.dispose = function()
{
    if( this.aRootNode )
    {
        this.aRootNode.dispose();
    }
};

SlideAnimations.prototype.clearNextEffectEvents = function()
{
    ANIMDBG.print( 'SlideAnimations.clearNextEffectEvents: current slide: ' + nCurSlide );
    this.aNextEffectEventArray.clear();
    this.aContext.bFirstRun = undefined;
};



/**********************************************************************************************
 *      Event classes and helper functions
 **********************************************************************************************/

// ------------------------------------------------------------------------------------------ //
function Event()
{
    this.nId = Event.getUniqueId();
}


Event.CURR_UNIQUE_ID = 0;

Event.getUniqueId = function()
{
    ++Event.CURR_UNIQUE_ID;
    return Event.CURR_UNIQUE_ID;
};

Event.prototype.getId = function()
{
    return this.nId;
};


// ------------------------------------------------------------------------------------------ //
function DelayEvent( aFunctor, nTimeout )
{
    DelayEvent.superclass.constructor.call( this );

    this.aFunctor = aFunctor;
    this.nTimeout = nTimeout;
    this.bWasFired = false;
}
extend( DelayEvent, Event );


DelayEvent.prototype.fire = function()
{
    assert( this.isCharged(), 'DelayEvent.fire: assertion isCharged failed' );

    this.bWasFired = true;
    this.aFunctor();
    return true;
};

DelayEvent.prototype.isCharged = function()
{
    return !this.bWasFired;
};

DelayEvent.prototype.getActivationTime = function( nCurrentTime )
{
    return ( this.nTimeout + nCurrentTime );
};

DelayEvent.prototype.dispose = function()
{
    // don't clear unconditionally, because it may currently be executed:
    if( this.isCharged() )
        this.bWasFired = true;
};

DelayEvent.prototype.charge = function()
{
    if( !this.isCharged() )
        this.bWasFired = false;
};


// ------------------------------------------------------------------------------------------ //
function makeEvent( aFunctor )
{
    return new DelayEvent( aFunctor, 0.0 );
}



// ------------------------------------------------------------------------------------------ //
function makeDelay( aFunctor, nTimeout )
{
    return new DelayEvent( aFunctor, nTimeout );
}



// ------------------------------------------------------------------------------------------ //
function registerEvent( aTiming, aEvent, aNodeContext )
{
    var aSlideShowContext = aNodeContext.aContext;
    var eTimingType = aTiming.getType();

    registerEvent.DBG( aTiming );

    if( eTimingType == OFFSET_TIMING )
    {
        aSlideShowContext.aTimerEventQueue.addEvent( aEvent );
    }
    else if ( aNodeContext.bFirstRun )
    {
        var aEventMultiplexer = aSlideShowContext.aEventMultiplexer;
        if( !aEventMultiplexer )
        {
            log( 'registerEvent: event multiplexer not initialized' );
            return;
        }
        var aNextEffectEventArray = aSlideShowContext.aNextEffectEventArray;
        if( !aNextEffectEventArray )
        {
            log( 'registerEvent: next effect event array not initialized' );
            return;
        }
        switch( eTimingType )
        {
            case EVENT_TIMING:
                var eEventType = aTiming.getEventType();
                var sEventBaseElemId = aTiming.getEventBaseElementId();
                if( sEventBaseElemId )
                {
                    var aEventBaseElem = document.getElementById( sEventBaseElemId );
                    if( !aEventBaseElem )
                    {
                        log( 'generateEvent: EVENT_TIMING: event base element not found: ' + sEventBaseElemId );
                        return;
                    }
                    var aSourceEventElement = aNodeContext.makeSourceEventElement( sEventBaseElemId, aEventBaseElem );

                    var bEventRegistered = false;
                    switch( eEventType )
                    {
                        case EVENT_TRIGGER_ON_CLICK:
                            aEventMultiplexer.registerEvent( eEventType, aSourceEventElement.getId(), aEvent );
                            bEventRegistered = true;
                            break;
                        default:
                            log( 'generateEvent: not handled event type: ' + eEventType );
                    }
                    if( bEventRegistered )
                        aSourceEventElement.addEventListener( eEventType  );
                }
                else  // no base event element present
                {
                    switch( eEventType )
                    {
                        case EVENT_TRIGGER_ON_NEXT_EFFECT:
                            aNextEffectEventArray.appendEvent( aEvent );
                            break;
                        default:
                            log( 'generateEvent: not handled event type: ' + eEventType );
                    }
                }
                break;
            case SYNCBASE_TIMING:
                var eEventType = aTiming.getEventType();
                var sEventBaseElemId = aTiming.getEventBaseElementId();
                if( sEventBaseElemId )
                {
                    var aAnimationNode = aNodeContext.aAnimationNodeMap[ sEventBaseElemId ];
                    if( !aAnimationNode )
                    {
                        log( 'generateEvent: SYNCBASE_TIMING: event base element not found: ' + sEventBaseElemId );
                        return;
                    }
                    aEventMultiplexer.registerEvent( eEventType, aAnimationNode.getId(), aEvent );
                }
                else
                {
                    log( 'generateEvent: SYNCBASE_TIMING: event base element not specified' );
                }
                break;
            default:
                log( 'generateEvent: not handled timing type: ' + eTimingType );
        }
    }
}

registerEvent.DEBUG = aRegisterEventDebugPrinter.isEnabled();

registerEvent.DBG = function( aTiming, nTime )
{
    if( registerEvent.DEBUG )
    {
        aRegisterEventDebugPrinter.print( 'registerEvent( timing: ' + aTiming.info() + ' )', nTime );
    }
};



// ------------------------------------------------------------------------------------------ //
function SourceEventElement( aElement, aEventMulyiplexer )
{
    this.nId = getUniqueId();
    this.aElement = aElement;
    this.aEventMultiplexer = aEventMulyiplexer;
    this.aEventListenerStateArray = new Array();
}


SourceEventElement.prototype.getId = function()
{
    return this.nId;
};

SourceEventElement.prototype.isEqualTo = function( aSourceEventElement )
{
    return ( this.getId() == aSourceEventElement.getId() );
};

SourceEventElement.prototype.onClick = function()
{
    aEventMulyiplexer.notifyClickEvent( this );
};

SourceEventElement.prototype.isEventListenerRegistered = function( eEventType )
{
    return this.aEventListenerStateArray[ eEventType ];
};

SourceEventElement.prototype.addEventListener = function( eEventType )
{
    if( !this.aElement )
        return false;

    this.aEventListenerStateArray[ eEventType ] = true;
    switch( eEventType )
    {
        case EVENT_TRIGGER_ON_CLICK:
            this.aElement.addEventListener( 'click', this.onClick, false );
            break;
        default:
            log( 'SourceEventElement.addEventListener: not handled event type: ' + eEventType );
            return false;
    }
    return true;
};

SourceEventElement.prototype.removeEventListener = function( eEventType )
{
    if( !this.aElement )
        return false;

    this.aEventListenerStateArray[ eEventType ] = false;
    switch( eEventType )
    {
        case EVENT_TRIGGER_ON_CLICK:
            this.aElement.removeEventListener( 'click', this.onClick, false );
            break;
        default:
            log( 'SourceEventElement.removeEventListener: not handled event type: ' + eEventType );
            return false;
    }
    return true;
};


// ------------------------------------------------------------------------------------------ //
function EventMultiplexer( aTimerEventQueue )
{
    this.aTimerEventQueue = aTimerEventQueue;
    this.aEventMap = new Object();

}


EventMultiplexer.prototype.registerEvent = function( eEventType, aNotifierId, aEvent )
{
    this.DBG( 'registerEvent', eEventType, aNotifierId );
    if( !this.aEventMap[ eEventType ] )
    {
        this.aEventMap[ eEventType ] = new Object();
    }
    if( !this.aEventMap[ eEventType ][ aNotifierId ] )
    {
        this.aEventMap[ eEventType ][ aNotifierId ] = new Array();
    }
    this.aEventMap[ eEventType ][ aNotifierId ].push( aEvent );
};


EventMultiplexer.prototype.notifyEvent = function( eEventType, aNotifierId )
{
    this.DBG( 'notifyEvent', eEventType, aNotifierId );
    if( this.aEventMap[ eEventType ] )
    {
        if( this.aEventMap[ eEventType ][ aNotifierId ] )
        {
            var aEventArray = this.aEventMap[ eEventType ][ aNotifierId ];
            var nSize = aEventArray.length;
            for( var i = 0; i < nSize; ++i )
            {
                this.aTimerEventQueue.addEvent( aEventArray[i] );
            }
        }
    }
};

EventMultiplexer.DEBUG = aEventMultiplexerDebugPrinter.isEnabled();

EventMultiplexer.prototype.DBG = function( sMethodName, eEventType, aNotifierId, nTime )
{
    if( EventMultiplexer.DEBUG )
    {
        var sInfo = 'EventMultiplexer.' + sMethodName;
        sInfo += '( type: ' + aEventTriggerOutMap[ eEventType ];
        sInfo += ', notifier: ' + aNotifierId + ' )';
        aEventMultiplexerDebugPrinter.print( sInfo, nTime );
    }
};



/**********************************************************************************************
 *      Interpolator Handler and KeyStopLerp
 **********************************************************************************************/

var aInterpolatorHandler = new Object();

aInterpolatorHandler.getInterpolator = function( eCalcMode, eValueType, eValueSubtype )
{
    var bHasSubtype = ( typeof( eValueSubtype ) === typeof( 0 ) );

    if( !bHasSubtype && aInterpolatorHandler.aLerpFunctorMap[ eCalcMode ][ eValueType ] )
    {
        return aInterpolatorHandler.aLerpFunctorMap[ eCalcMode ][ eValueType ];
    }
    else if( bHasSubtype && aInterpolatorHandler.aLerpFunctorMap[ eCalcMode ][ eValueType ][ eValueSubtype ] )
    {
        return aInterpolatorHandler.aLerpFunctorMap[ eCalcMode ][ eValueType ][ eValueSubtype ];
    }
    else
    {
        log( 'aInterpolatorHandler.getInterpolator: not found any valid interpolator for clalc mode '
                + aCalcModeOutMap[eCalcMode]  + 'and value type ' + aValueTypeOutMap[eValueType]  );
        return null;
    }
};

aInterpolatorHandler.aLerpFunctorMap = new Array();
aInterpolatorHandler.aLerpFunctorMap[ CALC_MODE_DISCRETE ] = new Array();
aInterpolatorHandler.aLerpFunctorMap[ CALC_MODE_LINEAR ] = new Array();


// interpolators for linear calculation

aInterpolatorHandler.aLerpFunctorMap[ CALC_MODE_LINEAR ][ NUMBER_PROPERTY ] =
    function ( nFrom, nTo, nT )
    {
        return ( ( 1.0 - nT )* nFrom + nT * nTo );
    };

aInterpolatorHandler.aLerpFunctorMap[ CALC_MODE_LINEAR ][ COLOR_PROPERTY ] = new Array();

aInterpolatorHandler.aLerpFunctorMap[ CALC_MODE_LINEAR ][ COLOR_PROPERTY ][ COLOR_SPACE_RGB ] =
    function ( nFrom, nTo, nT )
    {
        return RGBColor.interpolate( nFrom, nTo, nT );
    };

// For HSLColor we do not return the interpolator but a function
// that generate the interpolator. The AnimationColorNode is 'aware' of that.
aInterpolatorHandler.aLerpFunctorMap[ CALC_MODE_LINEAR ][ COLOR_PROPERTY ][ COLOR_SPACE_HSL ] =
    function ( bCCW  )
    {
        return  function ( nFrom, nTo, nT )
                {
                    return HSLColor.interpolate( nFrom, nTo, nT, bCCW );
                };
    };



// ------------------------------------------------------------------------------------------ //
function KeyStopLerp( aValueList )
{
    KeyStopLerp.validateInput( aValueList );

    this.aKeyStopList = new Array();
    this.nLastIndex = 0;
    this.nKeyStopDistance = aValueList[1] - aValueList[0];
    if( this.nKeyStopDistance <= 0 )
        this.nKeyStopDistance = 0.001;

    for( var i = 0; i < aValueList.length; ++i )
        this.aKeyStopList.push( aValueList[i] );

    this.nUpperBoundIndex = this.aKeyStopList.length - 2;
}


KeyStopLerp.validateInput = function( aValueList )
{
    var nSize = aValueList.length;
    assert( nSize > 1, 'KeyStopLerp.validateInput: key stop vector must have two entries or more' );

    for( var i = 1; i < nSize; ++i )
    {
        if( aValueList[i-1] > aValueList[i] )
            log( 'KeyStopLerp.validateInput: time vector is not sorted in ascending order!' );
    }
};

KeyStopLerp.prototype.reset = function()
{
    KeyStopLerp.validateInput( this.aKeyStopList );
    this.nLastIndex = 0;
    this.nKeyStopDistance = this.aKeyStopList[1] - this.aKeyStopList[0];
    if( this.nKeyStopDistance <= 0 )
        this.nKeyStopDistance = 0.001;

};

KeyStopLerp.prototype.lerp = function( nAlpha )
{
    if( nAlpha > this.aKeyStopList[ this.nLastIndex + 1 ] )
    {
        do
        {
            var nIndex = this.nLastIndex + 1;
            this.nLastIndex = clamp( nIndex, 0, this.nUpperBoundIndex );
            this.nKeyStopDistance = this.aKeyStopList[ this.nLastIndex + 1 ] - this.aKeyStopList[ this.nLastIndex ];
        }
        while( ( this.nKeyStopDistance <= 0 ) && ( this.nLastIndex < this.nUpperBoundIndex ) );
    }

    var nRawLerp = ( nAlpha - this.aKeyStopList[ this.nLastIndex ] ) / this.nKeyStopDistance;

    nRawLerp = clamp( nRawLerp, 0.0, 1.0 );

    var aResult = new Object();
    aResult.nIndex = this.nLastIndex;
    aResult.nLerp = nRawLerp;

    return aResult;
};

KeyStopLerp.prototype.lerp_ported = function( nAlpha )
{
    if( ( this.aKeyStopList[ this.nLastIndex ] < nAlpha ) ||
        ( this.aKeyStopList[ this.nLastIndex + 1 ] >= nAlpha ) )
    {
        var i = 0;
        for( ; i < this.aKeyStopList.length; ++i )
        {
            if( this.aKeyStopList[i] >= nAlpha )
                break;
        }
        if( this.aKeyStopList[i] > nAlpha )
            --i;
        var nIndex = i - 1;
        this.nLastIndex = clamp( nIndex, 0, this.aKeyStopList.length - 2 );
    }

    var nRawLerp = ( nAlpha - this.aKeyStopList[ this.nLastIndex ] ) /
                    ( this.aKeyStopList[ this.nLastIndex+1 ] - this.aKeyStopList[ this.nLastIndex ] );

    nRawLerp = clamp( nRawLerp, 0.0, 1.0 );

    var aResult = new Object();
    aResult.nIndex = this.nLastIndex;
    aResult.nLerp = nRawLerp;

    return aResult;
};



/**********************************************************************************************
 *      Operators
 **********************************************************************************************/

var aOperatorSetMap = new Array();

// number operators
aOperatorSetMap[ NUMBER_PROPERTY ] = new Object();

aOperatorSetMap[ NUMBER_PROPERTY ].add = function( a, b )
{
    return ( a + b );
};

aOperatorSetMap[ NUMBER_PROPERTY ].scale = function( k, v )
{
    return ( k * v );
};

// color operators
aOperatorSetMap[ COLOR_PROPERTY ] = new Object();

aOperatorSetMap[ COLOR_PROPERTY ].add = function( a, b )
{
    var c = a.clone();
    c.add( b );
    return c;
};

aOperatorSetMap[ COLOR_PROPERTY ].scale = function( k, v )
{
    var r = v.clone();
    r.scale( k );
    return r;
};



/**********************************************************************************************
 *      Activity Class Hierarchy
 **********************************************************************************************/

// ------------------------------------------------------------------------------------------ //
function ActivityParamSet()
{
    this.aEndEvent = null;
    this.aTimerEventQueue = null;
    this.aActivityQueue = null;
    this.nRepeatCount = 1.0;
    this.nAccelerationFraction = 0.0;
    this.nDecelerationFraction = 0.0;
    this.bAutoReverse = false;
    this.nMinDuration = undefined;
    this.nMinNumberOfFrames = MINIMUM_FRAMES_PER_SECONDS;
    this.aDiscreteTimes = new Array();
}

// ------------------------------------------------------------------------------------------ //
function AnimationActivity()
{
    this.nId = AnimationActivity.getUniqueId();
}


AnimationActivity.CURR_UNIQUE_ID = 0;

AnimationActivity.getUniqueId = function()
{
    ++AnimationActivity.CURR_UNIQUE_ID;
    return AnimationActivity.CURR_UNIQUE_ID;
};

AnimationActivity.prototype.getId = function()
{
    return this.nId;
};



// ------------------------------------------------------------------------------------------ //
function SetActivity( aCommonParamSet, aAnimation, aToAttr  )
{
    SetActivity.superclass.constructor.call( this );

    this.aAnimation = aAnimation;
    this.aTargetElement = null;
    this.aEndEvent = aCommonParamSet.aEndEvent;
    this.aTimerEventQueue = aCommonParamSet.aTimerEventQueue;
    this.aToAttr = aToAttr;
    this.bIsActive = true;
}
extend( SetActivity, AnimationActivity );


SetActivity.prototype.activate = function( aEndEvent )
{
    this.aEndEvent = aEndEvent;
    this.bIsActive = true;
};

SetActivity.prototype.dispose = function()
{
    this.bIsActive = false;
    if( this.aEndEvent && this.aEndEvent.isCharged() )
        this.aEndEvent.dispose();
};

SetActivity.prototype.calcTimeLag = function()
{
    return 0.0;
};

SetActivity.prototype.perform = function()
{
    if( !this.isActive() )
        return false;

    // we're going inactive immediately:
        this.bIsActive = false;

        if( this.aAnimation && this.aTargetElement )
        {
            this.aAnimation.start( this.aTargetElement );
            this.aAnimation.perform( this.aToAttr );
            this.aAnimation.end();
        }

        if( this.aEndEvent )
            this.aTimerEventQueue.addEvent( this.aEndEvent );

    };

    SetActivity.prototype.isActive = function()
    {
        return this.bIsActive;
    };

    SetActivity.prototype.dequeued = function()
    {
        // empty body
};

SetActivity.prototype.end = function()
{
    this.perform();
};

SetActivity.prototype.setTargets = function( aTargetElement )
{
    assert( aTargetElement, 'SetActivity.setTargets: target element is not valid' );
    this.aTargetElement = aTargetElement;
};



// ------------------------------------------------------------------------------------------ //
    function ActivityBase( aCommonParamSet )
    {
        ActivityBase.superclass.constructor.call( this );

        this.aTargetElement = null;
        this.aEndEvent = aCommonParamSet.aEndEvent;
        this.aTimerEventQueue = aCommonParamSet.aTimerEventQueue;
        this.nRepeats = aCommonParamSet.nRepeatCount;
        this.nAccelerationFraction = aCommonParamSet.nAccelerationFraction;
        this.nDecelerationFraction = aCommonParamSet.nDecelerationFraction;
        this.bAutoReverse = aCommonParamSet.bAutoReverse;

        this.bFirstPerformCall = true;
        this.bIsActive = true;

    }
    extend( ActivityBase, AnimationActivity );


    ActivityBase.prototype.activate = function( aEndEvent )
    {
        this.aEndEvent = aEndEvent;
        this.bFirstPerformCall = true;
        this.bIsActive = true;
    };

    ActivityBase.prototype.dispose = function()
    {
        // deactivate
    this.bIsActive = false;

    // dispose event
    if( this.aEndEvent )
        this.aEndEvent.dispose();

    this.aEndEvent = null;
};

ActivityBase.prototype.perform = function()
{
    // still active?
    if( !this.isActive() )
        return false; // no, early exit.

    assert( !this.FirstPerformCall, 'ActivityBase.perform: assertion (!this.FirstPerformCall) failed' );

    return true;
};

ActivityBase.prototype.calcTimeLag = function()
{
    // TODO(Q1): implement different init process!
    if( this.isActive() && this.bFirstPerformCall )
    {
        this.bFirstPerformCall = false;

        // notify derived classes that we're
        // starting now
        this.startAnimation();
    }
    return 0.0;
};

ActivityBase.prototype.isActive = function()
{
    return this.bIsActive;
};

ActivityBase.prototype.isDisposed = function()
{
    return ( !this.bIsActive && !this.aEndEvent );
};

ActivityBase.prototype.dequeued = function()
{
    if( !this.isActive() )
        this.endAnimation();
};

ActivityBase.prototype.setTargets = function( aTargetElement )
{
    assert( aTargetElement, 'ActivityBase.setTargets: target element is not valid' );

    this.aTargetElement = aTargetElement;
};

ActivityBase.prototype.startAnimation = function()
{
    throw ( 'ActivityBase.startAnimation: abstract method invoked' );
};

ActivityBase.prototype.endAnimation = function()
{
    throw ( 'ActivityBase.endAnimation: abstract method invoked' );
};

ActivityBase.prototype.endActivity = function()
{
    // this is a regular activity end
    this.bIsActive = false;

    // Activity is ending, queue event, then
    if( this.aEndEvent )
        this.aTimerEventQueue.addEvent( this.aEndEvent );

    this.aEndEvent = null;

};

ActivityBase.prototype.calcAcceleratedTime = function( nT )
{
    // Handle acceleration/deceleration


    // clamp nT to permissible [0,1] range
    nT = clamp( nT, 0.0, 1.0 );

    // take acceleration/deceleration into account. if the sum
    // of nAccelerationFraction and nDecelerationFraction
    // exceeds 1.0, ignore both (that's according to SMIL spec)
    if( ( this.nAccelerationFraction > 0.0 || this.nDecelerationFraction > 0.0 ) &&
         ( this.nAccelerationFraction + this.nDecelerationFraction <= 1.0 ) )
    {
        var nC = 1.0 - 0.5*this.nAccelerationFraction - 0.5*this.nDecelerationFraction;

        // this variable accumulates the new time value
        var nTPrime = 0.0;

        if( nT < this.nAccelerationFraction )
        {
            nTPrime += 0.5 * nT * nT / this.nAccelerationFraction; // partial first interval
        }
        else
        {
            nTPrime += 0.5 * this.nAccelerationFraction; // full first interval

            if( nT <= ( 1.0 - this.nDecelerationFraction ) )
            {
                nTPrime += nT - this.nAccelerationFraction; // partial second interval
            }
            else
            {
                nTPrime += 1.0 - this.nAccelerationFraction - this.nDecelerationFraction; // full second interval

                var nTRelative = nT - 1.0 + this.nDecelerationFraction;

                nTPrime += nTRelative - 0.5*nTRelative*nTRelative / this.nDecelerationFraction;
            }
        }

        // normalize, and assign to work variable
        nT = nTPrime / nC;

    }
    return nT;
};

ActivityBase.prototype.getEventQueue = function()
{
    return this.aTimerEventQueue;
};

ActivityBase.prototype.getTargetElement = function()
{
    return this.aTargetElement;
};

ActivityBase.prototype.isRepeatCountValid = function()
{
    if( this.nRepeats )
        return true;
    else
        return false;
};

ActivityBase.prototype.getRepeatCount = function()
{
    return this.nRepeats;
};

ActivityBase.prototype.isAutoReverse = function()
{
    return this.bAutoReverse;
};

ActivityBase.prototype.end = function()
{
    if( !this.isActive() || this.isDisposed() )
        return;

    // assure animation is started:
    if( this.bFirstPerformCall )
    {
        this.bFirstPerformCall = false;
        // notify derived classes that we're starting now
        this.startAnimation();
    }

    this.performEnd();
    this.endAnimation();
    this.endActivity();
};

ActivityBase.prototype.performEnd = function()
{
    throw ( 'ActivityBase.performEnd: abstract method invoked' );
    };



    // ------------------------------------------------------------------------------------------ //
function SimpleContinuousActivityBase( aCommonParamSet )
{
    SimpleContinuousActivityBase.superclass.constructor.call( this, aCommonParamSet );

    // Time elapsed since activity started
    this.aTimer = new ElapsedTime( aCommonParamSet.aActivityQueue.getTimer() );
    // Simple duration of activity
    this.nMinSimpleDuration = aCommonParamSet.nMinDuration;
    // Minimal number of frames to show
    this.nMinNumberOfFrames = aCommonParamSet.nMinNumberOfFrames;
    // Actual number of frames shown until now.
    this.nCurrPerformCalls = 0;

}
extend( SimpleContinuousActivityBase, ActivityBase );


SimpleContinuousActivityBase.prototype.startAnimation = function()
{
    // init timer. We measure animation time only when we're
    // actually started.
    this.aTimer.reset();

};

SimpleContinuousActivityBase.prototype.calcTimeLag = function()
{
    SimpleContinuousActivityBase.superclass.calcTimeLag.call( this );

    if( !this.isActive() )
        return 0.0;

    // retrieve locally elapsed time
    var nCurrElapsedTime = this.aTimer.getElapsedTime();

    // go to great length to ensure a proper animation
    // run. Since we don't know how often we will be called
    // here, try to spread the animator calls uniquely over
    // the [0,1] parameter range. Be aware of the fact that
    // perform will be called at least mnMinNumberOfTurns
    // times.

    // fraction of time elapsed
    var nFractionElapsedTime = nCurrElapsedTime / this.nMinSimpleDuration;

    // fraction of minimum calls performed
    var nFractionRequiredCalls = this.nCurrPerformCalls / this.nMinNumberOfFrames;

    // okay, so now, the decision is easy:
    //
    // If the fraction of time elapsed is smaller than the
    // number of calls required to be performed, then we calc
    // the position on the animation range according to
    // elapsed time. That is, we're so to say ahead of time.
    //
    // In contrary, if the fraction of time elapsed is larger,
    // then we're lagging, and we thus calc the position on
    // the animation time line according to the fraction of
    // calls performed. Thus, the animation is forced to slow
    // down, and take the required minimal number of steps,
    // sufficiently equally distributed across the animation
    // time line.

    if( nFractionElapsedTime < nFractionRequiredCalls )
    {
        return 0.0;
    }
    else
    {
        // lag global time, so all other animations lag, too:
        return ( ( nFractionElapsedTime - nFractionRequiredCalls ) * this.nMinSimpleDuration );
    }
};

SimpleContinuousActivityBase.prototype.perform = function()
{
    // call base class, for start() calls and end handling
    if( !SimpleContinuousActivityBase.superclass.perform.call( this ) )
        return false; // done, we're ended

    // get relative animation position
    var nCurrElapsedTime = this.aTimer.getElapsedTime();
    var nT = nCurrElapsedTime / this.nMinSimpleDuration;


    // one of the stop criteria reached?

    // will be set to true below, if one of the termination criteria matched.
    var bActivityEnding = false;

    if( this.isRepeatCountValid() )
    {
        // Finite duration case

        // When we've autoreverse on, the repeat count doubles
        var nRepeatCount = this.getRepeatCount();
        var nEffectiveRepeat = this.isAutoReverse() ? 2.0 * nRepeatCount : nRepeatCount;

        // time (or frame count) elapsed?
        if( nEffectiveRepeat <= nT )
        {
            // Ok done for now. Will not exit right here,
            // to give animation the chance to render the last
            // frame below
            bActivityEnding = true;

            // clamp animation to max permissible value
            nT = nEffectiveRepeat;
        }
    }


    // need to do auto-reverse?

    var nRepeats;
    var nRelativeSimpleTime;
    // TODO(Q3): Refactor this mess
    if( this.isAutoReverse() )
    {
        // divert active duration into repeat and
        // fractional part.
        nRepeats = Math.floor( nT );
        var nFractionalActiveDuration =  nT - nRepeats;

        // for auto-reverse, map ranges [1,2), [3,4), ...
        // to ranges [0,1), [1,2), etc.
        if( nRepeats % 2 )
        {
            // we're in an odd range, reverse sweep
            nRelativeSimpleTime = 1.0 - nFractionalActiveDuration;
        }
        else
        {
            // we're in an even range, pass on as is
            nRelativeSimpleTime = nFractionalActiveDuration;
        }

        // effective repeat count for autoreverse is half of
        // the input time's value (each run of an autoreverse
        // cycle is half of a repeat)
        nRepeats /= 2;
    }
    else
    {
        // determine repeat

        // calc simple time and number of repeats from nT
        // Now, that's easy, since the fractional part of
        // nT gives the relative simple time, and the
        // integer part the number of full repeats:
        nRepeats = Math.floor( nT );
        nRelativeSimpleTime = nT - nRepeats;

        // clamp repeats to max permissible value (maRepeats.getValue() - 1.0)
        if( this.isRepeatCountValid() && ( nRepeats >= this.getRepeatCount() ) )
        {
            // Note that this code here only gets
            // triggered if this.nRepeats is an
            // _integer_. Otherwise, nRepeats will never
            // reach nor exceed
            // maRepeats.getValue(). Thus, the code below
            // does not need to handle cases of fractional
            // repeats, and can always assume that a full
            // animation run has ended (with
            // nRelativeSimpleTime = 1.0 for
            // non-autoreversed activities).

            // with modf, nRelativeSimpleTime will never
            // become 1.0, since nRepeats is incremented and
            // nRelativeSimpleTime set to 0.0 then.
            //
            // For the animation to reach its final value,
            // nRepeats must although become this.nRepeats - 1.0,
            // and nRelativeSimpleTime = 1.0.
            nRelativeSimpleTime = 1.0;
            nRepeats -= 1.0;
        }
    }


    // actually perform something

    this.simplePerform( nRelativeSimpleTime, nRepeats );

    // delayed endActivity() call from end condition check
    // below. Issued after the simplePerform() call above, to
    // give animations the chance to correctly reach the
    // animation end value, without spurious bail-outs because
    // of isActive() returning false.
    if( bActivityEnding )
        this.endActivity();

    // one more frame successfully performed
    ++this.nCurrPerformCalls;

    return this.isActive();
};

SimpleContinuousActivityBase.prototype.simplePerform = function( nSimpleTime, nRepeatCount )
{
    throw ( 'SimpleContinuousActivityBase.simplePerform: abstract method invoked' );
};



// ------------------------------------------------------------------------------------------ //
function ContinuousKeyTimeActivityBase( aCommonParamSet )
{
    var nSize = aCommonParamSet.aDiscreteTimes.length;
    assert( nSize > 1,
            'ContinuousKeyTimeActivityBase constructor: assertion (aDiscreteTimes.length > 1) failed' );

    assert( aCommonParamSet.aDiscreteTimes[0] == 0.0,
            'ContinuousKeyTimeActivityBase constructor: assertion (aDiscreteTimes.front() == 0.0) failed' );

    assert( aCommonParamSet.aDiscreteTimes[ nSize - 1 ] <= 1.0,
            'ContinuousKeyTimeActivityBase constructor: assertion (aDiscreteTimes.back() <= 1.0) failed' );

    ContinuousKeyTimeActivityBase.superclass.constructor.call( this, aCommonParamSet );

    this.aLerper = new KeyStopLerp( aCommonParamSet.aDiscreteTimes );
}
extend( ContinuousKeyTimeActivityBase, SimpleContinuousActivityBase );


ContinuousKeyTimeActivityBase.prototype.activate = function( aEndElement )
{
    ContinuousKeyTimeActivityBase.superclass.activate.call( this, aEndElement );

    this.aLerper.reset();
};

ContinuousKeyTimeActivityBase.prototype.performHook = function( nIndex, nFractionalIndex, nRepeatCount )
{
    throw ( 'ContinuousKeyTimeActivityBase.performHook: abstract method invoked' );
};

ContinuousKeyTimeActivityBase.prototype.simplePerform = function( nSimpleTime, nRepeatCount )
{
    var nAlpha = this.calcAcceleratedTime( nSimpleTime );

    var aLerpResult = this.aLerper.lerp( nAlpha );

    this.performHook( aLerpResult.nIndex, aLerpResult.nLerp, nRepeatCount );
};



// ------------------------------------------------------------------------------------------ //
function ContinuousActivityBase( aCommonParamSet )
{
    ContinuousActivityBase.superclass.constructor.call( this, aCommonParamSet );

}
extend( ContinuousActivityBase, SimpleContinuousActivityBase );


ContinuousActivityBase.prototype.performHook = function( nModifiedTime, nRepeatCount )
{
    throw ( 'ContinuousActivityBase.performHook: abstract method invoked' );
};

ContinuousActivityBase.prototype.simplePerform = function( nSimpleTime, nRepeatCount )
{
    this.performHook( this.calcAcceleratedTime( nSimpleTime ), nRepeatCount );
};



// ------------------------------------------------------------------------------------------ //
function SimpleActivity( aCommonParamSet, aNumberAnimation, eDirection )
{
    assert( ( eDirection == BACKWARD ) || ( eDirection == FORWARD ),
            'SimpleActivity constructor: animation direction is not valid' );

    assert( aNumberAnimation, 'SimpleActivity constructor: animation object is not valid' );

    SimpleActivity.superclass.constructor.call( this, aCommonParamSet );

    this.aAnimation = aNumberAnimation;
    this.nDirection = ( eDirection == FORWARD ) ? 1.0 : 0.0;
}
extend( SimpleActivity, ContinuousActivityBase );


SimpleActivity.prototype.startAnimation = function()
{
    if( this.isDisposed() || !this.aAnimation )
        return;

    ANIMDBG.print( 'SimpleActivity.startAnimation invoked' );
    SimpleActivity.superclass.startAnimation.call( this );

    // start animation
        this.aAnimation.start( this.getTargetElement() );
    };

    SimpleActivity.prototype.endAnimation = function()
    {
        if( this.aAnimation )
            this.aAnimation.end();

    };

    SimpleActivity.prototype.performHook = function( nModifiedTime, nRepeatCount )
    {
        // nRepeatCount is not used

    if( this.isDisposed() || !this.aAnimation )
        return;

    var nT = 1.0 - this.nDirection + nModifiedTime * ( 2.0*this.nDirection - 1.0 );
    //ANIMDBG.print( 'SimpleActivity.performHook: nT = ' + nT );
    this.aAnimation.perform( nT );
};

SimpleActivity.prototype.performEnd = function()
{
    if( this.aAnimation )
        this.aAnimation.perform( this.nDirection );
};



// ------------------------------------------------------------------------------------------ //
//  FromToByActivity< BaseType > template class


function FromToByActivityTemplate( BaseType ) // template parameter
{

    function FromToByActivity( aFromValue, aToValue, aByValue,
                               aActivityParamSet, aAnimation,
                               aInterpolator, aOperatorSet, bAccumulate )
    {
        assert( aAnimation, 'FromToByActivity constructor: invalid animation object' );
        assert( ( aToValue != undefined ) || ( aByValue != undefined ),
                'FromToByActivity constructor: one of aToValue or aByValue must be valid' );

        FromToByActivity.superclass.constructor.call( this, aActivityParamSet );

        this.aFrom = aFromValue;
        this.aTo = aToValue;
        this.aBy = aByValue;
        this.aStartValue;
        this.aEndValue;
        this.aAnimation = aAnimation;
        this.aInterpolator = aInterpolator;
        this.add = aOperatorSet.add;
        this.scale = aOperatorSet.scale;
        this.bDynamicStartValue = false;
        this.bCumulative = bAccumulate;

        this.initAnimatedElement();

    }
    extend( FromToByActivity, BaseType );

    FromToByActivity.prototype.initAnimatedElement = function()
    {
        if( this.aAnimation && this.aFrom )
            this.aAnimation.perform( this.aFrom );
    };

    FromToByActivity.prototype.startAnimation = function()
    {
        if( this.isDisposed() || !this.aAnimation  )
        {
            log( 'FromToByActivity.startAnimation: activity disposed or not valid animation' );
            return;
        }

        FromToByActivity.superclass.startAnimation.call( this );

        this.aAnimation.start( this.getTargetElement() );


        var aAnimationStartValue = this.aAnimation.getUnderlyingValue();

        // first of all, determine general type of
        // animation, by inspecting which of the FromToBy values
        // are actually valid.
        // See http://www.w3.org/TR/smil20/animation.html#AnimationNS-FromToBy
        // for a definition
        if( this.aFrom )
        {
            // From-to or From-by animation. According to
            // SMIL spec, the To value takes precedence
            // over the By value, if both are specified
            if( this.aTo )
            {
                // From-To animation
                this.aStartValue = this.aFrom;
                this.aEndValue = this.aTo;
            }
            else if( this.aBy )
            {
                // From-By animation
                this.aStartValue = this.aFrom;

                // this.aEndValue = this.aStartValue + this.aBy;
                this.aEndValue = this.add( this.aStartValue, this.aBy );
            }
        }
        else
        {
            // By or To animation. According to SMIL spec,
            // the To value takes precedence over the By
            // value, if both are specified
            if( this.aTo )
            {
                // To animation

                // According to the SMIL spec
                // (http://www.w3.org/TR/smil20/animation.html#animationNS-ToAnimation),
                // the to animation interpolates between
                // the _running_ underlying value and the to value (as the end value)
                this.bDynamicStartValue = true;
                this.aEndValue = this.aTo;
            }
            else if( this.aBy )
            {
                // By animation
                this.aStartValue = aAnimationStartValue;

                // this.aEndValue = this.aStartValue + this.aBy;
                this.aEndValue = this.add( this.aStartValue, this.aBy );
            }
        }

        ANIMDBG.print( 'FromToByActivity.startAnimation: aStartValue = ' + this.aStartValue + ', aEndValue = ' + this.aEndValue );
    };

    FromToByActivity.prototype.endAnimation = function()
    {
        if( this.aAnimation )
            this.aAnimation.end();
    };

    // performHook override for ContinuousActivityBase
    FromToByActivity.prototype.performHook = function( nModifiedTime, nRepeatCount )
    {
        if( this.isDisposed() || !this.aAnimation  )
        {
            log( 'FromToByActivity.performHook: activity disposed or not valid animation' );
            return;
        }

        var aValue = this.bDynamicStartValue ? this.aAnimation.getUnderlyingValue()
                                             : this.aStartValue;

        aValue = this.aInterpolator( aValue, this.aEndValue, nModifiedTime );

        if( this.bCumulative )
        {
            // aValue = this.aEndValue * nRepeatCount + aValue;
            aValue = this.add( this.scale( nRepeatCount, this.aEndValue ), aValue );
        }

        this.aAnimation.perform( aValue );
    };

    FromToByActivity.prototype.performEnd = function()
    {
        if( this.aAnimation )
        {
            if( this.isAutoreverse() )
                this.aAnimation.perform( this.aStartValue );
            else
                this.aAnimation.perform( this.aEndValue );
        }
    };

    FromToByActivity.prototype.dispose = function()
    {
        FromToByActivity.superclass.dispose.call( this );
    };


    return FromToByActivity;
}


// FromToByActivity< ContinuousActivityBase > instantiation
var LinearFromToByActivity = instantiate( FromToByActivityTemplate, ContinuousActivityBase );



// ------------------------------------------------------------------------------------------ //
//  ValueListActivity< BaseType > template class


function  ValueListActivityTemplate( BaseType ) // template parameter
{

    function ValueListActivity( aValueList, aActivityParamSet,
                                aAnimation, aInterpolator,
                                aOperatorSet, bAccumulate )
    {
        assert( aAnimation, 'ValueListActivity constructor: invalid animation object' );
        assert( aValueList.length != 0, 'ValueListActivity: value list is empty' );

        ValueListActivity.superclass.constructor.call( this, aActivityParamSet );

        this.aValueList = aValueList;
        this.aAnimation = aAnimation;
        this.aInterpolator = aInterpolator;
        this.add = aOperatorSet.add;
        this.scale = aOperatorSet.scale;
        this.bCumulative = bAccumulate;
        this.aLastValue = this.aValueList[ this.aValueList.length - 1 ];

        this.initAnimatedElement();
    }
    extend( ValueListActivity, BaseType );

    ValueListActivity.prototype.activate = function( aEndEvent )
    {
        ValueListActivity.superclass.activate.call( this, aEndEvent );
        for( var i = 0; i < this.aValueList.length; ++i )
        {
            ANIMDBG.print( 'createValueListActivity: value[' + i + '] = ' + this.aValueList[i] );
        }

        this.initAnimatedElement();
    };

    ValueListActivity.prototype.initAnimatedElement = function()
    {
        if( this.aAnimation )
            this.aAnimation.perform( this.aValueList[0] );
    };

    ValueListActivity.prototype.startAnimation = function()
    {
        if( this.isDisposed() || !this.aAnimation  )
        {
            log( 'ValueListActivity.startAnimation: activity disposed or not valid animation' );
                return;
            }

            ValueListActivity.superclass.startAnimation.call( this );

            this.aAnimation.start( this.getTargetElement() );
        };

        ValueListActivity.prototype.endAnimation = function()
        {
            if( this.aAnimation )
                this.aAnimation.end();
        };

        // performHook override for ContinuousKeyTimeActivityBase base
    ValueListActivity.prototype.performHook = function( nIndex, nFractionalIndex, nRepeatCount )
    {
        if( this.isDisposed() || !this.aAnimation  )
        {
            log( 'ValueListActivity.performHook: activity disposed or not valid animation' );
            return;
        }

        assert( ( nIndex + 1 ) < this.aValueList.length,
                'ValueListActivity.performHook: assertion (nIndex + 1 < this.aValueList.length) failed' );

        // interpolate between nIndex and nIndex+1 values

        var aValue = this.aInterpolator( this.aValueList[ nIndex ],
                                         this.aValueList[ nIndex+1 ],
                                         nFractionalIndex );

        if( this.bCumulative )
        {
            aValue = this.add( aValue, this.scale( nRepeatCount, this.aLastValue ) );
            //aValue = aValue + nRepeatCount * this.aLastValue;
        }
        this.aAnimation.perform( aValue );
    };

    ValueListActivity.prototype.performEnd = function()
    {
        if( this.aAnimation )
        {
            this.aAnimation.perform( this.aLastValue );
        }
    };

    ValueListActivity.prototype.dispose = function()
    {
        ValueListActivity.superclass.dispose.call( this );
    };


    return ValueListActivity;
}


//  ValueListActivity< ContinuousKeyTimeActivityBase > instantiation
var LinearValueListActivity = instantiate( ValueListActivityTemplate, ContinuousKeyTimeActivityBase );



/**********************************************************************************************
 *      Activity Factory
 **********************************************************************************************/

// ------------------------------------------------------------------------------------------ //
function createActivity( aActivityParamSet, aAnimationNode, aAnimation, aInterpolator )
{
    var eCalcMode = aAnimationNode.getCalcMode();

    var sAttributeName = aAnimationNode.getAttributeName();
    var aAttributeProp = aAttributeMap[ sAttributeName ];

    var eValueType = aAttributeProp[ 'type' ];
    var eValueSubtype = aAttributeProp[ 'subtype' ];

    // do we need to get an interpolator ?
    if( ! aInterpolator )
    {
        aInterpolator = aInterpolatorHandler.getInterpolator( eCalcMode,
                                                              eValueType,
                                                              eValueSubtype );
    }

    // is it cumulative ?
    var bAccumulate = ( aAnimationNode.getAccumulate() === ACCUMULATE_MODE_SUM )
                            && !( eValueType === BOOL_PROPERTY ||
                                  eValueType === STRING_PROPERTY ||
                                  eValueType === ENUM_PROPERTY );



    aActivityParamSet.aDiscreteTimes = aAnimationNode.getKeyTimes();

    // do we have a value list ?
    var aValueSet = aAnimationNode.getValues();
    var nValueSetSize = aValueSet.length;

    if( nValueSetSize != 0 )
    {
        // Value list activity

        if( aActivityParamSet.aDiscreteTimes.length == 0 )
        {
            for( var i = 0; i < nValueSetSize; ++i )
                aActivityParamSet.aDiscreteTimes[i].push( i / nValueSetSize );
        }

        switch( eCalcMode )
        {
            case CALC_MODE_DISCRETE:
                log( 'createActivity: discrete calculation case not yet implemented' );
                break;

            default:
                log( 'createActivity: unexpected calculation mode: ' + eCalcMode );
                // FALLTHROUGH intended
            case CALC_MODE_PACED :
                // FALLTHROUGH intended
            case CALC_MODE_SPLINE :
             // FALLTHROUGH intended
            case CALC_MODE_LINEAR:
                return createValueListActivity( aActivityParamSet,
                                                aAnimationNode,
                                                aAnimation,
                                                aInterpolator,
                                                LinearValueListActivity,
                                                bAccumulate,
                                                eValueType );
        }
    }
    else
    {
        // FromToBy activity

        switch( eCalcMode )
        {
            case CALC_MODE_DISCRETE:
                log( 'createActivity: discrete calculation case not yet implemented' );
                break;

            default:
                log( 'createActivity: unexpected calculation mode: ' + eCalcMode );
                // FALLTHROUGH intended
            case CALC_MODE_PACED :
                // FALLTHROUGH intended
            case CALC_MODE_SPLINE :
                // FALLTHROUGH intended
            case CALC_MODE_LINEAR:
                return createFromToByActivity(  aActivityParamSet,
                                                aAnimationNode,
                                                aAnimation,
                                                aInterpolator,
                                                LinearFromToByActivity,
                                                bAccumulate,
                                                eValueType );
        }
    }
}



// ------------------------------------------------------------------------------------------ //
function createValueListActivity( aActivityParamSet, aAnimationNode, aAnimation,
                                  aInterpolator, ClassTemplateInstance, bAccumulate, eValueType )
{
    var aAnimatedElement = aAnimationNode.getAnimatedElement();
    var aOperatorSet = aOperatorSetMap[ eValueType ];
    assert( aOperatorSet, 'createFromToByActivity: no operator set found' );

    var aValueSet = aAnimationNode.getValues();

    var aValueList = new Array();

    extractAttributeValues( eValueType,
                            aValueList,
                            aValueSet,
                            aAnimatedElement.getBaseBBox(),
                            aActivityParamSet.nSlideWidth,
                            aActivityParamSet.nSlideHeight );

    for( var i = 0; i < aValueList.length; ++i )
    {
        ANIMDBG.print( 'createValueListActivity: value[' + i + '] = ' + aValueList[i] );
    }

    return new ClassTemplateInstance( aValueList, aActivityParamSet, aAnimation,
                                      aInterpolator, aOperatorSet, bAccumulate );
}



// ------------------------------------------------------------------------------------------ //
function createFromToByActivity( aActivityParamSet, aAnimationNode, aAnimation,
                                 aInterpolator, ClassTemplateInstance, bAccumulate, eValueType )
{

    var aAnimatedElement = aAnimationNode.getAnimatedElement();
    var aOperatorSet = aOperatorSetMap[ eValueType ];
    assert( aOperatorSet, 'createFromToByActivity: no operator set found' );

    var aValueSet = new Array();
    aValueSet[0] = aAnimationNode.getFromValue();
    aValueSet[1] = aAnimationNode.getToValue();
    aValueSet[2] = aAnimationNode.getByValue();

    ANIMDBG.print( 'createFromToByActivity: value type: ' + aValueTypeOutMap[eValueType] +
                    ', aFrom = ' + aValueSet[0] +
                    ', aTo = ' + aValueSet[1] +
                    ', aBy = ' + aValueSet[2] );

    var aValueList = new Array();

    extractAttributeValues( eValueType,
                            aValueList,
                            aValueSet,
                            aAnimatedElement.getBaseBBox(),
                            aActivityParamSet.nSlideWidth,
                            aActivityParamSet.nSlideHeight );

    ANIMDBG.print( 'createFromToByActivity: ' +
                    ', aFrom = ' + aValueList[0] +
                    ', aTo = ' + aValueList[1] +
                    ', aBy = ' + aValueList[2] );

    return new ClassTemplateInstance( aValueList[0], aValueList[1], aValueList[2],
                                      aActivityParamSet, aAnimation,
                                      aInterpolator, aOperatorSet, bAccumulate );
}


// ------------------------------------------------------------------------------------------ //
function extractAttributeValues( eValueType, aValueList, aValueSet, aBBox, nSlideWidth, nSlideHeight )
{
    switch( eValueType )
    {
        case NUMBER_PROPERTY :
            evalValuesAttribute( aValueList, aValueSet, aBBox, nSlideWidth, nSlideHeight );
            break;
        case BOOL_PROPERTY :
            for( var i = 0; i < aValueSet.length; ++i )
            {
                var aValue = booleanParser( aValueSet[i] );
                aValueList.push( aValue );
            }
            break;
        case STRING_PROPERTY :
            for( var i = 0; i < aValueSet.length; ++i )
            {
                aValueList.push( aValueSet[i] );
            }
            break;
        case ENUM_PROPERTY :
            for( var i = 0; i < aValueSet.length; ++i )
            {
                aValueList.push( aValueSet[i] );
            }
            break;
        case COLOR_PROPERTY :
            for( var i = 0; i < aValueSet.length; ++i )
            {
                var aValue = colorParser( aValueSet[i] );
                aValueList.push( aValue );
            }
            break;
        default:
            log( 'createValueListActivity: unexpeded value type: ' + eValueType );
    }

}

// ------------------------------------------------------------------------------------------ //
function evalValuesAttribute( aValueList, aValueSet, aBBox, nSlideWidth, nSlideHeight )
{
    var width = aBBox.width / nSlideWidth;
    var height = aBBox.height / nSlideHeight;
    var x = ( aBBox.x + aBBox.width / 2 ) / nSlideWidth;
    var y = ( aBBox.y + aBBox.height / 2 ) / nSlideHeight;

    for( var i = 0; i < aValueSet.length; ++i )
    {
        var aValue =  eval( aValueSet[i] );
        aValueList.push( aValue );
    }
}



/**********************************************************************************************
 *      SlideShow, SlideShowContext and FrameSynchronization
 **********************************************************************************************/

// ------------------------------------------------------------------------------------------ //

// direction of animation, important: not change the values!
var BACKWARD    = 0;
var FORWARD     = 1;

var MAXIMUM_FRAME_COUNT                 = 60;
var MINIMUM_TIMEOUT                     = 1.0 / MAXIMUM_FRAME_COUNT;
var MAXIMUM_TIMEOUT                     = 4.0;
var MINIMUM_FRAMES_PER_SECONDS          = 10;
var PREFERRED_FRAMES_PER_SECONDS        = 50;
var PREFERRED_FRAME_RATE                = 1.0 / PREFERRED_FRAMES_PER_SECONDS;



function SlideShow()
{
    this.aTimer = new ElapsedTime();
    this.aFrameSynchronization = new FrameSynchronization( PREFERRED_FRAME_RATE );
    this.aTimerEventQueue = new TimerEventQueue( this.aTimer );
    this.aActivityQueue = new ActivityQueue( this.aTimer );
    this.aNextEffectEventArray = null;
    this.aEventMultiplexer = null;

    this.aContext = new SlideShowContext( this.aTimerEventQueue, this.aEventMultiplexer,
                                          this.aNextEffectEventArray, this.aActivityQueue );
    this.nCurrentEffect = 0;
    this.eDirection = FORWARD;
    this.bIsIdle = true;
    this.bIsEnabled = true;
}


SlideShow.prototype.setSlideEvents = function( aNextEffectEventArray, aEventMultiplexer )
{
    if( !aNextEffectEventArray )
        log( 'SlideShow.setSlideEvents: aNextEffectEventArray is not valid' );

    if( !aEventMultiplexer )
        log( 'SlideShow.setSlideEvents: aEventMultiplexer is not valid' );

    this.aContext.aNextEffectEventArray = aNextEffectEventArray;
    this.aNextEffectEventArray = aNextEffectEventArray;
    this.aContext.aEventMultiplexer = aEventMultiplexer;
    this.aEventMultiplexer = aEventMultiplexer;
    this.nCurrentEffect = 0;
};

SlideShow.prototype.isRunning = function()
{
    return !this.bIsIdle;
};

SlideShow.prototype.isEnabled = function()
{
    return this.bIsEnabled;
};

SlideShow.prototype.notifyNextEffectStart = function()
{
    var aAnimatedElementMap = theMetaDoc.aMetaSlideSet[nCurSlide].aSlideAnimationsHandler.aAnimatedElementMap;
    for( sId in aAnimatedElementMap )
        aAnimatedElementMap[ sId ].notifyNextEffectStart( this.nCurrentEffect );
};

SlideShow.prototype.notifySlideStart = function( nSlideIndex )
{
    var aAnimatedElementMap = theMetaDoc.aMetaSlideSet[nSlideIndex].aSlideAnimationsHandler.aAnimatedElementMap;
    for( sId in aAnimatedElementMap )
        aAnimatedElementMap[ sId ].notifySlideStart();
};

SlideShow.prototype.nextEffect = function()
{
    if( !this.isEnabled() )
        return false;

    if( this.isRunning() )
        return true;

    if( !this.aNextEffectEventArray )
        return false;

    this.notifyNextEffectStart();

    if( this.nCurrentEffect >= this.aNextEffectEventArray.size() )
        return false;

    this.eDirection = FORWARD;
    this.aNextEffectEventArray.at( this.nCurrentEffect ).fire();
    ++this.nCurrentEffect;
    this.update();
    return true;
};

SlideShow.prototype.previousEffect = function()
{
    if( this.nCurrentEffect <= 0 )
        return false;
    this.eDirection = BACKWARD;
    this.aNextEffectEventArray.at( this.nCurrentEffect ).fire();
    --this.nCurrentEffect;
    return true;
};

SlideShow.prototype.displaySlide = function( nNewSlide, bSkipSlideTransition )
{
    var aMetaDoc = theMetaDoc;
    var nSlides = aMetaDoc.nNumberOfSlides;
    if( nNewSlide < 0 && nSlides > 0 )
        nNewSlide = nSlides - 1;
    else if( nNewSlide >= nSlides )
        nNewSlide = 0;

    if( ( currentMode === INDEX_MODE ) && ( nNewSlide === nCurSlide ) )
    {
        var newMetaSlide = aMetaDoc.aMetaSlideSet[nNewSlide];
        newMetaSlide.show();
        return;
    }

    // handle current slide
    var nOldSlide = nCurSlide;
    if( nOldSlide !== undefined )
    {
        var oldMetaSlide = aMetaDoc.aMetaSlideSet[nOldSlide];
        if( this.isEnabled() )
        {
            // hide current slide
            oldMetaSlide.hide();
            if( oldMetaSlide.aSlideAnimationsHandler.isAnimated() )
            {
                // force end animations
                oldMetaSlide.aSlideAnimationsHandler.end( bSkipSlideTransition );

                // clear all queues
                this.dispose();
            }
        }
        else
        {
            oldMetaSlide.hide();
        }
    }

    // handle new slide
    nCurSlide = nNewSlide;
    var newMetaSlide = aMetaDoc.aMetaSlideSet[nNewSlide];
    if( this.isEnabled() )
    {
        // prepare to show a new slide
        this.notifySlideStart( nNewSlide );

        if( !bSkipSlideTransition )
        {
            // create slide transition and add to activity queue
            // to be implemented
        }

        // show next slide and start animations
        newMetaSlide.show();
        newMetaSlide.aSlideAnimationsHandler.start();
        this.update();
    }
    else
    {
        newMetaSlide.show();
    }



    /*
    var nOldSlide = nCurSlide;
    nCurSlide = nNewSlide;

    var oldMetaSlide = aMetaDoc.aMetaSlideSet[nOldSlide];
    var newMetaSlide = aMetaDoc.aMetaSlideSet[nNewSlide];

    if( !this.isEnabled() )
    {
        oldMetaSlide.hide();
        newMetaSlide.show();
        return;
    }

    // force end animations and hide current slide
    oldMetaSlide.hide();
    oldMetaSlide.aSlideAnimationsHandler.end( bSkipSlideTransition );

    // clear all queues
    this.dispose();

    // prepare to show a new slide
    this.notifySlideStart();

    if( !bSkipSlideTransition )
    {
        // create slide transition and add to activity queue
        // to be implemented
    }

    // show next slide and start animations
    newMetaSlide.show();
    newMetaSlide.aSlideAnimationsHandler.start();
    this.update();
    */
};

SlideShow.prototype.update = function()
{
    this.aTimer.holdTimer();
    var suspendHandle = ROOT_NODE.suspendRedraw( PREFERRED_FRAME_RATE * 1000 );

    // process queues
    this.aTimerEventQueue.process();
    this.aActivityQueue.process();

    this.aFrameSynchronization.synchronize();

    ROOT_NODE.unsuspendRedraw(suspendHandle);
    ROOT_NODE.forceRedraw();
    this.aTimer.releaseTimer();

    var bActivitiesLeft = ( ! this.aActivityQueue.isEmpty() );
    var bTimerEventsLeft = ( ! this.aTimerEventQueue.isEmpty() );
    var bEventsLeft = ( bActivitiesLeft || bTimerEventsLeft );


    if( bEventsLeft )
    {
        var nNextTimeout;
        if( bActivitiesLeft )
        {
            nNextTimeout = MINIMUM_TIMEOUT;
            this.aFrameSynchronization.activate();
        }
        else
        {
            nNextTimeout = this.aTimerEventQueue.nextTimeout();
            if( nNextTimeout < MINIMUM_TIMEOUT )
                nNextTimeout = MINIMUM_TIMEOUT;
            else if( nNextTimeout > MAXIMUM_TIMEOUT )
                nNextTimeout = MAXIMUM_TIMEOUT;
            this.aFrameSynchronization.deactivate();
        }

        this.bIsIdle = false;
        window.setTimeout( 'aSlideShow.update()', nNextTimeout * 1000 );
    }
    else
    {
        this.bIsIdle = true;
    }
};

SlideShow.prototype.dispose = function()
{
    // clear all queues
    this.aTimerEventQueue.clear();
    this.aActivityQueue.clear();
    this.aNextEffectEventArray = null;
    this.aEventMultiplexer = null;
};

SlideShow.prototype.getContext = function()
{
    return this.aContext;
};

// the SlideShow global instance
var aSlideShow = null;



// ------------------------------------------------------------------------------------------ //
function SlideShowContext( aTimerEventQueue, aEventMultiplexer, aNextEffectEventArray, aActivityQueue)
{
    this.aTimerEventQueue = aTimerEventQueue;
    this.aEventMultiplexer = aEventMultiplexer;
    this.aNextEffectEventArray = aNextEffectEventArray;
    this.aActivityQueue = aActivityQueue;
}



// ------------------------------------------------------------------------------------------ //
function FrameSynchronization( nFrameDuration )
{
    this.nFrameDuration = nFrameDuration;
    this.aTimer = new ElapsedTime();
    this.nNextFrameTargetTime = 0.0;
    this.bIsActive = false;

    this.markCurrentFrame();
}


FrameSynchronization.prototype.markCurrentFrame = function()
{
    this.nNextFrameTargetTime = this.aTimer.getElapsedTime() + this.nFrameDuration;
};

FrameSynchronization.prototype.synchronize = function()
{
    if( this.bIsActive )
    {
        // Do busy waiting for now.
        while( this.aTimer.getElapsedTime() < this.nNextFrameTargetTime )
            ;
    }

    this.markCurrentFrame();

};

FrameSynchronization.prototype.activate = function()
{
    this.bIsActive = true;
};

FrameSynchronization.prototype.deactivate = function()
{
    this.bIsActive = false;
};



/**********************************************************************************************
 *      TimerEventQueue, ActivityQueue and ElapsedTime
 **********************************************************************************************/

//------------------------------------------------------------------------------------------- //
function NextEffectEventArray()
{
    this.aEventArray = new Array();
}


NextEffectEventArray.prototype.size = function()
{
    return this.aEventArray.length;
};

NextEffectEventArray.prototype.at = function( nIndex )
{
    return this.aEventArray[ nIndex ];
};

NextEffectEventArray.prototype.appendEvent = function( aEvent )
{
    var nSize = this.size();
    for( var i = 0; i < nSize; ++i )
    {
        if( this.aEventArray[i].getId() == aEvent.getId() )
        {
            aNextEffectEventArrayDebugPrinter.print( 'NextEffectEventArray.appendEvent: event already present' );
            return false;
        }
    }
    this.aEventArray.push( aEvent );
    aNextEffectEventArrayDebugPrinter.print( 'NextEffectEventArray.appendEvent: event appended' );
        return true;
    };

    NextEffectEventArray.prototype.clear = function( )
    {
        this.aEventArray = new Array();
    };



    //------------------------------------------------------------------------------------------- //
function TimerEventQueue( aTimer )
{
    this.aTimer = aTimer;
    this.aEventSet = new PriorityQueue( EventEntry.compare );
}


TimerEventQueue.prototype.addEvent = function( aEvent )
{
    this.DBG( 'TimerEventQueue.addEvent invoked' );
    if( !aEvent )
    {
        log( 'error: TimerEventQueue.addEvent: null event' );
        return false;
    }

    var nTime = aEvent.getActivationTime( this.aTimer.getElapsedTime() );
    var aEventEntry = new EventEntry( aEvent, nTime );
    this.aEventSet.push( aEventEntry );

    return true;
};

TimerEventQueue.prototype.process = function()
{
    var nCurrentTime = this.aTimer.getElapsedTime();

    while( !this.isEmpty() && ( this.aEventSet.top().nActivationTime <= nCurrentTime ) )
    {
        var aEventEntry = this.aEventSet.top();
        this.aEventSet.pop();

        var aEvent = aEventEntry.aEvent;
        if( aEvent.isCharged() )
            aEvent.fire();
    }
};

TimerEventQueue.prototype.isEmpty = function()
{
    return this.aEventSet.isEmpty();
};

TimerEventQueue.prototype.nextTimeout = function()
{
    var nTimeout = Number.MAX_VALUE;
    var nCurrentTime = this.aTimer.getElapsedTime();
    if( !this.isEmpty() )
        nTimeout = this.aEventSet.top().nActivationTime - nCurrentTime;
    return nTimeout;
};

TimerEventQueue.prototype.clear = function()
{
    this.DBG( 'TimerEventQueue.clear invoked' );
    this.aEventSet.clear();
};

TimerEventQueue.prototype.getTimer = function()
{
    return this.aTimer;
};

TimerEventQueue.prototype.DBG = function( sMessage, nTime )
{
    aTimerEventQueueDebugPrinter.print( sMessage, nTime );
};


TimerEventQueue.prototype.insert = function( aEventEntry )
{
    var nHoleIndex = this.aEventSet.length;
    var nParent = Math.floor( ( nHoleIndex - 1 ) / 2 );

    while( ( nHoleIndex > 0 ) && this.aEventSet[ nParent ].compare( aEventEntry ) )
    {
        this.aEventSet[ nHoleIndex ] = this.aEventSet[ nParent ];
        nHoleIndex = nParent;
        nParent = Math.floor( ( nHoleIndex - 1 ) / 2 );
    }
    this.aEventSet[ nHoleIndex ] = aEventEntry;
};



// ------------------------------------------------------------------------------------------ //
function EventEntry( aEvent, nTime )
{
    this.aEvent = aEvent;
    this.nActivationTime = nTime;
}


EventEntry.compare = function( aLhsEventEntry, aRhsEventEntry )
{
    return ( aLhsEventEntry.nActivationTime > aRhsEventEntry.nActivationTime );
};



// ------------------------------------------------------------------------------------------ //
function ActivityQueue( aTimer )
{
    this.aTimer = aTimer;
    this.aCurrentActivityWaitingSet = new Array();
    this.aCurrentActivityReinsertSet = new Array();
    this.aDequeuedActivitySet = new Array();
}


ActivityQueue.prototype.dispose = function()
{
    var nSize = this.aCurrentActivityWaitingSet.length;
    for( var i = 0; i < nSize; ++i )
        this.aCurrentActivityWaitingSet[i].dispose();

    nSize = this.aCurrentActivityReinsertSet.length;
    for( var i = 0; i < nSize; ++i )
        this.aCurrentActivityReinsertSet[i].dispose();
};

ActivityQueue.prototype.addActivity = function( aActivity )
{
    if( !aActivity )
    {
        log( 'ActivityQueue.addActivity: activity is not valid' );
        return false;
    }

    this.aCurrentActivityWaitingSet.push( aActivity );
    aActivityQueueDebugPrinter.print( 'ActivityQueue.addActivity: activity appended' );
    return true;
};

ActivityQueue.prototype.process = function()
{
    var nSize = this.aCurrentActivityWaitingSet.length;
    var nLag = 0.0;
    for( var i = 0; i < nSize; ++i )
    {
        nLag = Math.max( nLag,this.aCurrentActivityWaitingSet[i].calcTimeLag()  );
    }

    if( nLag > 0.0 )
        this.aTimer.adjustTimer( -nLag, true );


    while( this.aCurrentActivityWaitingSet.length != 0 )
    {
        var aActivity = this.aCurrentActivityWaitingSet.shift();
        var bReinsert = false;

        bReinsert = aActivity.perform();

        if( bReinsert )
        {
            this.aCurrentActivityReinsertSet.push( aActivity );
        }
        else
        {
            this.aDequeuedActivitySet.push( aActivity );
        }
    }

    if( this.aCurrentActivityReinsertSet.length != 0 )
    {
        // TODO: optimization, try to swap reference here
        this.aCurrentActivityWaitingSet = this.aCurrentActivityReinsertSet;
        this.aCurrentActivityReinsertSet = new Array();
    }
};

ActivityQueue.prototype.processDequeued = function()
{
    // notify all dequeued activities from last round
    var nSize = this.aDequeuedActivitySet.length;
    for( var i = 0; i < nSize; ++i )
        this.aDequeuedActivitySet[i].dequeued();

    this.aDequeuedActivitySet = new Array();
};

ActivityQueue.prototype.isEmpty = function()
{
    return ( ( this.aCurrentActivityWaitingSet.length == 0 ) &&
             ( this.aCurrentActivityReinsertSet.length == 0 ) );
};

ActivityQueue.prototype.clear = function()
{
    aActivityQueueDebugPrinter.print( 'ActivityQueue.clear invoked' );
    var nSize = this.aCurrentActivityWaitingSet.length;
    for( var i = 0; i < nSize; ++i )
        this.aCurrentActivityWaitingSet[i].dequeued();
    this.aCurrentActivityWaitingSet = new Array();

    nSize = this.aCurrentActivityReinsertSet.length;
    for( var i = 0; i < nSize; ++i )
        this.aCurrentActivityReinsertSet[i].dequeued();
    this.aCurrentActivityReinsertSet = new Array();
};

ActivityQueue.prototype.getTimer = function()
{
    return this.aTimer;
};

ActivityQueue.prototype.size = function()
{
    return ( this.aCurrentActivityWaitingSet.length +
             this.aCurrentActivityReinsertSet.length +
             this.aDequeuedActivitySet.length );
};



// ------------------------------------------------------------------------------------------ //
    function ElapsedTime( aTimeBase )
    {
        this.aTimeBase = aTimeBase;
        this.nLastQueriedTime = 0.0;
        this.nStartTime = this.getCurrentTime();
        this.nFrozenTime = 0.0;
        this.bInPauseMode = false;
        this.bInHoldMode = false;
    }


    ElapsedTime.prototype.getTimeBase = function()
    {
        return aTimeBase;
    };

    ElapsedTime.prototype.reset = function()
    {
        this.nLastQueriedTime = 0.0;
        this.nStartTime = this.getCurrentTime();
        this.nFrozenTime = 0.0;
        this.bInPauseMode = false;
        this.bInHoldMode = false;
    };

    ElapsedTime.prototype.getElapsedTime = function()
    {
        this.nLastQueriedTime = this.getElapsedTimeImpl();
        return this.nLastQueriedTime;
    };

    ElapsedTime.prototype.pauseTimer = function()
    {
        this.nFrozenTime = this.getElapsedTimeImpl();
        this.bInPauseMode = true;
    };

    ElapsedTime.prototype.continueTimer = function()
    {
        this.bInPauseMode = false;

        // stop pausing, time runs again. Note that
    // getElapsedTimeImpl() honors hold mode, i.e. a
    // continueTimer() in hold mode will preserve the latter
    var nPauseDuration = this.getElapsedTimeImpl() - this.nFrozenTime;

    // adjust start time, such that subsequent getElapsedTime() calls
    // will virtually start from m_fFrozenTime.
    this.nStartTime += nPauseDuration;
};

ElapsedTime.prototype.adjustTimer = function( nOffset, bLimitToLastQueriedTime )
{
    if( bLimitToLastQueriedTime == undefined )
        bLimitToLastQueriedTime = true;

    // to make getElapsedTime() become _larger_, have to reduce nStartTime.
    this.nStartTime -= nOffset;

    // also adjust frozen time, this method must _always_ affect the
    // value returned by getElapsedTime()!
    if( this.bInHoldMode || this.bInPauseMode )
        this.nFrozenTime += nOffset;
};

ElapsedTime.prototype.holdTimer = function()
{
    // when called during hold mode (e.g. more than once per time
    // object), the original hold time will be maintained.
    this.nFrozenTime = this.getElapsedTimeImpl();
    this.bInHoldMode = true;
};

ElapsedTime.prototype.releaseTimer = function()
{
    this.bInHoldMode = false;
};

ElapsedTime.prototype.getSystemTime = function()
{
    return ( getCurrentSystemTime() / 1000.0 );
};

ElapsedTime.prototype.getCurrentTime = function()
{
    var nCurrentTime;
    if ( !this.aTimeBase )
    {
        nCurrentTime = this.getSystemTime();
//            if( !isFinite(nCurrentTime) )
//            {
//                log( 'ElapsedTime.getCurrentTime: this.getSystemTime() == ' + nCurrentTime );
//            }
    }
    else
    {
        nCurrentTime = this.aTimeBase.getElapsedTimeImpl();
//            if( !isFinite(nCurrentTime) )
//            {
//                log( 'ElapsedTime.getCurrentTime: this.aTimeBase.getElapsedTimeImpl() == ' + nCurrentTime );
//            }
    }

    assert( ( typeof( nCurrentTime ) === typeof( 0 ) ) && isFinite( nCurrentTime ),
            'ElapsedTime.getCurrentTime: assertion failed: nCurrentTime == ' + nCurrentTime );


    return nCurrentTime;
};

ElapsedTime.prototype.getElapsedTimeImpl = function()
{
    if( this.bInHoldMode || this.bInPauseMode )
    {
//            if( !isFinite(this.nFrozenTime) )
//            {
//                log( 'ElapsedTime.getElapsedTimeImpl: nFrozenTime == ' + this.nFrozenTime );
//            }

        return this.nFrozenTime;
    }

    var nCurTime = this.getCurrentTime();
    return ( nCurTime - this.nStartTime );
};


/*****
 * @libreofficeend
 *
 * Several parts of the above code are the result of the porting,
 * started on August 2011, of the C++ code included in the source files
 * placed under the folder '/slideshow/source' and subfolders.
 * @source http://cgit.freedesktop.org/libreoffice/core/tree/slideshow/source
 *
 */


