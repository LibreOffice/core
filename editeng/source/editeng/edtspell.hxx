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



#ifndef _EDTSPELL_HXX
#define _EDTSPELL_HXX

#include <svtools/svxbox.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/splwrap.hxx>
#include <editeng/svxacorr.hxx>
#include <com/sun/star/uno/Reference.h>
#include <editeng/editengdllapi.h>

namespace com { namespace sun { namespace star { namespace linguistic2 {
    class XSpellChecker1;
}}}}


class EditView;
class ImpEditEngine;
class ContentNode;

class EditSpellWrapper : public SvxSpellWrapper
{
private:
    EditView*               pEditView;
    void                    CheckSpellTo();

protected:
    virtual void            SpellStart( SvxSpellArea eArea );
    virtual sal_Bool            SpellContinue();    // Bereich pruefen
    virtual void            ReplaceAll( const String &rNewText, sal_Int16 nLanguage );
    virtual void            SpellEnd();
    virtual sal_Bool            SpellMore();
    virtual sal_Bool            HasOtherCnt();
    virtual void            ScrollArea();
    virtual void            ChangeWord( const String& rNewWord, const sal_uInt16 nLang );
    virtual void            ChangeThesWord( const String& rNewWord );
    virtual void            AutoCorrect( const String& rOldWord, const String& rNewWord );

public:
    EditSpellWrapper( Window* pWin,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XSpellChecker1 > &xChecker,
            sal_Bool bIsStart,
            sal_Bool bIsAllRight, EditView* pView );

};


struct WrongRange
{
    sal_uInt16 nStart;
    sal_uInt16 nEnd;

    WrongRange( sal_uInt16 nS, sal_uInt16 nE ) { nStart = nS; nEnd = nE; }
};

SV_DECL_VARARR( WrongRanges, WrongRange, 4, 4 )
#define NOT_INVALID 0xFFFF

class WrongList : private WrongRanges
{
private:
    sal_uInt16  nInvalidStart;
    sal_uInt16  nInvalidEnd;

    sal_Bool    DbgIsBuggy() const;

public:
            WrongList();
            ~WrongList();

    sal_Bool    IsInvalid() const       { return nInvalidStart != NOT_INVALID; }
    void    SetValid()              { nInvalidStart = NOT_INVALID; nInvalidEnd = 0; }
    void    MarkInvalid( sal_uInt16 nS, sal_uInt16 nE );

    sal_uInt16          Count() const               { return WrongRanges::Count(); }

    // Wenn man weiss was man tut:
    WrongRange&     GetObject( sal_uInt16 n ) const { return WrongRanges::GetObject( n ); }
    void            InsertWrong( const WrongRange& rWrong, sal_uInt16 nPos );

    sal_uInt16  GetInvalidStart() const { return nInvalidStart; }
    sal_uInt16& GetInvalidStart()       { return nInvalidStart; }

    sal_uInt16  GetInvalidEnd() const   { return nInvalidEnd; }
    sal_uInt16& GetInvalidEnd()         { return nInvalidEnd; }

    void    TextInserted( sal_uInt16 nPos, sal_uInt16 nChars, sal_Bool bPosIsSep );
    void    TextDeleted( sal_uInt16 nPos, sal_uInt16 nChars );

    void    ResetRanges()           { Remove( 0, Count() ); }
    sal_Bool    HasWrongs() const       { return Count() != 0; }
    void    InsertWrong( sal_uInt16 nStart, sal_uInt16 nEnd, sal_Bool bClearRange );
    sal_Bool    NextWrong( sal_uInt16& rnStart, sal_uInt16& rnEnd ) const;
    sal_Bool    HasWrong( sal_uInt16 nStart, sal_uInt16 nEnd ) const;
    sal_Bool    HasAnyWrong( sal_uInt16 nStart, sal_uInt16 nEnd ) const;
    void    ClearWrongs( sal_uInt16 nStart, sal_uInt16 nEnd, const ContentNode* pNode );
    void    MarkWrongsInvalid();

    WrongList*  Clone() const;

    // #i102062#
    bool operator==(const WrongList& rCompare) const;
};

inline void WrongList::InsertWrong( const WrongRange& rWrong, sal_uInt16 nPos )
{
    WrongRanges::Insert( rWrong, nPos );
#ifdef DBG_UTIL
    DBG_ASSERT( !DbgIsBuggy(), "Insert: WrongList kaputt!" );
#endif
}



class EdtAutoCorrDoc : public SvxAutoCorrDoc
{
    ImpEditEngine*  pImpEE;
    ContentNode*    pCurNode;
    sal_uInt16          nCursor;

    sal_Bool            bAllowUndoAction;
    sal_Bool            bUndoAction;

protected:
    void            ImplStartUndoAction();

public:
                    EdtAutoCorrDoc( ImpEditEngine* pImpEE, ContentNode* pCurNode, sal_uInt16 nCrsr, xub_Unicode cIns );
                    ~EdtAutoCorrDoc();

    virtual sal_Bool    Delete( sal_uInt16 nStt, sal_uInt16 nEnd );
    virtual sal_Bool    Insert( sal_uInt16 nPos, const String& rTxt );
    virtual sal_Bool    Replace( sal_uInt16 nPos, const String& rTxt );
    virtual sal_Bool    ReplaceRange( xub_StrLen nPos, xub_StrLen nLen, const String& rTxt );

    virtual sal_Bool    SetAttr( sal_uInt16 nStt, sal_uInt16 nEnd, sal_uInt16 nSlotId, SfxPoolItem& );
    virtual sal_Bool    SetINetAttr( sal_uInt16 nStt, sal_uInt16 nEnd, const String& rURL );

    virtual sal_Bool    HasSymbolChars( sal_uInt16 nStt, sal_uInt16 nEnd );

    virtual const String* GetPrevPara( sal_Bool bAtNormalPos );

    virtual sal_Bool    ChgAutoCorrWord( sal_uInt16& rSttPos, sal_uInt16 nEndPos,
                                  SvxAutoCorrect& rACorrect, const String** ppPara );

    virtual LanguageType GetLanguage( sal_uInt16 nPos, sal_Bool bPrevPara = sal_False ) const;

    sal_uInt16          GetCursor() const { return nCursor; }

};

#endif  // EDTSPELL

