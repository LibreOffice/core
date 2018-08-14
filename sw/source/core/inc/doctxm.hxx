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
class  SwTextNode;
class  SwTextFormatColl;
struct SwPosition;
struct SwTOXSortTabBase;

class SwTOXBaseSection : public SwTOXBase, public SwSection
{
    std::vector<std::unique_ptr<SwTOXSortTabBase>> aSortArr;

    void UpdateMarks( const SwTOXInternational& rIntl,
                      const SwTextNode* pOwnChapterNode );
    void UpdateOutline( const SwTextNode* pOwnChapterNode );
    void UpdateTemplate( const SwTextNode* pOwnChapterNode );
    void UpdateContent( SwTOXElement eType,
                      const SwTextNode* pOwnChapterNode );
    void UpdateTable( const SwTextNode* pOwnChapterNode );
    void UpdateSequence( const SwTextNode* pOwnChapterNode );
    void UpdateAuthorities( const SwTOXInternational& rIntl );

    // insert sorted into array for creation
    void InsertSorted(std::unique_ptr<SwTOXSortTabBase> pBase);

    // insert alpha delimiter at creation
    void InsertAlphaDelimitter( const SwTOXInternational& rIntl );

    // replace page num placeholder with actual page number
    void UpdatePageNum_( SwTextNode* pNd,
                         const std::vector<sal_uInt16>& rNums,
                         const std::vector<SwPageDesc*>& rDescs,
                         const std::vector<sal_uInt16>* pMainEntryNums,
                         const SwTOXInternational& rIntl );

    // get section for entering keywords
    Range GetKeyRange( const OUString& rStr, const OUString& rStrReading,
                       const SwTOXSortTabBase& rNew, sal_uInt16 nLevel,
                       const Range& rRange );

    // return text collection via name/ from format pool
    SwTextFormatColl* GetTextFormatColl( sal_uInt16 nLevel );

public:
    SwTOXBaseSection(SwTOXBase const& rBase, SwSectionFormat & rFormat);
    virtual ~SwTOXBaseSection() override;

    // <_bNewTOX> : distinguish between the creation of a new table-of-content
    //              (true) or an update of a table-of-content (false)
    void Update( const SfxItemSet* pAttr = nullptr,
                 const bool        _bNewTOX = false );
    void UpdatePageNum();               // insert page numbering

    const std::vector<std::unique_ptr<SwTOXSortTabBase>>& GetTOXSortTabBases() const { return aSortArr; }

    bool SetPosAtStartEnd( SwPosition& rPos ) const;
};

struct SwDefTOXBase_Impl
{
    std::unique_ptr<SwTOXBase> pContBase;
    std::unique_ptr<SwTOXBase> pIdxBase;
    std::unique_ptr<SwTOXBase> pUserBase;
    std::unique_ptr<SwTOXBase> pTableBase;
    std::unique_ptr<SwTOXBase> pObjBase;
    std::unique_ptr<SwTOXBase> pIllBase;
    std::unique_ptr<SwTOXBase> pAuthBase;
    std::unique_ptr<SwTOXBase> pBiblioBase;

    SwDefTOXBase_Impl()
    {
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
