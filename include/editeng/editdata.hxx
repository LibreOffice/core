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
#include <memory>
#include <ostream>

class SfxItemSet;
class SfxPoolItem;
template<typename T> class SvParser;
class SvxFieldItem;
class SvxRTFItemStackType;
enum class HtmlTokenId : sal_Int16;

enum class EETextFormat { Text = 0x20, Rtf, Html = 0x32, Xml };
enum class EEHorizontalTextDirection { Default, L2R, R2L };
enum class EESelectionMode  { Std, Hidden };
    // EESelectionMode::Hidden can be used to completely hide the selection. This is useful e.g. when you want show the selection
    // only as long as your window (which the edit view works on) has the focus
enum class EESpellState  { Ok, NoSpeller, ErrorFound };
enum class EEAnchorMode {
            TopLeft,     TopHCenter,     TopRight,
            VCenterLeft, VCenterHCenter, VCenterRight,
            BottomLeft,  BottomHCenter,  BottomRight };

#define EE_PARA_NOT_FOUND       SAL_MAX_INT32
#define EE_PARA_APPEND          SAL_MAX_INT32
#define EE_PARA_ALL             SAL_MAX_INT32
#define EE_PARA_MAX_COUNT       SAL_MAX_INT32

#define EE_INDEX_NOT_FOUND      SAL_MAX_INT32
#define EE_TEXTPOS_ALL          SAL_MAX_INT32
#define EE_TEXTPOS_MAX_COUNT    SAL_MAX_INT32

EDITENG_DLLPUBLIC extern const size_t EE_APPEND;

// Error messages for Read / Write Method
#define EE_READWRITE_WRONGFORMAT     ErrCode(ErrCodeArea::Svx, 1)

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

template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, EPosition const& pos)
{
    return stream << "EPosition(" << pos.nPara << ',' << pos.nIndex << ")";
}

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
    bool    operator==( const ESelection& rS ) const;
    bool    operator!=( const ESelection& rS ) const { return !operator==(rS); }
    bool    operator<( const ESelection& rS ) const;
    bool    operator>( const ESelection& rS ) const;
    bool    IsZero() const;
    bool    HasRange() const;
};

template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, ESelection const& sel)
{
    return stream << "ESelection(" << sel.nStartPara << ',' << sel.nStartPos << "," << sel.nEndPara << "," << sel.nEndPos << ")";
}

inline bool ESelection::HasRange() const
{
    return ( nStartPara != nEndPara ) || ( nStartPos != nEndPos );
}

inline bool ESelection::IsZero() const
{
    return ( ( nStartPara == 0 ) && ( nStartPos == 0 ) &&
             ( nEndPara == 0 ) && ( nEndPos == 0 ) );
}

inline bool ESelection::operator==( const ESelection& rS ) const
{
    return ( ( nStartPara == rS.nStartPara ) && ( nStartPos == rS.nStartPos ) &&
             ( nEndPara == rS.nEndPara ) && ( nEndPos == rS.nEndPos ) );
}

inline bool ESelection::operator<( const ESelection& rS ) const
{
    // The selection must be adjusted.
    // => Only check if end of 'this' < Start of rS
    return ( nEndPara < rS.nStartPara ) ||
        ( ( nEndPara == rS.nStartPara ) && ( nEndPos < rS.nStartPos ) && !operator==( rS ) );
}

inline bool ESelection::operator>( const ESelection& rS ) const
{
    // The selection must be adjusted.
    // => Only check if end of 'this' < Start of rS
    return ( nStartPara > rS.nEndPara ) ||
        ( ( nStartPara == rS.nEndPara ) && ( nStartPos > rS.nEndPos ) && !operator==( rS ) );
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
    std::unique_ptr<SvxFieldItem>   pFieldItem;
    OUString                        aCurrentText;
    EPosition                       aPosition;

    EFieldInfo();
    EFieldInfo( const SvxFieldItem& rFieldItem, sal_Int32 nPara, sal_Int32 nPos );
    ~EFieldInfo();

    EFieldInfo( const EFieldInfo& );
    EFieldInfo& operator= ( const EFieldInfo& );
};

enum class RtfImportState {
                    Start, End,               // only pParser, nPara, nIndex
                    NextToken, UnknownAttr,   // nToken+nTokenValue
                    SetAttr,                  // pAttrs
                    InsertText,               // aText
                    InsertPara,               // -
                    };
enum class HtmlImportState {
                    Start, End,               // only pParser, nPara, nIndex
                    NextToken,                // nToken
                    SetAttr,                  // pAttrs
                    InsertText,               // aText
                    InsertPara, InsertField   // -
                    };

struct HtmlImportInfo
{
    SvParser<HtmlTokenId>*  pParser;
    ESelection              aSelection;
    HtmlImportState         eState;

    HtmlTokenId             nToken;

    OUString                aText;

    HtmlImportInfo( HtmlImportState eState, SvParser<HtmlTokenId>* pPrsrs, const ESelection& rSel );
    ~HtmlImportInfo();
};

struct RtfImportInfo
{
    SvParser<int>*          pParser;
    ESelection              aSelection;
    RtfImportState          eState;

    int                     nToken;
    short                   nTokenValue;

    RtfImportInfo( RtfImportState eState, SvParser<int>* pPrsrs, const ESelection& rSel );
    ~RtfImportInfo();
};

struct ParagraphInfos
{
    ParagraphInfos()
        : nFirstLineHeight( 0 )
        , nFirstLineTextHeight ( 0 )
        , nFirstLineMaxAscent( 0 )
        , bValid( false )
        {}

    sal_uInt16  nFirstLineHeight;
    sal_uInt16  nFirstLineTextHeight;
    sal_uInt16  nFirstLineMaxAscent;

    bool        bValid; // A query during formatting is not valid!
};

struct EECharAttrib
{
    const SfxPoolItem*  pAttr;

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
    EE_NOTIFY_TextHeightChanged,

    /// The view area of the EditEngine scrolled
    EE_NOTIFY_TEXTVIEWSCROLLED,

    /// The selection and/or the cursor position has changed
    EE_NOTIFY_TEXTVIEWSELECTIONCHANGED,

    /// The EditEngine is in a valid state again. Process pending notifications.
    EE_NOTIFY_PROCESSNOTIFICATIONS,

    EE_NOTIFY_TEXTVIEWSELECTIONCHANGED_ENDD_PARA
};

struct EENotify
{
    EENotifyType    eNotificationType;

    sal_Int32       nParagraph; // only valid in PARAGRAPHINSERTED/EE_NOTIFY_PARAGRAPHREMOVED

    sal_Int32       nParam1;
    sal_Int32       nParam2;

    EENotify( EENotifyType eType )
        { eNotificationType = eType; nParagraph = EE_PARA_NOT_FOUND; nParam1 = 0; nParam2 = 0; }
};

#endif // INCLUDED_EDITENG_EDITDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
