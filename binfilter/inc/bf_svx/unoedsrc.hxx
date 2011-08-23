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

#ifndef _SVX_UNOEDSRC_HXX
#define _SVX_UNOEDSRC_HXX

#include <tools/solar.h>
#include <i18npool/lang.h>
#include <vcl/mapmod.hxx>

#include <bf_svtools/itempool.hxx>
class OutputDevice;
class String;
namespace binfilter {

class SvUShorts;
class SfxItemSet;
class SfxBroadcaster;

struct ESelection;
struct EFieldInfo;
struct EBulletInfo;
class SvxTextForwarder;
class SvxViewForwarder;
class SvxEditViewForwarder;
class SvxFieldItem;
class EECharAttribArray;

/**	Wrapper class for unified EditEngine/Outliner access

    This class wraps a textual object, which might or might not
    contain an EditEngine/Outliner. Is used e.g. for filling an
    EditEngine with the content of a cell, note page or page template.
*/
class SvxEditSource
{
public:
    virtual					~SvxEditSource();

    /// Returns a new reference to the same object. This is a shallow copy
    virtual SvxEditSource*		Clone() const = 0;

    /** Query the text forwarder

        @return the text forwarder, or NULL if the underlying object is dead
     */
    virtual SvxTextForwarder*	GetTextForwarder() = 0;

    /** Query the view forwarder    	

        @return the view forwarder, or NULL if the underlying object
        is dead, or if no view is available

        @derive default implementation provided, overriding is optional
     */
     virtual SvxViewForwarder*	GetViewForwarder();

    /** Query the edit view forwarder

        @param bCreate
        Determines whether an EditView should be created, if there is
        none active. If set to sal_True, and the underlying object is
        not in EditMode, the text forwarder changes and the object is
        set to EditMode.

        @return the edit view forwarder, or NULL if the underlying
        object is dead, or if no view is available (if bCreate is
        sal_False, NULL is also returned if the object is not in
        EditMode)

        @derive default implementation provided, overriding is optional

        @attention If this method is called with bCreate equal to
        sal_True, all previously returned text forwarder can become
        invalid
     */
     virtual SvxEditViewForwarder*	GetEditViewForwarder( sal_Bool bCreate = sal_False );

    /// Write back data to model
    virtual void				UpdateData() = 0;

    /** Returns broadcaster the underlying edit engine's events are sent from

        @derive default implementation provided, overriding is optional
     */
    virtual SfxBroadcaster&		GetBroadcaster() const;

};

/**	Contains an EditEngine or an Outliner and unifies access to them.

    The EditEngine-UNO objects use this class only. To reflect changes
    not only in the EditEngine, but also in the model data, call
    SvxEditSource::UpdateData(). This copies back the data to the model's
    EditTextObject/OutlinerParaObject.
 */
class SvxTextForwarder
{
public:
    virtual				~SvxTextForwarder();

    virtual USHORT		GetParagraphCount() const = 0;
    virtual USHORT		GetTextLen( USHORT nParagraph ) const = 0;
    virtual String		GetText( const ESelection& rSel ) const = 0;
    virtual SfxItemSet	GetAttribs( const ESelection& rSel, BOOL bOnlyHardAttrib = 0 ) const = 0;
    virtual	SfxItemSet  GetParaAttribs( USHORT nPara ) const = 0;
    virtual void		SetParaAttribs( USHORT nPara, const SfxItemSet& rSet ) = 0;
    virtual void		GetPortions( USHORT nPara, SvUShorts& rList ) const = 0;

    virtual USHORT		GetItemState( const ESelection& rSel, USHORT nWhich ) const = 0;
    virtual USHORT		GetItemState( USHORT nPara, USHORT nWhich ) const = 0;

