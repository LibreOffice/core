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

// MyEDITDATA, wegen exportiertem EditData
#ifndef INCLUDED_EDITENG_EDITDATA_HXX
#define INCLUDED_EDITENG_EDITDATA_HXX

#include <rtl/ustring.hxx>
#include <editeng/editengdllapi.h>

class SfxItemSet;
class SfxPoolItem;
class SvParser;
class SvxFieldItem;

enum EETextFormat       { EE_FORMAT_TEXT = 0x20, EE_FORMAT_RTF, EE_FORMAT_BIN = 0x31, EE_FORMAT_HTML, EE_FORMAT_XML };
enum EEHorizontalTextDirection { EE_HTEXTDIR_DEFAULT, EE_HTEXTDIR_L2R, EE_HTEXTDIR_R2L };
enum EESelectionMode    { EE_SELMODE_STD, EE_SELMODE_TXTONLY, EE_SELMODE_HIDDEN };
    // EE_SELMODE_HIDDEN can be used to completely hide the selection. This is useful e.g. when you want show the selection
    // only as long as your window (which the edit view works on) has the focus
enum EESpellState       { EE_SPELL_OK, EE_SPELL_NOLANGUAGE, EE_SPELL_LANGUAGENOTINSTALLED, EE_SPELL_NOSPELLER, EE_SPELL_ERRORFOUND };
enum EVAnchorMode       {
            ANCHOR_TOP_LEFT,    ANCHOR_VCENTER_LEFT,    ANCHOR_BOTTOM_LEFT,
            ANCHOR_TOP_HCENTER, ANCHOR_VCENTER_HCENTER, ANCHOR_BOTTOM_HCENTER,
            ANCHOR_TOP_RIGHT,   ANCHOR_VCENTER_RIGHT,   ANCHOR_BOTTOM_RIGHT };

#define EE_PARA_NOT_FOUND       SAL_MAX_INT32
#define EE_PARA_APPEND          SAL_MAX_INT32
#define EE_PARA_ALL             SAL_MAX_INT32
#define EE_PARA_MAX_COUNT       SAL_MAX_INT32

#define EE_INDEX_NOT_FOUND      SAL_MAX_INT32
#define EE_TEXTPOS_ALL          SAL_MAX_INT32
#define EE_TEXTPOS_MAX_COUNT    SAL_MAX_INT32

EDITENG_DLLPUBLIC extern const size_t EE_APPEND;

// Error messages for Read / Write Method
#define EE_READWRITE_OK              (SVSTREAM_OK)
#define EE_READWRITE_WRONGFORMAT     (SVSTREAM_ERRBASE_USER+1)
#define EE_READWRITE_GENERALERROR    (SVSTREAM_ERRBASE_USER+2)

#define EDITUNDO_REMOVECHARS        100
#define EDITUNDO_CONNECTPARAS       101
#define EDITUNDO_MOVEPARAGRAPHS     103
#define EDITUNDO_INSERTFEATURE      104
#define EDITUNDO_SPLITPARA          105
#define EDITUNDO_INSERTCHARS        106
#define EDITUNDO_DELCONTENT         107
#define EDITUNDO_DELETE             108
#define EDITUNDO_CUT                109
#define EDITUNDO_PASTE              110
#define EDITUNDO_INSERT             111
#define EDITUNDO_MOVEPARAS          113
#define EDITUNDO_PARAATTRIBS        114
#define EDITUNDO_ATTRIBS            115
#define EDITUNDO_DRAGANDDROP        116
#define EDITUNDO_READ               117
#define EDITUNDO_STYLESHEET         118
#define EDITUNDO_REPLACEALL         119
#define EDITUNDO_RESETATTRIBS       121
#define EDITUNDO_INDENTBLOCK        122
#define EDITUNDO_UNINDENTBLOCK      123
#define EDITUNDO_MARKSELECTION      124
#define EDITUNDO_TRANSLITERATE      125

#define EDITUNDO_USER               200

class EditView;
class EditEngine;
class ImpEditView;
class ImpEditEngine;
class EditTextObject;
class SfxStyleSheet;

struct EPosition
{
    sal_Int32   nPara;
    sal_Int32   nIndex;

    EPosition()
        : nPara( EE_PARA_NOT_FOUND )
        , nIndex( EE_INDEX_NOT_FOUND )
        { }

