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
#ifndef _DOCARY_HXX
#define _DOCARY_HXX

#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#include <vector>
#include <algorithm>

class SwFieldType;
class SwFrmFmt;
class SwCharFmt;
class SwTOXType;
class SwUndo;
class SwSectionFmt;
class SwNumRule;
class SwRedline;
class SwUnoCrsr;
class SwOLENode;

namespace com { namespace sun { namespace star { namespace i18n {
    struct ForbiddenCharacters;    // comes from the I18N UNO interface
}}}}

#include <swtypes.hxx>
#include <svl/svarray.hxx>

// PageDescriptor-interface
// typedef SwPageDesc * SwPageDescPtr;
// SV_DECL_PTRARR_DEL(SwPageDescs, SwPageDescPtr,1);

typedef SwFrmFmt* SwFrmFmtPtr;
SV_DECL_PTRARR_DEL(SwFrmFmts,SwFrmFmtPtr,4)

// Specific frame formats (frames, DrawObjects).
SV_DECL_PTRARR_DEL(SwSpzFrmFmts,SwFrmFmtPtr,0)

typedef SwCharFmt* SwCharFmtPtr;
SV_DECL_PTRARR_DEL(SwCharFmts,SwCharFmtPtr,4)

class SwFldTypes : public std::vector<SwFieldType*> {
public:
    // the destructor will free all objects still in the vector
    ~SwFldTypes();
    sal_uInt16 GetPos(const SwFieldType* pFieldType) const;
};

class SwTOXTypes : public std::vector<SwTOXType*> {
public:
    // the destructor will free all objects still in the vector
    ~SwTOXTypes();
    sal_uInt16 GetPos(const SwTOXType* pTOXType) const;
};

// Array of Undo-history.
typedef SwSectionFmt* SwSectionFmtPtr;
SV_DECL_PTRARR_DEL(SwSectionFmts,SwSectionFmtPtr,0)

class SW_DLLPUBLIC SwNumRuleTbl : public std::vector<SwNumRule*> {
public:
    // the destructor will free all objects still in the vector
    ~SwNumRuleTbl();
    sal_uInt16 GetPos(const SwNumRule* pRule) const;
};

typedef SwRedline* SwRedlinePtr;
SV_DECL_PTRARR_SORT_DEL( _SwRedlineTbl, SwRedlinePtr, 0 )

class SwRedlineTbl : private _SwRedlineTbl
{
public:
    SwRedlineTbl( sal_uInt8 nSize = 0 )
        : _SwRedlineTbl( nSize ) {}
    ~SwRedlineTbl() {}

    sal_Bool SavePtrInArr( SwRedlinePtr p ) { return _SwRedlineTbl::Insert( p ); }

    sal_Bool Insert( SwRedlinePtr& p, sal_Bool bIns = sal_True );
    sal_Bool Insert( SwRedlinePtr& p, sal_uInt16& rInsPos, sal_Bool bIns = sal_True );
    sal_Bool InsertWithValidRanges( SwRedlinePtr& p, sal_uInt16* pInsPos = 0 );

    void Remove( sal_uInt16 nP, sal_uInt16 nL = 1 );
    void DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL=1 );

    // Search next or previous Redline with the same Seq. No.
    // Search can be restricted via Lookahaed.
    // Using 0 or USHRT_MAX makes search the whole array.
    sal_uInt16 FindNextOfSeqNo( sal_uInt16 nSttPos, sal_uInt16 nLookahead = 20 ) const;
    sal_uInt16 FindPrevOfSeqNo( sal_uInt16 nSttPos, sal_uInt16 nLookahead = 20 ) const;
    sal_uInt16 FindNextSeqNo( sal_uInt16 nSeqNo, sal_uInt16 nSttPos,
                            sal_uInt16 nLookahead = 20 ) const;
    sal_uInt16 FindPrevSeqNo( sal_uInt16 nSeqNo, sal_uInt16 nSttPos,
                            sal_uInt16 nLookahead = 20 ) const;

    using _SwRedlineTbl::Count;
    using _SwRedlineTbl::operator[];
    using _SwRedlineTbl::GetObject;
    using _SwRedlineTbl::Seek_Entry;
    using _SwRedlineTbl::GetPos;
};

typedef SwUnoCrsr* SwUnoCrsrPtr;
SV_DECL_PTRARR_DEL( SwUnoCrsrTbl, SwUnoCrsrPtr, 0 )

class SwOLENodes : public std::vector<SwOLENode*> {};


#endif  //_DOCARY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
