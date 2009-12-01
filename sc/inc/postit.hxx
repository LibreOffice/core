/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: postit.hxx,v $
 * $Revision: 1.7.128.9 $
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

#ifndef SC_POSTIT_HXX
#define SC_POSTIT_HXX

#include <tools/gen.hxx>
#include "address.hxx"
#include "scdllapi.h"

class EditTextObject;
class SdrCaptionObj;
class SdrPage;
class SfxItemSet;
class ScDocument;

// ============================================================================

struct SC_DLLPUBLIC ScNoteData
{
    String              maDate;             /// Creation date of the note.
    String              maAuthor;           /// Author of the note.
    SdrCaptionObj*      mpCaption;          /// Drawing object representing the cell note.
    bool                mbShown;            /// True = note is visible.

    explicit            ScNoteData( bool bShown = false );
};

// ============================================================================

class SC_DLLPUBLIC ScPostIt
{
public:
    /** Creates an empty note and its caption object and places it according to
        the passed cell position. */
    explicit            ScPostIt( ScDocument& rDoc, const ScAddress& rPos, bool bShown );

    /** Copy constructor. Clones the note and its caption to a new document. */
    explicit            ScPostIt( ScDocument& rDoc, const ScAddress& rPos, const ScPostIt& rNote );

    /** Creates a note from the passed note data with existing caption object. */
    explicit            ScPostIt( ScDocument& rDoc, const ScNoteData& rNoteData );

    /** Removes the caption object from drawing layer, if this note is its owner. */
                        ~ScPostIt();

    /** Returns the data struct containing note settings. */
    inline const ScNoteData& GetNoteData() const { return maNoteData; }

    /** Returns the creation date of this note. */
    inline const String& GetDate() const { return maNoteData.maDate; }
    /** Sets a new creation date for this note. */
    inline void         SetDate( const String& rDate ) { maNoteData.maDate = rDate; }

    /** Returns the author date of this note. */
    inline const String& GetAuthor() const { return maNoteData.maAuthor; }
    /** Sets a new author date for this note. */
    inline void         SetAuthor( const String& rAuthor ) { maNoteData.maAuthor = rAuthor; }

    /** Sets date and author from system settings. */
    void                AutoStamp();

    /** Returns the pointer to the current edit text object, or null. */
    const EditTextObject* GetEditTextObject() const;
    /** Returns the caption text of this note. */
    String              GetText() const;
    /** Returns true, if the caption text of this note contains line breaks. */
    bool                HasMultiLineText() const;
    /** Changes the caption text of this note. All text formatting will be lost. */
    void                SetText( const String& rText );

    /** Returns the note caption object. */
    inline SdrCaptionObj* GetCaption() const { return maNoteData.mpCaption; }
    /** Returns and forgets the note caption object. */
    inline SdrCaptionObj* ForgetCaption() { SdrCaptionObj* pCapt = maNoteData.mpCaption; maNoteData.mpCaption = 0; return pCapt; }

    /** Shows or hides the note caption object. */
    void                ShowCaption( bool bShow = true );
    /** Hides the note caption object. */
    inline void         HideCaption() { ShowCaption( false ); }
    /** Returns true, if the caption object is visible. */
    inline bool         IsCaptionShown() const { return maNoteData.mbShown; }

    /** Shows or hides the caption temporarily (does not change internal visibility state). */
    void                ShowCaptionTemp( bool bShow = true );
    /** Hides caption if it has been shown temporarily (does not change internal visibility state). */
    inline void         HideCaptionTemp() { ShowCaptionTemp( false ); }

    /** Updates caption position according to position of the passed cell. */
    void                UpdateCaptionPos( const ScAddress& rPos );

    /** Sets caption itemset to default items. */
    void                SetCaptionDefaultItems();
    /** Updates caption itemset according to the passed item set while removing shadow items. */
    void                SetCaptionItems( const SfxItemSet& rItemSet );

private:
                        ScPostIt( const ScPostIt& );
    ScPostIt&           operator=( const ScPostIt& );

    /** Creates a new caption object at the passed cell position, clones passed existing caption. */
    void                CreateCaption( const ScAddress& rPos, const SdrCaptionObj* pCaption = 0 );
    /** Removes the caption object from the drawing layer, if this note is its owner. */
    void                RemoveCaption();
    /** Updates caption visibility. */
    void                UpdateCaptionLayer( bool bShow );

private:
    ScDocument&         mrDoc;              /// Parent document containing the note.
    ScNoteData          maNoteData;         /// Note data with pointer to caption object.
};

// ============================================================================

class SC_DLLPUBLIC ScNoteUtil
{
public:
    /** Clones the note and its caption object, if specified.
        @param bCloneCaption  True = clones the caption object and inserts it
            into the drawing layer of the destination document. False = the
            cloned note will refer to the old caption object. */
    static ScPostIt*    CloneNote( ScDocument& rDoc, const ScAddress& rPos,
                            const ScPostIt& rNote, bool bCloneCaption );

    /** Tries to update the position of note caption objects in the specified range. */
    static void         UpdateCaptionPositions( ScDocument& rDoc, const ScRange& rRange );

    /** Creates and returns a caption object for a temporary caption. */
    static SdrCaptionObj* CreateTempCaption( ScDocument& rDoc, const ScAddress& rPos,
                            SdrPage& rPage, const String& rUserText,
                            const Rectangle& rVisRect, bool bTailFront );

    /** Creates a cell note based on the passed string and inserts it into the document. */
    static ScPostIt*    CreateNoteFromString( ScDocument& rDoc, const ScAddress& rPos,
                            const String& rNoteText, bool bShown );
};

// ============================================================================

#endif
