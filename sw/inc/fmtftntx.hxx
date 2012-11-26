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


#ifndef _FMTFTNTX_HXX
#define _FMTFTNTX_HXX

#include <svl/eitem.hxx>
#include <hintids.hxx>
#include <format.hxx>
#include <numrule.hxx>
#include "swdllapi.h"

enum SwFtnEndPosEnum
{
    FTNEND_ATPGORDOCEND,                // at page or document end
    FTNEND_ATTXTEND,                    // at end of the current text end
    FTNEND_ATTXTEND_OWNNUMSEQ,          // -""- and with own number sequence
    FTNEND_ATTXTEND_OWNNUMANDFMT,       // -""- and with onw numberformat
    FTNEND_ATTXTEND_END
};


class SW_DLLPUBLIC SwFmtFtnEndAtTxtEnd : public SfxEnumItem
{
    String      sPrefix;
    String      sSuffix;
    SvxNumberType aFmt;
    sal_uInt16      nOffset;

protected:
    SwFmtFtnEndAtTxtEnd( sal_uInt16 nWhichL, SwFtnEndPosEnum ePos )
        : SfxEnumItem( nWhichL, sal::static_int_cast< sal_uInt16 >(ePos) ), nOffset( 0 )
    {}
    SwFmtFtnEndAtTxtEnd( const SwFmtFtnEndAtTxtEnd& rAttr )
        : SfxEnumItem( rAttr ), sPrefix( rAttr.sPrefix ),
        sSuffix( rAttr.sSuffix ), aFmt( rAttr.aFmt ),
        nOffset( rAttr.nOffset )
    {}

public:
    virtual sal_uInt16          GetValueCount() const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

// will be used at time??
//  void                    FillVariable( SbxVariable &rVar,
//                                        SfxMapUnit eCoreMetric,
//                                        SfxMapUnit eUserMetric ) const;
//  virtual SfxArgumentError SetVariable( const SbxVariable &rVal,
//                                        SfxMapUnit eCoreMetric,
//                                        SfxMapUnit eUserMetric );
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;

    inline sal_Bool IsAtEnd() const { return FTNEND_ATPGORDOCEND != GetValue(); }

    SwFmtFtnEndAtTxtEnd & operator=( const SwFmtFtnEndAtTxtEnd & rAttr );

    sal_Int16 GetNumType() const        { return aFmt.GetNumberingType(); }
    void SetNumType( sal_Int16 eType )  { aFmt.SetNumberingType(eType); }

    const SvxNumberType& GetSwNumType() const   { return aFmt; }

    sal_uInt16 GetOffset() const                { return nOffset; }
    void SetOffset( sal_uInt16 nOff )           { nOffset = nOff; }

    const String& GetPrefix() const         { return sPrefix; }
    void SetPrefix(const String& rSet)      { sPrefix = rSet; }

    const String& GetSuffix() const         { return sSuffix; }
    void SetSuffix(const String& rSet)      { sSuffix = rSet; }
};

class SW_DLLPUBLIC SwFmtFtnAtTxtEnd : public SwFmtFtnEndAtTxtEnd
{
public:
    SwFmtFtnAtTxtEnd( SwFtnEndPosEnum ePos = FTNEND_ATPGORDOCEND )
        : SwFmtFtnEndAtTxtEnd( RES_FTN_AT_TXTEND, ePos )
    {}

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
};

class SW_DLLPUBLIC SwFmtEndAtTxtEnd : public SwFmtFtnEndAtTxtEnd
{
public:
    SwFmtEndAtTxtEnd( SwFtnEndPosEnum ePos = FTNEND_ATPGORDOCEND )
        : SwFmtFtnEndAtTxtEnd( RES_END_AT_TXTEND, ePos )
    {
        SetNumType( SVX_NUM_ROMAN_LOWER );
    }

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
};

inline const SwFmtFtnAtTxtEnd &SwAttrSet::GetFtnAtTxtEnd(sal_Bool bInP) const
    { return (const SwFmtFtnAtTxtEnd&)Get( RES_FTN_AT_TXTEND, bInP); }
inline const SwFmtEndAtTxtEnd &SwAttrSet::GetEndAtTxtEnd(sal_Bool bInP) const
    { return (const SwFmtEndAtTxtEnd&)Get( RES_END_AT_TXTEND, bInP); }


inline const SwFmtFtnAtTxtEnd &SwFmt::GetFtnAtTxtEnd(sal_Bool bInP) const
    { return aSet.GetFtnAtTxtEnd(bInP); }
inline const SwFmtEndAtTxtEnd &SwFmt::GetEndAtTxtEnd(sal_Bool bInP) const
    { return aSet.GetEndAtTxtEnd(bInP); }

#endif

