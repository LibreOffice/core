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
#ifndef _ENVIMG_HXX
#define _ENVIMG_HXX

#include <svl/poolitem.hxx>
#include <unotools/configitem.hxx>
#include "swdllapi.h"

// MakeSender -----------------------------------------------------------------
SW_DLLPUBLIC String MakeSender();

// enum -----------------------------------------------------------------------
enum SwEnvAlign
{
    ENV_HOR_LEFT = 0,
    ENV_HOR_CNTR,
    ENV_HOR_RGHT,
    ENV_VER_LEFT,
    ENV_VER_CNTR,
    ENV_VER_RGHT
};

// class SwEnvItem ------------------------------------------------------------
class SW_DLLPUBLIC SwEnvItem : public SfxPoolItem
{
public:

    OUString   aAddrText;       // text for receiver
    sal_Bool        bSend;           // sender?
    OUString   aSendText;       // text for sender
    sal_Int32       lAddrFromLeft;   // left gap for receiver (twips)
    sal_Int32       lAddrFromTop;    // upper gap for receiver (twips)
    sal_Int32       lSendFromLeft;   // left gap for sender (twips)
    sal_Int32       lSendFromTop;    // upper gap for sender (twips)
    sal_Int32       lWidth;          // envelope's width (twips)
    sal_Int32       lHeight;         // envelope's height (twips)
    SwEnvAlign      eAlign;          // alignment at indent
    sal_Bool        bPrintFromAbove; // print from above?
    sal_Int32       lShiftRight;     // shift to right (twips)
    sal_Int32       lShiftDown;      // shift down (twips)

    SwEnvItem();
    SwEnvItem(const SwEnvItem& rItem);

    TYPEINFO();

    SwEnvItem& operator =(const SwEnvItem& rItem);

    virtual int operator ==(const SfxPoolItem& rItem) const;

    virtual SfxPoolItem*     Clone(SfxItemPool* = 0) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

// class SwEnvCfgItem -------------------------------------------------------
class SwEnvCfgItem : public utl::ConfigItem
{
    SwEnvItem aEnvItem;

    com::sun::star::uno::Sequence<OUString> GetPropertyNames();
public:
    SwEnvCfgItem();
    ~SwEnvCfgItem();

    SwEnvItem& GetItem() {return aEnvItem;}

    virtual void            Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
