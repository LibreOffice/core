/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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
 */

#ifndef INCLUDED_SC_INC_POSTIT_HXX
#define INCLUDED_SC_INC_POSTIT_HXX

#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include "address.hxx"
#include "scdllapi.h"

#include <memory>

class EditTextObject;
class OutlinerParaObject;
class SdrCaptionObj;
class SdrPage;
class ScNoteCaptionCreator;

class SfxItemSet;
class ScDocument;
namespace tools { class Rectangle; }



/** Used for undo. */
struct SC_DLLPUBLIC ScNoteDataSaved
{
    sal_uInt32   mnPostItId;
    OUString     maDate;             /// Creation date of the note.
    OUString     maAuthor;           /// Author of the note.
    std::unique_ptr< SfxItemSet >       mxItemSet;          /// Caption object formatting.
    std::unique_ptr< OutlinerParaObject >  mxOutlinerObj;      /// Text object with all text portion formatting.
    OUString     maSimpleText;       /// Simple text without formatting.
    Point        maCaptionOffset;    /// Caption position relative to cell corner.
    Size         maCaptionSize;      /// Size of the caption object.
    bool         mbDefaultPosSize;   /// True = use default position and size for caption.

    ScNoteDataSaved();
    ScNoteDataSaved(ScNoteDataSaved&&);
    ~ScNoteDataSaved();
    ScNoteDataSaved& operator=(ScNoteDataSaved&&);
};

/**
 * Additional class containing cell annotation data.
 */
class SC_DLLPUBLIC ScPostIt
{
friend class ScNoteCaptionCreator;
public:

    /** Creates an empty note. */
    explicit            ScPostIt( ScDocument& rDoc );

    /** Copy constructor. Clones the note to a new document. */
    explicit            ScPostIt( ScDocument& rDoc, const ScPostIt& rNote  );

    /** Creates note from saved data, used by undo. */
    explicit            ScPostIt( ScDocument& rDoc, const ScNoteDataSaved& rNoteData  );

    /** create note from already visible, temporary caption */
    explicit            ScPostIt( ScDocument& rDoc, SdrCaptionObj* pCaption );

    explicit            ScPostIt( ScDocument& rDoc,
                            std::unique_ptr<SfxItemSet> pItemSet,
                            OutlinerParaObject* pOutlinerObj,
                            Point aCaptionOffset, Size aCaptionSize, bool bDefaultPosSize);

    explicit            ScPostIt( ScDocument& rDoc, const OUString& rNoteText);

    /** Removes the caption object from drawing layer, if this note is its owner. */
                        ~ScPostIt();

    std::unique_ptr<ScPostIt> Clone( ScDocument& rDestDoc ) const;

    /** Returns the note id. */
    sal_uInt32 GetId() const { return mnPostItId; }

    /** Returns the creation date of this note. */
    const OUString& GetDate() const { return maDate;}
    /** Sets a new creation date for this note. */
    void SetDate( const OUString& rDate );

    /** Returns the author date of this note. */
    const OUString& GetAuthor() const { return maAuthor;}
    /** Sets a new author date for this note. */
    void SetAuthor( const OUString& rAuthor );

    /** Sets date and author from system settings. */
    void                AutoStamp();

    /** Returns the pointer to the current outliner object, or null. */
    const OutlinerParaObject* GetOutlinerObject() const;
    /** Returns the pointer to the current edit text object, or null. */
    const EditTextObject* GetEditTextObject() const;

    /** Returns the caption text of this note. */
    OUString            GetText() const;
    /** Returns true, if the caption text of this note contains line breaks. */
    bool                HasMultiLineText() const;
    /** Changes the caption text of this note. All text formatting will be lost. */
    void                SetText( const OUString& rText );

    /** Update note from saved data, used by undo. */
    void                Update( const ScNoteDataSaved& rNoteData );

    /** Returns the caption object of this note. Creates the caption object, if
        the note contains initial caption data instead of the caption.
        The SdrCaptionObj* returned is still managed by the underlying
        ScNoteData::ScCaptionPtr and must not be stored elsewhere. */
    SdrCaptionObj*      GetOrCreateCaption( const ScAddress& rPos ) const;

    // only valid to call this when the caption is visible
    SdrCaptionObj*      GetShownCaption() const;

    // only valid to call this when the caption is invisible
    ScNoteDataSaved     GetNoteData() const;

    /** Shows or hides the note caption object. */
    void                ShowCaption( const ScAddress& rPos, bool bShow );
    void                ShowCaption( const ScAddress& rPos );
    void                HideCaption();
    /** Returns true, if the caption object is visible. */
    bool                IsCaptionShown() const { return mbShown; }

