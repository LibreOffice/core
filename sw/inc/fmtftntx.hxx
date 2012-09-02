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
#ifndef _FMTFTNTX_HXX
#define _FMTFTNTX_HXX

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
    rtl::OUString sPrefix;
    rtl::OUString sSuffix;
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

    virtual int             operator==( const SfxPoolItem& ) const;

    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

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

    const rtl::OUString& GetPrefix() const      { return sPrefix; }
    void SetPrefix(const rtl::OUString& rSet)   { sPrefix = rSet; }

    const rtl::OUString& GetSuffix() const      { return sSuffix; }
    void SetSuffix(const rtl::OUString& rSet)   { sSuffix = rSet; }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
