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
#ifndef _FMTPDSC_HXX
#define _FMTPDSC_HXX


#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>
#include <calbck.hxx>

class SwPageDesc;
class SwHistory;
class SwPaM;
class IntlWrapper;
class SwEndNoteInfo;

//Pagedescriptor
//Client vom SwPageDesc der durch das Attribut "beschrieben" wird.

#define IVER_FMTPAGEDESC_NOAUTO ((sal_uInt16)0x0001)
#define IVER_FMTPAGEDESC_LONGPAGE   ((sal_uInt16)0x0002)

class SW_DLLPUBLIC SwFmtPageDesc : public SfxPoolItem, public SwClient
{
    // diese "Doc"-Funktion ist friend, um nach dem kopieren das
    // Auto-Flag setzen zu koennen !!
    friend sal_Bool InsAttr( SwDoc*, const SwPaM &, const SfxItemSet&, sal_uInt16,
                        SwHistory* );
    sal_uInt16 nNumOffset;          // Seitennummer Offset
    sal_uInt16 nDescNameIdx;        // SW3-Reader: Stringpool-Index des Vorlagennamens
    SwModify* pDefinedIn;       // Verweis auf das Objekt, in dem das
                                // Attribut gesetzt wurde (CntntNode/Format)
protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew );
    virtual void SwClientNotify( const SwModify&, const SfxHint& rHint );

public:
    SwFmtPageDesc( const SwPageDesc *pDesc = 0 );
    SwFmtPageDesc( const SwFmtPageDesc &rCpy );
    SwFmtPageDesc &operator=( const SwFmtPageDesc &rCpy );
    ~SwFmtPageDesc();

    TYPEINFO();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

          SwPageDesc *GetPageDesc() { return (SwPageDesc*)GetRegisteredIn(); }
    const SwPageDesc *GetPageDesc() const { return (SwPageDesc*)GetRegisteredIn(); }

    sal_uInt16  GetNumOffset() const        { return nNumOffset; }
    void    SetNumOffset( sal_uInt16 nNum ) { nNumOffset = nNum; }

    // erfrage/setze, wo drin das Attribut verankert ist
    inline const SwModify* GetDefinedIn() const { return pDefinedIn; }
    void ChgDefinedIn( const SwModify* pNew ) { pDefinedIn = (SwModify*)pNew; }
    void RegisterToEndNotInfo( SwEndNoteInfo& );
    void RegisterToPageDesc( SwPageDesc& );
    bool KnowsPageDesc() const;
};


inline const SwFmtPageDesc &SwAttrSet::GetPageDesc(sal_Bool bInP) const
    { return (const SwFmtPageDesc&)Get( RES_PAGEDESC,bInP); }

inline const SwFmtPageDesc &SwFmt::GetPageDesc(sal_Bool bInP) const
    { return aSet.GetPageDesc(bInP); }

#endif