    /** Updates caption position according to position of the passed cell. */
    void                UpdateCaptionPos( const ScAddress& rPos );

private:
                        ScPostIt( const ScPostIt& ) = delete;
    ScPostIt&           operator=( const ScPostIt& ) = delete;

    ScDocument&         mrDoc;              /// Parent document containing the note.
    sal_uInt32          mnPostItId;
    OUString            maDate;             /// Creation date of the note.
    OUString            maAuthor;           /// Author of the note.

    /// The lifecycle is strictly controlled as such:
    /// When mbShown is true, we have an mxCaption pointer, and the maInitData does not exist.
    /// When mbShown is false, we have no mxCaption, and the maInitData is valid.
    /// This means that we never have mxCaption when on the undo stack, or on the clip document.
    bool                mbShown;            /// True = note is visible.
    struct ScInitData {
        std::unique_ptr< SfxItemSet >       mxItemSet;          /// Caption object formatting.
        std::unique_ptr< OutlinerParaObject >  mxOutlinerObj;      /// Text object with all text portion formatting.
        OUString     maSimpleText;       /// Simple text without formatting.
        Point        maCaptionOffset;    /// Caption position relative to cell corner.
        Size         maCaptionSize;      /// Size of the caption object.
        bool         mbDefaultPosSize;   /// True = use default position and size for caption.
    };
    union {
        SdrCaptionObj* mpCaption;   /// Drawing object representing the cell note.
        ScInitData     maInitData;
    };

    static sal_uInt32 mnLastPostItId;
};

class SC_DLLPUBLIC ScNoteUtil
{
public:

    /** Creates and returns a caption object for a temporary caption. */
    static SdrCaptionObj* CreateTempCaption( ScDocument& rDoc, const ScAddress& rPos,
                            SdrPage& rDrawPage, const OUString& rUserText,
                            const tools::Rectangle& rVisRect, bool bTailFront );

    /** Creates a cell note using the passed caption drawing object.

        This function is used in import filters to reuse the imported drawing
        object as note caption object.

        @param pCaption  The drawing object for the cell note. This object MUST
            be inserted into the document at the correct drawing page already.
            The underlying ScPostIt::ScNoteData::ScCaptionPtr takes managing
            ownership of the pointer.

        @return  Pointer to the new cell note object if insertion was
            successful (i.e. the passed cell position was valid), null
            otherwise. The Calc document is the owner of the note object. The
            passed item set and outliner object are deleted automatically if
            creation of the note was not successful.
     */
    static ScPostIt*    CreateNoteFromCaption(
                            ScDocument& rDoc, const ScAddress& rPos,
                            SdrCaptionObj* pCaption );

    /** Creates a cell note based on the passed caption object data.

        This function is used in import filters to use an existing imported
        item set and outliner object to create a note caption object. For
        performance reasons, it is possible to specify that the caption drawing
        object for the cell note is not created yet but the note caches the
        passed data needed to create the caption object on demand (see
        parameter bAlwaysCreateCaption).

        @param pItemSet  Pointer to an item set on heap memory containing all
            formatting attributes of the caption object. This function takes
            ownership of the passed item set.

        @param pOutlinerObj  Pointer to an outliner object on heap memory
            containing (formatted) text for the caption object. This function
            takes ownership of the passed outliner object.

        @param rCaptionRect  The absolute position and size of the caption
            object. The rectangle may be empty, in this case the default
            position and size is used.

        @return  Pointer to the new cell note object if insertion was
            successful (i.e. the passed cell position was valid), null
            otherwise. The Calc document is the owner of the note object.
     */
    static ScPostIt*    CreateNoteFromObjectData(
                            ScDocument& rDoc, const ScAddress& rPos,
                            std::unique_ptr<SfxItemSet> pItemSet, OutlinerParaObject* pOutlinerObj,
                            const tools::Rectangle& rCaptionRect, bool bShown );

    /** Creates a cell note based on the passed string and inserts it into the
        document.

        @param rNoteText  The text used to create the note caption object. Must
            not be empty.

        @return  Pointer to the new cell note object if insertion was
            successful (i.e. the passed cell position was valid), null
            otherwise. The Calc document is the owner of the note object.
     */
    static ScPostIt*    CreateNoteFromString(
                            ScDocument& rDoc, const ScAddress& rPos,
                            const OUString& rNoteText, bool bShown );

};

namespace sc {

struct NoteEntry
{
    ScAddress maPos;
    const ScPostIt* mpNote;

    NoteEntry( const ScAddress& rPos, const ScPostIt* pNote ) : maPos(rPos), mpNote(pNote) {}
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
