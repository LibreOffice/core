/*************************************************************************
 *
 *  $RCSfile: fmtornt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2001-03-20 10:50:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _FMTORNT_HXX
#define _FMTORNT_HXX

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
#ifndef _ORNTENUM_HXX
#include <orntenum.hxx>
#endif

class International;

#define IVER_VERTORIENT_REL ((USHORT)0x0001)

class SwFmtVertOrient: public SfxPoolItem
{
    SwTwips          nYPos; //Enthaelt _immer_ die aktuelle RelPos.
    SwVertOrient     eOrient;
    SwRelationOrient eRelation;
public:
    TYPEINFO();
    SwFmtVertOrient( SwTwips nY = 0, SwVertOrient eVert = VERT_NONE,
                     SwRelationOrient eRel = PRTAREA );
    inline SwFmtVertOrient &operator=( const SwFmtVertOrient &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nVer) const;
    virtual SvStream&       Store(SvStream &, USHORT nIVer) const;
    virtual USHORT          GetVersion( USHORT nFFVer ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const International*    pIntl = 0 ) const;
    virtual BOOL             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    SwVertOrient GetVertOrient() const { return eOrient; }
    SwRelationOrient GetRelationOrient() const { return eRelation; }
    void   SetVertOrient( SwVertOrient eNew ) { eOrient = eNew; }
    void   SetRelationOrient( SwRelationOrient eNew ) { eRelation = eNew; }

    SwTwips GetPos() const { return nYPos; }
    void    SetPos( SwTwips nNew ) { nYPos = nNew; }

    SwTwips GetPosConvertedToSw31( const SvxULSpaceItem *pULSpace ) const;
    SwTwips GetPosConvertedFromSw31( const SvxULSpaceItem *pULSpace ) const;

    virtual BOOL        importXML( const ::rtl::OUString& rValue,USHORT,
                                   const SvXMLUnitConverter& rUnitConv );
    virtual BOOL        exportXML( ::rtl::OUString& rValue, USHORT,
                                   const SvXMLUnitConverter& rUnitConv ) const;
};

//SwFmtHoriOrient, wie und woran orientiert --
//  sich der FlyFrm in der Hoizontalen ----------

#define IVER_HORIORIENT_TOGGLE ((USHORT)0x0001)
#define IVER_HORIORIENT_REL ((USHORT)0x0002)

class SwFmtHoriOrient: public SfxPoolItem
{
    SwTwips          nXPos; //Enthaelt _immer_ die aktuelle RelPos.
    SwHoriOrient     eOrient;
    SwRelationOrient eRelation;
    BOOL             bPosToggle : 1; // auf geraden Seiten Position spiegeln
public:
    TYPEINFO();
    SwFmtHoriOrient( SwTwips nX = 0, SwHoriOrient eHori = HORI_NONE,
        SwRelationOrient eRel = PRTAREA, BOOL bPos = FALSE );
    inline SwFmtHoriOrient &operator=( const SwFmtHoriOrient &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nVer) const;
    virtual SvStream&       Store(SvStream &, USHORT nIVer) const;
    virtual USHORT          GetVersion( USHORT nFFVer ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const International*    pIntl = 0 ) const;
    virtual BOOL             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    SwHoriOrient GetHoriOrient() const { return eOrient; }
    SwRelationOrient GetRelationOrient() const { return eRelation; }
    void SetHoriOrient( SwHoriOrient eNew ) { eOrient = eNew; }
    void SetRelationOrient( SwRelationOrient eNew ) { eRelation = eNew; }

    SwTwips GetPos() const { return nXPos; }
    void    SetPos( SwTwips nNew ) { nXPos = nNew; }

    SwTwips GetPosConvertedToSw31( const SvxLRSpaceItem *pLRSpace ) const;
    SwTwips GetPosConvertedFromSw31( const SvxLRSpaceItem *pLRSpace ) const;

    BOOL IsPosToggle() const { return bPosToggle; }
    void SetPosToggle( BOOL bNew ) { bPosToggle = bNew; }

    virtual BOOL        importXML( const ::rtl::OUString& rValue,USHORT,
                                   const SvXMLUnitConverter& rUnitConv );
    virtual BOOL        exportXML( ::rtl::OUString& rValue, USHORT,
                                   const SvXMLUnitConverter& rUnitConv ) const;
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

