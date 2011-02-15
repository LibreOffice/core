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
#ifndef _FCHRFMT_HXX
#define _FCHRFMT_HXX


#include <svl/poolitem.hxx>
#include <calbck.hxx>
#include <format.hxx>

class SwCharFmt;
class SwTxtCharFmt;
class IntlWrapper;

// ATT_CHARFMT *********************************************


class SW_DLLPUBLIC SwFmtCharFmt: public SfxPoolItem, public SwClient
{
    friend class SwTxtCharFmt;
    SwTxtCharFmt* pTxtAttr;     // mein TextAttribut

public:
    SwFmtCharFmt() : pTxtAttr(0) {}

    // single argument ctors shall be explicit.
    explicit SwFmtCharFmt( SwCharFmt *pFmt );
    virtual ~SwFmtCharFmt();

    // @@@ public copy ctor, but no copy assignment?
    SwFmtCharFmt( const SwFmtCharFmt& rAttr );
private:
    // @@@ public copy ctor, but no copy assignment?
    SwFmtCharFmt & operator= (const SwFmtCharFmt &);
public:

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

    // an das SwTxtCharFmt weiterleiten (vom SwClient)
    virtual void    Modify( SfxPoolItem*, SfxPoolItem* );
    virtual sal_Bool    GetInfo( SfxPoolItem& rInfo ) const;

    void SetCharFmt( SwFmt* pFmt ) { pFmt->Add(this); }
    SwCharFmt* GetCharFmt() const { return (SwCharFmt*)GetRegisteredIn(); }
};
#endif

