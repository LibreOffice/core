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

#include <com/sun/star/drawing/Hatch.hpp>
#include "UnoNameItemTable.hxx"

#include <svx/svdmodel.hxx>
#include <svx/xdef.hxx>
#include <svx/xflhtit.hxx>
#include <svx/unomid.hxx>
#include <svx/unofill.hxx>

using namespace ::com::sun::star;
using namespace ::cppu;

namespace {

class SvxUnoHatchTable : public SvxUnoNameItemTable
{
public:
    explicit SvxUnoHatchTable( SdrModel* pModel ) noexcept;

    virtual NameOrIndex* createItem() const override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override;
};

}

SvxUnoHatchTable::SvxUnoHatchTable( SdrModel* pModel ) noexcept
: SvxUnoNameItemTable( pModel, XATTR_FILLHATCH, MID_FILLHATCH )
{
}

OUString SAL_CALL SvxUnoHatchTable::getImplementationName()
{
    return u"SvxUnoHatchTable"_ustr;
}

uno::Sequence< OUString > SAL_CALL SvxUnoHatchTable::getSupportedServiceNames(  )
{
    return { u"com.sun.star.drawing.HatchTable"_ustr };
}

NameOrIndex* SvxUnoHatchTable::createItem() const
{
    return new XFillHatchItem();
}

// XElementAccess
uno::Type SAL_CALL SvxUnoHatchTable::getElementType(  )
{
    return cppu::UnoType<drawing::Hatch>::get();
}

/**
 * Create a hatchtable
 */
uno::Reference< uno::XInterface > SvxUnoHatchTable_createInstance( SdrModel* pModel )
{
    return *new SvxUnoHatchTable(pModel);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
