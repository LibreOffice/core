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

#ifndef INCLUDED_EDITENG_SOURCE_EDITENG_EDTSPELL_HXX
#define INCLUDED_EDITENG_SOURCE_EDITENG_EDTSPELL_HXX

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
    virtual void            SpellStart( SvxSpellArea eArea ) SAL_OVERRIDE;
    virtual bool            SpellContinue() SAL_OVERRIDE;    // Check area
    virtual void            ReplaceAll( const OUString &rNewText, sal_Int16 nLanguage ) SAL_OVERRIDE;
    virtual void            SpellEnd() SAL_OVERRIDE;
    virtual bool            SpellMore() SAL_OVERRIDE;
    virtual bool            HasOtherCnt() SAL_OVERRIDE;

public:
    EditSpellWrapper( vcl::Window* pWin,
            css::uno::Reference<
                css::linguistic2::XSpellChecker1 > &xChecker,
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

    const std::vector<editeng::MisspellRange>& GetRanges() const { return maRanges;}
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
    sal_Int32       nCursor;

    bool            bAllowUndoAction;
    bool            bUndoAction;

protected:
    void            ImplStartUndoAction();

public:
    EdtAutoCorrDoc(EditEngine* pE, ContentNode* pCurNode, sal_Int32 nCrsr, sal_Unicode cIns);
    virtual ~EdtAutoCorrDoc();

    virtual bool    Delete( sal_Int32 nStt, sal_Int32 nEnd ) SAL_OVERRIDE;
    virtual bool    Insert( sal_Int32 nPos, const OUString& rTxt ) SAL_OVERRIDE;
    virtual bool    Replace( sal_Int32 nPos, const OUString& rTxt ) SAL_OVERRIDE;
    virtual bool    ReplaceRange( sal_Int32 nPos, sal_Int32 nLen, const OUString& rTxt ) SAL_OVERRIDE;

    virtual bool    SetAttr( sal_Int32 nStt, sal_Int32 nEnd, sal_uInt16 nSlotId, SfxPoolItem& ) SAL_OVERRIDE;
    virtual bool    SetINetAttr( sal_Int32 nStt, sal_Int32 nEnd, const OUString& rURL ) SAL_OVERRIDE;

    virtual OUString const* GetPrevPara(bool bAtNormalPos) SAL_OVERRIDE;

    virtual bool        ChgAutoCorrWord( sal_Int32& rSttPos, sal_Int32 nEndPos,
                                  SvxAutoCorrect& rACorrect, OUString* pPara ) SAL_OVERRIDE;

    virtual LanguageType GetLanguage( sal_Int32 nPos, bool bPrevPara = false ) const SAL_OVERRIDE;

    sal_Int32       GetCursor() const { return nCursor; }

};

#endif  // EDTSPELL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
