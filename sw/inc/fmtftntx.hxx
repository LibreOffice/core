/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_INC_FMTFTNTX_HXX
#define INCLUDED_SW_INC_FMTFTNTX_HXX

#include <svl/eitem.hxx>
#include <hintids.hxx>
#include <format.hxx>
#include <numrule.hxx>
#include "swdllapi.h"

enum SwFtnEndPosEnum
{
    FTNEND_ATPGORDOCEND,                ///< at page or document end
    FTNEND_ATTXTEND,                    ///< at end of the current text end
    FTNEND_ATTXTEND_OWNNUMSEQ,          ///< -""- and with own number sequence
    FTNEND_ATTXTEND_OWNNUMANDFMT,       ///< -""- and with own numberformat
    FTNEND_ATTXTEND_END
};

class SW_DLLPUBLIC SwFmtFtnEndAtTxtEnd : public SfxEnumItem
{
    OUString sPrefix;
    OUString sSuffix;
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
    virtual sal_uInt16          GetValueCount() const SAL_OVERRIDE;

    virtual bool             operator==( const SfxPoolItem& ) const SAL_OVERRIDE;

    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper* pIntl = 0 ) const SAL_OVERRIDE;

    inline sal_Bool IsAtEnd() const { return FTNEND_ATPGORDOCEND != GetValue(); }

    SwFmtFtnEndAtTxtEnd & operator=( const SwFmtFtnEndAtTxtEnd & rAttr );

    sal_Int16 GetNumType() const        { return aFmt.GetNumberingType(); }
    void SetNumType( sal_Int16 eType )  { aFmt.SetNumberingType(eType); }

    const SvxNumberType& GetSwNumType() const   { return aFmt; }

    sal_uInt16 GetOffset() const                { return nOffset; }
    void SetOffset( sal_uInt16 nOff )           { nOffset = nOff; }

    const OUString& GetPrefix() const      { return sPrefix; }
    void SetPrefix(const OUString& rSet)   { sPrefix = rSet; }

    const OUString& GetSuffix() const      { return sSuffix; }
    void SetSuffix(const OUString& rSet)   { sSuffix = rSet; }
};

class SW_DLLPUBLIC SwFmtFtnAtTxtEnd : public SwFmtFtnEndAtTxtEnd
{
public:
    SwFmtFtnAtTxtEnd( SwFtnEndPosEnum ePos = FTNEND_ATPGORDOCEND )
        : SwFmtFtnEndAtTxtEnd( RES_FTN_AT_TXTEND, ePos )
    {}

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
};

class SW_DLLPUBLIC SwFmtEndAtTxtEnd : public SwFmtFtnEndAtTxtEnd
{
public:
    SwFmtEndAtTxtEnd( SwFtnEndPosEnum ePos = FTNEND_ATPGORDOCEND )
        : SwFmtFtnEndAtTxtEnd( RES_END_AT_TXTEND, ePos )
    {
        SetNumType( SVX_NUM_ROMAN_LOWER );
    }

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
