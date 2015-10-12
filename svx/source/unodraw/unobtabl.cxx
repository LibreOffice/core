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

#include <svl/itempool.hxx>
#include <vcl/cvtgrf.hxx>
#include <svl/itemset.hxx>
#include <svx/xit.hxx>
#include "UnoNameItemTable.hxx"

#include <svx/xbtmpit.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xflhtit.hxx>
#include "svx/unoapi.hxx"
#include <svx/unomid.hxx>
#include <editeng/unoprnms.hxx>
#include "svx/unofill.hxx"
#include <editeng/memberids.hrc>

using namespace ::com::sun::star;
using namespace ::cppu;

class SvxUnoBitmapTable : public SvxUnoNameItemTable
{
public:
    explicit SvxUnoBitmapTable( SdrModel* pModel ) throw();
    virtual ~SvxUnoBitmapTable() throw();

    virtual NameOrIndex* createItem() const throw() override;
    virtual bool isValid( const NameOrIndex* pItem ) const override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException, std::exception ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException, std::exception) override;
};

SvxUnoBitmapTable::SvxUnoBitmapTable( SdrModel* pModel ) throw()
: SvxUnoNameItemTable( pModel, XATTR_FILLBITMAP, MID_GRAFURL )
{
}

SvxUnoBitmapTable::~SvxUnoBitmapTable() throw()
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

OUString SAL_CALL SvxUnoBitmapTable::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SvxUnoBitmapTable");
}

uno::Sequence< OUString > SAL_CALL SvxUnoBitmapTable::getSupportedServiceNames(  )
    throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS[0] = "com.sun.star.drawing.BitmapTable";
    return aSNS;
}

NameOrIndex* SvxUnoBitmapTable::createItem() const throw()
{
    return new XFillBitmapItem();
}

// XElementAccess
uno::Type SAL_CALL SvxUnoBitmapTable::getElementType(  )
    throw( uno::RuntimeException, std::exception )
{
    return ::cppu::UnoType<OUString>::get();
}

/**
 * Create a bitmaptable
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoBitmapTable_createInstance( SdrModel* pModel )
{
    return *new SvxUnoBitmapTable(pModel);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
