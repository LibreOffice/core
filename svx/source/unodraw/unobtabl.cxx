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

#include <svx/xit.hxx>
#include "UnoNameItemTable.hxx"

#include <svx/xbtmpit.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unomid.hxx>
#include <svx/unofill.hxx>
#include <com/sun/star/awt/XBitmap.hpp>

using namespace ::com::sun::star;
using namespace ::cppu;

namespace {

class SvxUnoBitmapTable : public SvxUnoNameItemTable
{
public:
    explicit SvxUnoBitmapTable( SdrModel* pModel ) noexcept;

    virtual NameOrIndex* createItem() const override;
    virtual bool isValid( const NameOrIndex* pItem ) const override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override;
};

}

SvxUnoBitmapTable::SvxUnoBitmapTable( SdrModel* pModel ) noexcept
: SvxUnoNameItemTable( pModel, XATTR_FILLBITMAP, MID_BITMAP )
{
}

bool SvxUnoBitmapTable::isValid( const NameOrIndex* pItem ) const
{
    if( SvxUnoNameItemTable::isValid( pItem ) )
    {
        const XFillBitmapItem* pBitmapItem = dynamic_cast< const XFillBitmapItem* >( pItem );
        if( pBitmapItem )
        {
            const Graphic& rGraphic = pBitmapItem->GetGraphicObject().GetGraphic();

            return rGraphic.GetSizeBytes() > 0;
        }
    }

    return false;
}

OUString SAL_CALL SvxUnoBitmapTable::getImplementationName()
{
    return "SvxUnoBitmapTable";
}

uno::Sequence< OUString > SAL_CALL SvxUnoBitmapTable::getSupportedServiceNames(  )
{
    return { "com.sun.star.drawing.BitmapTable" };
}

NameOrIndex* SvxUnoBitmapTable::createItem() const
{
    return new XFillBitmapItem();
}

// XElementAccess
uno::Type SAL_CALL SvxUnoBitmapTable::getElementType(  )
{
    return ::cppu::UnoType<awt::XBitmap>::get();
}

/**
 * Create a bitmaptable
 */
uno::Reference< uno::XInterface > SvxUnoBitmapTable_createInstance( SdrModel* pModel )
{
    return *new SvxUnoBitmapTable(pModel);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