    EPosition( sal_Int32 nPara_, sal_Int32 nPos_ )
        : nPara( nPara_ )
        , nIndex( nPos_ )
        { }
};

struct ESelection
{
    sal_Int32   nStartPara;
    sal_Int32   nStartPos;
    sal_Int32   nEndPara;
    sal_Int32   nEndPos;

    ESelection() : nStartPara( 0 ), nStartPos( 0 ), nEndPara( 0 ), nEndPos( 0 ) {}

    ESelection( sal_Int32 nStPara, sal_Int32 nStPos,
                sal_Int32 nEPara, sal_Int32 nEPos )
        : nStartPara( nStPara )
        , nStartPos( nStPos )
        , nEndPara( nEPara )
        , nEndPos( nEPos )
        { }

    ESelection( sal_Int32 nPara, sal_Int32 nPos )
        : nStartPara( nPara )
        , nStartPos( nPos )
        , nEndPara( nPara )
        , nEndPos( nPos )
        { }

    void    Adjust();
    bool    IsEqual( const ESelection& rS ) const;
    bool    IsLess( const ESelection& rS ) const;
    bool    IsGreater( const ESelection& rS ) const;
    bool    IsZero() const;
    bool    HasRange() const;
};

inline bool ESelection::HasRange() const
{
    return ( nStartPara != nEndPara ) || ( nStartPos != nEndPos );
}

inline bool ESelection::IsZero() const
{
    return ( ( nStartPara == 0 ) && ( nStartPos == 0 ) &&
             ( nEndPara == 0 ) && ( nEndPos == 0 ) );
}

inline bool ESelection::IsEqual( const ESelection& rS ) const
{
    return ( ( nStartPara == rS.nStartPara ) && ( nStartPos == rS.nStartPos ) &&
             ( nEndPara == rS.nEndPara ) && ( nEndPos == rS.nEndPos ) );
}

inline bool ESelection::IsLess( const ESelection& rS ) const
{
    // The selection must be adjusted.
    // => Only check if end of 'this' < Start of rS
    return ( nEndPara < rS.nStartPara ) ||
        ( ( nEndPara == rS.nStartPara ) && ( nEndPos < rS.nStartPos ) && !IsEqual( rS ) );
}

inline bool ESelection::IsGreater( const ESelection& rS ) const
{
    // The selection must be adjusted.
    // => Only check if end of 'this' < Start of rS
    return ( nStartPara > rS.nEndPara ) ||
        ( ( nStartPara == rS.nEndPara ) && ( nStartPos > rS.nEndPos ) && !IsEqual( rS ) );
}

inline void ESelection::Adjust()
{
    bool bSwap = false;
    if ( nStartPara > nEndPara )
        bSwap = true;
    else if ( ( nStartPara == nEndPara ) && ( nStartPos > nEndPos ) )
        bSwap = true;

    if ( bSwap )
    {
        sal_Int32  nSPar = nStartPara; sal_Int32 nSPos = nStartPos;
        nStartPara = nEndPara; nStartPos = nEndPos;
        nEndPara = nSPar; nEndPos = nSPos;
    }
}

struct EDITENG_DLLPUBLIC EFieldInfo
{
    SvxFieldItem*   pFieldItem;
    OUString        aCurrentText;
    EPosition       aPosition;

    EFieldInfo();
    EFieldInfo( const SvxFieldItem& rFieldItem, sal_Int32 nPara, sal_Int32 nPos );
    ~EFieldInfo();

    EFieldInfo( const EFieldInfo& );
    EFieldInfo& operator= ( const EFieldInfo& );
};

enum ImportState {
                    RTFIMP_START, RTFIMP_END,               // only pParser, nPara, nIndex
                    RTFIMP_NEXTTOKEN, RTFIMP_UNKNOWNATTR,   // nToken+nTokenValue
                    RTFIMP_SETATTR,                         // pAttrs
                    RTFIMP_INSERTTEXT,                      // aText
                    RTFIMP_INSERTPARA,                      // -
                    HTMLIMP_START, HTMLIMP_END,             // only pParser, nPara, nIndex
                    HTMLIMP_NEXTTOKEN, HTMLIMP_UNKNOWNATTR, // nToken
                    HTMLIMP_SETATTR,                        // pAttrs
                    HTMLIMP_INSERTTEXT,                     // aText
                    HTMLIMP_INSERTPARA, HTMLIMP_INSERTFIELD // -
                    };

