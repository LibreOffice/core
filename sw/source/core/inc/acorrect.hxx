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
        pDontExpItems(0){}
    ~SwDontExpandItem();

    void SaveDontExpandItems( const SwPosition& rPos );
    void RestoreDontExpandItems( const SwPosition& rPos );

};

class SwAutoCorrDoc : public SvxAutoCorrDoc
{
    SwEditShell& rEditSh;
    SwPaM& rCrsr;
    SwNodeIndex* pIdx;
    int m_nEndUndoCounter;
    bool    bUndoIdInitialized;

    void DeleteSel( SwPaM& rDelPam );

public:
    SwAutoCorrDoc( SwEditShell& rEditShell, SwPaM& rPam, sal_Unicode cIns = 0 );
    virtual ~SwAutoCorrDoc();

    virtual bool Delete( sal_Int32 nStt, sal_Int32 nEnd ) SAL_OVERRIDE;
    virtual bool Insert( sal_Int32 nPos, const OUString& rTxt ) SAL_OVERRIDE;
    virtual bool Replace( sal_Int32 nPos, const OUString& rTxt ) SAL_OVERRIDE;
    virtual bool ReplaceRange( sal_Int32 nPos, sal_Int32 nLen, const OUString& rTxt ) SAL_OVERRIDE;

    virtual bool SetAttr( sal_Int32 nStt, sal_Int32 nEnd, sal_uInt16 nSlotId,
                            SfxPoolItem& ) SAL_OVERRIDE;

    virtual bool SetINetAttr( sal_Int32 nStt, sal_Int32 nEnd, const OUString& rURL ) SAL_OVERRIDE;

    // return text of a previous paragraph
    // If it does not exist or if there is nothing before, return blank.
    //  - sal_True:  paragraph before "normal" insertion position
    //  - sal_False: paragraph in that the corrected word was inserted
    //               (does not need to be the same paragraph)
    virtual OUString const* GetPrevPara(bool bAtNormalPos) SAL_OVERRIDE;

    virtual bool ChgAutoCorrWord( sal_Int32& rSttPos, sal_Int32 nEndPos,
                                  SvxAutoCorrect& rACorrect,
                                  OUString* pPara ) SAL_OVERRIDE;

    // Will be called after swapping characters by the functions
    //  - FnCptlSttWrd and
    //  - FnCptlSttSntnc.
    // Afterwards the words can be added into exception list if needed.
    virtual void SaveCpltSttWord( sal_uLong nFlag, sal_Int32 nPos,
                                    const OUString& rExceptWord, sal_Unicode cChar ) SAL_OVERRIDE;
    virtual LanguageType GetLanguage( sal_Int32 nPos, bool bPrevPara ) const SAL_OVERRIDE;
};

class SwAutoCorrExceptWord
{
    OUString sWord;
    sal_uLong nFlags, nNode;
    sal_Int32 nCntnt;
    sal_Unicode cChar;
    LanguageType eLanguage;
    sal_Bool bDeleted;

public:
    SwAutoCorrExceptWord( sal_uLong nAFlags, sal_uLong nNd, sal_Int32 nContent,
                                        const OUString& rWord, sal_Unicode cChr,
                                        LanguageType eLang )
        : sWord(rWord), nFlags(nAFlags), nNode(nNd), nCntnt(nContent),
        cChar(cChr), eLanguage(eLang), bDeleted(sal_False)
    {}

    sal_Bool IsDeleted() const                          { return bDeleted; }
    void CheckChar( const SwPosition& rPos, sal_Unicode cChar );
    sal_Bool CheckDelChar( const SwPosition& rPos );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
