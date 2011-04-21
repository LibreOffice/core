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

    rtl::OUString   aAddrText;       // text for reciever
    sal_Bool        bSend;           // sender?
    rtl::OUString   aSendText;       // text for sender
    sal_Int32       lAddrFromLeft;   // left gap for reciever (twips)
    sal_Int32       lAddrFromTop;    // upper gap for reciever (twips)
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

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
public:
    SwEnvCfgItem();
    ~SwEnvCfgItem();

    SwEnvItem& GetItem() {return aEnvItem;}

    virtual void            Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
