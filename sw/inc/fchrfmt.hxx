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
    SwTxtCharFmt* pTxtAttr;     ///< My text attribute.

public:
    SwFmtCharFmt() : pTxtAttr(0) {}

    /// single argument ctors shall be explicit.
    explicit SwFmtCharFmt( SwCharFmt *pFmt );
    virtual ~SwFmtCharFmt();

    /// @@@ public copy ctor, but no copy assignment?
    SwFmtCharFmt( const SwFmtCharFmt& rAttr );
protected:
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

private:
    /// @@@ public copy ctor, but no copy assignment?
    SwFmtCharFmt & operator= (const SwFmtCharFmt &);
public:

    TYPEINFO();

    /// "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual bool    GetInfo( SfxPoolItem& rInfo ) const;

    void SetCharFmt( SwFmt* pFmt ) { pFmt->Add(this); }
    SwCharFmt* GetCharFmt() const { return (SwCharFmt*)GetRegisteredIn(); }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
