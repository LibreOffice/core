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

#ifndef _EDTSPELL_HXX
#define _EDTSPELL_HXX

#include <com/sun/star/uno/Reference.h>
#include <editeng/editengdllapi.h>
#include <editeng/splwrap.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/svxenum.hxx>
#include "editeng/misspellrange.hxx"
#include <tools/link.hxx>
#include <vcl/outdev.hxx>

namespace com { namespace sun { namespace star { namespace linguistic2 {
    class XSpellChecker1;
}}}}


class EditView;
class EditEngine;
class ContentNode;

class EditSpellWrapper : public SvxSpellWrapper
{
private:
    EditView*               pEditView;
    void                    CheckSpellTo();

protected:
    virtual void            SpellStart( SvxSpellArea eArea );
    virtual sal_Bool            SpellContinue();    // Check area
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
            bool bIsStart,
            bool bIsAllRight, EditView* pView );

};

/**
 * Keeps track of misspelled ranges in paragraph.
 */
class WrongList
{
    static size_t Valid;

    std::vector<editeng::MisspellRange> maRanges;
    size_t mnInvalidStart;
    size_t mnInvalidEnd;

    bool DbgIsBuggy() const;

public:
    typedef std::vector<editeng::MisspellRange>::iterator iterator;
    typedef std::vector<editeng::MisspellRange>::const_iterator const_iterator;

    WrongList();
    WrongList(const WrongList& r);
    ~WrongList();

    const std::vector<editeng::MisspellRange>& GetRanges() const;
    void SetRanges( const std::vector<editeng::MisspellRange>& rRanges );

    bool IsValid() const;
    void SetValid();
    void SetInvalidRange( size_t nStart, size_t nEnd );
    void ResetInvalidRange( size_t nStart, size_t nEnd );

    size_t GetInvalidStart() const { return mnInvalidStart; }
    size_t GetInvalidEnd() const   { return mnInvalidEnd; }

    void TextInserted( size_t nPos, size_t nLength, bool bPosIsSep );
    void TextDeleted( size_t nPos, size_t nLength );

    void InsertWrong( size_t nStart, size_t nEnd );
    bool NextWrong( size_t& rnStart, size_t& rnEnd ) const;
    bool HasWrong( size_t nStart, size_t nEnd ) const;
    bool HasAnyWrong( size_t nStart, size_t nEnd ) const;
    void ClearWrongs( size_t nStart, size_t nEnd, const ContentNode* pNode );
    void MarkWrongsInvalid();

    WrongList*  Clone() const;

    // #i102062#
    bool operator==(const WrongList& rCompare) const;

    bool empty() const;
    void push_back(const editeng::MisspellRange& rRange);
    editeng::MisspellRange& back();
    const editeng::MisspellRange& back() const;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
};

class EdtAutoCorrDoc : public SvxAutoCorrDoc
{
    EditEngine* mpEditEngine;
    ContentNode*    pCurNode;
    sal_uInt16          nCursor;

    bool            bAllowUndoAction;
    bool            bUndoAction;

protected:
    void            ImplStartUndoAction();

public:
    EdtAutoCorrDoc(EditEngine* pE, ContentNode* pCurNode, sal_uInt16 nCrsr, sal_Unicode cIns);
    ~EdtAutoCorrDoc();

    virtual sal_Bool    Delete( sal_uInt16 nStt, sal_uInt16 nEnd );
    virtual sal_Bool    Insert( sal_uInt16 nPos, const OUString& rTxt );
    virtual sal_Bool    Replace( sal_uInt16 nPos, const OUString& rTxt );
    virtual sal_Bool    ReplaceRange( xub_StrLen nPos, xub_StrLen nLen, const OUString& rTxt );

    virtual sal_Bool    SetAttr( sal_uInt16 nStt, sal_uInt16 nEnd, sal_uInt16 nSlotId, SfxPoolItem& );
    virtual sal_Bool    SetINetAttr( sal_uInt16 nStt, sal_uInt16 nEnd, const OUString& rURL );

    virtual OUString    GetPrevPara( sal_Bool bAtNormalPos );

    virtual sal_Bool    ChgAutoCorrWord( sal_uInt16& rSttPos, sal_uInt16 nEndPos,
                                  SvxAutoCorrect& rACorrect, OUString* pPara );

    virtual LanguageType GetLanguage( sal_uInt16 nPos, sal_Bool bPrevPara = sal_False ) const;

    sal_uInt16          GetCursor() const { return nCursor; }

};

#endif  // EDTSPELL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