    virtual void		QuickInsertText( const String& rText, const ESelection& rSel ) = 0;
    virtual void		QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel ) = 0;
    virtual void		QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel ) = 0;
    virtual void		QuickInsertLineBreak( const ESelection& rSel ) = 0;

    virtual XubString    CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor ) = 0;

    virtual SfxItemPool* GetPool() const = 0;

    /** Query state of forwarder

        @return sal_False, if no longer valid
     */
    virtual BOOL			IsValid() const = 0;

    /** Query language of character at given position on the underlying edit engine

        @param nPara[0 .. n-1]
        Index of paragraph to query language in

        @param nIndex[0 .. m-1]
        Index of character to query language of
     */
    virtual LanguageType	GetLanguage( USHORT nPara, USHORT nIndex ) const = 0;

    /** Query number of fields in the underlying edit engine

        @param nPara[0 .. n-1]
        Index of paragraph to query field number in
     */
    virtual USHORT			GetFieldCount( USHORT nPara ) const = 0;

    /** Query information for given field number in the underlying edit engine

        @param nPara[0 .. n-1]
        Index of paragraph to query field info in

        @param nField[0 .. m-1]
        Index of field to query information of
     */
    virtual EFieldInfo		GetFieldInfo( USHORT nPara, USHORT nField ) const = 0;

    /** Query information regarding bullets for given paragraph on the underlying edit engine

        @param nPara[0 .. n-1]
        Index of paragraph to query bullet info on
     */
    virtual EBulletInfo     GetBulletInfo( USHORT nPara ) const = 0;

    /** Query the bounding rectangle of the given character 

        @param nPara[0 .. n]
        Index of paragraph to query the bounds in.  <p>The virtual
        character after the last character of the represented text,
        i.e. the one at position n is a special case.  Because it does
        not represent an existing character its bounding box is
        defined in relation to preceding characters.  It should be
        rougly equivalent to the bounding box of some character when
        inserted at the end of the text.  Its height typically being
        the maximal height of all the characters in the text or the
        height of the preceding character, its width being at least
        one pixel so that the bounding box is not degenerate.<br>

        @param nIndex[0 .. m-1]
        Index of character to query the bounds of

        @return rectangle in logical coordinates, relative to upper
        left corner of text. The coordinates returned here are to be
        interpreted in the map mode given by GetMapMode().
    */
    virtual Rectangle		GetCharBounds( USHORT nPara, USHORT nIndex ) const = 0;

    /** Query the bounding rectangle of the given paragraph

        @param nPara[0 .. n-1]
        Index of paragraph to query the bounds of

        @return rectangle in logical coordinates, relative to upper
        left corner of text. The coordinates returned here are to be
        interpreted in the map mode given by GetMapMode().
     */
    virtual Rectangle		GetParaBounds( USHORT nPara ) const = 0;

    /** Query the map mode of the underlying EditEngine/Outliner

        @return the map mode used on the EditEngine/Outliner. The
        values returned by GetParaBounds() and GetCharBounds() are to
        be interpreted in this map mode, the point given to
        GetIndexAtPoint() is interpreted in this map mode.
     */
    virtual MapMode			GetMapMode() const = 0;

    /** Query the reference output device of the underlying EditEngine/Outliner

        @return the OutputDevice used from the EditEngine/Outliner to
        format the text. It should be used when performing e.g. font
        calculations, since this is usually a printer with fonts and
        resolution different from the screen.
     */
    virtual OutputDevice*	GetRefDevice() const = 0;

    /** Query paragraph and character index of the character at the
        given point. Returns sal_True on success, sal_False otherwise

        @param rPoint
        Point to query text position of. Is interpreted in logical
        coordinates, relativ to the upper left corner of the text, and
        in the map mode given by GetMapMode()

        @param rPara[0 .. n-1]
        Index of paragraph the point is within

        @param rIndex[0 .. m-1]
        Index of character the point is over

        @return sal_True, if the point is over any text and both rPara and rIndex are valid

     */
    virtual sal_Bool		GetIndexAtPoint( const Point& rPoint, USHORT& rPara, USHORT& rIndex ) const = 0;

    /** Get the start and the end index of the word at the given index

        An index value on a word leads from the first character of
        that word up to and including the last space before the next
        word. The index values returned do not contain any leading or
        trailing white-space. If the input indices are invalid,
        sal_False is returned.

        @param nPara[0 .. n-1]
        Index of paragraph to start the search in

        @param nIndex[0 .. m-1]
        Index of character to query the search on

        @param rStart
        Start index (in the same paragraph)

        @param rEnd
        End index (in the same paragraph), this point to the last
        character still contained in the query

        @return sal_True, if the result is non-empty
     */
    virtual sal_Bool		GetWordIndices( USHORT nPara, USHORT nIndex, USHORT& rStart, USHORT& rEnd ) const = 0;

    /** Query range of similar attributes

        Please note that the range returned is half-open: [nStartIndex,nEndIndex)

        @param nStartIndex 
        Herein, the start index of the range of similar attributes is returned

        @param nEndIndex 
        Herein, the end index (exclusive) of the range of similar attributes is returned

        @param nIndex
        The character index from which the range of similar attributed characters is requested

        @return sal_True, if the range has been successfully determined
     */
    virtual sal_Bool 		GetAttributeRun( USHORT& nStartIndex, USHORT& nEndIndex, USHORT nPara, USHORT nIndex ) const = 0;

    /** Query number of lines in the formatted paragraph

        @param nPara[0 .. n-1]
        Index of paragraph to query number of lines in

        @return number of lines in given paragraph

     */
    virtual USHORT			GetLineCount( USHORT nPara ) const = 0;

    /** Query line length

        @param nPara[0 .. n-1]
        Index of paragraph to query line length in

        @param nLine[0 .. m-1]
        Index of line in paragraph to query line length of

     */
    virtual USHORT			GetLineLen( USHORT nPara, USHORT nLine ) const = 0;

    /** Delete given text range and reformat text

        @param rSelection
        The text range to be deleted

        @return sal_True if text has been successfully deleted
     */
    virtual sal_Bool		Delete( const ESelection& rSelection ) = 0;

     /** Insert/Replace given text in given range and reformat text

          @param rText 
         Text to insert/replace

         @param rSel
         Selection where text should be replaced. The empty selection inserts

         @return sal_True if text has been successfully inserted
      */
    virtual sal_Bool		InsertText( const String& rText, const ESelection& rSel ) = 0;

     /** Updates the formatting

          @see EditEngine::QuickFormatDoc() for details

         @return sal_True if text have been successfully reformatted
      */
    virtual sal_Bool		QuickFormatDoc( BOOL bFull=FALSE ) = 0;

    /** Get the outline depth of given paragraph

        @param nPara
        Index of the paragraph to query the depth of

        @return the outline level of the given paragraph. The range is
        [0,n), where n is the maximal outline level.
     */
    virtual USHORT			GetDepth( USHORT nPara ) const = 0;

    /** Set the outline depth of given paragraph

        @param nPara
        Index of the paragraph to set the depth of

        @param nNewDepth
        The depth to set on the given paragraph. The range is
        [0,n), where n is the maximal outline level.

        @return TRUE, if depth could be successfully set. Reasons for
        failure are e.g. the text does not support outline level
        (EditEngine), or the depth range is exceeded.
     */
    virtual sal_Bool		SetDepth( USHORT nPara, USHORT nNewDepth ) = 0;

};

