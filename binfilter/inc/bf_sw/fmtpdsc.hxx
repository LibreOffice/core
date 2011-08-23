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


#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
class IntlWrapper;
namespace binfilter {

class SwPageDesc;
class SwHistory;
class SwPaM;


//Pagedescriptor
//Client vom SwPageDesc der durch das Attribut "beschrieben" wird.

#define	IVER_FMTPAGEDESC_NOAUTO	((USHORT)0x0001)
#define IVER_FMTPAGEDESC_LONGPAGE	((USHORT)0x0002)

class SwFmtPageDesc : public SfxPoolItem, public SwClient
{
    // diese "Doc"-Funktion ist friend, um nach dem kopieren das
    // Auto-Flag setzen zu koennen !!
    friend BOOL InsAttr( SwDoc*, const SwPaM &, const SfxItemSet&, USHORT,
                        SwHistory* );
    USHORT nNumOffset;			// Seitennummer Offset
    USHORT nDescNameIdx;		// SW3-Reader: Stringpool-Index des Vorlagennamens
    SwModify* pDefinedIn;		// Verweis auf das Objekt, in dem das
                                // Attribut gesetzt wurde (CntntNode/Format)

public:
    SwFmtPageDesc( const SwPageDesc *pDesc = 0 );
    SwFmtPageDesc( const SwFmtPageDesc &rCpy );
    SwFmtPageDesc &operator=( const SwFmtPageDesc &rCpy );
    ~SwFmtPageDesc();

    TYPEINFO();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer) const;
    virtual USHORT			GetVersion( USHORT nFFVer ) const;
    virtual	BOOL        	 QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	BOOL			 PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );

          SwPageDesc *GetPageDesc() { return (SwPageDesc*)GetRegisteredIn(); }
    const SwPageDesc *GetPageDesc() const { return (SwPageDesc*)GetRegisteredIn(); }

    USHORT	GetNumOffset() const		{ return nNumOffset; }
    void	SetNumOffset( USHORT nNum ) { nNumOffset = nNum; }

    // erfrage/setze, wo drin das Attribut verankert ist
    inline const SwModify* GetDefinedIn() const { return pDefinedIn; }
    void ChgDefinedIn( const SwModify* pNew ) { pDefinedIn = (SwModify*)pNew; }

    // fuer den SW3-Reader:
    USHORT GetDescNameIdx() const { return nDescNameIdx; }
    void SetDescNameIdx( USHORT n ) { nDescNameIdx = n;  }
};


#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline const SwFmtPageDesc &SwAttrSet::GetPageDesc(BOOL bInP) const
    { return (const SwFmtPageDesc&)Get( RES_PAGEDESC,bInP); }

inline const SwFmtPageDesc &SwFmt::GetPageDesc(BOOL bInP) const
    { return aSet.GetPageDesc(bInP); }
#endif

} //namespace binfilter
#endif

