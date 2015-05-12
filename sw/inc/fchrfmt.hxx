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
#ifndef INCLUDED_SW_INC_FCHRFMT_HXX
#define INCLUDED_SW_INC_FCHRFMT_HXX

#include <svl/poolitem.hxx>
#include <calbck.hxx>
#include <format.hxx>
#include <charfmt.hxx>

class SwTextCharFormat;
class IntlWrapper;

class SW_DLLPUBLIC SwFormatCharFormat: public SfxPoolItem, public SwClient
{
    friend class SwTextCharFormat;
    SwTextCharFormat* pTextAttr;     ///< My text attribute.

public:
    SwFormatCharFormat() : pTextAttr(0) {}

    /// single argument ctors shall be explicit.
    explicit SwFormatCharFormat( SwCharFormat *pFormat );
    virtual ~SwFormatCharFormat();

    /// @@@ public copy ctor, but no copy assignment?
    SwFormatCharFormat( const SwFormatCharFormat& rAttr );
protected:
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) SAL_OVERRIDE;

private:
    /// @@@ public copy ctor, but no copy assignment?
    SwFormatCharFormat & operator= (const SwFormatCharFormat &) SAL_DELETED_FUNCTION;
public:

    TYPEINFO_OVERRIDE();

    /// "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const SAL_OVERRIDE;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    virtual bool    GetInfo( SfxPoolItem& rInfo ) const SAL_OVERRIDE;

    void SetCharFormat( SwFormat* pFormat ) { pFormat->Add(this); }
    SwCharFormat* GetCharFormat() const { return const_cast<SwCharFormat*>(static_cast<const SwCharFormat*>(GetRegisteredIn())); }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
