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

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#include <swtypes.hxx>
#include <svl/svarray.hxx>

typedef SwFieldType* SwFldTypePtr;
#define GROW_FLDTYPES   16

//PageDescriptor-Schnittstelle
//typedef SwPageDesc * SwPageDescPtr;
//SV_DECL_PTRARR_DEL(SwPageDescs, SwPageDescPtr,1,1);

typedef SwFrmFmt* SwFrmFmtPtr;
SV_DECL_PTRARR_DEL(SwFrmFmts,SwFrmFmtPtr,4,4)

//Spezifische Frameformate (Rahmen, DrawObjecte)
SV_DECL_PTRARR_DEL(SwSpzFrmFmts,SwFrmFmtPtr,0,4)

typedef SwCharFmt* SwCharFmtPtr;
SV_DECL_PTRARR_DEL(SwCharFmts,SwCharFmtPtr,4,4)

SV_DECL_PTRARR_DEL( SwFldTypes, SwFldTypePtr, INIT_FLDTYPES, GROW_FLDTYPES )

typedef SwTOXType* SwTOXTypePtr;
SV_DECL_PTRARR_DEL( SwTOXTypes, SwTOXTypePtr, 0, 1 )

typedef SwSectionFmt* SwSectionFmtPtr;
SV_DECL_PTRARR_DEL(SwSectionFmts,SwSectionFmtPtr,0,4)


typedef SwNumRule* SwNumRulePtr;
SV_DECL_PTRARR_DEL_VISIBILITY( SwNumRuleTbl, SwNumRulePtr, 0, 5, SW_DLLPUBLIC )

typedef SwRedline* SwRedlinePtr;
SV_DECL_PTRARR_SORT_DEL( _SwRedlineTbl, SwRedlinePtr, 0, 16 )

class SwRedlineTbl : private _SwRedlineTbl
{
public:
    SwRedlineTbl( sal_uInt8 nSize = 0, sal_uInt8 nG = 16 )
        : _SwRedlineTbl( nSize, nG ) {}
    ~SwRedlineTbl() {}

    sal_Bool SavePtrInArr( SwRedlinePtr p ) { return _SwRedlineTbl::Insert( p ); }

    sal_Bool Insert( SwRedlinePtr& p, sal_Bool bIns = sal_True );
    sal_Bool Insert( SwRedlinePtr& p, sal_uInt16& rInsPos, sal_Bool bIns = sal_True );
    sal_Bool InsertWithValidRanges( SwRedlinePtr& p, sal_uInt16* pInsPos = 0 );

    void Remove( sal_uInt16 nP, sal_uInt16 nL = 1 );
    void DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL=1 );

    // suche den naechsten oder vorherigen Redline mit dergleichen Seq.No
    // Mit dem Lookahead kann die Suche eingeschraenkt werden. 0 oder
    // USHRT_MAX suchen im gesamten Array.
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
SV_DECL_PTRARR_DEL( SwUnoCrsrTbl, SwUnoCrsrPtr, 0, 4 )

typedef SwOLENode* SwOLENodePtr;
SV_DECL_PTRARR(SwOLENodes,SwOLENodePtr,16,16)


#endif  //_DOCARY_HXX

