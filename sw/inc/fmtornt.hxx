/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtornt.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:03:25 $
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
#ifndef _FMTORNT_HXX
#define _FMTORNT_HXX

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SWTYPES_HXX //autogen
#include <swtypes.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif


class IntlWrapper;

#define IVER_VERTORIENT_REL ((USHORT)0x0001)

class SW_DLLPUBLIC SwFmtVertOrient: public SfxPoolItem
{
    SwTwips         nYPos;  //Enthaelt _immer_ die aktuelle RelPos.
    sal_Int16       eOrient;
    sal_Int16       eRelation;
public:
    TYPEINFO();
    SwFmtVertOrient( SwTwips nY = 0, sal_Int16 eVert = com::sun::star::text::VertOrientation::NONE,
                     sal_Int16 eRel = com::sun::star::text::RelOrientation::PRINT_AREA );
    inline SwFmtVertOrient &operator=( const SwFmtVertOrient &rCpy );

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

    sal_Int16 GetVertOrient() const { return eOrient; }
    sal_Int16 GetRelationOrient() const { return eRelation; }
    void   SetVertOrient( sal_Int16 eNew ) { eOrient = eNew; }
    void   SetRelationOrient( sal_Int16 eNew ) { eRelation = eNew; }

    SwTwips GetPos() const { return nYPos; }
    void    SetPos( SwTwips nNew ) { nYPos = nNew; }
};

//SwFmtHoriOrient, wie und woran orientiert --
//  sich der FlyFrm in der Hoizontalen ----------

#define IVER_HORIORIENT_TOGGLE ((USHORT)0x0001)
#define IVER_HORIORIENT_REL ((USHORT)0x0002)

class SW_DLLPUBLIC SwFmtHoriOrient: public SfxPoolItem
{
    SwTwips         nXPos;  //Enthaelt _immer_ die aktuelle RelPos.
    sal_Int16       eOrient;
    sal_Int16       eRelation;
    BOOL            bPosToggle : 1; // auf geraden Seiten Position spiegeln
public:
    TYPEINFO();
    SwFmtHoriOrient( SwTwips nX = 0, sal_Int16 eHori = com::sun::star::text::HoriOrientation::NONE,
        sal_Int16 eRel = com::sun::star::text::RelOrientation::PRINT_AREA, BOOL bPos = FALSE );
    inline SwFmtHoriOrient &operator=( const SwFmtHoriOrient &rCpy );

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

    sal_Int16 GetHoriOrient() const { return eOrient; }
    sal_Int16 GetRelationOrient() const { return eRelation; }
    void SetHoriOrient( sal_Int16 eNew ) { eOrient = eNew; }
    void SetRelationOrient( sal_Int16 eNew ) { eRelation = eNew; }

    SwTwips GetPos() const { return nXPos; }
    void    SetPos( SwTwips nNew ) { nXPos = nNew; }

    BOOL IsPosToggle() const { return bPosToggle; }
    void SetPosToggle( BOOL bNew ) { bPosToggle = bNew; }
};

inline SwFmtVertOrient &SwFmtVertOrient::operator=( const SwFmtVertOrient &rCpy )
{
    nYPos = rCpy.GetPos();
    eOrient = rCpy.GetVertOrient();
    eRelation = rCpy.GetRelationOrient();
    return *this;
}
inline SwFmtHoriOrient &SwFmtHoriOrient::operator=( const SwFmtHoriOrient &rCpy )
{
    nXPos = rCpy.GetPos();
    eOrient = rCpy.GetHoriOrient();
    eRelation = rCpy.GetRelationOrient();
    bPosToggle = rCpy.IsPosToggle();
    return *this;
}

inline const SwFmtVertOrient &SwAttrSet::GetVertOrient(BOOL bInP) const
    { return (const SwFmtVertOrient&)Get( RES_VERT_ORIENT,bInP); }
inline const SwFmtHoriOrient &SwAttrSet::GetHoriOrient(BOOL bInP) const
    { return (const SwFmtHoriOrient&)Get( RES_HORI_ORIENT,bInP); }

inline const SwFmtVertOrient &SwFmt::GetVertOrient(BOOL bInP) const
    { return aSet.GetVertOrient(bInP); }
inline const SwFmtHoriOrient &SwFmt::GetHoriOrient(BOOL bInP) const
    { return aSet.GetHoriOrient(bInP); }

#endif

