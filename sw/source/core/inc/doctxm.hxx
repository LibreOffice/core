/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DOCTXM_HXX
#define _DOCTXM_HXX

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
    Range GetKeyRange( const String& rStr, const String& rStrReading,
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
    TYPEINFO();                         // for rtti

    sal_Bool SetPosAtStartEnd( SwPosition& rPos, sal_Bool bAtStart = sal_True ) const;
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

    SwDefTOXBase_Impl() :
    pContBase(0),
    pIdxBase(0),
    pUserBase(0),
    pTblBase(0),
    pObjBase(0),
    pIllBase(0),
    pAuthBase(0)
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
    }

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
