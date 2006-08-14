/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtpdsc.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:24:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _FMTPDSC_HXX
#define _FMTPDSC_HXX


#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
#ifndef _CALBCK_HXX //autogen
#include <calbck.hxx>
#endif

class SwPageDesc;
class SwHistory;
class SwPaM;
class IntlWrapper;

//Pagedescriptor
//Client vom SwPageDesc der durch das Attribut "beschrieben" wird.

#define IVER_FMTPAGEDESC_NOAUTO ((USHORT)0x0001)
#define IVER_FMTPAGEDESC_LONGPAGE   ((USHORT)0x0002)

class SW_DLLPUBLIC SwFmtPageDesc : public SfxPoolItem, public SwClient
{
    // diese "Doc"-Funktion ist friend, um nach dem kopieren das
    // Auto-Flag setzen zu koennen !!
    friend BOOL InsAttr( SwDoc*, const SwPaM &, const SfxItemSet&, USHORT,
                        SwHistory* );
    USHORT nNumOffset;          // Seitennummer Offset
    USHORT nDescNameIdx;        // SW3-Reader: Stringpool-Index des Vorlagennamens
    SwModify* pDefinedIn;       // Verweis auf das Objekt, in dem das
                                // Attribut gesetzt wurde (CntntNode/Format)

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
    virtual BOOL             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );

          SwPageDesc *GetPageDesc() { return (SwPageDesc*)GetRegisteredIn(); }
    const SwPageDesc *GetPageDesc() const { return (SwPageDesc*)GetRegisteredIn(); }

    USHORT  GetNumOffset() const        { return nNumOffset; }
    void    SetNumOffset( USHORT nNum ) { nNumOffset = nNum; }

    // erfrage/setze, wo drin das Attribut verankert ist
    inline const SwModify* GetDefinedIn() const { return pDefinedIn; }
    void ChgDefinedIn( const SwModify* pNew ) { pDefinedIn = (SwModify*)pNew; }
};


inline const SwFmtPageDesc &SwAttrSet::GetPageDesc(BOOL bInP) const
    { return (const SwFmtPageDesc&)Get( RES_PAGEDESC,bInP); }

inline const SwFmtPageDesc &SwFmt::GetPageDesc(BOOL bInP) const
    { return aSet.GetPageDesc(bInP); }

#endif

