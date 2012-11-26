/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _FMTORNT_HXX
#define _FMTORNT_HXX

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include "swdllapi.h"
#include <hintids.hxx>
#include <swtypes.hxx>
#include <format.hxx>
#include <svl/poolitem.hxx>


class IntlWrapper;

#define IVER_VERTORIENT_REL ((sal_uInt16)0x0001)

class SW_DLLPUBLIC SwFmtVertOrient: public SfxPoolItem
{
    SwTwips         nYPos;  //Enthaelt _immer_ die aktuelle RelPos.
    sal_Int16       eOrient;
    sal_Int16       eRelation;
public:
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
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    sal_Int16 GetVertOrient() const { return eOrient; }
    sal_Int16 GetRelationOrient() const { return eRelation; }
    void   SetVertOrient( sal_Int16 eNew ) { eOrient = eNew; }
    void   SetRelationOrient( sal_Int16 eNew ) { eRelation = eNew; }

    SwTwips GetPos() const { return nYPos; }
    void    SetPos( SwTwips nNew ) { nYPos = nNew; }
};

//SwFmtHoriOrient, wie und woran orientiert --
//  sich der FlyFrm in der Hoizontalen ----------

#define IVER_HORIORIENT_TOGGLE ((sal_uInt16)0x0001)
#define IVER_HORIORIENT_REL ((sal_uInt16)0x0002)

class SW_DLLPUBLIC SwFmtHoriOrient: public SfxPoolItem
{
    SwTwips         nXPos;  //Enthaelt _immer_ die aktuelle RelPos.
    sal_Int16       eOrient;
    sal_Int16       eRelation;
    sal_Bool            bPosToggle : 1; // auf geraden Seiten Position spiegeln
public:
    SwFmtHoriOrient( SwTwips nX = 0, sal_Int16 eHori = com::sun::star::text::HoriOrientation::NONE,
        sal_Int16 eRel = com::sun::star::text::RelOrientation::PRINT_AREA, sal_Bool bPos = sal_False );
    inline SwFmtHoriOrient &operator=( const SwFmtHoriOrient &rCpy );

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

    sal_Int16 GetHoriOrient() const { return eOrient; }
    sal_Int16 GetRelationOrient() const { return eRelation; }
    void SetHoriOrient( sal_Int16 eNew ) { eOrient = eNew; }
    void SetRelationOrient( sal_Int16 eNew ) { eRelation = eNew; }

    SwTwips GetPos() const { return nXPos; }
    void    SetPos( SwTwips nNew ) { nXPos = nNew; }

    sal_Bool IsPosToggle() const { return bPosToggle; }
    void SetPosToggle( sal_Bool bNew ) { bPosToggle = bNew; }
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

inline const SwFmtVertOrient &SwAttrSet::GetVertOrient(sal_Bool bInP) const
    { return (const SwFmtVertOrient&)Get( RES_VERT_ORIENT,bInP); }
inline const SwFmtHoriOrient &SwAttrSet::GetHoriOrient(sal_Bool bInP) const
    { return (const SwFmtHoriOrient&)Get( RES_HORI_ORIENT,bInP); }

inline const SwFmtVertOrient &SwFmt::GetVertOrient(sal_Bool bInP) const
    { return aSet.GetVertOrient(bInP); }
inline const SwFmtHoriOrient &SwFmt::GetHoriOrient(sal_Bool bInP) const
    { return aSet.GetHoriOrient(bInP); }

#endif

