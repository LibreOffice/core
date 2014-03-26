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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCTXM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCTXM_HXX

#include <tools/gen.hxx>
#include <tox.hxx>
#include <section.hxx>

class  SwTOXInternational;
class  SwPageDesc;
class  SwTxtNode;
class  SwTxtFmtColl;
struct SwPosition;
struct SwTOXSortTabBase;

typedef std::vector<SwTOXSortTabBase*> SwTOXSortTabBases;

class SwTOXBaseSection : public SwTOXBase, public SwSection
{
    SwTOXSortTabBases aSortArr;

    void UpdateMarks( const SwTOXInternational& rIntl,
                      const SwTxtNode* pOwnChapterNode );
    void UpdateOutline( const SwTxtNode* pOwnChapterNode );
    void UpdateTemplate( const SwTxtNode* pOwnChapterNode );
    void UpdateCntnt( SwTOXElement eType,
                      const SwTxtNode* pOwnChapterNode );
    void UpdateTable( const SwTxtNode* pOwnChapterNode );
    void UpdateSequence( const SwTxtNode* pOwnChapterNode );
    void UpdateAuthorities( const SwTOXInternational& rIntl );
    void UpdateAll();

    // insert sorted into array for creation
    void InsertSorted(SwTOXSortTabBase* pBase);

    // insert alpha delimiter at creation
    void InsertAlphaDelimitter( const SwTOXInternational& rIntl );

    // generate text body
    void GenerateText( sal_uInt16 nArrayIdx,
                       sal_uInt16 nCount,
                       const sal_uInt32   _nTOXSectNdIdx,
                       const SwPageDesc*  _pDefaultPageDesc );

    // replace page num placeholder with actual page number
    void _UpdatePageNum( SwTxtNode* pNd,
                         const std::vector<sal_uInt16>& rNums,
                         const std::vector<SwPageDesc*>& rDescs,
                         const std::vector<sal_uInt16>* pMainEntryNums,
                         const SwTOXInternational& rIntl );

    // get section for entering keywords
    Range GetKeyRange( const OUString& rStr, const OUString& rStrReading,
                       const SwTOXSortTabBase& rNew, sal_uInt16 nLevel,
                       const Range& rRange );

    // return text collection via name/ from format pool
    SwTxtFmtColl* GetTxtFmtColl( sal_uInt16 nLevel );

public:
    SwTOXBaseSection(SwTOXBase const& rBase, SwSectionFmt & rFmt);
    virtual ~SwTOXBaseSection();

    // <_bNewTOX> : distinguish between the creation of a new table-of-content
    //              (true) or an update of a table-of-content (false)
    void Update( const SfxItemSet* pAttr = 0,
                 const bool        _bNewTOX = false );
    void UpdatePageNum();               // insert page numbering
    TYPEINFO_OVERRIDE();                         // for rtti

    SwTOXSortTabBases* GetTOXSortTabBases() { return &aSortArr; }

    bool SetPosAtStartEnd( SwPosition& rPos, bool bAtStart = true ) const;
};

struct SwDefTOXBase_Impl
{
    SwTOXBase* pContBase;
    SwTOXBase* pIdxBase;
    SwTOXBase* pUserBase;
    SwTOXBase* pTblBase;
    SwTOXBase* pObjBase;
    SwTOXBase* pIllBase;
    SwTOXBase* pAuthBase;
    SwTOXBase* pBiblioBase;

    SwDefTOXBase_Impl() :
    pContBase(0),
    pIdxBase(0),
    pUserBase(0),
    pTblBase(0),
    pObjBase(0),
    pIllBase(0),
    pAuthBase(0),
    pBiblioBase(0)
    {
    }
    ~SwDefTOXBase_Impl()
    {
        delete pContBase;
        delete pIdxBase;
        delete pUserBase;
        delete pTblBase;
        delete pObjBase;
        delete pIllBase;
        delete pAuthBase;
        delete pBiblioBase;
    }

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
