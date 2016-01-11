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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_ACORRECT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_ACORRECT_HXX

#include <editeng/svxacorr.hxx>
#include <swundo.hxx>

class SwEditShell;
class SwPaM;
class SwNodeIndex;
struct SwPosition;
class SfxItemSet;

class SwDontExpandItem
{
    SfxItemSet* pDontExpItems;

public:
    SwDontExpandItem() :
        pDontExpItems(nullptr){}
    ~SwDontExpandItem();

    void SaveDontExpandItems( const SwPosition& rPos );
    void RestoreDontExpandItems( const SwPosition& rPos );

};

class SwAutoCorrDoc : public SvxAutoCorrDoc
{
    SwEditShell& rEditSh;
    SwPaM& rCursor;
    SwNodeIndex* pIdx;
    int m_nEndUndoCounter;
    bool    bUndoIdInitialized;

    void DeleteSel( SwPaM& rDelPam );

public:
    SwAutoCorrDoc( SwEditShell& rEditShell, SwPaM& rPam, sal_Unicode cIns = 0 );
    virtual ~SwAutoCorrDoc();

    virtual bool Delete( sal_Int32 nStt, sal_Int32 nEnd ) override;
    virtual bool Insert( sal_Int32 nPos, const OUString& rText ) override;
    virtual bool Replace( sal_Int32 nPos, const OUString& rText ) override;
    virtual bool ReplaceRange( sal_Int32 nPos, sal_Int32 nLen, const OUString& rText ) override;

    virtual void SetAttr( sal_Int32 nStt, sal_Int32 nEnd, sal_uInt16 nSlotId,
                            SfxPoolItem& ) override;

    virtual bool SetINetAttr( sal_Int32 nStt, sal_Int32 nEnd, const OUString& rURL ) override;

    // return text of a previous paragraph
    // If it does not exist or if there is nothing before, return blank.
    //  - true:  paragraph before "normal" insertion position
    //  - false: paragraph in that the corrected word was inserted
    //               (does not need to be the same paragraph)
    virtual OUString const* GetPrevPara(bool bAtNormalPos) override;

    virtual bool ChgAutoCorrWord( sal_Int32& rSttPos, sal_Int32 nEndPos,
                                  SvxAutoCorrect& rACorrect,
                                  OUString* pPara ) override;

    // Will be called after swapping characters by the functions
    //  - FnCapitalStartWord and
    //  - FnCapitalStartSentence.
    // Afterwards the words can be added into exception list if needed.
    virtual void SaveCpltSttWord( sal_uLong nFlag, sal_Int32 nPos,
                                    const OUString& rExceptWord, sal_Unicode cChar ) override;
    virtual LanguageType GetLanguage( sal_Int32 nPos, bool bPrevPara ) const override;
};

class SwAutoCorrExceptWord
{
    OUString m_sWord;
    sal_uLong m_nFlags, m_nNode;
    sal_Int32 m_nContent;
    sal_Unicode m_cChar;
    LanguageType m_eLanguage;
    bool m_bDeleted;

public:
    SwAutoCorrExceptWord(sal_uLong nAFlags, sal_uLong nNd, sal_Int32 nContent,
                         const OUString& rWord, sal_Unicode cChr,
                         LanguageType eLang)
        : m_sWord(rWord), m_nFlags(nAFlags), m_nNode(nNd), m_nContent(nContent),
          m_cChar(cChr), m_eLanguage(eLang), m_bDeleted(false)
    {}

    bool IsDeleted() const { return m_bDeleted; }
    void CheckChar(const SwPosition& rPos, sal_Unicode cChar);
    bool CheckDelChar(const SwPosition& rPos);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