struct ImportInfo
{
    SvParser*               pParser;
    ESelection              aSelection;
    ImportState             eState;

    int                     nToken;
    short                   nTokenValue;

    OUString                aText;

    void*                   pAttrs; // RTF: SvxRTFItemStackType*, HTML: SfxItemSet*

    ImportInfo( ImportState eState, SvParser* pPrsrs, const ESelection& rSel );
    ~ImportInfo();
};

struct ParagraphInfos
{
    ParagraphInfos()
        : nParaHeight( 0 )
        , nLines( 0 )
        , nFirstLineStartX( 0 )
        , nFirstLineOffset( 0 )
        , nFirstLineHeight( 0 )
        , nFirstLineTextHeight ( 0 )
        , nFirstLineMaxAscent( 0 )
        , bValid( false )
        {}
    sal_uInt16  nParaHeight;
    sal_uInt16  nLines;

    sal_uInt16  nFirstLineStartX;

    sal_uInt16  nFirstLineOffset;
    sal_uInt16  nFirstLineHeight;
    sal_uInt16  nFirstLineTextHeight;
    sal_uInt16  nFirstLineMaxAscent;

    bool        bValid; // A query during formatting is not valid!
};

struct EECharAttrib
{
    const SfxPoolItem*  pAttr;

    sal_Int32           nPara;
    sal_Int32           nStart;
    sal_Int32           nEnd;
};

struct MoveParagraphsInfo
{
    sal_Int32  nStartPara;
    sal_Int32  nEndPara;
    sal_Int32  nDestPara;

    MoveParagraphsInfo( sal_Int32 nS, sal_Int32 nE, sal_Int32 nD )
        { nStartPara = nS; nEndPara = nE; nDestPara = nD; }
};

struct PasteOrDropInfos
{
    sal_Int32              nStartPara;
    sal_Int32              nEndPara;

    PasteOrDropInfos() : nStartPara(-1), nEndPara(-1)  {}
};

enum EENotifyType
{
    /// EditEngine text was modified
    EE_NOTIFY_TEXTMODIFIED,

    /// A paragraph was inserted into the EditEngine
    EE_NOTIFY_PARAGRAPHINSERTED,

    /// A paragraph was removed from the EditEngine
    EE_NOTIFY_PARAGRAPHREMOVED,

    /// Multiple paragraphs have been removed from the EditEngine
    EE_NOTIFY_PARAGRAPHSMOVED,

    /// The height of at least one paragraph has changed
    EE_NOTIFY_TEXTHEIGHTCHANGED,

    /// The view area of the EditEngine scrolled
    EE_NOTIFY_TEXTVIEWSCROLLED,

    /// The selection and/or the cursor position has changed
    EE_NOTIFY_TEXTVIEWSELECTIONCHANGED,

    /** Denotes the beginning of a collected amount of EditEngine
        notification events. This event itself is not queued, but sent
        immediately
     */
    EE_NOTIFY_BLOCKNOTIFICATION_START,

    /** Denotes the end of a collected amount of EditEngine
        notification events. After this event, the queue is empty, and
        a high-level operation such as "insert paragraph" is finished
     */
    EE_NOTIFY_BLOCKNOTIFICATION_END,

    /// Denotes the beginning of a high-level action triggered by a key press
    EE_NOTIFY_INPUT_START,

    /// Denotes the end of a high-level action triggered by a key press
    EE_NOTIFY_INPUT_END,
    EE_NOTIFY_TEXTVIEWSELECTIONCHANGED_ENDD_PARA
};

struct EENotify
{
    EENotifyType    eNotificationType;
    EditEngine*     pEditEngine;
    EditView*       pEditView;

    sal_Int32       nParagraph; // only valid in PARAGRAPHINSERTED/EE_NOTIFY_PARAGRAPHREMOVED

    sal_Int32       nParam1;
    sal_Int32       nParam2;

    EENotify( EENotifyType eType )
        { eNotificationType = eType; pEditEngine = nullptr; pEditView = nullptr; nParagraph = EE_PARA_NOT_FOUND; nParam1 = 0; nParam2 = 0; }
};

#endif // INCLUDED_EDITENG_EDITDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