/** Encapsulates the document view for the purpose of unified
    EditEngine/Outliner access. 

    This one has to be different from the SvxEditViewForwarder, since
    the latter is only valid in edit mode.
 */
class SvxViewForwarder
{
public:
    virtual				~SvxViewForwarder();

    /** Query state of forwarder

        @return sal_False, if no longer valid
     */
    virtual BOOL		IsValid() const = 0;

    /** Query visible area of the view containing the text    	

        @return the visible rectangle of the text, i.e. the part of
        the EditEngine or Outliner that is currently on screen. The
        values are already in screen coordinates (pixel), and have to
        be relative to the EditEngine/Outliner's upper left corner.
     */
    virtual Rectangle	GetVisArea() const = 0;

    /** Convert from logical, EditEngine-relative coordinates to screen coordinates 

        @param rPoint
        Point in logical, EditEngine-relative coordinates.

        @param rMapMode
        The map mode to interpret the coordinates in.

        @return the point in screen coordinates
     */
    virtual Point		LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const = 0;

    /** Convert from screen to logical, EditEngine-relative coordinates

        @param rPoint
        Point in screen coordinates

        @param rMapMode
        The map mode to interpret the coordinates in.

        @return the point in logical coordinates.
     */
    virtual Point		PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const = 0;

};

/** Encapsulates EditView and OutlinerView for the purpose of unified
    EditEngine/Outliner access
 */
class SvxEditViewForwarder : public SvxViewForwarder
{
public:

    /** Query current selection. 

        @param rSelection 
        Contains the current selection after method call

        @return sal_False, if there is no view or no selection (the empty selection _is_ a selection)

     */
    virtual sal_Bool GetSelection( ESelection& rSelection ) const = 0;

    /** Set selection in view. 

        @param rSelection
        The selection to set

        @return sal_False, if there is no view or selection is invalid
     */
    virtual sal_Bool SetSelection( const ESelection& rSelection ) = 0;

    /** Copy current selection to clipboard. 

        @return sal_False if no selection or no view (the empty selection _is_ a selection)
     */
    virtual sal_Bool Copy() = 0;

    /** Cut current selection to clipboard. 

        @eturn sal_False if no selection or no view (the empty selection _is_ a selection)
     */
    virtual sal_Bool Cut() = 0;

    /** Paste clipboard into current selection. 

        @return sal_False if no view or no selection (the empty selection _is_ a selection)
     */
    virtual sal_Bool Paste() = 0;

};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
